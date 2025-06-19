cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0167 NEW)

project(tgbot_new)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(external/tgbot-cpp)

find_package(SQLiteCpp REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(tgbot_new ${SOURCES})

target_link_libraries(tgbot_new
    PRIVATE
        TgBot
        SQLiteCpp
)
