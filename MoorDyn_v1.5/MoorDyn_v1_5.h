#ifndef _MoorDyn_v1_5_h_
#define _MoorDyn_v1_5_h_

extern "C" {
	int DECLDIR LinesInit(double X[], double XD[]) noexcept;
	int DECLDIR LinesCalc(double X[], double XD[], double Flines[], double* t_in, double* dt_in) noexcept;
	int DECLDIR LinesClose() noexcept;
	
	int DECLDIR FairleadsCalc2(double rFairIn[], double rdFairIn[], double fFairIn[], double* t_in, double *dt_in); // easier to call version
	int DECLDIR FairleadsCalc(double **rFairIn, double **rdFairIn, double ** fFairIn, double* t_in, double *dt_in);
	
	int DECLDIR GetConnectPos(int l, double pos[3]);
	int DECLDIR GetConnectForce(int l, double force[3]);
	
	int DECLDIR DrawWithGL(void);	
};

#endif
