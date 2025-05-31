cmake_minimum_required(VERSION 3.10)
project(GitHubSearcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGIT2 REQUIRED libgit2)

add_executable(github-searcher
    master/main.cpp
    master/curl_downloader.cpp
)

add_executable(github-searcher-cli
    master/alternative_main/main_cli.cpp
    master/curl_downloader.cpp
)

target_include_directories(github-searcher PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${LIBGIT2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/master
)

target_include_directories(github-searcher-cli PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${LIBGIT2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/master
)

target_link_libraries(github-searcher
    ${CURL_LIBRARIES}
    ${LIBGIT2_LIBRARIES}
)

target_link_libraries(github-searcher-cli
    ${CURL_LIBRARIES}
    ${LIBGIT2_LIBRARIES}
)

target_include_directories(github-searcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/master)
target_include_directories(github-searcher-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/master)