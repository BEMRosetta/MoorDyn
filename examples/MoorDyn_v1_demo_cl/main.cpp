#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Surface/Surface.h>
#include <ScatterDraw/DataSource.h>
#include <STEM4U/Iir_fir.h>

#include <MoorDyn_v1/MoorDyn.h>

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
	UVector<UVector<Value>> positionsV = ReadCSVFile(AFX(systemFolder, "mooring/positions.csv"), ',', false, false, '.', false, 1);
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
    if (!SaveFile(AFX(systemFolder, "mooring/positions_new.csv"), str))
		throw Exc("Problem saving translated positions");

	velocities.SetCount(num);
	for (int r = 0; r < num; ++r)
		velocities[r].SetCount(6);	
	
	int deg = 2;
	int wsize = 5;
	int der = 1;
	//VectorXd tm_ = Eigen::Map<VectorXd>(tm.begin(), num);
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
    if (!SaveFile(AFX(hiveFolder, "mooring/velocities.csv"), vstr))
		throw Exc("Problem saving translated positions");
}


CONSOLE_APP_MAIN
{
	try {
		bool useDLL = true;
		
		UVector<double> tm;
		UVector<UVector<double>> positions, velocities;
		
		//BasicTest(tm, positions, velocities);
		Calculation(tm, positions, velocities);
	
		
		Dll dll;
		dll.Load_throw(AFX(hiveFolder, "moordyn/MoorDyn_Win64.dll"));

		auto LinesInit_ = (int(*)(double X[], double XD[]))dll.GetFunction_throw("LinesInit");
		auto LinesCalc_ = (int(*)(double X[], double XD[], double Flines[], double* t_in, double* dt_in))dll.GetFunction_throw("LinesCalc");
		auto LinesClose_ = (int(*)())dll.GetFunction_throw("LinesClose");
		auto GetFairTen_ = (double(*)(int i))dll.GetFunction_throw("GetFairTen");
		
		if (!useDLL) {
			LinesInit_ = LinesInit;
			LinesCalc_ = LinesCalc;
			LinesClose_ = LinesClose;
			GetFairTen_ = GetFairTen;
		}
		
		LinesInit_(positions[0].begin(), velocities[0].begin());
	    
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
	    	LinesCalc_(positions[i].begin(), velocities[i].begin(), Flines_dummy, &t, &dt);
	    	forces[i][1] = f1.Filter(GetFairTen_(1)/1000);
	    	forces[i][2] = f2.Filter(GetFairTen_(2)/1000);
	    	forces[i][3] = f3.Filter(GetFairTen_(3)/1000);
	    }
	    
	    Cout() << "\n";
	    
	    LinesClose_();
	    
	    String str = "t,f1,f2,f3";
	    for (int r = 0; r < positions.size(); ++r) {
			str << "\n";
			for (int c = 0; c < 4; ++c) {
				if (c > 0)
					str << ",";
				str << forces[r][c];
	        }
	    }
	    if (!SaveFile(AFX(hiveFolder, "mooring/forces.csv"), str))
			throw Exc("Problem saving forces");
	    
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}



/*
CONSOLE_APP_MAIN
{
	try {
		Vector<double> tm;
		Vector<Vector<double>> positions, velocities;
		
		if (true) {
			Vector<Vector<Value>> positionsV = ReadCSVFile("mooring/positions.txt", '\t', false, false, '.', false, 1);
			tm.SetCount(positionsV.size());
			positions.SetCount(positionsV.size());
			for (int r = 0; r < positionsV.size(); ++r) {
				tm[r] = positionsV[r][0]; 
				positions[r].SetCount(6);
				for (int c = 0; c < 3; ++c) 
					positions[r][c] = positionsV[r][c+1]; 
				for (int c = 3; c < 6; ++c) 
					positions[r][c] = 0;//ToRad(double(positionsV[r][c+1])); 
				positions[r][0] -= 22.498;
				positions[r][2] -= 13.585;
			}
			
			Vector<Vector<Value>> velocitiesV = ReadCSVFile("mooring/velocities.txt", '\t', false, false, '.', false, 1);
			velocities.SetCount(velocitiesV.size());
			for (int r = 0; r < velocitiesV.size(); ++r) {
				velocities[r].SetCount(6);
				for (int c = 0; c < 3; ++c) 
					velocities[r][c] = 0;//velocitiesV[r][c+1]; 
				for (int c = 3; c < 6; ++c) 
					velocities[r][c] = 0;//ToRad(double(velocitiesV[r][c+1])); 
			}
		} else {
			double t = 10;
			double ddt = 0.05;
			tm.SetCount(t/ddt+1);
			positions.SetCount(t/ddt+1);
			velocities.SetCount(t/ddt+1);
			for (int r = 0; r < positions.size(); ++r) {
				tm[r] = r*ddt;
				positions[r].SetCount(6);		velocities[r].SetCount(6);
				for (int c = 0; c < 6; ++c) {
					positions[r][c] = 0; 
					velocities[r][c] = 0; 
				}
				positions[r][0] = 22.5;
				positions[r][2] = 13.5;
			}
		}
	
		Vector<Vector<double>> forces(positions.size());
		
		Dll dll;
		dll.Load_throw("C:\\Desarrollo\\Aplicaciones\\MoorDyn\\Originales\\MoorDyn v1\\MoorDyn_Win64.dll");

		auto LinesInit_DLL = (int(*)(double X[], double XD[]))dll.GetFunction_throw("LinesInit");
		auto LinesCalc_DLL = (int(*)(double X[], double XD[], double Flines[], double* t_in, double* dt_in))dll.GetFunction_throw("LinesCalc");
		auto LinesClose_DLL = (int(*)())dll.GetFunction_throw("LinesClose");
		auto GetFairTen_DLL = (double(*)(int i))dll.GetFunction_throw("GetFairTen");
		
		LinesInit(positions[0].begin(), velocities[0].begin());
	    
	    double dt = tm[1] - tm[0];
	    double Flines[6];
	    
	    Cout() << "\n";
	    
	    for (int i = 0; i < positions.size(); ++i) {
	        double t = tm[i];
	        printf("\rt: %f", t);
	        forces[i].SetCount(4);
	        forces[i][0] = t;
	    	LinesCalc(positions[i].begin(), velocities[i].begin(), Flines, &t, &dt);
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
	    if (!SaveFile("mooring/forces.csv", str))
			throw Exc("Problem saving forces");
	    
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
	}
	
    Cout() << "\nPress enter to end";
	ReadStdIn();
}
*/