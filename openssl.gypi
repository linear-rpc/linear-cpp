{
  'conditions': [
    ['with_ssl != "false"', {
      'target_defaults': {
        'target_conditions': [
          ['OS != "win"', {
            'target_conditions': [
              ['with_ssl != "true"', {
                'include_dirs': [ '<(with_ssl)/include' ],
                'library_dirs': [ '<(with_ssl)/lib' ],
              }],
            ],
            'libraries': [ '-lcrypto', '-lssl' ],
          }],
        ],
        'msbuild_settings': {
          'ClCompile': {
            'AdditionalIncludeDirectories': [
              '<(with_ssl)\include',
            ],
          },
        },
        'configurations': {
          'Debug': {
            'msbuild_settings': {
              'Link': {
                'target_conditions': [
                  ['runtime_library == "default" and enable_shared == "false"', { # must be included openssl/applink.c in application.
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
                  }],
                  ['runtime_library == "default" and enable_shared == "true"', {
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
                  }],
                  ['runtime_library == "mt"', { # must be included openssl/applink.c in application.
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
                  }],
                  ['runtime_library == "md"', {
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
                  }],
                ],
              },
            },
          },
          'Release': {
            'msbuild_settings': {
              'Link': {
                'target_conditions': [
                  ['runtime_library == "default" and enable_shared == "false"', { # must be included openssl/applink.c in application.
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MT.lib', 'libeay32MT.lib' ],
                  }],
                  ['runtime_library == "default" and enable_shared == "true"', {
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MD.lib', 'libeay32MD.lib' ],
                  }],
                  ['runtime_library == "mt"', { # must be included openssl/applink.c in application.
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MT.lib', 'libeay32MT.lib' ],
                  }],
                  ['runtime_library == "md"', {
                    'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
                    'AdditionalDependencies': [ 'ssleay32MD.lib', 'libeay32MD.lib' ],
                  }],
                ],
              },
            },
          },
        },
      },
    }],
  ],
}
