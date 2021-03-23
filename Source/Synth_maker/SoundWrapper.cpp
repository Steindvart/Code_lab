#include "Shared/stdafx.h"

#include "SoundWrapper.h"

#include "Shared/Exception.h"

#define BITS_PER_SAMPLE 8

SoundWrapper::SoundWrapper(const std::wstring& outputDevice, int sampleRate, short channels, size_t blocks, int blockSamples)
	: m_sampleRate(sampleRate)
	, m_channels(channels)
	, m_blocksCount(blocks)
	, m_blocksFree(blocks)
	, m_blockSamples(blockSamples)
	, m_blockCurrent(0)
	, m_blocksData(m_blocksCount, std::vector<short>(m_blockSamples, 0))
	, m_waveHeaders(m_blocksCount)
	, m_noizeFunc(nullptr)
{
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = m_sampleRate;
	waveFormat.wBitsPerSample = sizeof(short) * BITS_PER_SAMPLE;
	waveFormat.nChannels = m_channels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / BITS_PER_SAMPLE) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	int deviceID = GetDeviceId(outputDevice);
	if (deviceID == ERROR_BAD_DEVICE)
		throw Shared::Exception(L"SoundWrapper::SoundWrapper: device " + outputDevice + L" is not valid.");

	if (waveOutOpen(&m_device, deviceID, &waveFormat, (DWORD_PTR)waveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
		throw Shared::Exception(L"SoundWrapper::SoundWrapper: cannot open device " + outputDevice);

	for (size_t n = 0; n < m_blocksCount; n++)
	{
		m_waveHeaders[n] = std::make_unique<WAVEHDR>();
		m_waveHeaders[n]->dwBufferLength = m_blockSamples * sizeof(short);
		m_waveHeaders[n]->lpData = (LPSTR)(m_blocksData[n].data());
	}

	m_isReady = true;

	m_thread = std::thread(&MainThread, this);

	// Start the ball rolling
	//std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
	//m_cvBlockNotZero.notify_one();

	//return true;
}

SoundWrapper::~SoundWrapper()
{
	m_isReady = false;
	m_thread.join();
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

bool SoundWrapper::deviceIsValid(const std::wstring& deviceName) const
{
	auto devices = GetAudioDevicesNames();
	if (std::find(devices.begin(), devices.end(), deviceName) != devices.end())
		return true;
	else
		return false;
}

// Handler for soundcard request for more data
void SoundWrapper::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg != WOM_DONE) 
		return;

	m_blocksFree++;
	std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
	m_cvBlockNotZero.notify_one();
}

// Static wrapper for sound card handler
void CALLBACK SoundWrapper::waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	((SoundWrapper*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
}

void SoundWrapper::MainThread()
{
	//m_dGlobalTime = 0.0;
	//double dTimeStep = 1.0 / (double)m_nSampleRate;

	//// Goofy hack to get maximum integer for a type at run-time
	//T nMaxSample = (T)pow(2, (sizeof(T) * 8) - 1) - 1;
	//double dMaxSample = (double)nMaxSample;
	//T nPreviousSample = 0;

	//while (m_bReady)
	//{
	//	// Wait for block to become available
	//	if (m_nBlockFree == 0)
	//	{
	//		unique_lock<mutex> lm(m_muxBlockNotZero);
	//		m_cvBlockNotZero.wait(lm);
	//	}

	//	// Block is here, so use it
	//	m_nBlockFree--;

	//	// Prepare block for processing
	//	if (m_pWaveHeaders[m_nBlockCurrent].dwFlags & WHDR_PREPARED)
	//		waveOutUnprepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));

	//	T nNewSample = 0;
	//	int nCurrentBlock = m_nBlockCurrent * m_nBlockSamples;

	//	for (unsigned int n = 0; n < m_nBlockSamples; n++)
	//	{
	//		// User Process
	//		if (m_userFunction == nullptr)
	//			nNewSample = (T)(clip(UserProcess(m_dGlobalTime), 1.0) * dMaxSample);
	//		else
	//			nNewSample = (T)(clip(m_userFunction(m_dGlobalTime), 1.0) * dMaxSample);

	//		m_pBlockMemory[nCurrentBlock + n] = nNewSample;
	//		nPreviousSample = nNewSample;
	//		m_dGlobalTime = m_dGlobalTime + dTimeStep;
	//	}

	//	// Send block to sound device
	//	waveOutPrepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
	//	waveOutWrite(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
	//	m_nBlockCurrent++;
	//	m_nBlockCurrent %= m_nBlockCount;
	//}
}