cmake_minimum_required(VERSION 3.15)
project(tgbot_new)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Подключаем tgbot-cpp из папки external
add_subdirectory(external/tgbot-cpp)

# Подключаем SQLite через vcpkg (если используешь)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(tgbot_new ${SOURCES})

target_link_libraries(tgbot_new
    PRIVATE
        TgBot          # библиотека из external/tgbot-cpp
        SQLite::SQLite3
        SQLiteCpp
)

