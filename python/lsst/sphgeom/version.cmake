execute_process(
    COMMAND git describe --all --always --dirty
    OUTPUT_VARIABLE GIT_REV
    ERROR_QUIET
)

if("${GIT_REV}" STREQUAL "")
    set(GIT_REV "unknown")
endif()

string(STRIP ${GIT_REV} GIT_REV)
string(REGEX REPLACE "^(heads|tags)/" "" GIT_REV "${GIT_REV}")
string(REPLACE "/" "_" GIT_REV "${GIT_REV}")

set(VERSION_PY "__version__ = '${GIT_REV}'
__all__ = ('__version__',)
")

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/version.py)
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/version.py VERSION_PY_)
else()
    set(VERSION_PY_ "")
endif()

if(NOT "${VERSION_PY}" STREQUAL "${VERSION_PY_}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.py "${VERSION_PY}")
endif()
