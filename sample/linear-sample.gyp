{
  'variables': {
    'linear_sample_parent_path': '/',
  },
  'targets': [
    {
      'target_name': 'tcp_client_sample',
      'type': 'executable',
      'sources': [
        'tcp_client_sample.cpp',
      ],
    },
    {
      'target_name': 'tcp_server_sample',
      'type': 'executable',
      'sources': [
        'tcp_server_sample.cpp',
      ],
    },
    {
      'target_name': 'ws_client_sample',
      'type': 'executable',
      'sources': [
        'ws_client_sample.cpp',
      ],
    },
    {
      'target_name': 'ws_server_sample',
      'type': 'executable',
      'sources': [
        'ws_server_sample.cpp',
      ],
    },
  ],
  'conditions': [
    [ 'with_ssl != "false"', {
      'targets': [
        {
          'target_name': 'ssl_client_sample',
          'type': 'executable',
          'sources': [
            'ssl_client_sample.cpp',
          ],
        },
        {
          'target_name': 'ssl_server_sample',
          'type': 'executable',
          'sources': [
            'ssl_server_sample.cpp',
          ],
        },
        {
          'target_name': 'wss_client_sample',
          'type': 'executable',
          'sources': [
            'wss_client_sample.cpp',
          ],
        },
        {
          'target_name': 'wss_server_sample',
          'type': 'executable',
          'sources': [
            'wss_server_sample.cpp',
          ],
        },
      ],
    }],
  ],
}
