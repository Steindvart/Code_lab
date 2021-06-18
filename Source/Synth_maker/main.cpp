#include "Shared/stdafx.h"
#include "Shared/Exception.h"

#include "SoundWrapper.h"
#include "ADSRModule.h"
#include "Octaves.h"
#include "Oscillator.h"

// Aliases for mask of GetAsyncKeyState() response
#define KEY_HOLD  0x8000
#define KEY_PRESS 0x01

using std::numbers::pi;

// Converts frequency (Hz) to angular velocity
double hzToAng(double hertz)
{
	return hertz * 2.0 * pi;
}

double oscFunc(double hertz, double time, Oscillator::Type osc)
{
	switch (osc)
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

// #TODO - better name?
struct MakeSoundFunct
{
	double& m_frequencyOutput;
	Oscillator::Type& m_currOsc;
	ADSRModule& m_ADSR;

	double m_masterVolume;

	MakeSoundFunct() = delete;
	explicit MakeSoundFunct(double& frequencyOutput, Oscillator::Type& osc, ADSRModule& adsr) 
		: m_frequencyOutput(frequencyOutput)
		, m_currOsc(osc)
		, m_ADSR(adsr)
		, m_masterVolume(0.25)
	{}
	MakeSoundFunct(double& frequencyOutput, Oscillator::Type& osc, ADSRModule& adsr, double volume)
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

void printControls(std::wostream& out, const Oscillator::Type osc, const Octaves::Type oct, const double freq, const bool noteOn)
{
	//#TODO - need buffer
	//out << L"\rOscillator: " << Oscillator::Markers[osc];
	//out << L"\Octave: "      << Octaves::Markers[osc];

	if (noteOn)
		out << L"\rNote On: " << freq << "Hz        ";
	else
		out << L"\rNote Off: " << freq << "Hz       ";
}

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
		Oscillator::Type currOsc = Oscillator::SINE;
		ADSRModule ADSR;
		Octaves::Type currOct = Octaves::A3;

		MakeSoundFunct soundFunc(frequency, currOsc, ADSR , 0.5);
		SoundWrapper soundMachine(usingDevice, 44100, 1, 8, 512, soundFunc);

		const double c_Root12Of2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per octave

		//#DEFECT - looking bad
		int currKey = -1;
		bool isKeyPressed = false;
		const std::string c_pianoKeys{ "AWSDRFTGHUJIKOL\xBA\xDB\xDE\xDD\xDC" };
		const std::string c_controlKeys{ "ZX12345" };

		while (true)
		{
			isKeyPressed = false;
			for (int k = 0; k < c_pianoKeys.size(); k++)
			{
				if (GetAsyncKeyState(static_cast<unsigned char>(c_pianoKeys[k])) & KEY_HOLD)
				{
					if (currKey != k)
					{
						frequency = Octaves::Values[currOct] * pow(c_Root12Of2, k);
						ADSR.NoteOn(soundMachine.GetGlobalTime());
						currKey = k;

						printControls(std::wcout, currOsc, currOct, frequency, true);
					}

					isKeyPressed = true;
				}
			}
			
			//#TODO - output current settings
			for (auto k : c_controlKeys)
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

					printControls(std::wcout, currOsc, currOct, frequency, false);
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
		std::wcout << ex.what() << std::endl;
		return 1;
	}

	return 0;
}