@del .\.test\*.* /q
@del .\.test\DLL\*.* /q

@title Compiling MoorDyn DLL
umk Anboto MoorDyn_v1 CLANGX64   +DLL  -r	.test/DLL/MoorDyn_v1.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto MoorDyn_v1 CLANG      +DLL  -r	.test/DLL/MoorDyn_v1_32.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto MoorDyn_v1_5 CLANGX64 +DLL  -r	.test/DLL/MoorDyn_v1_5.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto MoorDyn_v1_5 CLANG    +DLL  -r	.test/DLL/MoorDyn_v1_5_32.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto MoorDyn_v2 CLANGX64   +DLL  -r	.test/DLL/MoorDyn_v2.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto MoorDyn_v2 CLANG      +DLL  -r	.test/DLL/MoorDyn_v2_32.dll  		
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"

@del .\*.exe /q

@title Compiling MoorDyn tests 64 bits
umk Anboto examples/MoorDyn_v1_demo_cl CLANGX64 +MOORDYN_DLL    -r	.test/MoorDyn_v1_demo_cl_DLL.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_demo_cl CLANGX64                 -r	.test/MoorDyn_v1_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_5_demo_cl CLANGX64 +MOORDYN_DLL  -r	.test/MoorDyn_v1_5_demo_cl_DLL.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_5_demo_cl CLANGX64               -r	.test/MoorDyn_v1_5_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v2_demo_cl CLANGX64 +MOORDYN_DLL    -r	.test/MoorDyn_v2_demo_cl_DLL.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v2_demo_cl CLANGX64                 -r	.test/MoorDyn_v2_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"

@title Compiling MoorDyn tests 32 bits
umk Anboto examples/MoorDyn_v1_demo_cl CLANG +MOORDYN_DLL    -r	.test/MoorDyn_v1_demo_cl_DLL_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_demo_cl CLANG                 -r	.test/MoorDyn_v1_demo_cl_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_5_demo_cl CLANG +MOORDYN_DLL  -r	.test/MoorDyn_v1_5_demo_cl_DLL_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v1_5_demo_cl CLANG               -r	.test/MoorDyn_v1_5_demo_cl_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v2_demo_cl CLANG +MOORDYN_DLL    -r	.test/MoorDyn_v2_demo_cl_DLL_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"
umk Anboto examples/MoorDyn_v2_demo_cl CLANG                 -r	.test/MoorDyn_v2_demo_cl_32.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error compiling MoorDyn"

@title Testing MoorDyn v2
.test\MoorDyn_v2_demo_cl_DLL    .test/DLL/original/moordyn.dll ../examples/MoorDyn_v2_demo_cl/mooring/lines.txt
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
.test\MoorDyn_v2_demo_cl_DLL    .test/DLL/MoorDyn_v2.dll       ../examples/MoorDyn_v2_demo_cl/mooring/lines.txt
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
.test\MoorDyn_v2_demo_cl_DLL_32 .test/DLL/MoorDyn_v2_32.dll    ../examples/MoorDyn_v2_demo_cl/mooring/lines.txt
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"

.test\MoorDyn_v2_demo_cl                                       ../examples/MoorDyn_v2_demo_cl/mooring/lines.txt
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
.test\MoorDyn_v2_demo_cl_32                                    ../examples/MoorDyn_v2_demo_cl/mooring/lines.txt
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"

cd .test

@title Testing MoorDyn v1 64 bits
MoorDyn_v1_demo_cl_DLL    DLL/original/MoorDyn_Win64.dll   ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_demo_cl_DLL    DLL/MoorDyn_v1.dll               ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_5_demo_cl_DLL  DLL/MoorDyn_v1_5.dll             ../../examples/MoorDyn_v1_5_demo_cl/mooring/lines.txt    ../../examples/MoorDyn_v1_5_demo_cl/mooring/positions.csv -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_demo_cl                                         ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_5_demo_cl                                       ../../examples/MoorDyn_v1_5_demo_cl/mooring/lines.txt    ../../examples/MoorDyn_v1_5_demo_cl/mooring/positions.csv -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"

@title Testing MoorDyn v1 32 bits
MoorDyn_v1_demo_cl_DLL_32    DLL/original/MoorDyn_Win32.dll ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_demo_cl_DLL_32    DLL/MoorDyn_v1_32.dll          ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_5_demo_cl_DLL_32  DLL/MoorDyn_v1_5_32.dll        ../../examples/MoorDyn_v1_5_demo_cl/mooring/lines.txt    ../../examples/MoorDyn_v1_5_demo_cl/mooring/positions.csv -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_demo_cl_32                                       ../../examples/MoorDyn_v1_demo_cl/mooring/lines.txt      ../../examples/MoorDyn_v1_demo_cl/mooring/positions.csv   -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"
MoorDyn_v1_5_demo_cl_32                                     ../../examples/MoorDyn_v1_5_demo_cl/mooring/lines.txt    ../../examples/MoorDyn_v1_5_demo_cl/mooring/positions.csv -16.98526242 0 -10.25623606
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing MoorDyn"


copy MoorDyn_v1_demo_cl.exe MoorDyn_v1_test.exe
copy MoorDyn_v1_5_demo_cl.exe MoorDyn_v2_test.exe