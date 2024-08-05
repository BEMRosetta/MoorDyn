#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>
#include <ScatterDraw/DataSource.h>
#include <STEM4U/Iir_fir.h>

#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v1_DLL/MoorDyn_lib.h>
#else
	#include <MoorDyn_v1/MoorDyn.h>
#endif

using namespace Upp;


void BasicTest(UVector<double> &tm, UVector<UVector<double>> &positions, UVector<UVector<double>> &velocities) {
	double t = 500;
	double ddt = 0.05;
	int num = t/ddt+1;
	tm.SetCount(num);
	positions.SetCount(num);
	velocities.SetCount(num);
	for (int r = 0; r < num; ++r) {
		tm[r] = r*ddt;
		positions[r].SetCount(6);		
		velocities[r].SetCount(6);
		for (int c = 0; c < 6; ++c) {
			positions[r][c] = 0; 
			velocities[r][c] = 0; 
		}
		//positions[r][0] = 22.5;
		//positions[r][2] = 13.5;
	}	
}

void Calculation(UVector<double> &tm, UVector<UVector<double>> &positions, UVector<UVector<double>> &velocities) {
	UVector<UVector<Value>> positionsV = ReadCSVFile(AFX(GetSourceFolder(), "mooring/positions.csv"), ',', false, false, '.', false, 1);
	if (positionsV.IsEmpty())
		throw Exc("File not found");
	if (positionsV[0].size() != 7)
		throw Exc("Wrong column mumber");
	
	int num = positionsV.size();
	tm.SetCount(num);
	UVector<UVector<double>> positions0(num);
	for (int r = 0; r < num; ++r) {
		tm[r] = positionsV[r][0]; 
		positions0[r].SetCount(6);
		for (int c = 0; c < 3; ++c) 
			positions0[r][c] = positionsV[r][c+1]; 
		for (int c = 3; c < 6; ++c) 
			positions0[r][c] = ToRad(double(positionsV[r][c+1])); 
	}	
	
	/*
	VectorXd tm_ = Eigen::Map<VectorXd>(tm.begin(), num);
	double srate = 0.05;
	VectorXd ntm;
	UArray<VectorXd> ndata(6);
	for (int c = 0; c < 6; ++c) {
		VectorXd data(num);
		for (int r = 0; r < num; ++r) 
			data[r] = positions0[r][c];
			
		Resample(tm_, data, ntm, ndata[c], srate);
	}
	num = ntm.size();
	tm.SetCount(num);
	tm_.resize(num);
	positions0.SetCount(num);
	for (int r = 0; r < num; ++r) {
		tm[r] = tm_[r] = ntm[r];
		positions0[r].SetCount(6);
		for (int c = 0; c < 6; ++c) 
			positions0[r][c] = ndata[c][r];
	}*/
	
	
	positions.SetCount(num);
	const Point3D pos(-22.498, 0, -13.585);
	
	for (int r = 0; r < num; ++r) {
		Affine3d aff;
		GetTransform000(aff, positions0[r][0], positions0[r][1], positions0[r][2], positions0[r][3], positions0[r][4], positions0[r][5]);
		Point3D npos;
		TransRot(aff, pos, npos);
		positions[r].SetCount(6);
		positions[r][0] = npos[0];
		positions[r][1] = npos[1];
		positions[r][2] = npos[2];
		positions[r][3] = positions0[r][3];
		positions[r][4] = positions0[r][4];
		positions[r][5] = positions0[r][5];
	}
	
	String str = "t,x,y,z,rx,ry,rz";
    for (int r = 0; r < num; ++r) {
		str << "\n";
		str << tm[r];
		for (int c = 0; c < 6; ++c) 
			str << "," << positions[r][c];
    }
    if (!SaveFile(AFX(GetSourceFolder(), "mooring/positions_new.csv"), str))
		throw Exc("Problem saving translated positions");

	velocities.SetCount(num);
	for (int r = 0; r < num; ++r)
		velocities[r].SetCount(6);	
	
	int deg = 2;
	int wsize = 5;
	int der = 1;
	VectorXd tm_ = Eigen::Map<VectorXd>(tm.begin(), num);
	for (int c = 0; c < 6; ++c) {
		VectorXd resy, resx;
		VectorXd y(num);
		for (int r = 0; r < num; ++r)
			y(r) = positions[r][c];
		
		if (!SavitzkyGolay(y, tm_, deg, wsize, der, resy, resx))
			throw Exc("Problem in velocity calculation");
		
		for (int r = 0; r < num; ++r)
			velocities[r][c] = resy(r);
	}
	velocities.SetCount(num);
	
	String vstr = "t,x,y,z,rx,ry,rz";
    for (int r = 0; r < num; ++r) {
		vstr << "\n";
		vstr << tm_[r];
		for (int c = 0; c < 6; ++c) 
			vstr << "," << velocities[r][c];
    }
    if (!SaveFile(AFX(GetSourceFolder(), "mooring/velocities.csv"), vstr))
		throw Exc("Problem saving translated positions");
}


