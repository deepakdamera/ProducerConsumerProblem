cmake_minimum_required(VERSION 3.12)
project(producerConsumerReal C)

set(CMAKE_C_STANDARD 11)

add_executable(producerConsumerReal main.c)
target_link_libraries(producerConsumerReal pthread)