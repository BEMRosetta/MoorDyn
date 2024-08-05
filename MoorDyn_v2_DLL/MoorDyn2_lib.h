#ifndef _MoorDyn_v1_DLL_MoorDyn_lib_h_
#define _MoorDyn_v1_DLL_MoorDyn_lib_h_

#include "../MoorDyn_v2/MoorDynAPI.h"

void MoorDyn_v2_Load(const char *dllPath);

// Define a macro to manage extern linkage
#ifdef DEFINE_MOORDYN_LIB
#define MOORDYN_LIB extern
#else
#define MOORDYN_LIB
#endif

typedef struct __MoorDyn* MoorDyn;
typedef struct __MoorDynPoint* MoorDynPoint;
typedef struct __MoorDynLine* MoorDynLine;

MOORDYN_LIB MoorDyn(*MoorDyn_Create)(const char* infilename);
MOORDYN_LIB MoorDynPoint(*MoorDyn_GetPoint)(MoorDyn system, unsigned int l);
MOORDYN_LIB int(*MoorDyn_GetPointPos)(MoorDynPoint point, double pos[3]);
MOORDYN_LIB int(*MoorDyn_Init)(MoorDyn system, const double* x, const double* xd);
MOORDYN_LIB int(*MoorDyn_Step)(MoorDyn system, const double* x, const double* xd, double* f, double* t, double* dt);
MOORDYN_LIB int(*MoorDyn_GetNumberLines)(MoorDyn system, unsigned int* n);
MOORDYN_LIB MoorDynLine(*MoorDyn_GetLine)(MoorDyn system, unsigned int l);
MOORDYN_LIB int(*MoorDyn_GetLineNumberNodes)(MoorDynLine l, unsigned int* n);
MOORDYN_LIB int(*MoorDyn_GetLineNodePos)(MoorDynLine l, unsigned int i, double pos[3]);
MOORDYN_LIB int(*MoorDyn_GetLineNodeTen)(MoorDynLine l, unsigned int i, double ten[3]);
MOORDYN_LIB int(*MoorDyn_Close)(MoorDyn system);
	
#endif
