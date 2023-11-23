cmake_minimum_required(VERSION 3.1)

IF(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
ENDIF()

project(producer_consumer_problem_using_circular_buffer)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/plibsys)

add_executable(pcp_using_loop_array
               ${CMAKE_CURRENT_SOURCE_DIR}/producer_consumer_problem_using_circular_buffer.c
               ${CMAKE_CURRENT_SOURCE_DIR}/lib/sensor.c
               ${CMAKE_CURRENT_SOURCE_DIR}/lib/queue_using_array.c)

target_link_libraries(pcp_using_loop_array
                      plibsys)

target_include_directories(pcp_using_loop_array
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_executable(pcp_using_loop_linked_list
               ${CMAKE_CURRENT_SOURCE_DIR}/producer_consumer_problem_using_circular_buffer.c
               ${CMAKE_CURRENT_SOURCE_DIR}/lib/sensor.c
               ${CMAKE_CURRENT_SOURCE_DIR}/lib/queue_using_linked_list.c)

target_link_libraries(pcp_using_loop_linked_list
                      plibsys)

target_include_directories(pcp_using_loop_linked_list
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib)
