#ifndef SOUND_WRAPPER_H
#define SOUND_WRAPPER_H

#pragma comment(lib, "winmm.lib")

class SoundWrapper
{
public:
	SoundWrapper() = delete;
	SoundWrapper(const std::wstring& outputDevice, int sampleRate = 44100, short channels = 1, size_t blocks = 8, int blockSamples = 512);
	~SoundWrapper();

	//#TODO: to make GetAudioDevices by WinAPI?
	static std::vector<std::wstring> GetAudioDevicesNames();
	static size_t GetDeviceId(const std::wstring& device);

private:
	std::function<double()> m_noizeFunc;

	//#TODO: better names?
	int m_sampleRate;						// How much wave vibrations per second (Hz). Low - it's bass; Hight - it's soprano
	short m_channels;						// How much channels for output we use. Mono = 1, Stereo = 2, and so on
	size_t m_blocksCount; 					// Limit for blocks of samples in queue
	std::atomic<size_t> m_blocksFree; 		// How much we have free blocks of samples in queue
	int m_blockSamples;
	int m_blockCurrent;

	std::vector<std::vector<short>> m_blocksData;
	std::vector<std::unique_ptr<WAVEHDR>> m_waveHeaders;
	HWAVEOUT m_device;

	std::thread m_thread;
	std::atomic<bool> m_isReady;
	std::condition_variable m_cvBlockNotZero;
	std::mutex m_muxBlockNotZero;

	std::atomic<double> m_globalTime;

	bool deviceIsValid(const std::wstring& deviceName) const;

	// Handler for soundcard request for more data
	void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2);

	// Static wrapper for sound card handler
	static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	// Main thread. This loop responds to requests from the soundcard to fill 'blocks'
	// with audio data. If no requests are available it goes dormant until the sound
	// card is ready for more data. The block is fille by the "user" in some manner
	// and then issued to the soundcard.
	void MainThread();
};

#endif // !SOUND_WRAPPER_H