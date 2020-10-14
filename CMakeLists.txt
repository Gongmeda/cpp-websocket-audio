cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project ("cpp-ws-echo-server" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (Boost 1.74 REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads)

include_directories (
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${SERVER_PATH})

link_directories (
    ${Boost_LIBRARY_DIRS})

set (SERVER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/server")

file (GLOB_RECURSE SERVER_SOURCE_FILES "${SERVER_PATH}/*.cpp")

add_executable (${CMAKE_PROJECT_NAME}
    ${SERVER_SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME}
    Threads::Threads
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES})