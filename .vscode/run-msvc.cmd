@echo off
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
    call "%%i\VC\Auxiliary\Build\vcvarsall.bat" x64 2>nul
)
%*
