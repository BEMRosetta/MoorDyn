#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>
#include <ScatterDraw/DataSource.h>

#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v2_DLL/MoorDyn2_lib.h>
#else
	#include <MoorDyn_v2/MoorDyn2.h>
	#include <MoorDyn_v2/MoorDyn2.hpp>
#endif

using namespace Upp;


#define TOL 1.0e-1

bool
compare(double v1, double v2, double tol)
{
	return fabs(v1 - v2) <= tol;
}

#define CHECK_VALUE(name, v1, v2, tol, t)                                      \
	if (!compare(v1, v2, tol)) {                                               \
		cerr << setprecision(8) << "Checking " << name                         \
		     << " failed at t = " << t << " s. " << v1 << " was expected"      \
		     << " but " << v2 << " was computed" << endl;                      \
		MoorDyn_Close(system);                                                 \
		return false;                                                          \
	}

using namespace std;


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
/*
void Positions() {
	#ifdef flagMOORDYN_DLL
		MoorDyn_v2_Load(AFX(GetSourceFolder(), 
	#ifdef _WIN64
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2.dll"
	#else
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2_32.dll"
	#endif		
		));
	#endif
	
	MoorDyn system;
	try {
		UVector<double> tm;
		UVector<UVector<double>> positions, velocities;

		Calculation(tm, positions, velocities);

		LinesInit(positions[0].begin(), velocities[0].begin());
	    
	    double dt = tm[1] - tm[0];
	    double Flines_dummy[6];
	    
	    Cout() << "\n";
	    
	    UVector<UVector<double>> forces(positions.size());
	       
	    for (int i = 0; i < positions.size(); ++i) {
	        double t = tm[i];
	        printf("\rt: %f", t);
	        forces[i].SetCount(4);
	        forces[i][0] = t;
	    	LinesCalc(positions[i].begin(), velocities[i].begin(), Flines_dummy, &t, &dt);
	    	forces[i][1] = GetFairTen(1)/1000;
	    	forces[i][2] = GetFairTen(2)/1000;
	    	forces[i][3] = GetFairTen(3)/1000;
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
}
	*/	
	
void Demo() {
	MoorDyn system;
	try {
		String path = AFX(GetSourceFolder(), "mooring/lines.txt");
		
		system = MoorDyn_Create(path);
		if (!system)
	    	throw Exc("MoorDyn_Create");

		unsigned int n;
		if (MoorDyn_NCoupledDOF(system, &n) != MOORDYN_SUCCESS && n != 9)
			throw Exc("MoorDyn_NCoupledDOF");
		
	    // 3 coupled points x 3 components per point = 9 DoF
	    double x[9], xd[9];
	    memset(xd, 0, 9*sizeof(double));
	    
	    moordyn::MoorDyn *sys = (moordyn::MoorDyn*)system;
	    const vector<moordyn::Point*> points = sys->GetPoints();
	    for (int i = 0; i < points.size(); ++i) {
	        int typ = points[i]->type;
			if (points[i]->type == moordyn::Point::COUPLED)
				int kk = 1;
	    }
	    
	    // Get the initial positions from the system itself
	    if (MoorDyn_GetPointPos(MoorDyn_GetPoint(system, 1), x) != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_GetPointPos");
	    if (MoorDyn_GetPointPos(MoorDyn_GetPoint(system, 3), x + 3) != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_GetPointPos");
	    if (MoorDyn_GetPointPos(MoorDyn_GetPoint(system, 5), x + 6) != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_GetPointPos");
	
	    // Setup the initial condition
	    if (MoorDyn_Init(system, x, xd) != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_Init");
	
	    // Make the points move at 0.5 m/s to the positive x direction
	    xd[0] = xd[3] = xd[6] = 0.5;

	    double maxT = 1, dt = 0.1;
	    double f[9];
	    
	    for (double t = dt; t <= maxT; t += dt) {
	        x[0] += xd[0]*dt;
	        x[3] += xd[3]*dt;
	        x[6] += xd[6]*dt;
	        
		    if (MoorDyn_Step(system, x, xd, f, &t, &dt) != MOORDYN_SUCCESS)
		    	throw Exc("MoorDyn_Step");
		
		    // Print the position and tension of the line nodes
		    unsigned int n_lines;
		    if (MoorDyn_GetNumberLines(system, &n_lines) != MOORDYN_SUCCESS)
		        throw Exc("MoorDyn_GetNumberLines");
		    
		    for (unsigned int i = 0; i < n_lines; i++) {
		        const unsigned int line_id = i + 1;
		        printf("Line %u\n", line_id);
		        
		        MoorDynLine line = MoorDyn_GetLine(system, line_id);
		        if (!line)
		        	throw Exc("MoorDyn_GetLine");
		        
		        unsigned int n_nodes;
		        if (MoorDyn_GetLineNumberNodes(line, &n_nodes) != MOORDYN_SUCCESS)
		        	throw Exc("MoorDyn_GetLineNumberNodes");
		        
		        for (unsigned int j = 0; j < n_nodes; j++) {
		            printf("node %u:\t", j);
		            double pos[3], ten[3];
		            
		            if (MoorDyn_GetLineNodePos(line, j, pos) != MOORDYN_SUCCESS)
		            	throw Exc("MoorDyn_GetLineNodePos");
		            printf("pos = [%g, %g, %g]\t", pos[0], pos[1], pos[2]);
	
		            if (MoorDyn_GetLineNodeTen(line, j, ten) != MOORDYN_SUCCESS)
		                throw Exc("MoorDyn_GetLineNodeTen");
		            printf("ten = [%g, %g, %g]  %g\n", ten[0], ten[1], ten[2], sqrt(sqr(ten[0]) + sqr(ten[1]) + sqr(ten[2])));
		        }
				double tens;
				if (MoorDyn_GetLineFairTen(line, &tens) != MOORDYN_SUCCESS)
		        	throw Exc("MoorDyn_GetLineFairTen");
		        
				printf("fairtens = [%g]\n", tens);
		    }
	    }
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    // Alright, time to finish!
    if (system)
    	MoorDyn_Close(system);	
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
	
	#ifdef flagMOORDYN_DLL
		MoorDyn_v2_Load(AFX(GetSourceFolder(), 
	#ifdef _WIN64
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2.dll"
	#else
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2_32.dll"
	#endif		
		));
	#endif
	
	Demo();
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}

