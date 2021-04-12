#include "Shared/stdafx.h"
#include "Shared/Exception.h"

#include "SoundWrapper.h"
#include "Octaves.h"

// Aliases for mask of GetAsyncKeyState() response
#define KEY_HOLD  0x8000
#define KEY_PRESS 0x01

using std::numbers::pi;

// Converts frequency (Hz) to angular velocity
double hzToAng(double hertz)
{
	return hertz * 2.0 * pi;
}

//#TODO - transform to class or namespace and put to different translation unit
enum class Oscillator
{
	SINE,
	SQUARE,
	TRIANGLE,
	SAW_ANA,
	SAW_DIG,
	NOISE
};

double oscFunc(double hertz, double time, Oscillator type)
{
	switch (type)
	{
	case Oscillator::SINE:
		return sin(hzToAng(hertz) * time);
	case Oscillator::SQUARE:
		return sin(hzToAng(hertz) * time) > 0 ? 1.0 : -1.0;
	case Oscillator::TRIANGLE:
		return asin(sin(hzToAng(hertz) * time)) * (2.0 / pi);
	case Oscillator::SAW_ANA: // Saw wave (analogue / warm / slow)
	{
		double output = 0.0;

		for (double n = 1.0; n < 40.0; n++)
			output += (sin(n * hzToAng(hertz) * time)) / n;

		return output * (2.0 / pi);
	}
	case Oscillator::SAW_DIG: // Saw Wave (optimised / harsh / fast)
		return (2.0 / pi) * (hertz * pi * fmod(time, 1.0 / hertz) - (pi / 2.0));
	case Oscillator::NOISE: // Pseudorandom noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
	default:
		return 0.0;
	}
}

//#TODO - put to different translation unit
// Amplitude (Attack, Decay, Sustain, Release) Module
class ADSRModule
{
private:
	double m_attackTime;
	double m_decayTime;
	double m_releaseTime;

	double m_sustainAmplitude;
	double m_startAmplitude;
	double m_triggerOffTime;
	double m_triggerOnTime;
	bool   m_noteOn;

	enum Phase
	{
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};

	Phase getPhase(double lifeTime) const
	{
		if (lifeTime <= m_attackTime)
			return ATTACK;
		else if (lifeTime > m_attackTime && lifeTime <= (m_attackTime + m_decayTime))
			return DECAY;
		else if (lifeTime > (m_attackTime + m_decayTime))
			return SUSTAIN;
		else
			return RELEASE;
	}

public:
	ADSRModule()
		: m_attackTime(0.1)
		, m_decayTime(0.01)
		, m_releaseTime(0.2)
		, m_sustainAmplitude(0.8)
		, m_startAmplitude(1.0)
		, m_triggerOnTime(0.0)
		, m_triggerOffTime(0.0)
		, m_noteOn(false)
	{ }

	// Call when key is pressed
	void NoteOn(double timeOn)
	{
		m_triggerOnTime = timeOn;
		m_noteOn = true;
	}

	// Call when key is released
	void NoteOff(double timeOff)
	{
		m_triggerOffTime = timeOff;
		m_noteOn = false;
	}

	// Get the correct amplitude at the requested point in time
	double GetAmplitude(double time)
	{
		double amplitude = 0.0;
		double lifeTime = time - m_triggerOnTime;

		if (m_noteOn)
		{
			switch (getPhase(lifeTime))
			{
			case ADSRModule::ATTACK:
				amplitude = (lifeTime / m_attackTime) * m_startAmplitude;
				break;
			case ADSRModule::DECAY:
				amplitude = ((lifeTime - m_attackTime) / m_decayTime) * (m_sustainAmplitude - m_startAmplitude) + m_startAmplitude;
				break;
			case ADSRModule::SUSTAIN:
				amplitude = m_sustainAmplitude;
				break;
			default:
				break;
			}
		}
		else
		{
			// Note has been released, so in release phase
			amplitude = ((time - m_triggerOffTime) / m_releaseTime) * (0.0 - m_sustainAmplitude) + m_sustainAmplitude;
		}

		// Amplitude should not be negative
		if (amplitude <= 0.0001)
			amplitude = 0.0;

		return amplitude;
	}


};

struct MakeSoundFunct
{
	double& m_frequencyOutput;
	Oscillator& m_currOsc;
	ADSRModule& m_ADSR;

	double m_masterVolume;

