@echo off

cd %~dp0

if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help

@rem Process arguments.
set config=
set target=Rebuild
set nobuild=
set run=
set target_arch=x64
set vs_toolset=x86
set platform=x64
set shared=
set prefix=
set openssl=
set uvheader=
set runtime_library=

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="x86"               set target_arch=ia32&set platform=WIN32&set vs_toolset=x86&goto arg-ok
if /i "%1"=="ia32"              set target_arch=ia32&set platform=WIN32&set vs_toolset=x86&goto arg-ok
if /i "%1"=="x64"               set target_arch=x64&set platform=x64&set vs_toolset=x64&goto arg-ok
if /i "%1"=="mt"                set runtime_library="-Druntime_library=mt"&&goto arg-ok
if /i "%1"=="md"                set runtime_library="-Druntime_library=md"&&goto arg-ok
if /i "%1"=="shared"            set shared="-Denable_shared=true"&goto arg-ok
if /i "%1"=="--enable-uvheader" set uvheader="yes"&shift&goto arg-ok
if /i "%1"=="--prefix"          set prefix="%2"&shift&goto arg-ok
if /i "%1"=="--with-ssl"        set openssl="%2"&shift&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done

if defined WindowsSDKDir goto project-gen
if defined VCINSTALLDIR goto project-gen

@rem Look for Visual Studio 2013
if not defined VS120COMNTOOLS goto vc-set-2012
if not exist "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2012
call "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2013
goto project-gen

:vc-set-2012
@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2012
goto project-gen

:vc-set-2010
@rem Look for Visual Studio 2010
if not defined VS100COMNTOOLS goto vc-set-2008
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2008
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2010
goto project-gen

:vc-set-2008
@rem Look for Visual Studio 2008
if not defined VS90COMNTOOLS goto vc-set-notfound
if not exist "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-notfound
call "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2008
goto project-gen

:vc-set-notfound
echo Warning: Visual Studio not found
goto exit

:project-gen
if not defined PYTHON set PYTHON=python
if defined openssl (
   "%PYTHON%" configure.py -Dtarget_arch=%target_arch% -Dwith_ssl=%openssl% %shared% %runtime_library%
) else (
   "%PYTHON%" configure.py -Dtarget_arch=%target_arch% %shared% %runtime_library%
)
if errorlevel 1 goto create-msvs-files-failed
if not exist linear.sln goto create-msvs-files-failed
echo Project files generated.

:msbuild
@rem Check if VS build env is available
if defined VCINSTALLDIR goto msbuild-found
if defined WindowsSDKDir goto msbuild-found
echo Build skipped. To build, this file needs to run from VS cmd prompt.
goto exit

@rem Build the sln with msbuild.
:msbuild-found
msbuild /m linear.sln /t:%target% /p:Configuration=Debug /p:Platform="%platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1
msbuild /m linear.sln /t:%target% /p:Configuration=Release /p:Platform="%platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1
goto deploy

:create-msvs-files-failed
echo Failed to create vc project files.
exit /b 1

:help
echo vcbuild.bat [x86/x64] [mt/md] [shared] [--with-ssl=/path/to/OpenSSL]
echo Examples:
echo   vcbuild.bat md                         : builds lib with /MD(d) option
echo   vcbuild.bat shared                     : builds dll
echo   vcbuild.bat --prefix=C:\usr\local      : path to install headers and libraries [default := .\linear-package]
echo   vcbuild.bat --with-ssl=C:\OpenSSL-Win64: builds lib with SSL feature [default: without OpenSSL]
goto exit

:deploy
set msgpack_headers=^
  "deps\msgpack\include\msgpack.h" ^
  "deps\msgpack\include\msgpack.hpp"

set linear_headers_private=^
  "include\linear\private\extern.h" ^
  "include\linear\private\message_priv.h" ^
  "include\linear\private\observer.h"

set linear_headers_nossl=^
  "include\linear\addrinfo.h" ^
  "include\linear\any.h" ^
  "include\linear\auth_context.h" ^
  "include\linear\binary.h" ^
  "include\linear\client.h" ^
  "include\linear\condition_variable.h" ^
  "include\linear\error.h" ^
  "include\linear\group.h" ^
  "include\linear\handler.h" ^
  "include\linear\log.h" ^
  "include\linear\memory.h" ^
  "include\linear\message.h" ^
  "include\linear\msgpack_inc.h" ^
  "include\linear\mutex.h" ^
  "include\linear\nil.h" ^
  "include\linear\optional.h" ^
  "include\linear\server.h" ^
  "include\linear\socket.h" ^
  "include\linear\tcp_client.h" ^
  "include\linear\tcp_server.h" ^
  "include\linear\tcp_socket.h" ^
  "include\linear\timer.h" ^
  "include\linear\version.h" ^
  "include\linear\ws_client.h" ^
  "include\linear\ws_context.h" ^
  "include\linear\ws_server.h" ^
  "include\linear\ws_socket.h"

