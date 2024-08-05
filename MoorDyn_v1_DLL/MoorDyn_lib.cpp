#include <Functions4U/Functions4U.h>

using namespace Upp;

#define DEFINE_MOORDYN_LIB
#include "MoorDyn_lib.h"


void MoorDyn_v1_Load(const char *dllPath) {
	static Dll dll;

	dll.Load_throw(dllPath);
		
	LinesInit = (int(*)(double X[], double XD[]))dll.GetFunction_throw("LinesInit");
	LinesCalc = (int(*)(double X[], double XD[], double Flines[], double* t_in, double* dt_in))dll.GetFunction_throw("LinesCalc");
	LinesClose = (int(*)())dll.GetFunction_throw("LinesClose");
	GetFairTen = (double(*)(int l))dll.GetFunction_throw("GetFairTen");
}