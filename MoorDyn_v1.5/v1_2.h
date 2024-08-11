#ifndef _MoorDyn_v2_v1_2_h_
#define _MoorDyn_v2_v1_2_h_

extern "C" {
	int DECLDIR LinesInit(double X[], double XD[]) noexcept;
	int DECLDIR LinesCalc(double X[], double XD[], double Flines[], double* t_in, double* dt_in) noexcept;
	int DECLDIR LinesClose() noexcept;
};

#endif
