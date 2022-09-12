:: MSBuild.exe %*

@echo off
setlocal
:: cd /d %~dp0

set PF32=%ProgramFiles(x86)%
if not exist "%PF32%" set PF32=%ProgramFiles%

set VsWherePath="%PF32%\Microsoft Visual Studio\Installer\vswhere.exe"

:: vs2019 or later
for /f "usebackq tokens=*" %%i in (`%VsWherePath% -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    if exist "%%i" (
        set MSBuildPath="%%i"
        goto :run
    )
)

:: vs2017 or earlier
for /f "usebackq tokens=1* delims=: " %%i in (`%VsWherePath% -latest -requires Microsoft.Component.MSBuild`) do (
    if /i "%%i"=="installationPath" (
        if exist "%%j\MSBuild\15.0\Bin\MSBuild.exe" (
            set MSBuildPath="%%j\MSBuild\15.0\Bin\MSBuild.exe"
            goto :run
        )
        if exist "%%j\MSBuild\14.0\Bin\MSBuild.exe" (
            set MSBuildPath="%%j\MSBuild\14.0\Bin\MSBuild.exe"
            goto :run
        )
        if exist "%%j\MSBuild\12.0\Bin\MSBuild.exe" (
            set MSBuildPath="%%j\MSBuild\12.0\Bin\MSBuild.exe"
            goto :run
        )
    )
)

:run
if not exist %MSBuildPath% (
    echo "MSBuild.exe not find"
    exit /b 1
)

echo on
%MSBuildPath% %*