add_subdirectory(book)
add_subdirectory(order_entry)
add_executable(hft-engine main.cpp feed/itch_handler.cpp)
target_link_libraries(hft-engine PRIVATE book order_entry)