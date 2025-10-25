cmake_minimum_required(VERSION 3.15)
project(guess_tcp C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_definitions(WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0600)

add_executable(guess_server src/server.c)
add_executable(guess_client src/client.c)

target_link_libraries(guess_server ws2_32)
target_link_libraries(guess_client ws2_32)
