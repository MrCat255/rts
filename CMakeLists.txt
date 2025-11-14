cmake_minimum_required(VERSION 3.15)
project(LabWorks CXX)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поддиректории с лабораторными
add_subdirectory(lab1)
add_subdirectory(lab2)
