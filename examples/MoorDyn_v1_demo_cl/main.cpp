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


void Calculation(UVector<double> &tm, UVector<UVector<double>> &positions, UVector<UVector<double>> &velocities) {
	UVector<UVector<Value>> positionsV = ReadCSVFile(AFX(GetExeFolder(), "mooring/positions.csv"), ',', false, false, '.', false, 1);
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
	
	positions.SetCount(num);
	const Point3D pos(-22.498, 0, -13.585), 
				  c0(0, 0, 0);
	
	for (int r = 0; r < num; ++r) {
		Affine3d aff = GetTransform(Vector3D(positions0[r][0], positions0[r][1], positions0[r][2]), 
									Vector3D(positions0[r][3], positions0[r][4], positions0[r][5]), c0);
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
    if (!SaveFile(AFX(GetExeFolder(), "mooring/positions_new.csv"), str))
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
    if (!SaveFile(AFX(GetExeFolder(), "mooring/velocities.csv"), vstr))
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
#ifdef flagMOORDYN_DLL
		String pathDLL;
		if (GetFileTitle(GetSourceFolder()) == "MoorDyn_v1_demo_cl") 
	#ifdef _WIN64
			pathDLL = "../../bin/MoorDyn_v1.dll";
	#else
			pathDLL = "../../bin/MoorDyn_v1_32.dll";
	#endif		
		else
	#ifdef _WIN64
			pathDLL = "../../bin/MoorDyn_v1.5.dll";
	#else
			pathDLL = "../../bin/MoorDyn_v1.5_32.dll";
	#endif		
	
		MoorDyn_v1_Load(AFX(GetSourceFolder(), pathDLL));
#endif

		UVector<double> tm;
		UVector<UVector<double>> positions, velocities;
		
		if (!RealizeDirectory(AFX(GetExeFolder(), "mooring")))
			throw Exc(Format("Error creating 'mooring' folder: %s", GetLastErrorMessage()));
		if (!FileCopy(AFX(GetSourceFolder(), "mooring/lines.txt"), AFX(GetExeFolder(), "mooring/lines.txt")))
			throw Exc(Format("Error copying 'lines' file: %s", GetLastErrorMessage()));
		if (!FileCopy(AFX(GetSourceFolder(), "mooring/positions.csv"), AFX(GetExeFolder(), "mooring/positions.csv")))
			throw Exc(Format("Error copying 'positions' file: %s", GetLastErrorMessage()));
		
		Calculation(tm, positions, velocities);
		
		if (LinesInit(positions[0].begin(), velocities[0].begin()) != 0)
			throw Exc("Problem in LinesInit");
	    
	    double dt = tm[1] - tm[0];
	    
	    Cout() << "\n";
	    
	    UVector<UVector<double>> forces(positions.size()), fplat(positions.size());
	    
	    for (int i = 0; i < positions.size(); ++i) {
	        double t = tm[i];

	        forces[i].SetCount(4);
	        forces[i][0] = t;
	        fplat[i].SetCount(9);
	    	if (LinesCalc(positions[i].begin(), velocities[i].begin(), fplat[i].begin(), &t, &dt) != 0)
	    		throw Exc("Problem in LinesCalc");
	    	
	    	forces[i][1] = GetFairTen(1)/1000;
	    	forces[i][2] = GetFairTen(2)/1000;
	    	forces[i][3] = GetFairTen(3)/1000;
	    }
	    
	    Cout() << "\n";
	    
	    if (LinesClose() != 0)
	        throw Exc("Problem in LinesClose");
	    
	    String str = "t,f1,f2,f3,fx,fy,fz,frx,fry,frz";
	    for (int r = 0; r < positions.size(); ++r) {
			str << "\n";
			for (int c = 0; c < 4; ++c) {
				if (c > 0)
					str << ",";
				str << forces[r][c];
	        }
	        for (int c = 0; c < 6; ++c) 
	            str << "," << fplat[r][c]/1000;
	    }
	    if (!SaveFile(AFX(GetExeFolder(), "mooring/forces.csv"), str))
			throw Exc("Problem saving forces");
	    
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}

