set(MOOS_GIT_VERSION "not known")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  find_package(Git)

  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --abbrev --dirty --always
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE "MOOS_GIT_VERSION"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
endif()


message( STATUS "Git version: ${MOOS_GIT_VERSION}" )

configure_file(${CMAKE_MODULE_PATH}/MOOSVersion.h.in
    ${CMAKE_BINARY_DIR}/Core/libMOOS/include/MOOS/libMOOS/MOOSVersion.h
    @ONLY
)