if not defined prefix (
   set prefix=".\linear-package"
   if exist %prefix% rmdir /S /Q %prefix% > nul
)
mkdir %prefix%\include > nul

@rem copy msgpack headers
for %%f in (%msgpack_headers%) do copy /Y %%f %prefix%\include > nul
mkdir %prefix%\include\msgpack > nul
xcopy /e /q /y deps\msgpack\include\msgpack %prefix%\include\msgpack\ > nul

@rem copy linear headers (without ssl)
mkdir %prefix%\include\linear > nul
for %%f in (%linear_headers_nossl%) do copy /Y %%f %prefix%\include\linear > nul
mkdir %prefix%\include\linear\private > nul
for %%f in (%linear_headers_private%) do copy /Y %%f %prefix%\include\linear\private > nul

@rem copy uv headers if needed
if defined uvheader goto install-uv-headers
goto check-install-ssl-headers

:install-uv-headers
set libuv_headers=^
  "deps\libtv\deps\libuv\include\uv.h" ^
  "deps\libtv\deps\libuv\include\uv-errno.h" ^
  "deps\libtv\deps\libuv\include\uv-threadpool.h" ^
  "deps\libtv\deps\libuv\include\uv-version.h" ^
  "deps\libtv\deps\libuv\include\uv-win.h" ^
  "deps\libtv\deps\libuv\include\tree.h" ^
  "deps\libtv\deps\libuv\include\stdint-msvc2008.h"

for %%f in (%libuv_headers%) do copy /Y %%f %prefix%\include > nul

:check-install-ssl-headers
if defined openssl goto copy-linear-headers-ssl
goto deploy-lib

:copy-linear-headers-ssl
set linear_headers_ssl=^
  "include\linear\ssl_client.h" ^
  "include\linear\ssl_context.h" ^
  "include\linear\ssl_server.h" ^
  "include\linear\ssl_socket.h" ^
  "include\linear\wss_client.h" ^
  "include\linear\wss_server.h" ^
  "include\linear\wss_socket.h" ^
  "include\linear\x509_certificate.h"

@rem copy linear headers (with ssl)
for %%f in (%linear_headers_ssl%) do copy /Y %%f %prefix%\include\linear > nul

:deploy-lib
mkdir %prefix%\lib > nul
mkdir %prefix%\lib\Debug > nul
mkdir %prefix%\lib\Release > nul

if defined shared (
   copy /Y Debug\*.lib %prefix%\lib\Debug\ > nul
   copy /Y Debug\*.dll %prefix%\lib\Debug\ > nul
   copy /Y Release\*.lib %prefix%\lib\Release\ > nul
   copy /Y Release\*.dll %prefix%\lib\Release\ > nul
) else (
   copy /Y Debug\lib\*.lib %prefix%\lib\Debug\ > nul
   copy /Y Release\lib\*.lib %prefix%\lib\Release\ > nul
)

:deploy-sample
mkdir %prefix%\sample > nul
copy /Y sample\common.gypi %prefix%\sample > nul
copy /Y sample\linear-sample.gyp %prefix%\sample > nul
copy /Y sample\configure.py %prefix%\sample > nul
xcopy /e /q /y deps\libtv\deps\gyp %prefix%\sample\gyp\ > nul

set linear_samples_nossl=^
  "sample\tcp_client_sample.cpp" ^
  "sample\tcp_server_sample.cpp" ^
  "sample\ws_client_sample.cpp" ^
  "sample\ws_server_sample.cpp"

for %%f in (%linear_samples_nossl%) do copy /Y %%f %prefix%\sample > nul

if not defined openssl goto create-doc

set linear_samples_ssl=^
  "sample\ssl_client_sample.cpp" ^
  "sample\ssl_server_sample.cpp" ^
  "sample\wss_client_sample.cpp" ^
  "sample\wss_server_sample.cpp"

for %%f in (%linear_samples_ssl%) do copy /Y %%f %prefix%\sample > nul

:create-doc
cd doc
doxygen Doxyfile > nul
if errorlevel 1 (
   echo "not found doxygen.so not create api documents"
   cd ..\
   goto post-deploy
)
mkdir ..\%prefix%\doc > nul
xcopy /e /q /y html ..\%prefix%\doc\ > nul
cd ..

:post-deploy
cd %prefix%\sample
if defined openssl (
   "%PYTHON%" configure.py -Dwith_ssl=%openssl% %shared% %runtime_library%
) else (
   "%PYTHON%" configure.py %shared% %runtime_library%
)
cd ..\..\

:exit
