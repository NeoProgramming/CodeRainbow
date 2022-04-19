set path=c:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.14.26428\bin\Hostx64\x64;c:\Qt\Qt5.10.1\5.10.1\msvc2017_64\bin;%PATH%
rem "c:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
qmake ../qtcreator.pro -recursive
nmake
