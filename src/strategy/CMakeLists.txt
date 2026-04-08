add_library(strategy STATIC passive_quoter.cpp)
target_include_directories(strategy PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(strategy PUBLIC book order_entry)
