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


goto next-arg

:help
echo vcbuild.bat [--with-ssl=/path/to/OpenSSL] [--prefix=/path/to/install]
echo Examples:
echo   vcbuild.bat --prefix=C:\usr\local      : path to install headers and libraries [default := .\linear-package]
echo   vcbuild.bat --with-ssl=C:\OpenSSL-Win64: builds with openssl library
goto exit

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="--with-ssl"        set openssl="%2"&shift&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done

mkdir build
pushd build
if defined openssl (
   cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DOPENSSL_ROOT_DIR=%openssl% ../
) else (
   cmake -DCMAKE_GENERATOR_PLATFORM=x64 ../
)
popd

call MSBuild.bat build/liblinear.sln /p:Configuration=Release

call MSBuild.bat build/liblinear.sln /p:Configuration=Debug

set msgpack_headers=^
  "deps\msgpack\include\msgpack.h" ^
  "deps\msgpack\include\msgpack.hpp"

set linear_headers_private=^
  "include\linear\private\extern.h" ^
  "include\linear\private\message_priv.h"

set linear_headers_nossl=^
  "include\linear\addrinfo.h" ^
  "include\linear\any.h" ^
  "include\linear\auth_context.h" ^
  "include\linear\binary.h" ^
  "include\linear\client.h" ^
  "include\linear\condition_variable.h" ^
  "include\linear\error.h" ^
  "include\linear\event_loop.h" ^
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
mkdir %prefix%\include > nul 2>&1


@rem copy msgpack headers
for %%f in (%msgpack_headers%) do copy /Y %%f %prefix%\include > nul
mkdir %prefix%\include\msgpack > nul 2>&1
xcopy /e /q /y deps\msgpack\include\msgpack %prefix%\include\msgpack\ > nul

@rem copy linear headers (without ssl)
mkdir %prefix%\include\linear > nul 2>&1
for %%f in (%linear_headers_nossl%) do copy /Y %%f %prefix%\include\linear > nul
mkdir %prefix%\include\linear\private > nul 2>&1
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
mkdir %prefix%\lib > nul 2>&1
mkdir %prefix%\lib\Debug > nul 2>&1
mkdir %prefix%\lib\Release > nul 2>&1

if defined shared (
   copy /Y build\Debug\*.lib %prefix%\lib\Debug\ > nul
   copy /Y build\Debug\*.dll %prefix%\lib\Debug\ > nul
   copy /Y build\Release\*.lib %prefix%\lib\Release\ > nul
   copy /Y build\Release\*.dll %prefix%\lib\Release\ > nul
) else (
   copy /Y build\Debug\*.lib %prefix%\lib\Debug\ > nul
   copy /Y build\Release\*.lib %prefix%\lib\Release\ > nul
)



:exit
