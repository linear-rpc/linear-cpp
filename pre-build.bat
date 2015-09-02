@echo off

setlocal

rem ####################
rem Create version.h
rem ####################

set configure_ac_=configure.ac
set version_h_in_=include\linear\version.h.in
set version_h_=include\linear\version.h

for /f "usebackq delims=" %%a in (`findstr AC_INIT %configure_ac_%`) do (
    for /f "tokens=2,3 delims=[" %%t in ("%%a") do (
        for /f "tokens=1 delims=]" %%v in ("%%t") do (
            set packageName=%%v
        )
        for /f "tokens=1 delims=]" %%w in ("%%u") do (
            set packageVersion=%%w
        )
    )
)
set versionId=%packageName%-%packageVersion%

for /f "usebackq delims=" %%c in (`git log --pretty^=format:"%%H" -1`) do (
    set commitId=%%c
)

for /f "usebackq delims=" %%d in (`git log --pretty^=format:"%%ad" -1`) do (
    set commitDate=%%d
)

type nul > %version_h_%

for /f "delims=" %%l in (%version_h_in_%) do (
    set line=%%l
    call :replace1
)

goto :end1

:replace1
    set line=%line:>=^>%
    set line=%line:<=^<%
    set line=%line:&=^&%
    set line=%line:|=^|%
    call set line=%%line:VersionId="%versionId%"%%
    call set line=%%line:CommitId="%commitId%"%%
    call set line=%%line:CommitDate="%commitDate%"%%
    echo %line%>>%version_h_%
    exit /B

:end1

rem ####################
rem Create memory.h
rem ####################

set memory_h_in_=include\linear\memory.h.in
set memory_h_=include\linear\memory.h

if "%1" == "TR1" set sharedPtrDefinition=HAVE_TR1_SHARED_PTR
if "%1" == "STD" set sharedPtrDefinition=HAVE_STD_SHARED_PTR

type nul > %memory_h_%

for /f "delims=" %%m in (%memory_h_in_%) do (
    set "line2=%%m"
    call :replace2
)

goto :end2

:replace2
    set "line2=%line2:>=^>%"
    set "line2=%line2:<=^<%"
    set "line2=%line2:&=^&%"
    set "line2=%line2:|=^|%"
    call set "line2=%%line2:SharedPtrDefinition=#define %sharedPtrDefinition% 1 %%"
    echo %line2%>>%memory_h_%
    exit /B

:end2
