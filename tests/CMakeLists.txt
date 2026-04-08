include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(hft-tests placeholder.cpp test_order_book.cpp test_ouch_encoder.cpp test_passive_quoter.cpp)
target_link_libraries(hft-tests PRIVATE GTest::gtest_main book order_entry strategy)
gtest_discover_tests(hft-tests)