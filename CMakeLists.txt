cmake_minimum_required(VERSION 3.20)

project(spigot-builder LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_subdirectory(app)