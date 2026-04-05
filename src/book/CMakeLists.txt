add_library(book STATIC order_book.cpp book_manager.cpp)
target_include_directories(book PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/..)