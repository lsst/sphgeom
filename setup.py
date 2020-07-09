from setuptools import setup
from setuptools_cpp import ExtensionBuilder, Pybind11Extension
import os
import glob

# Importing this automatically enables parallelized builds
import numpy.distutils.ccompiler  # noqa: F401

# Currently a fake version for testing
version = '0.0.1'

# To enable consistency with sconsUtils we write the version to the
# same location
with open("./python/lsst/sphgeom/version.py", "w") as f:
    print(f"""
__all__ = ("__version__", )
__version__ = '{version}'""", file=f)


# read the contents of the README file
this_directory = os.path.abspath(os.path.dirname(__file__))
with open(os.path.join(this_directory, "README.md"), encoding='utf-8') as f:
    long_description = f.read()

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
    long_description=long_description,
    ext_modules=ext_modules,
    long_description_content_type="text/markdown",
    cmdclass={'build_ext': ExtensionBuilder},
)
