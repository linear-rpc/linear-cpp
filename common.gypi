{
  'variables': {
    'visibility%': 'default',
    'target_arch%': 'x64',
    'host_arch%': 'x64',
    'linear_library%': 'static_library', # allow override to 'shared_library' for DLL/.so builds
    'msvs_multi_core_compile%': '1',
    'runtime_library%': 'default',
    'with_ssl%': 'false',
  },

  'target_defaults': {
    'default_configuration': 'Debug',
    'target_conditions': [
      ['with_ssl != "false" and with_ssl != "true"', {
        'include_dirs': [ '<(with_ssl)/include' ],
        'target_conditions': [
          ['OS != "win"', {
            'library_dirs': [ '<(with_ssl)/lib' ],
          }],
        ],
      }],
      ['OS != "win" and with_ssl != "false"', {
        'libraries': [ '-lcrypto', '-lssl' ],
      }],
    ],
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG' ],
        'cflags': [ '-g', '-O0' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'target_conditions': [
              [ 'runtime_library=="default" and linear_library=="static_library"', {
                'RuntimeLibrary': 1, # /MTd == MultiThread debug
              }],
              [ 'runtime_library=="default" and linear_library=="shared_library"', {
                'RuntimeLibrary': 3, # /MDd == MultiThread debug
              }],
              ['runtime_library=="mt"', {
                'RuntimeLibrary': 1, # /MTd == MultiThread debug
              }],
              ['runtime_library=="md"', {
                'RuntimeLibrary': 3, # /MDd == MultiThread debug
              }],
            ],
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
            'DebugInformationFormat': 3, # Generate a PDB /Zi
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
            'GenerateDebugInformation': 'true',
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'cflags': [ '-O3' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'target_conditions': [
              [ 'runtime_library=="default" and linear_library=="static_library"', {
                'RuntimeLibrary': 0, # /MT
              }],
              [ 'runtime_library=="default" and linear_library=="shared_library"', {
                'RuntimeLibrary': 2, # /MD
              }],
              ['runtime_library=="mt"', {
                'RuntimeLibrary': 0, # /MT
              }],
              ['runtime_library=="md"', {
                'RuntimeLibrary': 2, # /MD
              }],
            ],
            'Optimization': 3, # /Ox, full optimization
            'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
            'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
            'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
            'OmitFramePointers': 'true',
            'EnableFunctionLevelLinking': 'true',
            'EnableIntrinsicFunctions': 'true',
            'DebugInformationFormat': 0, # not Generate a PDB
          },
          'VCLibrarianTool': {
            'AdditionalOptions': [
              '/LTCG', # link time code generation
            ],
          },
          'VCLinkerTool': {
            'LinkIncremental': 1, # disable incremental linking
            'LinkTimeCodeGeneration': 1, # link-time code generation
            'OptimizeReferences': 2, # /OPT:REF
            'EnableCOMDATFolding': 2, # /OPT:ICF
            'GenerateDebugInformation': 'false',
          },
        },
      }
    },
    'msvs_settings': {
      'VCCLCompilerTool': {
        'StringPooling': 'true', # pool string literals
        'WarningLevel': 3,
        'BufferSecurityCheck': 'true',
        'ExceptionHandling': 1, # /EHsc
        'SuppressStartupBanner': 'true',
        'WarnAsError': 'false',
        'AdditionalOptions': [
           '/MP', # compile across multiple CPUs
         ],
        'DisableSpecificWarnings': ['4244', '4267'], # XXX: /wd"4244", /ws"4267"
      },
      'VCLibrarianTool': {
      },
      'VCLinkerTool': {
        'RandomizedBaseAddress': 2, # enable ASLR
        'DataExecutionPrevention': 2, # enable DEP
        'AllowIsolation': 'true',
        'SuppressStartupBanner': 'true',
        'target_conditions': [
          [ 'runtime_library=="default" and linear_library=="shared_library" and with_ssl != "false"', {
            'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
            'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
          }],
          [ 'runtime_library=="mt" and linear_library=="shared_library" and with_ssl != "false"', {
            'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
            'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
          }],
          [ 'runtime_library=="md" and linear_library=="shared_library" and with_ssl != "false"', {
            'AdditionalLibraryDirectories': [ '<(with_ssl)\lib\VC' ],
            'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
          }],
        ],
      },
    },
    'conditions': [
      ['OS == "win"', {
        'msvs_cygwin_shell': 0, # prevent actions from trying to use cygwin
        'defines': [
          'WIN32',
          # we don't really want VC++ warning us about
          # how dangerous C functions are...
          '_CRT_SECURE_NO_DEPRECATE',
          # ... or that C implementations shouldn't use
          # POSIX names
          '_CRT_NONSTDC_NO_DEPRECATE',
          'WIN32_LEAN_AND_MEAN',
        ],
        'target_conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64'
          }]
        ]
      }],
    ],
  },
}
