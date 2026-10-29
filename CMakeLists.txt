cmake_minimum_required(VERSION 3.16)
project(terminal-emulator C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(glad libs/glad/src/gl.c)
target_include_directories(glad PUBLIC libs/glad/include)

find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(cglm REQUIRED)

add_executable(terminal-emulator src/main.c)
target_link_libraries(terminal-emulator PRIVATE 
	glad glfw Freetype::Freetype cglm
	${CMAKE_DL_LIBS})

