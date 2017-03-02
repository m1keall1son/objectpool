cmake_minimum_required (VERSION 2.6)
project (objectpool)

add_subdirectory(src)
add_subdirectory(src/test)

enable_testing()
add_test( NAME UnitTest COMMAND Catch )