CONSOLE_APP_MAIN
{
#ifdef PLATFORM_WIN32
 #ifdef flagMOORDYN_DLL
	Cout() << "Windows DLL ";
 #else
 	Cout() << "Windows source ";
 #endif
 #ifdef _WIN64
	Cout() << "64 bits";
 #elif _WIN32
	Cout() << "32 bits";
 #else
 	Cout() << "Not supported";
 #endif
#elif PLATFORM_LINUX
	Cout() << "Linux source ";
#else
	Cout() << "Not supported";
#endif

	try {
		UVector<double> tm;
		UVector<UVector<double>> positions, velocities;
		
		//BasicTest(tm, positions, velocities);
		Calculation(tm, positions, velocities);
	
#ifdef flagMOORDYN_DLL
		MoorDyn_v1_Load(AFX(GetSourceFolder(), 
	#ifdef _WIN64
			"../../MoorDyn_v1_DLL/bin/MoorDyn_Win64.dll"
	#else
			"../../MoorDyn_v1_DLL/bin/MoorDyn_Win32.dll"
	#endif		
		));
#endif

		
		LinesInit(positions[0].begin(), velocities[0].begin());
	    
	    double dt = tm[1] - tm[0];
	    double Flines_dummy[6];
	    
	    Cout() << "\n";
	    
	    UVector<UVector<double>> forces(positions.size());
	    
	    BiquadFilter<double> f1, f2, f3;
	    //Linear_IIR_Filter<double> f1, f2, f3;
	    //f1.LowPassCoefficients(1/4., dt);
	    //FIRFilter<double> f1, f2, f3;
	    f1.LowPassCoefficients(1/3., dt, 1/sqrt(2));
	    //f1.LowPassCoefficients(1/0.6, dt);
	    f2 = clone(f1);
	    f3 = clone(f1);
	    
	    for (int i = 0; i < positions.size(); ++i) {
	        double t = tm[i];
	        printf("\rt: %f", t);
	        forces[i].SetCount(4);
	        forces[i][0] = t;
	    	LinesCalc(positions[i].begin(), velocities[i].begin(), Flines_dummy, &t, &dt);
	    	forces[i][1] = f1.Filter(GetFairTen(1)/1000);
	    	forces[i][2] = f2.Filter(GetFairTen(2)/1000);
	    	forces[i][3] = f3.Filter(GetFairTen(3)/1000);
	    }
	    
	    Cout() << "\n";
	    
	    LinesClose();
	    
	    String str = "t,f1,f2,f3";
	    for (int r = 0; r < positions.size(); ++r) {
			str << "\n";
			for (int c = 0; c < 4; ++c) {
				if (c > 0)
					str << ",";
				str << forces[r][c];
	        }
	    }
	    if (!SaveFile(AFX(GetSourceFolder(), "mooring/forces.csv"), str))
			throw Exc("Problem saving forces");
	    
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}

