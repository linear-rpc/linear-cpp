{
  'variables': {
    'visibility%': 'default',
    'target_arch%': 'x64',
    'host_arch%': 'x64',
    'msvs_multi_core_compile%': '1',
    'with_linear%': '..\\',
    'with_ssl%': 'false',
    'enable_shared%': 'false',
    'runtime_library%': 'default',
  },

  'target_defaults': {
    'msbuild_toolset': 'v120',
    'default_configuration': 'Debug',
    'target_conditions': [
      ['with_ssl != "false"', {
        'include_dirs': [
          '<(with_linear)/include',
          '<(with_ssl)/include'
        ],
      }, {
        'include_dirs': [ '<(with_linear)/include' ],
      }],
    ],
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'target_conditions': [
              [ 'runtime_library=="default" and enable_shared!="true"', {
                'RuntimeLibrary': 1, # /MTd
              }],
              [ 'runtime_library=="default" and enable_shared=="true"', {
                'RuntimeLibrary': 3, # /MDd
              }],
              ['runtime_library=="mt"', {
                'RuntimeLibrary': 1, # /MTd
              }],
              ['runtime_library=="md"', {
                'RuntimeLibrary': 3, # /MDd
              }],
            ],
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'true',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
            'DebugInformationFormat': 3, # Generate a PDB
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
            'GenerateDebugInformation': 'true',
            'target_conditions': [
              [ 'with_ssl != "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug' ],
              }],
              [ 'runtime_library=="default" and enable_shared=="false" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
              }],
              [ 'runtime_library=="default" and enable_shared=="true" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
              }],
              [ 'runtime_library=="mt" and enable_shared=="false" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
              }],
              [ 'runtime_library=="mt" and enable_shared=="true" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MTd.lib', 'libeay32MTd.lib' ],
              }],
              [ 'runtime_library=="md" and enable_shared=="false" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
              }],
              [ 'runtime_library=="md" and enable_shared=="true" and with_ssl != "false"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Debug', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MDd.lib', 'libeay32MDd.lib' ],
              }],
            ],
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'target_conditions': [
              [ 'runtime_library=="default" and enable_shared!="true"', {
                'RuntimeLibrary': 0, # /MT
              }],
              [ 'runtime_library=="default" and enable_shared=="true"', {
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
            'target_conditions': [
              [ 'with_ssl != "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release' ],
              }],
              [ 'runtime_library=="default" and enable_shared=="false" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MT.lib', 'libeay32MT.lib' ],
              }],
              [ 'runtime_library=="default" and enable_shared=="true" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MD.lib', 'libeay32MD.lib' ],
              }],
              [ 'runtime_library=="mt" and enable_shared=="false" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MT.lib', 'libeay32MT.lib' ],
              }],
              [ 'runtime_library=="mt" and enable_shared=="true" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MT.lib', 'libeay32MT.lib' ],
              }],
              [ 'runtime_library=="md" and enable_shared=="false" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC\static' ],
                'AdditionalDependencies': [ 'ssleay32MD.lib', 'libeay32MD.lib' ],
              }],
              [ 'runtime_library=="md" and enable_shared=="true" and with_ssl == "true"', {
                'AdditionalLibraryDirectories': [ '<(with_linear)\lib\Release', '<(with_ssl)\lib\VC' ],
                'AdditionalDependencies': [ 'ssleay32MD.lib', 'libeay32MD.lib' ],
              }],
            ],
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
        'DisableSpecificWarnings': ['4267', '4290'], # for msgpack
      },
      'VCLibrarianTool': {
      },
      'VCLinkerTool': {
        'RandomizedBaseAddress': 2, # enable ASLR
        'DataExecutionPrevention': 2, # enable DEP
        'AllowIsolation': 'true',
        'SuppressStartupBanner': 'true',
        'SubSystem': 1, # console executable
        'AdditionalDependencies': [
          'liblinear.lib',
          'libtv.lib',
          'libuv.lib',
        ],
      },
    },
    'conditions': [
      [ 'OS == "win"', {
        'msvs_cygwin_shell': 0, # prevent actions from trying to use cygwin
        'msvs_configuration_platform': 'x64',
        'defines': [
          'WIN32',
          '_WIN32_WINNT=0x0600',
          '_GNU_SOURCE',
          '_CRT_SECURE_NO_DEPRECATE',
          '_CRT_NONSTDC_NO_DEPRECATE',
        ],
        'link_settings': {
          'libraries': [
            '-ladvapi32',
            '-liphlpapi',
            '-lpsapi',
            '-lshell32',
            '-luserenv',
            '-lws2_32',
          ],
          'target_conditions': [
            [ 'with_ssl != "true"', {
              'libraries': [ '-lgdi32', '-luser32' ],
            }],
          ],
        },
      }],
      [ 'enable_shared == "true"', {
        'defines': [
          'USING_LINEAR_SHARED=1',
          'USING_TV_SHARED=1',
          'USING_UV_SHARED=1',
          'USING_WS_SHARED=1',
        ],
      }],
    ],
  },
}
