/** @file platform.h A set of definitions automatically exported by CMake
 *
 * This file can be used to programatically get the software version, as well
 * as some configuration options
 *
 * If you want to use the C++ API, you probably want to include this file first
 */

#ifndef __MOORDYN_PLATFORM_H__
#define __MOORDYN_PLATFORM_H__

// Set in MSVC options /std:c++17

#define MOORDYN_MAJOR_VERSION	2
#define MOORDYN_MINOR_VERSION   3
#define MOORDYN_VERSION         2.3
#define MOORDYN_VERSION_STRING "2.3"
#define MOORDYN_PATCH_VERSION	0

//#define MOORDYN_SINGLEPRECISSION

#ifndef flagDLL
#define MoorDyn_EXPORTS
#endif

#endif // __MOORDYN_PLATFORM_H__
