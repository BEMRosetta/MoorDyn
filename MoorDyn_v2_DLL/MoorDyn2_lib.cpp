#include <Functions4U/Functions4U.h>

using namespace Upp;

#define DEFINE_MOORDYN_LIB
#include "MoorDyn2_lib.h"


void MoorDyn_v2_Load(const char *dllPath) {
	static Dll dll;

	dll.Load_throw(dllPath);
		
	MoorDyn_Create = (MoorDyn(*)(const char* infilename))dll.GetFunction_throw("MoorDyn_Create");
	MoorDyn_GetPoint = (MoorDynPoint(*)(MoorDyn system, unsigned int l))dll.GetFunction_throw("MoorDyn_GetPoint");
	MoorDyn_GetPointPos = (int(*)(MoorDynPoint point, double pos[3]))dll.GetFunction_throw("MoorDyn_GetPointPos");
	MoorDyn_Init = (int(*)(MoorDyn system, const double* x, const double* xd))dll.GetFunction_throw("MoorDyn_Init");
	MoorDyn_Step = (int(*)(MoorDyn system, const double* x, const double* xd, double* f, double* t, double* dt))dll.GetFunction_throw("MoorDyn_Step");
	MoorDyn_GetNumberLines = (int(*)(MoorDyn system, unsigned int* n))dll.GetFunction_throw("MoorDyn_GetNumberLines");
	MoorDyn_GetLine = (MoorDynLine(*)(MoorDyn system, unsigned int l))dll.GetFunction_throw("MoorDyn_GetLine");
	MoorDyn_GetLineNumberNodes = (int(*)(MoorDynLine l, unsigned int* n))dll.GetFunction_throw("MoorDyn_GetLineNumberNodes");
	MoorDyn_GetLineNodePos = (int(*)(MoorDynLine l, unsigned int i, double pos[3]))dll.GetFunction_throw("MoorDyn_GetLineNodePos");
	MoorDyn_GetLineNodeTen = (int(*)(MoorDynLine l, unsigned int i, double ten[3]))dll.GetFunction_throw("MoorDyn_GetLineNodeTen");
	MoorDyn_NCoupledDOF = (int(*)(MoorDyn system, unsigned int* n))dll.GetFunction_throw("MoorDyn_NCoupledDOF");
	MoorDyn_GetLineFairTen = (int(*)(MoorDynLine l, double* t))dll.GetFunction_throw("MoorDyn_GetLineFairTen");
	MoorDyn_Close = (int(*)(MoorDyn system))dll.GetFunction_throw("MoorDyn_Close");
}