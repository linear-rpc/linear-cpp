SUBDIRS = . include src

if WITH_TEST
SUBDIRS += test
endif

if WITH_SAMPLE
SUBDIRS += sample
endif

all-local:
	(cd deps/libtv; $(MAKE))
	@if [ $(WITH_TEST) ]; then \
		(cd deps/gmock-1.7.0; $(MAKE)); \
	fi

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(includedir)
	cp -fr deps/msgpack/include/* $(DESTDIR)$(includedir)
	(cd deps/libtv; $(MAKE) install)
	rm -f $(DESTDIR)$(includedir)/tv.h
	rm -fr $(DESTDIR)$(includedir)/websocket
	rm -f $(DESTDIR)$(libdir)/pkgconfig/libtv.pc
	if [ $(ENABLE_UVHEADER) -eq 0 ]; then \
		rm -f $(DESTDIR)$(includedir)/uv.h; \
		rm -f $(DESTDIR)$(includedir)/uv-*.h; \
		rm -f $(DESTDIR)$(includedir)/tree.h; \
		rm -f $(DESTDIR)$(includedir)/android-ifaddrs.h; \
		rm -f $(DESTDIR)$(includedir)/pthread-fixes.h; \
		rm -f $(DESTDIR)$(includedir)/stdint-msvc2008.h; \
		rm -f $(DESTDIR)$(libdir)/pkgconfig/libuv.pc; \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(libdir)/libuv.* $(DESTDIR)$(libdir)/libtv.*
	rm -f $(DESTDIR)$(includedir)/msgpack.h $(DESTDIR)$(includedir)/msgpack.hpp $(DESTDIR)$(includedir)/msgpack_fwd.hpp
	rm -fr $(DESTDIR)$(includedir)/msgpack

clean-local:
	(cd deps/libtv; $(MAKE) clean)
	rm -f *~

if HAVE_PKG_CONFIG
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lib@PACKAGE_NAME@.pc
endif