	MakeSoundFunct() = delete;
	explicit MakeSoundFunct(double& frequencyOutput, Oscillator& osc, ADSRModule& adsr) 
		: m_frequencyOutput(frequencyOutput)
		, m_currOsc(osc)
		, m_ADSR(adsr)
		, m_masterVolume(0.25)
	{}
	MakeSoundFunct(double& frequencyOutput, Oscillator& osc, ADSRModule& adsr, double volume)
		: m_frequencyOutput(frequencyOutput)
		, m_currOsc(osc)
		, m_ADSR(adsr)
		, m_masterVolume(volume)
	{}

	// time variable need, because it's shift on `x` coordinate 
	double operator() (double time)
	{
		return m_ADSR.GetAmplitude(time) * oscFunc(m_frequencyOutput, time, m_currOsc) * m_masterVolume;
	}
};

int main(int, char*[])
{
	//#DEFECT - for cyrillic char-page in windows. Need to chose more elegant alternative
	setlocale(LC_ALL, "Russian");
	std::wcout << "Synth_maker - simple software synthesizer.\n" << std::endl;

	const auto availableDevices = SoundWrapper::GetAudioDevicesNames();

	std::wcout << "Available devices:" << std::endl;
	for (size_t i = 0; i < availableDevices.size(); i++)
		std::wcout << i << ". " << availableDevices[i] << std::endl;

	const auto usingDevice = *availableDevices.begin();
	std::wcout << std::endl << "Using device: 0. " << usingDevice;

	std::wcout << std::endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |   | |   |   |" << std::endl <<
		"|   | W |   |   | R | | T |   |   | U | | I | | O |   |   | [ | | ] |   |" << std::endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |___| |___|   |" << std::endl <<
		"|     |     |     |     |     |     |     |     |     |     |     |     |" << std::endl <<
		"|  A  |  S  |  D  |  F  |  G  |  H  |  J  |  K  |  L  |  ;  |  '  |  \\  |" << std::endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << std::endl << std::endl;

	try
	{
		//#TODO - to make atomic?
		double frequency = 0.0;
		Oscillator currOsc = Oscillator::SINE;
		ADSRModule ADSR;
		Octaves::Type currOct = Octaves::A3;

		MakeSoundFunct soundFunc(frequency, currOsc, ADSR , 0.5);
		SoundWrapper soundMachine(usingDevice, 44100, 1, 8, 512, soundFunc);

		const double Root12Of2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per octave

		//#DEFECT - looking bad
		int currKey = -1;
		bool isKeyPressed = false;
		const std::string pianoKeys{ "AWSDRFTGHUJIKOL\xBA\xDB\xDE\xDD\xDC" };
		const std::string controlKeys{ "ZX12345" };

		while (true)
		{
			isKeyPressed = false;
			for (int k = 0; k < pianoKeys.size(); k++)
			{
				if (GetAsyncKeyState(static_cast<unsigned char>(pianoKeys[k])) & KEY_HOLD)
				{
					if (currKey != k)
					{
						frequency = Octaves::All[currOct] * pow(Root12Of2, k);
						ADSR.NoteOn(soundMachine.GetGlobalTime());
						currKey = k;

						std::wcout << "\rNote On : " << soundMachine.GetGlobalTime() << "s " << frequency << "Hz";
					}

					isKeyPressed = true;
				}
			}
			
			//#TODO - output current settings
			for (auto k : controlKeys)
			{
				if (GetAsyncKeyState(k) & KEY_PRESS)
				{
					if (k == 'Z' && currOct > Octaves::A0)
						currOct = static_cast<Octaves::Type>(currOct - 1);
					if (k == 'X' && currOct < Octaves::A8)
						currOct = static_cast<Octaves::Type>(currOct + 1);

					if (k == '1')
						currOsc = Oscillator::SINE;
					if (k == '2')
						currOsc = Oscillator::SQUARE;
					if (k == '3')
						currOsc = Oscillator::TRIANGLE;
					if (k == '4')
						currOsc = Oscillator::SAW_ANA;
					if (k == '5')
						currOsc = Oscillator::SAW_DIG;
				}
			}

			if (!isKeyPressed)
			{
				if (currKey != -1)
				{
					ADSR.NoteOff(soundMachine.GetGlobalTime());
					currKey = -1;

					std::wcout << "\rNote Off: " << soundMachine.GetGlobalTime() << "s               ";
				}
			}
		}
	}
	catch (Shared::Exception& ex)
	{
		std::wcout << ex.Reason() << std::endl;
		return 1;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return 1;
	}

	return 0;
}