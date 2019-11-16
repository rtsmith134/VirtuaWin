if exist Output rd /Q /S Output
md Output
md Output\x64
md Output\PDB32
md Output\PDB64

echo -- Compiling

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set MSBuildDir=%%i\MSBuild\15.0\Bin\

REM ********* Build 64-bit solution
echo --- 64bit
"%MSBuildDir%MSBuild.exe" ..\VS2017\VirtuaWin10.sln /m /t:Rebuild /p:Configuration="Release" /p:Platform="x64" /verbosity:quiet /nologo
@if ERRORLEVEL 1 exit /b 1

REM ********* Build 32-bit solution (must be after 64-bit)
echo --- 32bit
"%MSBuildDir%MSBuild.exe" ..\VS2017\VirtuaWin10.sln /m /t:Rebuild /p:Configuration="Release" /p:Platform="Win32" /verbosity:quiet /nologo
@if ERRORLEVEL 1 exit /b 1


REM ********* Copy binaries

copy /B ..\VS2017\x64\Release\VirtuaWin10.exe Output > nul


exit /b 0
