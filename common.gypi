## Preferred settings in each platforms.
#
{
  'variables': {
    'visibility%': 'hidden',
    'host_arch%': 'x64',
    'target_arch%': 'x64',
    'enable_shared%': 'false', # 'false' or 'true'
    'runtime_library%': 'default', # 'md' or 'mt' or 'default'
    'with_ssl%': 'false',
    'linear_library%': 'static_library',
    'debug_cflags%': [ '-g', '-fwrapv', '-Wno-gnu-folding-constant' ],
    'release_cflags%': [ '-Wno-gnu-folding-constant' ],
  },

  'includes': [
    'openssl.gypi',
  ],

  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'cflags': [ '<@(debug_cflags)' '-O0' ],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'OTHER_CFLAGS': [ '<@(debug_cflags)' ],
        },
        'msbuild_settings': {
          'ClCompile': {
            'Optimization': 'Disabled',
            'MinimalRebuild': 'false',                     # /Gm
            'BasicRuntimeChecks': 'EnableFastChecks',      # /RTC1
            'OmitFramePointers': 'false',
            'DebugInformationFormat': 'ProgramDatabase',   # Generate a PDB /Zi
            'target_conditions': [
              ['runtime_library == "default" and enable_shared == "false"', {
                'RuntimeLibrary': 'MultiThreadedDebug',    # /MTd
              }],
              ['runtime_library == "default" and enable_shared == "true"', {
                'RuntimeLibrary': 'MultiThreadedDebugDLL', # /MDd
              }],
              ['runtime_library == "mt"', {
                'RuntimeLibrary': 'MultiThreadedDebug',    # /MTd
              }],
              ['runtime_library == "md"', {
                'RuntimeLibrary': 'MultiThreadedDebugDLL', # /MDd
              }],
            ],
          },
          'Link': {
            # 'LinkIncremental': 'true',                    # /INCREMENTAL
            'GenerateDebugInformation': 'true',
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG', ],
        'cflags': [ '<@(release_cflags)' '-O3' ],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '3',
          'OTHER_CFLAGS': [ '<@(release_cflags)' ],
        },
        'msbuild_settings': {
          'ClCompile': {
            'WholeProgramOptimization': 'true',
            'Optimization': 'Full',                       # /Ox
            'FunctionLevelLinking': 'true',               # /Gy
            'IntrinsicFunctions': 'true',                 # /Oi
            'target_conditions': [
              ['runtime_library == "default" and enable_shared == "false"', {
                'RuntimeLibrary': 'MultiThreaded',        # /MT
              }],
              ['runtime_library == "default" and enable_shared == "true"', {
                'RuntimeLibrary': 'MultiThreadedDLL',     # /MD
              }],
              ['runtime_library == "mt"', {
                'RuntimeLibrary': 'MultiThreaded',        # /MT
              }],
              ['runtime_library == "md"', {
                'RuntimeLibrary': 'MultiThreadedDLL',     # /MD
              }],
            ],
          },
          'target_conditions': [
            ['_type == "executable"', {
              'Link': {
                'LinkTimeCodeGeneration': 'UseLinkTimeCodeGeneration', # /LTCG
              },
            }, { # != "executable"
              'target_conditions': [
                ['enable_shared == "false"', {
                  'Lib': {
                    'LinkTimeCodeGeneration': 'true',
                  },
                }, { # != "false"
                  'Link': {
                    'LinkTimeCodeGeneration': 'UseLinkTimeCodeGeneration',
                  },
                }],
              ],
            }],
          ],
          'Link': {
            'GenerateDebugInformation': 'false',
          },
        },
      },
    },

    'conditions': [
      ['OS == "win"', {
        'target_conditions': [
          # if you want to change platform, use 'msvs_target_platform' key in the 'target'.
          ['target_arch == "ia32"', {
            'msvs_configuration_platform': 'Win32',
          }],
          ['target_arch == "x64"', {
            'msvs_configuration_platform': 'x64',
          }],
        ],
      }],
      ['OS == "mac"', {
        'target_conditions': [
          # if you want to change platform, use 'msvs_target_platform' key in the 'target'.
          ['target_arch == "ia32"', {
            'xcode_settings': { 'ARCHS': ['i386'] },
          }],
          ['target_arch == "x64"', {
            'xcode_settings': { 'ARCHS': ['x86_64'] },
          }],
        ],
        'xcode_settings': {
          'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
          'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic (Equivalent to -fPIC)
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
        },
      }],
      ['OS != "win"' and 'visibility == "hidden"', {
        'cflags': [ '-fvisibility=hidden' ],
        'xcode_settings': {
          'OTHER_CFLAGS':  [ '-fvisibility=hidden' ],
        },
      }],
      ['enable_shared == "true"', {
        'cflags': [ '-fPIC' ],
        'xcode_settings': {
          'OTHER_CFLAGS': [ '-fPIC' ],
        },
      }],
    ],
    'msbuild_settings': {
      'ClCompile': {
        'PreprocessorDefinitions': [
          'WIN32',
          # we don't really want VC++ warning us about
          # how dangerous C functions are...
          '_CRT_SECURE_NO_DEPRECATE',
          # ... or that C implementations shouldn't use
          # POSIX names
          '_CRT_NONSTDC_NO_DEPRECATE',
        ],
        'WarningLevel': 'Level3',
        'DisableSpecificWarnings': [ '4244', '4267' ], # TODO: convert type
        'target_conditions': [
          ['_type == "executable"', {
            'PreprocessorDefinitions': [ '_CONSOLE', '_LIB', ], # Visual Studio default definitions
          }, { # != "executable"
            'target_conditions': [
              ['enable_shared == "false"', {
                'PreprocessorDefinitions': [ '_LIB', ], # Visual Studio default definitions
              }, { # != "false"
                'PreprocessorDefinitions': [ '_WINDOWS', '_USRDLL', ], # Visual Studio default definitions
              }],
            ],
          }],
        ],
      },
      'Link': {
        'EnableCOMDATFolding': 'true', # /Zc:inline
        'OptimizeReferences': 'true',
      },
    },
  },
}
