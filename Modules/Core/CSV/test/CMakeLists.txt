add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv PRIVATE TradingEngine::Core::CSV gtest_main gmock)
add_test(NAME Core.CSV.Parser COMMAND test_csv)
