#include "Shared/stdafx.h"

#include "SoundWrapper.h"
#include "Shared/Exception.h"

int main()
{
	try
	{
		SoundWrapper soundMachine(L"", 44100, 1, 8, 512);
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