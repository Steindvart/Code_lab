#ifndef SOUND_WRAPPER_H
#define SOUND_WRAPPER_H

#pragma comment(lib, "winmm.lib")

class SoundWrapper
{
public:
	SoundWrapper() = delete;
	SoundWrapper(const std::wstring& outputDevice, int sampleRate, short channels, size_t blocks, size_t samplesPerBlock, std::function<double(double)> f);
	~SoundWrapper();

	void Stop();

	double GetGlobalTime() const;

	void SetSoundFunc(std::function<double(double)> f);

	//#TODO: to make GetAudioDevices by WinAPI?
	static std::vector<std::wstring> GetAudioDevicesNames();
	static size_t GetDeviceId(const std::wstring& device);

private:
	std::function<double(double)> m_soundFunc;

	//#TODO: better names?
	int m_sampleRate;						// How much wave vibrations per second (Hz). Low - it's bass; Hight - it's soprano
	short m_channels;						// How much channels for output we use. Mono = 1, Stereo = 2, and so on
	HWAVEOUT m_device;

	size_t m_blockCurrent;
	std::atomic<size_t> m_blocksFree; 		// How much we have free blocks of samples
	std::vector<std::vector<short>> m_blocksData;
	std::vector<std::unique_ptr<WAVEHDR>> m_waveHeaders;

	std::atomic<bool> m_isReady;
	std::thread m_thread;
	std::condition_variable m_cvNoFreeBlocks;
	std::mutex m_muxNoFreeBlocks;

	std::atomic<double> m_globalTime;

	// Override for definition of behavior without noizeFunc()
	virtual double defaultSound(double time) const;

	bool deviceIsValid(const std::wstring& deviceName) const;

	void linkHeaders();
	double clip(double sample, double max) const;

	// Handler for soundcard request for more data
	void waveOutProc(HWAVEOUT waveOut, UINT msg, DWORD param1, DWORD param2);
	static void CALLBACK waveOutProcWrap(HWAVEOUT waveOut, UINT msg, DWORD instance, DWORD param1, DWORD param2);

	// Main thread. This loop responds to requests from the soundcard to fill 'blocks'
	// with audio data. If no requests are available it goes dormant until the sound
	// card is ready for more data. The block is fill by the "user" in some manner
	// and then issued to the soundcard.
	void MainThread();
};

#endif // !SOUND_WRAPPER_H