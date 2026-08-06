add_executable(${PROJECT_NAME} main.cpp)

if(CLANG_FORMAT_EXE)
  add_dependencies(${PROJECT_NAME} format)
endif()
