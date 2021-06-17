cmake_minimum_required(VERSION 3.0.0)
project(MiniSQL)

aux_source_directory(. DIR_SRCS)


add_executable(MiniSQL 
    ./main.cpp
    ./CatalogManager.cpp
    ./RecordManager.cpp)
