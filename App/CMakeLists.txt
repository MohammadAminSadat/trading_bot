add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC TradingEngine::CLI::REPL)

if(CLANG_FORMAT_EXE)
  add_dependencies(${PROJECT_NAME} format)
endif()
