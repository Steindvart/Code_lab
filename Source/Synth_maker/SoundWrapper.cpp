#define NOMINMAX

#include "Shared/stdafx.h"

#include "SoundWrapper.h"

#include "Shared/Exception.h"

#define BITS_PER_BYTE CHAR_BIT 

SoundWrapper::SoundWrapper(const std::wstring& outputDevice, int sampleRate, short channels, size_t blocks, size_t samplesPerBlock)
	: m_sampleRate(sampleRate)
	, m_channels(channels)
	, m_blocksFree(blocks)
	, m_blockCurrent(0)
	, m_blocksData(blocks, std::vector<short>(samplesPerBlock, 0))
	, m_waveHeaders(blocks)
	, m_soundFunc(nullptr)
	, m_globalTime(0.0)
{
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = m_sampleRate;
	waveFormat.wBitsPerSample = sizeof(short) * BITS_PER_BYTE;
	waveFormat.nChannels = m_channels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / BITS_PER_BYTE) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	size_t deviceID = GetDeviceId(outputDevice);
	if (deviceID == ERROR_BAD_DEVICE)
		throw Shared::Exception(L"SoundWrapper::SoundWrapper: device " + outputDevice + L" is not valid.");

	if (waveOutOpen(&m_device, static_cast<UINT>(deviceID), &waveFormat, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
		throw Shared::Exception(L"SoundWrapper::SoundWrapper: cannot open device " + outputDevice);

	linkHeaders();

	m_isReady = true;
	m_thread = std::thread(&SoundWrapper::MainThread, this);

	// Start the ball rolling
	std::scoped_lock<std::mutex> lm(m_muxNoFreeBlocks);
	m_cvNoFreeBlocks.notify_one();
}

SoundWrapper::~SoundWrapper()
{
	m_isReady = false;
	m_thread.join();
}

void SoundWrapper::Stop()
{
	m_isReady = false;
	m_thread.join();
}

double SoundWrapper::GetGlobalTime() const
{
	return m_globalTime;
}

void SoundWrapper::SetSoundFunc(std::function<double(double)> f)
{
	m_soundFunc = f;
}

std::vector<std::wstring> SoundWrapper::GetAudioDevicesNames()
{
	std::vector<std::wstring> devicesNames;

	WAVEOUTCAPS deviceInfo;
	int deviceCount = waveOutGetNumDevs();
	for (int n = 0; n < deviceCount; n++)
		if (waveOutGetDevCaps(n, &deviceInfo, sizeof(WAVEOUTCAPS)) == S_OK)
			devicesNames.push_back(deviceInfo.szPname);

	return devicesNames;
}

size_t SoundWrapper::GetDeviceId(const std::wstring& device)
{
	auto devices = GetAudioDevicesNames();
	auto d = std::find(devices.begin(), devices.end(), device);

	if (d == devices.end())
		return ERROR_BAD_DEVICE;

	return static_cast<size_t>(std::distance(devices.begin(), d));
}

double SoundWrapper::defaultSound(double /*time*/) const
{
	return 0.0;
}

bool SoundWrapper::deviceIsValid(const std::wstring& deviceName) const
{
	auto devices = GetAudioDevicesNames();
	if (std::find(devices.begin(), devices.end(), deviceName) != devices.end())
		return true;
	else
		return false;
}

void SoundWrapper::linkHeaders()
{
	for (size_t n = 0; n < m_blocksData.size(); n++)
	{
		m_waveHeaders[n] = std::make_unique<WAVEHDR>();
		m_waveHeaders[n]->dwBufferLength = static_cast<DWORD>(m_blocksData[n].size() * sizeof(short));
		m_waveHeaders[n]->lpData = (LPSTR)(m_blocksData[n].data());
	}
}

double SoundWrapper::clip(double sample, double max) const
{
	if (sample >= 0.0)
		return std::fmin(sample, max);
	else
		return std::fmax(sample, -max);
}

// Handler for soundcard request for more data
void SoundWrapper::waveOutProc(HWAVEOUT /*waveOut*/, UINT msg, DWORD /*param1*/, DWORD /*param2*/)
{
	if (msg != WOM_DONE) 
		return;

	m_blocksFree++;
	std::unique_lock<std::mutex> lm(m_muxNoFreeBlocks);
	m_cvNoFreeBlocks.notify_one();
}

void CALLBACK SoundWrapper::waveOutProcWrap(HWAVEOUT waveOut, UINT msg, DWORD instance, DWORD param1, DWORD param2)
{
	((SoundWrapper*)instance)->waveOutProc(waveOut, msg, param1, param2);
}

void SoundWrapper::MainThread()
{
	const double timeStep = 1.0 / m_sampleRate;

	constexpr short maxSample = std::numeric_limits<short>::max();
	constexpr double dMaxSample = static_cast<double>(maxSample);
	short previousSample = 0;

	const auto samplesPerBlock = m_blocksData[m_blockCurrent].size();
	const auto blocksQuantity  = m_blocksData.size();
	while (m_isReady)
	{
		// Wait for block to become available
		if (m_blocksFree == 0)
		{
			std::unique_lock<std::mutex> lock(m_muxNoFreeBlocks);
			m_cvNoFreeBlocks.wait(lock);
		}

		// Block is here, so use it
		m_blocksFree--;

		//#TODO - to make functions for logical pieces
		// Prepare block for processing
		if (m_waveHeaders[m_blockCurrent]->dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(m_device, m_waveHeaders[m_blockCurrent].get(), sizeof(WAVEHDR));

		short newSample = 0;
		for (size_t n = 0; n < samplesPerBlock; n++)
		{
			// User Process
			if (m_soundFunc == nullptr)
				newSample = (short)(clip(defaultSound(m_globalTime), 1.0) * dMaxSample);
			else
				newSample = (short)(clip(m_soundFunc(m_globalTime), 1.0) * dMaxSample);

			//#TODO - to think about this
			m_blocksData[m_blockCurrent][n] = newSample;
			previousSample = newSample;
			m_globalTime = m_globalTime + timeStep;
		}

		// Send block to sound device
		waveOutPrepareHeader(m_device, m_waveHeaders[m_blockCurrent].get(), sizeof(WAVEHDR));
		waveOutWrite(m_device, m_waveHeaders[m_blockCurrent].get(), sizeof(WAVEHDR));
		m_blockCurrent = (m_blockCurrent + 1) % blocksQuantity;
	}
}