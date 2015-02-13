import os, sys
import SCons.Script


# A path validator that only creates directories when installing.
def pathIsDirCreateIfInstall(key, value, env):
    if 'install' in map(str, BUILD_TARGETS):
        return PathVariable.PathIsDirCreate(key, value, env)
    else:
        return PathVariable.PathAccept(key, value, env)

env = Environment()
opts = SCons.Script.Variables('custom.py')
opts.AddVariables(
    PathVariable('PREFIX', 'Installation prefix',
                 Dir('.').abspath, pathIsDirCreateIfInstall),
    PathVariable('LIBDIR', 'Library installation directory',
                 '$PREFIX/lib', pathIsDirCreateIfInstall),
    PathVariable('INCLUDEDIR', 'Header file installation directory',
                 '$PREFIX/include/lsst/sphgeom', pathIsDirCreateIfInstall)
)

opts.Update(env)

AddOption('--debug-build', dest='debug_build', action='store_true',
          default=False, help='Perform a debug build.')
AddOption('--coverage', dest='coverage', action='store_true', default=False,
          help='Enable code coverage statistics (in debug builds only). '
               'Both gcov and gcovr are required to be in the PATH.')
AddOption('--no-color', dest='no_color', action='store_true', default=False,
          help='Do not use colored text in build messages.')

env['ENV']['PATH'] = os.environ['PATH']

env.Append(CCFLAGS=[
    '-g',
    '-Wall',
    '-Wextra',
    '-fvisibility-inlines-hidden',
    '-march=native',
])

if GetOption('debug_build'):
    variant_dir = 'build/debug'
    env.Append(CCFLAGS=['-O0'])
    if GetOption('coverage'):
        env.Append(CCFLAGS=['--coverage'])
        env.Append(LINKFLAGS=['--coverage'])
else:
    variant_dir = 'build/release'
    env.Append(CCFLAGS=['-O2'])
env.Alias('install', ['$INCLUDEDIR', '$LIBDIR'])
env.Install('$prefix/ups', 'ups/sphgeom.table')

env.Help('Spherical Geometry')
if not GetOption('help'):
    #if not GetOption('clean'):
    #    conf = Configure(env)
    #    env = conf.Finish()
    SConscript(dirs='.', variant_dir=variant_dir, exports='env')
