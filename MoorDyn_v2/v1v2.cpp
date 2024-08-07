#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>

#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v2_DLL/MoorDyn2_lib.h>
#else
	#include <MoorDyn_v2/MoorDyn2.h>
	#include <MoorDyn_v2/MoorDyn2.hpp>
#endif

using namespace Upp;

int LinesInit(double X[], double XD[]) noexcept {
	
}

int LinesCalc(double X[], double XD[], double Flines[], double* t_in, double* dt_in) noexcept {
	
}

int LinesClose() noexcept {
	
}

