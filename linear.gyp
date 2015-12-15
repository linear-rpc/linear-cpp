{
  'variables': {
    # linear_parent_path is the relative path to liblinear in the parent project
    # this is only relevant when dtrace is enabled and liblinear is a child project
    # as it's necessary to correctly locate the object files for post
    # processing.
    # XXX gyp is quite sensitive about paths with double / they don't normalize
    'linear_parent_path': '/',
    'linear_headers_without_ssl': [
      'include/linear/addrinfo.h',
      'include/linear/any.h',
      'include/linear/auth_context.h',
      'include/linear/binary.h',
      'include/linear/client.h',
      'include/linear/condition_variable.h',
      'include/linear/error.h',
      'include/linear/group.h',
      'include/linear/handler.h',
      'include/linear/log.h',
      'include/linear/memory.h',
      'include/linear/message.h',
      'include/linear/msgpack_inc.h',
      'include/linear/mutex.h',
      'include/linear/nil.h',
      'include/linear/optional.h',
      'include/linear/server.h',
      'include/linear/socket.h',
      'include/linear/tcp_client.h',
      'include/linear/tcp_server.h',
      'include/linear/tcp_socket.h',
      'include/linear/timer.h',
      'include/linear/version.h',
      'include/linear/ws_client.h',
      'include/linear/ws_context.h',
      'include/linear/ws_server.h',
      'include/linear/ws_socket.h',
    ],
    'linear_headers_with_ssl': [
      'include/linear/ssl_client.h',
      'include/linear/ssl_context.h',
      'include/linear/ssl_server.h',
      'include/linear/ssl_socket.h',
      'include/linear/wss_client.h',
      'include/linear/wss_server.h',
      'include/linear/wss_socket.h',
      'include/linear/x509_certificate.h',
    ],
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
      'sources': [
        'common.gypi',
        '<@(linear_headers_without_ssl)',
        'src/addrinfo.cpp',
        'src/any.cpp',
        'src/auth_context.cpp',
        'src/auth_context_impl.cpp',
        'src/condition_variable.cpp',
        'src/error.cpp',
        'src/event_loop.cpp',
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
        'src/tcp_client_impl.cpp',
        'src/tcp_server.cpp',
        'src/tcp_server_impl.cpp',
        'src/tcp_socket.cpp',
        'src/tcp_socket_impl.cpp',
        'src/timer.cpp',
        'src/timer_impl.cpp',
        'src/ws_client.cpp',
        'src/ws_client_impl.cpp',
        'src/ws_server.cpp',
        'src/ws_server_impl.cpp',
        'src/ws_socket.cpp',
        'src/ws_socket_impl.cpp',
      ],
      'conditions': [
        [ 'with_ssl != "false"', {
          'sources': [
            '<@(linear_headers_with_ssl)',
            'src/ssl_client.cpp',
            'src/ssl_client_impl.cpp',
            'src/ssl_context.cpp',
            'src/ssl_server.cpp',
            'src/ssl_server_impl.cpp',
            'src/ssl_socket.cpp',
            'src/ssl_socket_impl.cpp',
            'src/wss_client.cpp',
            'src/wss_client_impl.cpp',
            'src/wss_server.cpp',
            'src/wss_server_impl.cpp',
            'src/wss_socket.cpp',
            'src/wss_socket_impl.cpp',
            'src/x509_certificate.cpp',
          ],
          'defines': [
            'WITH_SSL',
          ],
        }],
        [ 'OS=="win"', {
          'defines': [
            '_WIN32_WINNT=0x0600',
            '_GNU_SOURCE',
          ],
          'link_settings': {
            'libraries': [
              '-ladvapi32',
              '-liphlpapi',
              '-lpsapi',
              '-lshell32',
              '-luserenv',
              '-lws2_32'
            ],
          },
        }],
        [ 'linear_library=="shared_library"', {
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
