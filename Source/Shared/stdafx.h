#ifndef STDAFX_H
#define STDAFX_H

#ifdef SKIP_PRECOMPILED_HEADERS

#pragma message("Building without precompiled headers")

#else // SKIP_PRECOMPILED_HEADERS

// STD
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>

#include <atomic>
#include <condition_variable>
#include <thread>

// Windows
#include <Windows.h>

#endif // !SKIP_PRECOMPILED_HEADERS

#endif // !STDAFX_H