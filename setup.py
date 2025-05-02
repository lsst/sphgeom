"""
Basic setuptools description.

This is not a complete definition.

* Version number is not correct.
* The shared library and include files are not installed.  This makes it
  unusable with other python packages that directly reference the C++
  interface.
"""

from skbuild import setup

setup(
    cmake_source_dir=".",
    cmake_install_dir=".",
)
