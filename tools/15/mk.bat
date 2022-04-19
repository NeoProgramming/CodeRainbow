set path=c:\Program Files (x86)\Microsoft Visual Studio 14.0\vc\bin\;c:\Qt\Qt5.10.1\5.10.1\msvc2015\bin;%PATH%
qmake ../qtcreator.pro -recursive
nmake
