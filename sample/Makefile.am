SUBDIRS = \
	.

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/deps/msgpack/include

LDADD = \
	$(top_srcdir)/src/.libs/liblinear.a \
	$(top_srcdir)/deps/libtv/src/.libs/libtv.a \
	$(top_srcdir)/deps/libtv/deps/libuv/.libs/libuv.a -ldl

noinst_PROGRAMS = \
	tcp_server_sample \
	tcp_client_sample \
	ws_server_sample \
	ws_client_sample \
	lperf

if WITH_SSL
noinst_PROGRAMS += \
	ssl_server_sample \
	ssl_client_sample \
	wss_server_sample \
	wss_client_sample
endif

tcp_server_sample_SOURCES = \
	tcp_server_sample.cpp

tcp_client_sample_SOURCES = \
	tcp_client_sample.cpp

ws_server_sample_SOURCES = \
	ws_server_sample.cpp

ws_client_sample_SOURCES = \
	ws_client_sample.cpp

lperf_SOURCES = \
	lperf.cpp

if WITH_SSL
ssl_server_sample_SOURCES = \
	ssl_server_sample.cpp

ssl_client_sample_SOURCES = \
	ssl_client_sample.cpp

wss_server_sample_SOURCES = \
	wss_server_sample.cpp

wss_client_sample_SOURCES = \
	wss_client_sample.cpp
endif
