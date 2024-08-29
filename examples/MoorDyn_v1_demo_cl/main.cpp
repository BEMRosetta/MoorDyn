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


void Calculation(UVector<double> &tm, UVector<UVector<double>> &positions, UVector<UVector<double>> &velocities, double dx, double dy, double dz) {
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
	const Point3D pos(dx , dy, dz), 
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
	Cout() << "Obtains with MoorDyn the loads on a platform, taking as input a record with its position.\n";
	Cout() << "This version runs in ";
	
	const UVector<String>& command = CommandLine();
	int cid = 0;
#ifdef PLATFORM_WIN32
 #ifdef flagMOORDYN_DLL
	Cout() << "Windows and is linked dynamically to a DLL ";
 #else
 	Cout() << "Windows and is compiled in this project ";
 #endif
 #ifdef _WIN64
	Cout() << " (64 bits)";
 #elif _WIN32
	Cout() << " (32 bits)";
 #else
 	Cout() << "Not supported";
 #endif
#elif PLATFORM_LINUX
	Cout() << "Linux and is compiled in this project ";
#else
	Cout() << "Not supported";
#endif

	int version;
	if (GetExeTitle().Find("MoorDyn_v1_demo_cl") >= 0)
		version = 1;
	else 
		version = 2;
	
	Cout() << "\nThe MoorDyn version used is " << version << ". Mooring definition file ('lines.txt') used has to be of this version";	
	Cout() << "\nIf run standalone the command line options are ";

	try {
#ifdef flagMOORDYN_DLL
		String pathDLL;
		if (version == 1) 
	#ifdef _WIN64
			#ifndef flagORIGINAL
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v1.dll");
			#else
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/original/MoorDyn_Win64.dll");
			#endif
	#else
			#ifndef flagORIGINAL
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v1_32.dll");
			#else
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/original/MoorDyn_Win32.dll");
			#endif
	#endif		
		else
	#ifdef _WIN64
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v1_5.dll");
	#else
			pathDLL = AFX(GetSourceFolder(), "../../unittest/.test/DLL/MoorDyn_v1_5_32.dll");
	#endif		
		Cout() << "<path to DLL> ";
		if (!IsTheIDE() && command.size() > cid)
			pathDLL = command[cid++];
		MoorDyn_v1_Load(pathDLL);
#endif
		
		// Setting mooring folder
		if (!RealizeDirectory(AFX(GetExeFolder(), "mooring")))
			throw Exc(Format("Error creating 'mooring' folder: %s", GetLastErrorMessage()));
		
		// Setting 'lines.txt'
		Cout() << "<path to 'lines.txt'> ";
		String linesPath = AFX(GetSourceFolder(), "mooring/lines.txt");
		if (!IsTheIDE()) {
			if (command.size() > cid)
				linesPath = command[cid++];
			else
				throw Exc("Path to lines.txt file not found");
		}
		RealizeDirectory(AFX(GetExeFolder(), "mooring"));	
		if (!FileCopy(linesPath, AFX(GetExeFolder(), "mooring/lines.txt")))
			throw Exc(Format("Error copying 'lines' file: %s", GetLastErrorMessage()));
					
		// Setting 'positions.csv'			
		Cout() << "<path to 'positions.csv'> ";
		String positionsPath = AFX(GetSourceFolder(), "mooring/positions.csv");
		if (!IsTheIDE()) {
			if (command.size() > cid)
				positionsPath = command[cid++];
			else
				throw Exc("Path to positions.csv file not found");
		}
		if (!FileCopy(positionsPath, AFX(GetExeFolder(), "mooring/positions.csv")))
			throw Exc(Format("Error copying 'positions' file: %s", GetLastErrorMessage()));
	
		// Translation to the positions.csv file
		Cout() << "<translation x> <translation y> <translation z> ";				
		double dx = -16.98526242, dy = 0, dz = -10.25623606;
		if (command.size() > cid)
			dx = ScanDouble(command[cid++]);
		if (command.size() > cid)
			dy = ScanDouble(command[cid++]);
		if (command.size() > cid)
			dz = ScanDouble(command[cid++]);

		if (!IsNum(dx) || !IsNum(dy) || !IsNum(dz))
			throw Exc("Wrong position values");

		UVector<double> tm;
		UVector<UVector<double>> positions, velocities;
	
		Calculation(tm, positions, velocities, dx, dy, dz);
		
		if (LinesInit(positions[0].begin(), velocities[0].begin()) != 0)
			throw Exc("Problem in LinesInit");
	    
	    double dt = tm[1] - tm[0];
	    
	    Cout() << "\n";
	    
	    UVector<UVector<double>> forces(4), fplat(positions.size());
	    for (int i = 0; i < 4; ++i)
	        forces[i].SetCount(positions.size());
	        
	    for (int i = 0; i < positions.size(); ++i) {
	        double t = tm[i];
			printf("\rt %.2f", t);
	        forces[0][i] = t;
	        fplat[i].SetCount(9);
	    	if (LinesCalc(positions[i].begin(), velocities[i].begin(), fplat[i].begin(), &t, &dt) != 0)
	    		throw Exc("Problem in LinesCalc");
	    	
	    	forces[1][i] = GetFairTen(1)/1000;
	    	forces[2][i] = GetFairTen(2)/1000;
	    	forces[3][i] = GetFairTen(3)/1000;
	    }
	    
	    linesPath.Replace("\\", "/");
	    if (linesPath.Find("examples/MoorDyn_v1_demo_cl/mooring/lines.txt") >= 0) {
	 		const UVector<double> realtensions = {919.2773916,918.3410074,914.6439312,910.49993,906.5263447,902.6786976,898.9739238,895.4187467,892.1229906,889.0582685,886.1898098,883.5368384,880.9460829,878.2037163,875.0652213,871.0810918,865.9802976,859.9023193,853.2595507,846.6185087,840.439466,834.9796913,830.4224428,826.8347729,824.1425369,822.3378422,821.3506026,821.0154281,821.1643912,821.4478073,821.5058856,821.1804783,820.4408808,819.4868027,818.642271,818.1586415,818.2997478,819.2470609,820.9847908,823.525383,826.8062558,830.6742499,834.9977706,839.5482994,844.0513257,848.3592028,852.3918041,856.1633021,859.8267898,863.5212843,867.3395664,871.4183343,875.7555526,880.3352825,885.191183,890.2624774,895.5087444,900.8843801,906.265727,911.5923029,916.8654219,922.0426902,927.1801814,932.3282174,937.4445954,942.532276,947.5388914,952.39964,957.1374222,961.7755988,966.3508358,970.9414401,975.5760133,980.278929,985.077103,989.9566366,994.8919689,999.8426624,1004.715403,1009.414385,1013.852376,1017.949552,1021.726281,1025.255974,1028.624654,1031.979738,1035.439407,1039.069226,1042.924257,1046.986379,1051.198306,1055.513091,1059.826231,1064.047597,1068.135435,1072.047804,1075.826943,1079.587689,1083.429416,1087.475298,1091.83342,1096.527099,1101.571037,1106.932329,1112.520649,1118.291069,1124.194613,1130.16023,1136.190765,1142.295624,1148.494238,1154.85814,1161.431431,1168.223695,1175.262348,1182.497702,1189.861371,1197.298399,1204.733361,1212.133623,1219.500816,1226.822664,1234.093746,1241.321096,1248.487487,1255.591567,1262.605524,1269.481672,1276.183787,1282.659228,1288.857679,1294.760932,1300.365693,1305.70661,1310.848662,1315.847342,1320.738058,1325.55404,1330.265525,1334.842305,1339.213321,1343.290462,1347.009617,1350.327371,1353.199794,1355.621425,1357.632984,1359.269566,1360.595932,1361.666822,1362.491431,1363.079504,1363.401035,1363.388684,1363.003818,1362.173068,1360.851308,1359.038964,1356.722336,1353.913984,1350.660636,1346.985492,1342.92295,1338.488648,1333.650984,1328.365116,1322.587705,1316.275449,1309.39547,1301.957912,1293.972424,1285.483499,1276.532615,1267.156129,1257.400399,1247.282321,1236.81477,1226.003527,1214.827188,1203.268595,1191.311319,1178.947019,1166.182433,1153.038402,1139.551523,1125.763002,1111.722477,1097.461685,1083.008083,1068.377936,1053.58793,1038.649371,1023.580373,1008.392646,993.1214703,977.8023978,962.4706491,947.1788819,931.9667832,916.8799403,901.968247,887.2432041,872.7357712,858.4741338,844.4891293,830.8010311,817.4180774,804.1331273,790.6456682,776.95555,763.4235362,750.4874311,738.4341801,727.3177624,717.0247638,707.3696714,698.2144721,689.5027345,681.2034292,673.3074165,665.8008716,658.6429816,651.775013,645.1259775,638.6574448,632.358236,626.2732611,620.4716741,615.0095744,609.9206089,605.1945405,600.8085515,596.723513,592.8962255,589.1267474,584.6399986,578.7172496,571.895144,565.0943744,558.7595824,553.2119697,548.8087476,545.626605,543.4362625,541.9471105,540.9715828,540.4181409,540.2072634,540.2493807,540.3921847,540.4716912,540.3706332,540.0685285,539.6700461,539.4028269,539.5198564,540.2574451,541.780498,544.1505118,547.3206778,551.2042918,555.6546513,560.5311883,565.6938794,571.025426,576.428095,581.8487136,587.2776534,592.7399312,598.1305955,602.6884813,605.9450704,609.0835245,613.3249157,618.5898094,624.5621457,631.4794293,639.686855,649.1011398,659.3337005,669.9832228,680.789511,691.5943091,702.2946942,712.78579,722.9774413,732.8638952,742.5112495,752.1054737,761.8510982,771.9625046,782.6094736,793.8960564,805.8494317,818.4203023,831.4804881,844.8462434,858.3335977,871.7585942,884.980285,897.9067133,910.4948915,922.7638497,934.777481,946.6083719,958.3513985,970.100968,981.9146018,993.8156583,1005.76088,1017.67055,1029.45912,1041.017303,1052.256926,1063.104745,1073.541058,1083.568989,1093.235946,1102.591403,1111.686705,1120.564651,1129.252653,1137.761903,1146.087545,1154.203271,1162.088352,1169.708829,1177.028074,1184.029668,1190.683991,1196.977038,1202.922578,1208.528231,1213.787676,1218.711655,1223.294229,1227.562109,1231.527717,1235.214922,1238.66209,1241.88083,1244.883278,1247.65575,1250.174224,1252.416407,1254.353176,1255.961501,1257.226456,1258.145773,1258.741215,1259.040303,1259.093135,1258.950826,1258.660176,1258.272199,1257.808233,1257.261179,1256.610947,1255.808417,1254.802157,1253.533111,1251.964397,1250.060132,1247.826076,1245.271984,1242.430361,1239.343086,1236.051404,1232.581652,1228.952777,1225.161543,1221.192807,1217.015638,1212.621347,1207.923623,1202.962598,1197.650303,1192.10095,1186.284199,1180.178815,1173.825385,1167.24158,1160.432805,1153.405683,1146.160292,1138.68965,1131.003082,1123.107021,1115.006567,1106.718472,1098.257755,1089.641835,1080.88089,1071.976704,1062.924294,1053.722856,1044.359926,1034.846538,1025.187295,1015.408868,1005.540761,995.6324914,985.7188252,975.8493737,966.050725,956.3462238,946.7424325,937.2465997,927.8514855,918.5615623,909.3835904,900.3355437,891.4495971,882.7649376,874.3305728,866.20339,858.4172861,851.0185168,844.0244811,837.4428994,831.2653327,825.4916124,820.1068867,815.114109,810.514628,806.324856,802.549993,799.2045063,796.296828,793.8269266,791.7946434,790.1798637,788.9698746,788.1326852,787.6416285,787.473759,787.6073419,788.0302229,788.7315675,789.6863423,790.7068466,791.3210292,791.3282496,791.1512334,791.2042175,791.6272894,792.5881116,794.1965687,796.3875378,798.9810533,801.8068333,804.7777239,807.8450763,810.9874205,814.1473951,817.2529069,820.2198751,823.0027052,825.6149612,828.1386332,830.7041185,833.4575888,836.5228039,839.9617693,843.7955576,847.9953094,852.4971747,857.2177573,862.0790227,867.004361,871.9311159,876.8098772,881.6260412,886.3841429,891.1194309,895.8980136,900.7919713,905.8644362,911.1712927,916.7308529,922.5365647,928.5539992,934.7362803,941.0164079,947.3442157,953.6771661,959.9785949,966.2338335,972.4422704,978.5993527,984.7139022,990.7965393,996.8531823,1002.900643,1008.944456,1014.993359,1021.038622,1027.082602,1033.111328,1039.106472,1045.038642,1050.88858,1056.609315,1062.18017,1067.558796,1072.721681,1077.651406,1082.353146,1086.832655,1091.12075,1095.251326,1099.244456,1103.128631,1106.911809,1110.576824,1114.094953,1117.399518,1120.438308,1123.160054,1125.525131,1127.525113,1129.177604,1130.522534,1131.627238,1132.546951,1133.346275,1134.070819,1134.758851,1135.414353,1136.043338,1136.628548,1137.147255,1137.574105,1137.896476,1138.095327,1138.169339,1138.124515,1137.963096,1137.700249,1137.328413,1136.858556,1136.279415,1135.589128,1134.779638,1133.848903,1132.800453,1131.633863,1130.346502,1128.940144,1127.400405,1125.711888,1123.854196,1121.797736,1119.515676,1116.990053,1114.20335,1111.157676,1107.867568,1104.352665,1100.643028,1096.762873,1092.727638,1088.549923,1084.22093,1079.72162,1075.035717,1070.128974,1064.992072,1059.609965,1053.983894,1048.133966,1042.087597,1035.876488,1029.529927,1023.086106,1016.553323,1009.929649,1003.222731,996.4241061,989.5257866,982.5233782,975.4305269,968.2578527,961.0333309,953.7849715,946.5461079,939.3477927,932.2070068,925.1463328,918.1660789,911.2738934,904.470589,897.7680773,891.1825706,884.7351203,878.4635089,872.3910982,866.532742,860.9180371,855.5535796,850.4486288,845.5824051,840.9437156,836.5138288,832.2751358,828.2167129,824.3424007,820.6601355,817.182903,813.9176301,810.867495,808.0180823,805.3585126,802.8439048,800.4474335,798.1265892,795.8467373,793.5908162,791.3455695,789.1110678,786.8974248,784.7133014,782.5185409,780.0173304,776.8577898,773.2246271,769.5297386,766.002551,762.7613052,759.9518903,757.6159274,755.6346586,753.8672027,752.2209158,750.6585145,749.1629768,747.7172499,746.2734811,744.7822043,743.1953758,741.5110129,739.7887398,738.1397598,736.6968038,735.5814652,734.8842416,734.6402734,734.8410202,735.4382155,736.3725327,737.5701482,738.9636141,740.5106394,742.1703573,743.9216163,745.7733007,747.7498496,749.9053869,752.2990734,754.9867532,758.0162439,761.398658,765.1383111,769.2066645,773.5574288,778.1508153,782.9391977,787.8883015,792.9760514,798.1850466,803.508882,808.9483863,814.4965643,820.169919,825.9609844,831.8742288,837.92368,844.0965239,850.4048852,856.8426651,863.4011772,870.0743315,876.8441136,883.6829362,890.5554891,897.4321581,904.2801474,911.0689523,917.7768817,924.4013539,930.9517475,937.4381147,943.894793,950.3379812,956.7775591,963.2285808,969.6653561,976.072489,982.4053836,988.6234922,994.6919273,1000.582571,1006.273456,1011.767175,1017.073893,1022.214059,1027.216141,1032.091793,1036.855365,1041.501255,1046.013444,1050.367041,1054.531696,1058.486378,1062.213953,1065.701982,1068.949328,1071.961692,1074.755691,1077.335406,1079.702628,1081.86217,1083.808331,1085.533155,1087.02784,1088.292963,1089.329363,1090.154309,1090.786353,1091.244091,1091.558239,1091.735358,1091.786022,1091.70762,1091.490684,1091.11419,1090.576784,1089.859599,1088.964858,1087.889039,1086.637534,1085.212138,1083.623702,1081.868424,1079.963243,1077.889962,1075.64305,1073.21237,1070.576258,1067.729866,1064.672425,1061.41528,1057.971942,1054.371376,1050.628309,1046.769365,1042.788551,1038.685539,1034.44887,1030.055253,1025.483475,1020.720429,1015.761866,1010.616738,1005.298192,999.8385293,994.2642233,988.6050649,982.8796716,977.0960588,971.2536626,965.3455616,959.346798,953.2554577,947.0591832,940.7704962,934.3982334,927.9732861,921.5254974,915.0830373,908.676998,902.3238749,896.0358419,889.8090072,883.6383061,877.5188534,871.4517749,865.4365542,859.5007564,853.6612601,847.9499563,842.3865254,837.0056224,831.8144977,826.7990947,821.803479,816.5310722,810.9388499,805.2989984,799.79988,794.5376839,789.6416568,785.1875504,781.1454215,777.4252085,773.9394681,770.6258207,767.4357926,764.3222994,761.2211056,758.0619886,754.7842029,751.3523784,747.7899358,744.1532834,740.5268636,737.0023412,733.6339575,730.4559833,727.4575957,724.6067547,721.8493626,719.1405337,716.4500673,713.7495607,711.0312635,708.2954866,705.5554837,702.8402981,700.1802069,697.613908,695.1770299,692.9111626,690.8306617,688.9593794,687.3055317,685.86752,684.6509046,683.6558806,682.8808298,682.3219953,681.9784762,681.8356102,681.8948453,682.1339442,682.567066,683.1408016,683.5379978,683.2265227,682.4853307,682.1150607,682.3258192,682.921142,683.9667984,685.7132375,688.2498948,691.4274309,695.0274695,698.874489,702.8712631,706.9519404,711.0654632,715.1653765,719.2152827,723.2420685,727.3046657,731.5051535,735.9349378,740.6807571,745.8006093,751.3065424,757.2054745,763.4527069,769.9866714,776.7481313,783.6686682,790.6955848,797.7720891,804.8602797,811.9304537,818.974997,826.0030322,833.0464073,840.1491912,847.3637729,854.7400105,862.3098436,870.0874602,878.0521233,886.1812257,894.4087147,902.6605625,910.8731846,918.9716329,926.9031294,934.6482263,942.1957641,949.5721475,956.8176627,963.9853827,971.1156718,978.2405972,985.3730402,992.49402,999.5694656,1006.533201,1013.30266,1019.794437,1025.953382,1031.738036,1037.128802,1042.130512,1046.755946,1051.047036,1055.036352,1058.766982,1062.272532,1065.590627,1068.740407,1071.744,1074.614453,1077.355697,1079.981812,1082.494542,1084.900609,1087.182958,1089.336864,1091.34127,1093.191196,1094.878124,1096.408664,1097.804865,1099.094365,1100.321879,1101.519033,1102.72011,1103.949047,1105.20218,1106.463959,1107.686551,1108.828983,1109.825953,1110.632041,1111.204574,1111.535845,1111.617896,1111.478137,1111.149842,1110.664193,1110.038871,1109.295841,1108.416832,1107.382748,1106.159094,1104.700329,1102.971642,1100.946782,1098.61842,1095.987424,1093.070999,1089.892597,1086.474577,1082.82875,1078.966351,1074.863994,1070.524013,1065.902693,1060.984039,1055.755666,1050.209587,1044.363824,1038.23791,1031.868033,1025.283837,1018.507138,1011.55099,1004.411555,997.0897536,989.5713354,981.8454541,973.9176747,965.7989778,957.5242202,949.117484,940.628408,932.0867025,923.5280186,914.9719814,906.4221523,897.8779729,889.3377965,880.7955824,872.2527216,863.7275812,855.237024,846.7131594,838.0054721,829.1448609,820.3487051,812.6577388,807.3421958};
			VERIFY(CompareDecimals(realtensions, forces[1], 1));
			Cout() << "\nAll tests passed";
	    } else if (linesPath.Find("examples/MoorDyn_v1_5_demo_cl/mooring/lines.txt") >= 0) {
			const UVector<double> realtensions = {896.1668172,885.0926045,882.9570764,880.4858031,877.8040237,874.8889575,871.7924302,868.5506278,865.2953579,862.1435291,859.5514787,858.5060552,859.7125992,862.4778894,864.3881421,862.5708534,855.9530393,845.8994045,834.8768696,824.9395404,816.9907963,810.9894562,806.6091457,803.5452933,801.5967433,800.8415209,801.3726825,803.0947806,805.6788715,808.34975,810.1997478,810.6202194,809.4334188,807.0423774,804.2152509,801.7039812,800.1574308,799.9590592,801.1433792,803.6711845,807.4081465,812.1231514,817.6027003,823.4943737,829.3427509,834.792616,839.5472757,843.473901,846.69505,849.4235334,851.9415185,854.6095605,857.6461769,861.2073124,865.4323039,870.2940565,875.7255006,881.6209139,887.7490373,893.9238896,900.0080858,905.8294107,911.3309428,916.4950267,921.2712756,925.713283,929.8696832,933.8019535,937.6708585,941.6189479,945.7588523,950.2143894,955.007216,960.1087945,965.4845491,971.0169185,976.5818081,982.0537401,987.2634315,992.073392,996.3938521,1000.170545,1003.482726,1006.484053,1009.343468,1012.283319,1015.468887,1018.989846,1022.909561,1027.183518,1031.710781,1036.389913,1041.050485,1045.547258,1049.798706,1053.735176,1057.401626,1060.931001,1064.454726,1068.146891,1072.164173,1076.571498,1081.417646,1086.686011,1092.28776,1098.162618,1104.228864,1110.381972,1116.58212,1122.805298,1129.042459,1135.351258,1141.772183,1148.329683,1155.06961,1161.973507,1168.995613,1176.113413,1183.271912,1190.446416,1197.64425,1204.841986,1212.021626,1219.170278,1226.243349,1233.224822,1240.070186,1246.723848,1253.145476,1259.290684,1265.118733,1270.631902,1275.84397,1280.80649,1285.603226,1290.293139,1294.922304,1299.51445,1304.038729,1308.444413,1312.650529,1316.554852,1320.080221,1323.172841,1325.784437,1327.923315,1329.627666,1330.945369,1331.964491,1332.744437,1333.309533,1333.682299,1333.828863,1333.685731,1333.205756,1332.307666,1330.936127,1329.075203,1326.707277,1323.841122,1320.52162,1316.775475,1312.648887,1308.171858,1303.324994,1298.084232,1292.407466,1286.244777,1279.565785,1272.365863,1264.645939,1256.443496,1247.793224,1238.724074,1229.275861,1219.469531,1209.320047,1198.832227,1187.995872,1176.795441,1165.226824,1153.280876,1140.961095,1128.290566,1115.295648,1102.017913,1088.497813,1074.757165,1060.827032,1046.714155,1032.424446,1017.96933,1003.356865,988.5978321,973.7273227,958.7818146,943.8008534,928.8392343,913.9340247,899.1315627,884.4829865,870.003276,855.7227907,841.6787133,827.8954264,814.3925915,801.0574266,787.5891051,773.8882615,760.2419318,747.0670167,734.7012524,723.2760339,712.71974,702.8916272,693.6473188,684.8946016,676.5926668,668.7072927,661.2096609,654.0546146,647.1805917,640.520896,634.0302211,627.6963918,621.5491325,615.6571045,610.0811355,604.8631054,600.0103947,595.4959357,591.2886658,587.3545108,583.6521068,579.9726216,575.5229686,569.6183167,562.8196287,556.038048,549.7059251,544.1669635,539.777673,536.6133044,534.4467396,532.9872294,532.0482918,531.5284819,531.3448773,531.3899206,531.5036158,531.5090187,531.2836348,530.8113031,530.2079597,529.7118049,529.6017135,530.1292923,531.4746898,533.7040591,536.7772676,540.5956738,545.0059817,549.8492132,554.9732071,560.2399729,565.5403413,570.8118961,576.0376535,581.2611976,586.4449092,590.9544254,594.1424933,596.9799264,600.8649483,605.9050175,611.7252745,618.4840068,626.540551,635.8748931,646.0979997,656.7769335,667.6000725,678.3867718,689.0166426,699.3775986,709.3845874,719.0188891,728.3664558,737.6196862,747.0267307,756.8309628,767.2317884,778.3549104,790.2352702,802.8161313,815.9465336,829.4187344,843.009843,856.5090951,869.747161,882.6205199,895.090979,907.1859544,918.9823535,930.5729407,942.0840496,953.6357896,965.3039209,977.1131646,989.0323161,1000.986889,1012.871664,1024.565866,1035.948564,1046.927462,1057.448537,1067.503059,1077.127598,1086.379994,1095.324085,1104.023529,1112.518643,1120.830976,1128.965414,1136.911008,1144.643379,1152.132174,1159.335493,1166.211344,1172.724432,1178.845872,1184.562895,1189.874633,1194.786072,1199.317686,1203.491325,1207.336486,1210.890801,1214.194362,1217.290622,1220.20799,1222.951275,1225.499742,1227.823713,1229.883003,1231.637295,1233.049613,1234.094078,1234.768747,1235.091709,1235.100034,1234.851466,1234.413153,1233.846471,1233.207191,1232.529006,1231.808768,1231.022369,1230.119787,1229.034815,1227.70772,1226.078835,1224.117009,1221.811101,1219.176876,1216.246729,1213.070272,1209.693513,1206.156095,1202.484394,1198.68247,1194.739665,1190.628268,1186.334732,1181.769075,1176.965038,1171.821103,1166.448296,1160.810229,1154.876397,1148.69542,1142.276804,1135.638017,1128.78844,1121.731559,1114.47321,1107.020617,1099.384692,1091.572255,1083.594512,1075.462325,1067.187392,1058.774068,1050.21659,1041.510148,1032.642869,1023.610478,1014.415213,1005.069215,995.5997954,986.0443126,976.4464864,966.851766,957.3023136,947.8245566,938.4367813,929.1417129,919.9374658,910.8159815,901.77491,892.8138743,883.9520533,875.2169682,866.6519527,858.3081444,850.242265,842.5049136,835.1336367,828.1611464,821.5965697,815.4436681,809.697004,804.3472763,799.3930319,794.8373743,790.6896225,786.9623497,783.6670896,780.8119241,778.3976858,776.4177851,774.8504118,773.6757087,772.8605676,772.3787996,772.204468,772.3198473,772.7164375,773.3816711,774.3053379,775.3593924,776.1106492,776.2413945,776.0793609,776.0788079,776.4219642,777.2618022,778.7345236,780.8189243,783.3430082,786.1330268,789.0808874,792.1353149,795.2647966,798.4188558,801.5247106,804.4932776,807.270339,809.8546318,812.3197016,814.7972204,817.4404645,820.3830559,823.7115349,827.4543017,831.5871137,836.048928,840.7519936,845.6101229,850.5365605,855.4644758,860.3385657,865.1329115,869.8511562,874.5275643,879.2228709,884.0202976,888.992019,894.1997795,899.6762417,905.4207999,911.4034685,917.5707683,923.8598071,930.2061646,936.5599466,942.8815559,949.1461502,955.344172,961.474343,967.5453685,973.5702413,979.5697998,985.5629964,991.5677647,997.5941134,1003.642301,1009.710052,1015.78219,1021.835568,1027.84123,1033.763227,1039.559321,1045.190415,1050.613822,1055.802603,1060.739528,1065.427222,1069.880044,1074.128842,1078.211526,1082.159094,1085.995838,1089.725135,1093.333397,1096.784082,1100.014109,1102.965505,1105.584854,1107.83607,1109.711346,1111.230707,1112.443546,1113.409294,1114.196924,1114.866338,1115.470032,1116.041529,1116.594674,1117.123548,1117.610993,1118.032151,1118.361689,1118.574923,1118.657383,1118.608174,1118.429377,1118.133039,1117.726484,1117.215421,1116.602254,1115.885328,1115.061346,1114.125835,1113.077963,1111.918828,1110.649299,1109.266521,1107.765393,1106.135905,1104.359649,1102.412793,1100.269005,1097.900289,1095.290713,1092.426013,1089.311065,1085.960425,1082.400459,1078.660397,1074.764564,1070.734872,1066.577479,1062.281642,1057.831444,1053.202468,1048.367468,1043.30465,1038.002674,1032.465766,1026.71162,1020.76709,1014.666845,1008.446562,1002.13555,995.7501921,989.2957188,982.7694041,976.1641519,969.4693923,962.6811531,955.8035283,948.8470818,941.8349179,934.7937501,927.7532164,920.7407189,913.783385,906.8940189,900.0829075,893.3541137,886.7103683,880.1638212,873.7289391,867.4304547,861.2995677,855.3642709,849.651781,844.1845883,838.9680427,834.0032091,829.2804368,824.7841237,820.5009682,816.4128828,812.5121353,808.7954477,805.2686041,801.9378581,798.807425,795.877334,793.1327743,790.554347,788.110357,785.7644476,783.4815135,781.2297989,778.9928273,776.7610042,774.5377326,772.3312512,770.1571927,767.9601286,765.4305956,762.24667,758.6169911,754.9506405,751.4429392,748.2215312,745.4287638,743.0877927,741.0835579,739.2815834,737.5938744,735.9877998,734.4490856,732.9538935,731.4597583,729.9065555,728.2496648,726.4923617,724.6925424,722.9662528,721.4479019,720.2623583,719.4976124,719.1891968,719.3266373,719.8647327,720.7340544,721.8669251,723.1940553,724.6633706,726.2348893,727.8889979,729.6295514,731.4859818,733.5147889,735.7797189,738.3438915,741.2548981,744.5367862,748.1845492,752.1708553,756.4535383,760.9810881,765.7061607,770.5905735,775.6048057,780.7329217,785.9662438,791.3030992,796.7492117,802.3141204,808.0028889,813.8241895,819.7867376,825.8935655,832.1481528,838.5462394,845.0809706,851.7377319,858.4938112,865.3192138,872.1780149,879.0338242,885.8521148,892.605524,899.2750483,905.8593535,912.3674978,918.8207586,925.2493512,931.6770685,938.1215395,944.5854982,951.0529054,957.4956732,963.8709455,970.1332052,976.2391103,982.1562024,987.8683436,993.3743157,998.686222,1003.828234,1008.822933,1013.6918,1018.446961,1023.086573,1027.5955,1031.94521,1036.110454,1040.059484,1043.775081,1047.241926,1050.458719,1053.431136,1056.163185,1058.666065,1060.945192,1063.004777,1064.842046,1066.455121,1067.84217,1069.006122,1069.954162,1070.701644,1071.265982,1071.671859,1071.937625,1072.074633,1072.089189,1071.975667,1071.721699,1071.307366,1070.720033,1069.953021,1069.003712,1067.874353,1066.56962,1065.096842,1063.466292,1061.681167,1059.747789,1057.658988,1055.403852,1052.969041,1050.3344,1047.491309,1044.436867,1041.181724,1037.740854,1034.141303,1030.40411,1026.550157,1022.584284,1018.504112,1014.296294,1009.940057,1005.415175,1000.706116,995.8070049,990.7240421,985.4730496,980.0817796,974.5784208,968.9899035,963.3360413,957.6292066,951.8681647,946.0411112,940.1323868,934.1319272,928.0331517,921.841305,915.5703921,909.2464429,902.897549,896.5554689,890.2439127,883.9781257,877.7673874,871.6096565,865.4989355,859.4296897,853.4015852,847.4198357,841.5032073,835.6793606,829.9770754,824.4267361,819.0545067,813.8735778,808.8618081,803.8399586,798.523175,792.9249675,787.3198501,781.8789802,776.7033345,771.9174762,767.5842065,763.6553081,760.0357953,756.6380893,753.4032674,750.2844348,747.2257135,744.1645871,741.0264608,737.7542041,734.3198799,730.7487446,727.1076704,723.4858718,719.9699816,716.6213875,713.4667061,710.4943938,707.6641288,704.9238634,702.2230942,699.525379,696.8047712,694.051559,691.2661506,688.4656899,685.6791245,682.9427291,680.2994168,677.7924637,675.4576347,673.3203969,671.3973574,669.6965677,668.2138035,666.9482977,665.8988178,665.0612603,664.4310957,664.0045487,663.7713985,663.7278089,663.8657541,664.184605,664.5743581,664.5352879,663.7147045,662.7371423,662.3085293,662.4111391,662.8939994,663.9448577,665.8012394,668.457693,671.7099664,675.3317905,679.1671543,683.1190933,687.1186414,691.1042567,695.0251926,698.8658351,702.6718495,706.5377781,710.5832714,714.9185162,719.630571,724.7724233,730.355975,736.3583331,742.7204963,749.3653583,756.2140019,763.1897082,770.2286965,777.2796187,784.3042036,791.2857178,798.2312914,805.1670789,812.1409156,819.2092038,826.4314991,833.8595481,841.5196844,849.4166848,857.5247126,865.7956902,874.1541354,882.5198892,890.8092411,898.9581907,906.9173372,914.6698831,922.2254242,929.6172564,936.8978394,944.1226172,951.3385496,958.5770805,965.8468898,973.1269062,980.3694778,987.5035855,994.4354561,1001.069207,1007.331681,1013.188178,1018.623819,1023.642013,1028.263788,1032.526776,1036.476985,1040.159463,1043.615586,1046.879075,1049.974229,1052.924413,1055.741366,1058.436522,1061.013128,1063.476099,1065.82405,1068.048386,1070.137489,1072.077618,1073.857252,1075.476209,1076.939682,1078.268427,1079.495108,1080.662437,1081.806459,1082.962631,1084.148969,1085.368215,1086.596055,1087.791225,1088.902289,1089.866342,1090.634511,1091.166268,1091.445492,1091.474785,1091.278683,1090.891928,1090.349473,1089.677833,1088.891487,1087.982962,1086.927991,1085.692387,1084.229655,1082.505439,1080.487611,1078.164468,1075.538433,1072.624227,1069.447458,1066.030489,1062.389561,1058.532836,1054.451358,1050.131269,1045.542068,1040.65876,1035.468249,1029.968783,1024.174842,1018.108637,1011.80095,1005.287661,998.5864965,991.7118879,984.6633213,977.4356462,970.0176714,962.4024452,954.5876758,946.5892512,938.4323867,930.1497623,921.7820868,913.3639957,904.9294985,896.4956682,888.0693799,879.6477575,871.2210053,862.7791336,854.3243301,845.8721974,837.4450663,829.0100588,820.4068045,811.5947336,802.7743526,795.0193536,789.5915103};
			VERIFY(CompareDecimals(realtensions, forces[1], 3));
			Cout() << "\nAll tests passed";
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
				str << forces[c][r];
	        }
	        for (int c = 0; c < 6; ++c) 
	            str << "," << fplat[r][c]/1000;
	    }
	    if (!SaveFile(AFX(GetExeFolder(), "mooring/forces.csv"), str))
			throw Exc("Problem saving forces");
	    
	} catch (Exc e) {
		Cout() << "\nError: " << e;	
		SetExitCode(-1);
	}
	
#ifdef flagDEBUG	
    Cout() << "\nPress enter to end";
	ReadStdIn();
#endif
}

