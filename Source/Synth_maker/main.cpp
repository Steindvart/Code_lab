#include "Shared/stdafx.h"
#include "Shared/Exception.h"

#include "SoundWrapper.h"

// Aliases for mask of GetAsyncKeyState() response
#define KEY_HOLD  0x8000
#define KEY_PRESS 0x01

namespace Octaves
{
constexpr double A0_val = 27.5;
constexpr double A1_val = A0_val * 2;
constexpr double A2_val = A1_val * 2;
constexpr double A3_val = A2_val * 2;
constexpr double A4_val = A3_val * 2;
constexpr double A5_val = A4_val * 2;
constexpr double A6_val = A5_val * 2;
constexpr double A7_val = A6_val * 2;
constexpr double A8_val = A7_val * 2;

enum Type
{
	A0, A1, A2, A3, A4, A5, A6, A7, A8
};
static const std::vector<double> All{ A0_val, A1_val, A2_val, A3_val, A4_val, A5_val, A6_val, A7_val, A8_val };
}

struct MakeSoundFunct
{
	double& m_frequencyOutput;
	double m_masterVolume;

	MakeSoundFunct() = delete;
	explicit MakeSoundFunct(double& frequencyOutput) : m_frequencyOutput(frequencyOutput), m_masterVolume(0.25) {}
	MakeSoundFunct(double& frequencyOutput, double volume) : m_frequencyOutput(frequencyOutput), m_masterVolume(volume) {}

	// time variable need, because it's shift on `x` coordinate 
	double operator() (double time)
	{
		const double radiansFromFrequency = m_frequencyOutput * 2.0 * std::numbers::pi;
		return sin(radiansFromFrequency * time) * m_masterVolume;
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
		MakeSoundFunct soundFunc(frequency, 0.1);
		SoundWrapper soundMachine(usingDevice, 44100, 1, 8, 512, soundFunc);

		const double Root12Of2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatave
		Octaves::Type currOct = Octaves::A3;

		//#DEFECT - looking bad
		int currKey = -1;
		bool isKeyPressed = false;
		const std::string pianoKeys{ "AWSDRFTGHUJIKOL\xBA\xDB\xDE\xDD\xDC" };
		const std::string controlKeys{ "ZX" };

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
						std::wcout << "\rNote On : " << soundMachine.GetGlobalTime() << "s " << frequency << "Hz";
						currKey = k;
					}

					isKeyPressed = true;
				}
			}
			
			//#TODO - output current octave
			for (auto k : controlKeys)
			{
				if (GetAsyncKeyState(k) & KEY_PRESS)
				{
					if (k == 'Z' && currOct > Octaves::A0)
						currOct = static_cast<Octaves::Type>(currOct - 1);
					if (k == 'X' && currOct < Octaves::A8)
						currOct = static_cast<Octaves::Type>(currOct + 1);
				}
			}

			if (!isKeyPressed)
			{
				frequency = 0.0;

				if (currKey != -1)
				{
					std::wcout << "\rNote Off: " << soundMachine.GetGlobalTime() << "s               ";
					currKey = -1;
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