cmake_minimum_required(VERSION 3.12)
project(MultiThreadedCrawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(crawler
  src/main.cpp
)

target_include_directories(crawler PRIVATE src)
target_link_libraries(crawler PRIVATE CURL::libcurl)