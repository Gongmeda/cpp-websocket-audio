cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project ("cpp-ws-echo-server" LANGUAGES CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif ()

find_package (Boost 1.74 REQUIRED)
find_package (OpenSSL REQUIRED)
find_package(Threads)

add_compile_definitions (
    BOOST_DATE_TIME_NO_LIB
    BOOST_REGEX_NO_LIB)

set (SERVER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/server")

file (GLOB_RECURSE SERVER_SOURCE_FILES "${SERVER_PATH}/*.cpp")
file (GLOB_RECURSE SERVER_HEADER_FILES "${SERVER_PATH}/*.hpp")

include_directories (
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${SERVER_PATH})

link_directories (${Boost_LIBRARY_DIRS})

target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES})

add_executable (${PROJECT_NAME}
    ${SERVER_HEADER_FILES}
    ${SERVER_SOURCE_FILES})

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/server/listner.cpp)