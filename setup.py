
import setuptools
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import glob
import os
import sysconfig


# Proposal to add this to CCompiler, but not yet implemented:
# http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.
    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, '-std=c++14'):
        return '-std=c++14'
    elif has_flag(compiler, '-std=c++11'):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- at least C++11 support '
                           'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'unix': [],
    }

    if sys.platform == 'darwin':
        c_opts['unix'] += ['-stdlib=libc++', '-mmacosx-version-min=10.9']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            # MacOS needs '-dynamiclib' here; by default it specifies '-bundle' which doesn't work.
            try:
                self.compiler.linker_so[self.compiler.linker_so.index('-bundle')] = '-dynamiclib'
            except ValueError:
                pass
        for ext in self.extensions:
            ext.extra_compile_args = opts
            full_name = ext.name.split('.')[-1] + sysconfig.get_config_var('EXT_SUFFIX')
            ext.extra_link_args = ['-stdlib=libc++',
                                   '-Wl,-dylib_install_name,@loader_path/{:s}'.format(full_name)]
            if sys.platform == 'darwin':
                ext.extra_link_args.append('-mmacosx-version-min=10.9')
        build_ext.build_extensions(self)


def _get_distutils_build_directory():
    """
    Returns the directory distutils uses to build its files.
    We need this directory since we build extensions which have to link
    other ones.
    """
    pattern = "lib.{platform}-{major}.{minor}"
    return os.path.join('build', pattern.format(platform=sysconfig.get_platform(),
                                                major=sys.version_info[0],
                                                minor=sys.version_info[1]))


library_dirs = [os.path.join(_get_distutils_build_directory(), "lsst/sphgeom")]

ext_modules = [
    Extension(
        'lsst.sphgeom.libsphgeom',
        glob.glob("src/*.cc"),
        include_dirs=["include/"],
        language='c++'
    )
]

for cc_filename in glob.glob("python/lsst/sphgeom/*cc"):
    module_name = cc_filename.split('/')[-1].split('.')[0]
    lib_extension = sysconfig.get_config_var('EXT_SUFFIX').split('.')[1]
    ext_modules.append(
        Extension(
            "lsst.sphgeom.{:s}".format(module_name),
            [cc_filename],
            include_dirs=[
                "include/",
                get_pybind_include(),
                get_pybind_include(user=True)
            ],
            libraries=["sphgeom.{:s}".format(lib_extension)],
            library_dirs=library_dirs,
            language='c++'
        )
    )

setup(name='lsst-sphgeom',
      version='1.0',
      # Include package lsst to copy lsst/__init__.py
      # That will probably break something to have multiple packages editing that file.
      packages=['lsst', 'lsst.sphgeom'],
      package_dir={'': "python/"},
      ext_modules=ext_modules,
      cmdclass={'build_ext': BuildExt},
      )


