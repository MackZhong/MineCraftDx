#pragma once

#ifdef _LIB
#define LIB_NBT_EXPORT __declspec(dllexport)
#else
#define LIB_NBT_EXPORT __declspec(dllimport)
#endif
#undef LIB_NBT_EXPORT
#define LIB_NBT_EXPORT 

#include "NbtTag.h"
