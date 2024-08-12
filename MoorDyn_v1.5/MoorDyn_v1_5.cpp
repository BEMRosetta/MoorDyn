#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif


#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v1_DLL/MoorDyn_lib.h>
#else
	#include <MoorDyn_v2/MoorDyn2.h>
	#include <MoorDyn_v2/MoorDyn2.hpp>
	#include "MoorDyn_v1_5.h"
#endif

using namespace Upp;

#ifdef WIN32
/// Console handle
extern int hConHandle;
/// Std output handle
extern intptr_t lStdHandle;

/// pointer to be made to environment variable PROMPT
extern char const* PromptPtr;
/// 0 if the system console is used, 1 if the console has been created by us
extern int OwnConsoleWindow;
#endif

extern MoorDyn md_singleton;

UArray<Point3D> fairleads;


int LinesInit(double X[], double XD[]) noexcept {
#ifdef WIN32
	// ------------ create console window for messages if none already available
	// ----------------- adapted from Andrew S. Tucker, "Adding Console I/O to a
	// Win32 GUI App" in Windows Developer Journal, December 1997. source code
	// at http://dslweb.nwnexus.com/~ast/dload/guicon.htm

	FILE* fp;
	// get pointer to environment variable "PROMPT" (NULL if not in console)
	PromptPtr = getenv("PROMPT");

	// TODO: simplify this to just keep the output parts I need

	HWND consoleWnd = GetConsoleWindow();
	if (!consoleWnd) {
		// if not in console, create our own
		OwnConsoleWindow = 1;

		// allocate a console for this app
		if (AllocConsole()) {
			// set the screen buffer to be big enough to let us scroll text
			static const WORD MAX_CONSOLE_LINES = 500;
			CONSOLE_SCREEN_BUFFER_INFO coninfo;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
			                           &coninfo);
			coninfo.dwSize.Y = MAX_CONSOLE_LINES;
			SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
			                           coninfo.dwSize);

			// redirect unbuffered STDOUT to the console
			// lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
			lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
			hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
			fp = _fdopen(hConHandle, "w");
			*stdout = *fp;
			setvbuf(stdout, NULL, _IONBF, 0);

			// redirect unbuffered STDERR to the console
			lStdHandle = (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
			hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
			fp = _fdopen(hConHandle, "w");
			*stderr = *fp;
			setvbuf(stderr, NULL, _IONBF, 0);

			// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
			// point to console as well
			std::ios::sync_with_stdio();

			std::cout << "(MoorDyn-initiated console window)" << std::endl;
		} else {
			// This is not a likely scenario, but we've run into some situations
			// where you can neither get the console nor allocate a console.
			// So just fall back to using whatever cout and cerr were before.
			std::cout << "AllocConsole failed" << std::endl;
			OwnConsoleWindow = 0;
		}
	}
#endif

	MoorDyn instance = MoorDyn_Create("mooring/lines.txt");
	if (!instance) {
		std::cout << "Error in MoorDyn_Create";
		return MOORDYN_UNHANDLED_ERROR;
	}
	
	unsigned int ndof;
	if (MoorDyn_NCoupledDOF(instance, &ndof) != MOORDYN_SUCCESS) {
		std::cout << "Error in MoorDyn_NCoupledDOF";
		return MOORDYN_UNHANDLED_ERROR;	
	}

	UVector<int> fairleadIds;	
	moordyn::MoorDyn *inst = (moordyn::MoorDyn*)instance;
    const vector<moordyn::Point*> points = inst->GetPoints();
    for (int i = 0; i < (int)points.size(); ++i) {
		if (points[i]->type == moordyn::Point::COUPLED)
			fairleadIds << (i+1);
    }

	if ((int)fairleadIds.size()*3 != (int)ndof) {
		std::cout << ~Format("Number of dof %d mismatch with the number of fairleads %d", (int)ndof, fairleadIds.size());
		return MOORDYN_UNHANDLED_ERROR;	
	}
	
	for (int i = 0; i < fairleadIds.size(); ++i) {
		double p[3];
		if (MoorDyn_GetPointPos(MoorDyn_GetPoint(instance, fairleadIds[i]), p) != MOORDYN_SUCCESS) {
	    	std::cout << "Error in MoorDyn_GetPointPos";	
	    	return MOORDYN_UNHANDLED_ERROR;	
		}
		fairleads << Point3D(p[0], p[1], p[2]);
	}
	
	Buffer<double> x(ndof), xd(ndof);
	
	for (int i = 0; i < fairleads.size(); ++i) {
		Point3D point = clone(fairleads[i]);
		point.TransRot(X[0], X[1], X[2], X[3], X[4], X[5], 0, 0, 0);
		x[3*i]   = point.x;
		x[3*i+1] = point.y;
		x[3*i+2] = point.z;
		
		Velocity6D v(XD);
		v.Translate(point);
		xd[3*i]   = v.t.x;
		xd[3*i+1] = v.t.y;
		xd[3*i+2] = v.t.z;
	}
	
	int err = MoorDyn_Init(instance, x, xd);
	if (err)
		return err;

	if (md_singleton)
		MoorDyn_Close(md_singleton); // We do not care if this fails
	md_singleton = instance;

	return MOORDYN_SUCCESS;
}

int LinesCalc(double X[], double XD[], double Flines[], double* t_in, double* dt_in) noexcept {
	int ndof = fairleads.size()*3;
	Point3D c0(0, 0, 0);
	
	Buffer<double> x(ndof), xd(ndof), f(ndof);
	UArray<Point3D> points = clone(fairleads);
	for (int i = 0; i < points.size(); ++i) {
		Point3D &p = points[i];
		p.TransRot(X[0], X[1], X[2], X[3], X[4], X[5], c0.x, c0.y, c0.z, RotationOrder::ZYX);
		x[3*i]   = p.x;
		x[3*i+1] = p.y;
		x[3*i+2] = p.z;
		
		Velocity6D v(XD);
		v.Translate(c0, p);
		xd[3*i]   = v.t.x;
		xd[3*i+1] = v.t.y;
		xd[3*i+2] = v.t.z;
	}
	int ret =  MoorDynStep(x, xd, f, t_in, dt_in);
	
	Force6D force = Force6D::Zero();
	for (int i = 0; i < fairleads.size(); ++i) {
		Vector3D forceFairlead(f[3*i], f[3*i+1], f[3*i+2]);
		force.Add(forceFairlead, points[i], c0);
	}
	for (int i = 0; i < 6; ++i)
		Flines[i] = force[i];		
	
	return ret;
}

int LinesClose() noexcept {
	return MoorDynClose();
}

int DECLDIR FairleadsCalc2(double rFairIn[], double rdFairIn[], double fFairIn[], double* t_in, double *dt_in) {
	std::cout << "FairleadsCalc2() is not included";
	return 0;
}

int DECLDIR FairleadsCalc(double **rFairIn, double **rdFairIn, double ** fFairIn, double* t_in, double *dt_in) {
	std::cout << "FairleadsCalc() is not included";
	return 0;
}

int DECLDIR GetConnectPos(int l, double pos[3]) {
	return GetPointPos(l, pos);
}

int DECLDIR GetConnectForce(int l, double force[3]) {
	return GetPointForce(l, force);
}

int DECLDIR DrawWithGL(void) {
#ifdef USEGL
	std::cout << "DrawWithGL() is not included";
#endif
	return 0;	
}
