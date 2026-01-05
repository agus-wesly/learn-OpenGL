@echo off

cl /c /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glm-1.0.2" /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glfw-3.4.bin.WIN64\include" /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glad\include" /ZI /JMC /nologo /W3 /WX- /diagnostics:column /sdl /Od /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /std:c++20 /permissive- /Fo"LearnOpenGL\x64\Debug\\" /Fd"LearnOpenGL\x64\Debug\vc145.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt main.cpp

cl /c /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glm-1.0.2" /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glfw-3.4.bin.WIN64\include" /I"C:\Users\agusw\Documents\Visual Studio\Libraries\glad\include" /ZI /JMC /nologo /W3 /WX- /diagnostics:column /sdl /Od /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /std:c++20 /permissive- /Fo"LearnOpenGL\x64\Debug\\" /Fd"LearnOpenGL\x64\Debug\vc145.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt glad.cpp

IF ERRORLEVEL 1 (
    echo Compile failed
    exit /b 1
)

link /ERRORREPORT:PROMPT /OUT:"C:\Users\agusw\Desktop\Projects\LearnOpenGL\x64\Debug\LearnOpenGL.exe" /INCREMENTAL /ILK:"LearnOpenGL\x64\Debug\LearnOpenGL.ilk" /NOLOGO /LIBPATH:"C:\Users\agusw\Documents\Visual Studio\Libraries\glm-1.0.2" /LIBPATH:"C:\Users\agusw\Documents\Visual Studio\Libraries\glfw-3.4.bin.WIN64\lib-vc2015" opengl32.lib glfw3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG /PDB:"C:\Users\agusw\Desktop\Projects\LearnOpenGL\x64\Debug\LearnOpenGL.pdb" /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"C:\Users\agusw\Desktop\Projects\LearnOpenGL\x64\Debug\LearnOpenGL.lib" /MACHINE:X64 LearnOpenGL\x64\Debug\main.obj LearnOpenGL\x64\Debug\glad.obj

IF ERRORLEVEL 1 (
    echo Linking failed
    exit /b 1
)

REM "x64\Debug\LearnOpenGL.exe"
