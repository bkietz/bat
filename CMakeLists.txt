cmake_minimum_required(VERSION 3.22)
project(BatTests) # (host side)

include(CTest)

foreach(NAME ButtonSequenceTest GestureTest)
  add_executable(${NAME} "test/${NAME}.cpp")
  target_include_directories(${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}")
  add_test(NAME "do_${NAME}" COMMAND ${NAME})
endforeach()
