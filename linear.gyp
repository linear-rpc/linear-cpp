{
  'variables': {
    'linear_library%': 'static_library',
    'warning_cflags%': [
      '-Wall -Wextra',
      '-Werror',
      '-Wcast-align -Wcast-qual',
      # '-Wconversion',
      '-Wdisabled-optimization',
      '-Wfloat-equal -Wformat=2',
      '-Winit-self -Winvalid-pch',
      # '-Wmissing-format-attribute',
      '-Wmissing-include-dirs -Wmissing-noreturn',
      '-Wpacked -Wpointer-arith',
      '-Wswitch-default',
      # '-Wswitch-enum',
      '-Wvolatile-register-var',
      '-Wwrite-strings',
      # '-Wlogical-op -Woverlength-strings -Wstrict-overflow=5 -Wvla',
      # '-Waggregate-return -Winline -Wpadded -Wunreachable-code -Wunsafe-loop-optimizations',
      # '-Wlarger-than-XXXXXX',
      '-Wno-unused-parameter',
    ],
    'other_cflags%': [
      '-ftrapv -D_FORTIFY_SOURCE=2',
      '-fstack-protector-all -Wstack-protector',
      # '-fmudflapth -lmudflapth',
      '-fstrict-aliasing -Wstrict-aliasing=2',
      '-fno-omit-frame-pointer',
    ],
  },
  'target_defaults': {
    'cflags': [ '<@(warning_cflags)', '<@(other_cflags)' ],
    'xcode_settings': {
      'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
      'GCC_OPTIMIZATION_LEVEL': '0',
      'WARNING_CFLAGS': [ '<@(warning_cflags)' ],
      'OTHER_CFLAGS': [ '<@(other_cflags)' ],
    },
  },
  'targets': [
    {
      'target_name': 'liblinear',
      'type': '<(linear_library)',
      'include_dirs': [
        'include',
        'src/',
        'deps/msgpack/include',
      ],
      'dependencies': [
        'deps/libtv/deps/libuv/uv.gyp:libuv',
        'deps/libtv/tv.gyp:libtv',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'include',
          'deps/msgpack/include',
        ],
      },
      'sources': [
        'src/addrinfo.cpp',
        'src/any.cpp',
        'src/auth_context.cpp',
        'src/auth_context_impl.cpp',
        'src/condition_variable.cpp',
        'src/error.cpp',
        'src/event_loop.cpp',
        'src/event_loop_impl.cpp',
        'src/group.cpp',
        'src/handler_delegate.cpp',
        'src/log.cpp',
        'src/log_file.cpp',
        'src/log_function.cpp',
        'src/log_stderr.cpp',
        'src/message.cpp',
        'src/mutex.cpp',
        'src/server.cpp',
        'src/socket.cpp',
        'src/socket_impl.cpp',
        'src/tcp_client.cpp',
        'src/tcp_server.cpp',
        'src/tcp_server_impl.cpp',
        'src/tcp_socket.cpp',
        'src/tcp_socket_impl.cpp',
        'src/timer.cpp',
        'src/timer_impl.cpp',
        'src/ws_client.cpp',
        'src/ws_server.cpp',
        'src/ws_server_impl.cpp',
        'src/ws_socket.cpp',
        'src/ws_socket_impl.cpp',
      ],
      'actions': [
        {
          'action_name': 'create_version_h',
          'inputs': [ 'tools/create_version_h.py', 'include/linear/version.h.in', 'configure.ac' ],
          'outputs': [ 'include/linear/version.h' ],
          'action': [ 'python', 'tools/create_version_h.py',
                      '-i', 'include/linear/version.h.in', '-o', 'include/linear/version.h',
                      '-c', 'configure.ac' ],
          'msvs_cygwin_shell': 0,
        },
        {
          'action_name': 'create_memory_h',
          'inputs': [ 'tools/create_memory_h.py', 'include/linear/memory.h.in' ],
          'outputs': [ 'include/linear/memory.h' ],
          'action': [ 'python', 'tools/create_memory_h.py',
                      '-i', 'include/linear/memory.h.in', '-o', 'include/linear/memory.h' ],
          'msvs_cygwin_shell': 0,
        },
      ],
      'conditions': [
        [ 'with_ssl != "false"', {
          'defines': [
            'WITH_SSL',
          ],
          'sources': [
            'src/ssl_client.cpp',
            'src/ssl_context.cpp',
            'src/ssl_server.cpp',
            'src/ssl_server_impl.cpp',
            'src/ssl_socket.cpp',
            'src/ssl_socket_impl.cpp',
            'src/wss_client.cpp',
            'src/wss_server.cpp',
            'src/wss_server_impl.cpp',
            'src/wss_socket.cpp',
            'src/wss_socket_impl.cpp',
            'src/x509_certificate.cpp',
          ],
        }],
        ['OS=="win"', {
          'defines': [
            # https://msdn.microsoft.com/en-US/library/windows/desktop/aa383745(v=vs.85).aspx
            '_WIN32_WINNT=0x0600', # supports after Windows Vista
          ],
        }, { # Not Windows i.e. POSIX
          'conditions': [
            ['_type == "shared_library" and OS != "mac"', {
              # This will cause gyp to set soname
              'product_extension': 'so.1',
            }],
          ],
        }],
        ['_type == "shared_library"', {
          'defines': [
            'BUILDING_LINEAR_SHARED=1',
            'USING_TV_SHARED=1',
            'USING_UV_SHARED=1',
            'USING_WS_SHARED=1',
          ]
        }],
      ]
    },
  ]
}
