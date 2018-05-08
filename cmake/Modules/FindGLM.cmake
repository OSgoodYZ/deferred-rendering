set(_glm_HEADER_SEARCH_DIRS
    "/usr/include"
    "/usr/local/include"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    "C:/Program Files (x86)/glm")

set(_glm_ENV_ROOT_DIR "$ENV{GLM_ROOT_DIR}")
if (NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)
    set(GLM_ROOT_DIR "${_glm_ENV_ROOT_DIR}")
endif()

if (GLM_ROOT_DIR)
    set(_glm_HEADERS_SEARCH_DIRS
        "${GLM_ROOT_DIR}"
        "${GLM_ROOT_DIR}/include"
        "${_glm_HEADER_SEARCH_DIRS}")
endif()

find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${_glm_HEADER_SEARCH_DIRS})
find_package(PkgConfig REQUIRED)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

if (GLM_FOUND)
    set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
endif()
