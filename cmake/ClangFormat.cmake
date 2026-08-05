find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-18 clang-format-17)

if(CLANG_FORMAT_EXE)
  file(
    GLOB_RECURSE ALL_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/App/*.cpp" "${CMAKE_SOURCE_DIR}/App/*.hpp"
    "${CMAKE_SOURCE_DIR}/Modules/*.cpp" "${CMAKE_SOURCE_DIR}/Modules/*.hpp")

  add_custom_target(
    format
    COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCES}
    COMMENT "Running clang-format on source files"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  if(NOT (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR))
    add_custom_target(
      check_format
      COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_SOURCES}
      COMMENT "Checking clang-format compliance"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif()
else()
  message(WARNING "clang-format not found, format/check_format targets disabled")
endif()
