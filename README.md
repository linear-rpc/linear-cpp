# Linear C++ [![Build Status](https://travis-ci.org/linear-rpc/linear-cpp.svg?branch=master)](https://travis-ci.org/linear-rpc/linear-cpp)

## Overview

a msgpack-rpc + α implementation for C++ language.

## Important Notice
### v2.9.0 -> v.3.0.0
* Constructor's var args are changed<br>
<pre class="fragment">
const linear::Handler& -&gt; const linear::shared_ptr&lt;linear::Handler&gt;&
</pre>
at TCPClient, TCPServer, SSLClient, SSLServer, WSClient, WSServer, WSSClient, WSSServer

## Build Instructions
### Required tools and Dependencies
* xNix
  * autotools and libtool<br>
    aclocal, autoheader, automake, autoconf and libtoolize
  * OpenSSL v1.0.1 or later<br>
    If you want to use {SSL, WSS} transport,
    install openssl developer packages.
* Windows
  * Python v2.x<br>
    needed by gyp for creating a solution file etc.<br>
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
$ vcbuild.bat [--prefix=/path/to/install] [--with-ssl=/path/to/OpenSSL] [shared]
</pre>
For more options, please check
<pre class="fragment">
$ vcbuild.bat --help
</pre>
You can find headers at /path/to/install/include,
libraries at /path/to/install/lib
and documents at ${top\_project\_dir}/doc/html/index.html.<br>

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
+-----------+               +---------------------------+         +-----------+
| WSClient  | - WebSocket - | lighttpd w/ mod_websocket | - TCP - | TCPServer |
+-----------+               +---------------------------+         +-----------+
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
