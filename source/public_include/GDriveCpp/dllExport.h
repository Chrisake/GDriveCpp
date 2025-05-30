#pragma once

#ifdef GDRIVECPP_BUILD_DLL
#define GDRIVE_API __declspec(dllexport)
#else
#define GDRIVE_API __declspec(dllimport)
#endif
