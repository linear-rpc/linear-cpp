# Linear C++ [![production CI](https://github.com/linear-rpc/linear-cpp/actions/workflows/production.yml/badge.svg)](https://github.com/linear-rpc/linear-cpp/actions/workflows/production.yml)

## Overview

a msgpack-rpc + α implementation for C++ language.

## Build Instructions
### Required tools and Dependencies
* xNix
  * autotools and libtool<br>
    aclocal, autoheader, automake, autoconf and libtoolize
  * OpenSSL v1.0.1 or later<br>
    If you want to use {SSL, WSS} transport,
    install openssl developer packages.
* Windows
  * Python v3.x<br>
    needed for creating version.h files etc.<br>
  * CMake v3.4 or later<br>
    needed for creating visual studio solution file.
  * OpenSSL v1.0.1 or later<br>
    If you want to use {SSL, WSS} transport,
    download binary distributions from https://www.openssl.org/community/binaries.html and install it.<br>
  * Windows SDK<br>
    Your application must link advapi32, iphlpapi, psapi, shell32, userenv and ws2_32.(needed by libuv)
    And must link gdi32 and user32 when using OpenSSL.
* xNix and Windows
  * Doxygen and Graphviz<br>
    If you want to create a manual, install doxygen and graphviz.

### How to make
```
$ git submodule update --init --recursive
```
* xNix<br>
<pre class="fragment">
$ ./bootstrap
$ ./configure [--prefix=/path/to/install] [--with-ssl=/path/to/OpenSSL]
$ make clean all install
$ cd doc; make doc
</pre>
For more options, please check
<pre class="fragment">
$ ./configure --help
</pre>
* Windows<br>
<pre class="fragment">
$ vcbuild.bat [--prefix=/path/to/install] [--with-ssl=/path/to/OpenSSL]
</pre>
For more options, please check
<pre class="fragment">
$ vcbuild.bat --help
</pre>


## Available Network Architecture
### Basic
<pre class="fragment">
+-----------+         +-----------+
| TCPClient | - TCP - | TCPServer |
+-----------+         +-----------+
</pre>

### Behind WebServer with WebSocket Proxy
<pre class="fragment">
+-----------+               +------------+               +----------+
| WSClient  | - WebSocket - | nginx etc. | - WebSocket - | WSServer |
+-----------+               +------------+               +----------+
</pre>

### Behind WebServer with WebSocket-TCP Proxy
<pre class="fragment">
+-----------+               +--------------------------+         +-----------+
| WSClient  | - WebSocket - | lighttpd w/ mod_wstunnel | - TCP - | TCPServer |
+-----------+               +--------------------------+         +-----------+
</pre>

### Combination
You can create an application that supports muliti protocols like follows.
<pre class="fragment">
+-----------+                   +-------------------------+         +-----------+
| WSSClient | - Sec-WebSocket - | WSSServer and TCPServer | - TCP - | TCPClient |
+-----------+                   +-------------------------+         +-----------+
</pre>

## Version Policy
* major<br>
  APIs and specifications are changed significantly,
  so you need to update server and client applications at the same time.
* minor<br>
  APIs are changed only slightly,
  so you need to rewrite applications if using appropriate APIs.
* patch<br>
  Bug fixes and security fixes etc.

## License
The MIT License (MIT)
See LICENSE for details.

And see some submodule LICENSEs(exist at deps dir).
