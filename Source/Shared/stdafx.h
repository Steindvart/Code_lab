#ifndef STDAFX_H
#define STDAFX_H

#ifdef SKIP_PRECOMPILED_HEADERS

#pragma message("Building without precompiled headers")

#else // SKIP_PRECOMPILED_HEADERS

// STD
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <numeric>
#include <numbers>

// STL-containers
#include <vector>

// Threads
#include <atomic>
#include <thread>
#include <condition_variable>

// Windows
#include <Windows.h>

#endif // !SKIP_PRECOMPILED_HEADERS

#endif // !STDAFX_H