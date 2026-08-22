cmake_minimum_required(VERSION 3.28)

project(infotecs_webservice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBPQXX REQUIRED IMPORTED_TARGET libpqxx)

add_executable(infotecs_webservice
    src/main.cpp
    src/db/journal_repository.cpp
    src/db/db_connection_pool.cpp
)
target_include_directories(infotecs_webservice PRIVATE src/
)

target_link_libraries(infotecs_webservice PRIVATE 
PkgConfig::LIBPQXX
    Threads::Threads)
