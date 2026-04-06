add_library(order_entry STATIC ouch_entry_session.cpp)
target_include_directories(order_entry PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
