#include "Shared/stdafx.h"

#include "SoundWrapper.h"
#include "Shared/Exception.h"

//#TODO - think about this
// Global synthesizer variables
std::atomic<double> g_frequencyOutput = 0.0;		// dominant output frequency of instrument, i.e. the note
double g_octaveBaseFrequency = 110.0; // A2			// frequency of octave represented by keyboard
double g_12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatave

// Function used by SoundWrapper to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double time)
{
	double output = sin(g_frequencyOutput * 2.0 * 3.14159 * time);
	return output * 0.5; // Master Volume
}

int main()
{
	setlocale(LC_ALL, "Russian");
	std::wcout << "Synth_maker - simple software synthesizer.\n" << std::endl;

	const auto availableDevices = SoundWrapper::GetAudioDevicesNames();

	std::wcout << "Available devices:" << std::endl;
	for (auto i = 0; i < availableDevices.size(); i++)
		std::wcout << i << ". " << availableDevices[i] << std::endl;

	const auto usingDevice = availableDevices[0];
	std::wcout << std::endl << "Using device: 0. " << usingDevice;

	std::wcout << std::endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << std::endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << std::endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << std::endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << std::endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << std::endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << std::endl << std::endl;

	try
	{
		SoundWrapper soundMachine(usingDevice, 44100, 1, 8, 512);
		soundMachine.SetNoizeFunc(MakeNoise);

		// Sit in loop, capturing keyboard state changes and modify
		// synthesizer output accordingly
		int currKey = -1;
		bool isKeyPressed = false;
		while (true)
		{
			isKeyPressed = false;
			for (int k = 0; k < 16; k++)
			{
				if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
				{
					if (currKey != k)
					{
						g_frequencyOutput = g_octaveBaseFrequency * pow(g_12thRootOf2, k);
						std::wcout << "\rNote On : " << soundMachine.GetGlobalTime() << "s " << g_frequencyOutput << "Hz";
						currKey = k;
					}

					isKeyPressed = true;
				}
			}

			if (!isKeyPressed)
			{
				if (currKey != -1)
				{
					std::wcout << "\rNote Off: " << soundMachine.GetGlobalTime() << "s                        ";
					currKey = -1;
				}

				g_frequencyOutput = 0.0;
			}
		}

		return 0;
	}
	catch (Shared::Exception& ex)
	{
		std::wcout << ex.Reason() << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	return 0;
}