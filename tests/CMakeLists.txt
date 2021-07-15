enable_testing()

include_directories("${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

macro(duck_add_test TESTNAME)
  add_executable(${TESTNAME} ${ARGN})
  target_compile_options(${TESTNAME} PUBLIC -Werror -Wall -Wextra -pedantic)
  target_link_libraries(
    ${TESTNAME}
    gtest
    gtest_main
    gmock
    gmock_main
    ${mylib_name}
    )
endmacro()  

duck_add_test(queue_test queue_test.cc)
duck_add_test(tree_test tree_test.cc)
duck_add_test(buffer_test buffer_test.cc)
