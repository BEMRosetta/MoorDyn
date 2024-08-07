#ifndef _MoorDyn_v2_v1v2_h_
#define _MoorDyn_v2_v1v2_h_

extern "C" {
	__declspec(dllexport) int LinesInit(double X[], double XD[]) noexcept;
	__declspec(dllexport) int LinesCalc(double X[], double XD[], double Flines[], double* t_in, double* dt_in) noexcept;
	//__declspec(dllexport) double GetFairTen(int l) noexcept;
	__declspec(dllexport) int LinesClose() noexcept;
};

#endif
