cmake_minimum_required(VERSION 3.23)
project(c_webserver_kqueue C)

set(CMAKE_C_STANDARD 11)

add_executable(c_webserver_kqueue main.c str.c str.h)
