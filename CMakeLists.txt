cmake_minimum_required(VERSION 4.0)
project(gsim LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(filelist utilities/CommandLineInterface.cpp
             utilities/FileHandler.cpp
             utilities/Utilities.cpp
             verilog/Verilog.cpp
             verilog/VerilogUtilities.cpp
             main.cpp
    )



add_executable(${PROJECT_NAME} ${filelist})
target_include_directories(${PROJECT_NAME} PUBLIC utilities verilog)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)



