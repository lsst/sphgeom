"""
Basic setuptools description.

This is not a complete definition.

* Version number is not correct.
* The shared library and include files are not installed.  This makes it
  unusable with other python packages that directly reference the C++
  interface.
"""

import glob

# Importing this automatically enables parallelized builds
import numpy.distutils.ccompiler  # noqa: F401
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

# Currently a fake version for testing
version = '0.0.1'

# To enable consistency with sconsUtils we write the version to the
# same location
with open("./python/lsst/sphgeom/version.py", "w") as f:
    print(f"""
__all__ = ("__version__", )
__version__ = '{version}'""", file=f)


# Find the source code -- we can combine it into a single module
pybind_src = sorted(glob.glob("python/lsst/sphgeom/*.cc"))
cpp_src = sorted(glob.glob("src/*.cc"))

# Very inefficient approach since this compiles the maing sphgeom
# library code for every extension rather than building everything once
ext_modules = [Pybind11Extension("lsst.sphgeom._sphgeom",
                                 sorted(cpp_src + pybind_src),
                                 include_dirs=["include"])]

setup(
    version=version,
    ext_modules=ext_modules,
    cmdclass={'build_ext': build_ext},
)
