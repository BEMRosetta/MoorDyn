#ifndef _MoorDyn_v1_DLL_MoorDyn_lib_h_
#define _MoorDyn_v1_DLL_MoorDyn_lib_h_


void MoorDyn_v1_Load(const char *dllPath);

// Define a macro to manage extern linkage
#ifdef DEFINE_MOORDYN_LIB
#define MOORDYN_LIB extern
#else
#define MOORDYN_LIB
#endif

MOORDYN_LIB int(*LinesInit)(double X[], double XD[]);
MOORDYN_LIB int(*LinesCalc)(double X[], double XD[], double Flines[], double* t_in, double* dt_in);
MOORDYN_LIB int(*LinesClose)();
MOORDYN_LIB double(*GetFairTen)(int l);
	
#endif
