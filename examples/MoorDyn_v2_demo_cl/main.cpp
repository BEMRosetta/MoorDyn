#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>
#include <ScatterDraw/DataSource.h>

#ifdef flagMOORDYN_DLL
	#include <MoorDyn_v2_DLL/MoorDyn2_lib.h>
#else
	#include <MoorDyn_v2/MoorDyn2.h>
#endif

using namespace Upp;

// -Wall -Wextra -Wno-unused-parameter -Wno-logical-op-parentheses -Wno-deprecated-copy-with-user-provided-copy -Wno-overloaded-virtual -Wno-missing-braces

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

	MoorDyn system;
	try {
#ifdef flagMOORDYN_DLL
		MoorDyn_v2_Load(AFX(GetSourceFolder(), 
	#ifdef _WIN64
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2.dll"
	#else
			"../../MoorDyn_v2_DLL/bin/MoorDyn_v2_32.dll"
	#endif		
		));
#endif

		String path = AFX(GetSourceFolder(), "mooring/lines.txt");
		
	    int err;
	    system = MoorDyn_Create(path);
	    if (!system)
	        throw Exc("MoorDyn_Create");
	
	    // 3 coupled points x 3 components per point = 9 DoF
	    double x[9], xd[9];
	    memset(xd, 0, sizeof(double));
	    // Get the initial positions from the system itself
	    for (unsigned int i = 0; i < 3; i++) {
	        // 4 = first fairlead id
	        MoorDynPoint point = MoorDyn_GetPoint(system, i + 4);
	        err = MoorDyn_GetPointPos(point, x + 3 * i);
	        if (err != MOORDYN_SUCCESS)
	            throw Exc("MoorDyn_GetPointPos");
	    }
	
	    // Setup the initial condition
	    err = MoorDyn_Init(system, x, xd);
	    if (err != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_Init 2");
	
	    // Make the points move at 0.5 m/s to the positive x direction
	    for (unsigned int i = 0; i < 3; i++)
	        xd[3 * i] = 0.5;
	    double t = 0.0, dt = 0.5;
	    double f[9];
	    err = MoorDyn_Step(system, x, xd, f, &t, &dt);
	    if (err != MOORDYN_SUCCESS)
	    	throw Exc("MoorDyn_Step");
	
	    // Print the position and tension of the line nodes
	    unsigned int n_lines;
	    err = MoorDyn_GetNumberLines(system, &n_lines);
	    if (err != MOORDYN_SUCCESS)
	        throw Exc("MoorDyn_GetNumberLines");
	    
	    for (unsigned int i = 0; i < n_lines; i++) {
	        const unsigned int line_id = i + 1;
	        printf("Line %u\n", line_id);
	        printf("=======\n");
	        MoorDynLine line = MoorDyn_GetLine(system, line_id);
	        if (!line)
	        	throw Exc("MoorDyn_GetLine");
	        
	        unsigned int n_nodes;
	        err = MoorDyn_GetLineNumberNodes(line, &n_nodes);
	        if (err != MOORDYN_SUCCESS)
	        	throw Exc("MoorDyn_GetLineNumberNodes");
	        
	        for (unsigned int j = 0; j < n_nodes; j++) {
	            printf("  node %u:\n", j);
	            double pos[3], ten[3];
	            err = MoorDyn_GetLineNodePos(line, j, pos);
	            if (err != MOORDYN_SUCCESS)
	            	throw Exc("MoorDyn_GetLineNodePos");
	            
	            printf("  pos = [%g, %g, %g]\n", pos[0], pos[1], pos[2]);
	            err = MoorDyn_GetLineNodeTen(line, j, ten);
	            if (err != MOORDYN_SUCCESS)
	                throw Exc("MoorDyn_GetLineNodeTen");
	            
	            printf("  ten = [%g, %g, %g]\n", ten[0], ten[1], ten[2]);
	        }
	    }
	
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    // Alright, time to finish!
    if (system)
    	MoorDyn_Close(system);
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}

