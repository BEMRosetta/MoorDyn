#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>
#include <ScatterDraw/DataSource.h>

#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v2/MoorDyn2.hpp>
	#include <MoorDyn_v2_DLL/MoorDyn2_lib.h>
#else
	#include <MoorDyn_v2/MoorDyn2.h>
	#include <MoorDyn_v2/MoorDyn2.hpp>
#endif

using namespace Upp;


bool Demo(String linesPath) {
	UVector<double> tensions;
	MoorDyn system;
	bool ret = true;
	try {
		system = MoorDyn_Create(linesPath);
		if (!system)
	    	throw Exc("MoorDyn_Create");

		unsigned int n;
		if (MoorDyn_NCoupledDOF(system, &n) != MOORDYN_SUCCESS && n != 9)
			throw Exc("MoorDyn_NCoupledDOF");
		
	    // 3 coupled points x 3 components per point = 9 DoF
	    double x[9], xd[9];
	    memset(xd, 0, 9*sizeof(double));
	    
	    unsigned int numPoints;
	    if (MoorDyn_GetNumberPoints(system, &numPoints) != MOORDYN_SUCCESS)
			throw Exc("MoorDyn_GetNumberPoints");
	    
	    int ipos = 0;
	    for (unsigned int i = 1; i <= numPoints; ++i) {
	        MoorDynPoint point = MoorDyn_GetPoint(system, i);
	        if (!point)
				throw Exc("MoorDyn_GetPoint");
	        int type;
	        if (MoorDyn_GetPointType(point, &type) != MOORDYN_SUCCESS)
				throw Exc("MoorDyn_GetNumberPoints");
			if (type == moordyn::Point::COUPLED) {
				// Get the initial positions from the system itself
	    		if (MoorDyn_GetPointPos(MoorDyn_GetPoint(system, 1), x + ipos) != MOORDYN_SUCCESS)
	    			throw Exc("MoorDyn_GetPointPos");
	    		ipos += 3;
			}
	    }
	
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
		    
		    for (unsigned int line_id = 1; line_id <= n_lines; line_id++) {
		        Cout() << Format("Line %d\n", (int)line_id);
		        
		        MoorDynLine line = MoorDyn_GetLine(system, line_id);
		        if (!line)
		        	throw Exc("MoorDyn_GetLine");
		        
		        unsigned int n_nodes;
		        if (MoorDyn_GetLineNumberNodes(line, &n_nodes) != MOORDYN_SUCCESS)
		        	throw Exc("MoorDyn_GetLineNumberNodes");
		        
		        for (unsigned int inod = 0; inod < n_nodes; inod++) {
		            Cout() << Format("node %d:\t", (int)inod);
		            double pos[3], ten[3];
		            
		            if (MoorDyn_GetLineNodePos(line, inod, pos) != MOORDYN_SUCCESS)
		            	throw Exc("MoorDyn_GetLineNodePos");
		            Cout() << Format("pos = [%g, %g, %g]\t", pos[0], pos[1], pos[2]);
	
		            if (MoorDyn_GetLineNodeTen(line, inod, ten) != MOORDYN_SUCCESS)
		                throw Exc("MoorDyn_GetLineNodeTen");
		            Cout() << Format("ten = [%g, %g, %g]  %g\n", ten[0], ten[1], ten[2], sqrt(sqr(ten[0]) + sqr(ten[1]) + sqr(ten[2])));
		        }
				double tens;
				if (MoorDyn_GetLineFairTen(line, &tens) != MOORDYN_SUCCESS)
		        	throw Exc("MoorDyn_GetLineFairTen");
		        
				Cout() << Format("fairtens = [%g]\n", tens);
				tensions << tens;
		    }
	    }
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
		ret = false;
	} catch (const runtime_error &e) {
		Cout() << "\nError: " << e.what();	
		ret = false;
	} catch (...) {
		Cout() << "\nError.";	
		ret = false;
	}	 	
    // Alright, time to finish!
    if (system)
    	MoorDyn_Close(system);	
    
    if (!ret)
        return false;
    
    const UVector<double> realtensions = {19494.7925879369, 245711860.898667, 245711860.898667, 13932.7732363972, 245710732.326922, 245710732.326922, 11456.9888200325, 245710965.150316, 245710965.150316, 9985.13119847774, 245711226.618707, 245711226.618707, 8985.3491290671, 245712140.409143, 245712140.409143, 8251.34292949198, 245723345.452046, 245723345.452046, 7684.17685668972, 245770240.265832, 245770240.265832, 7229.69791090904, 245835588.210278, 245835588.210278, 6855.42009832787, 245866376.325674, 245866376.325674};
	VERIFY(CompareDecimals(realtensions, tensions, 3));
	
	return true;
}

CONSOLE_APP_MAIN
{
	Cout() << "MoorDyn v2 test running on ";
	
	const UVector<String>& command = CommandLine();
	int cid = 0;
#ifdef PLATFORM_WIN32
 #ifdef flagMOORDYN_DLL
	Cout() << "Windows and dynamically linked to a DLL ";
	#ifndef flagORIGINAL
		Cout() << "compiled in this project ";
	#else
		Cout() << "original from NREL ";
	#endif	
 #else
 	Cout() << "Windows and statically linked (no DLL required) ";
 #endif
 #ifdef _WIN64
	Cout() << " (64 bits)";
 #elif _WIN32
	Cout() << " (32 bits)";
 #else
 	Cout() << "Not supported";
 #endif
#elif PLATFORM_LINUX
	Cout() << "Linux and linking statically (no SO required) ";
#else
	Cout() << "Not supported";
#endif
	
	Cout() << "\nIf run standalone the command line options are ";
		
	try {	
#ifdef flagMOORDYN_DLL
			String pathDLL;
	#ifdef _WIN64
			#ifndef flagORIGINAL
				pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v2.dll");
			#else
				pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/original/moordyn.dll");
			#endif
	#else
			#ifndef flagORIGINAL
				pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v2_32.dll");
			#else
				throw Exc("Original 32 bits DLL is not available");
			#endif
	#endif		
			Cout() << "<path to DLL> ";
			if (!IsTheIDE() && command.size() > cid)
				pathDLL = command[cid++];
			MoorDyn_v2_Load(pathDLL);
#endif
	
		// Setting mooring folder
		if (!RealizeDirectory(AFX(GetExeFolder(), "mooring")))
			throw Exc(Format("Error creating 'mooring' folder: %s", GetLastErrorMessage()));
		
		String linesPath;
		if (!IsTheIDE()) {
			if (command.size() > cid)
				linesPath = command[cid++];
			else
				throw Exc("Path to lines.txt file not found");
		} else
			linesPath = AFX(GetSourceFolder(), "mooring/lines.txt");
	
		String newLinesPath = AFX(GetExeFolder(), "mooring/lines.txt");
		if (!FileCopy(linesPath, newLinesPath))
			throw Exc(Format("Error copying 'lines' file  from '%s' to '%s': %s", linesPath, newLinesPath, GetLastErrorMessage()));
		linesPath = newLinesPath;
		
		Cout() << "<path to mooring definition (previously called 'lines.txt')>\n";
		
		if (Demo(linesPath))
			Cout() << "\nAll test passed";
		else
			SetExitCode(-1);	
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
		SetExitCode(-1);
	}
	
#ifdef flagDEBUG
    Cout() << "\nPress enter to end";
	ReadStdIn();
#endif
}

