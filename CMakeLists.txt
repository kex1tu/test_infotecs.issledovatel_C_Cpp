cmake_minimum_required(VERSION 3.28)

project(infotecs_webservice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system json)

pkg_check_modules(LIBPQXX REQUIRED IMPORTED_TARGET libpqxx)

add_executable(infotecs_webservice
    src/main.cpp
    src/db/journal_repository.cpp
    src/db/db_connection_pool.cpp
    src/http/url_utils.cpp
    src/http/json_serializer.cpp
    src/http/http_session.cpp
    src/http/http_server.cpp
)

target_include_directories(infotecs_webservice PRIVATE 
    src/
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(infotecs_webservice PRIVATE 
    PkgConfig::LIBPQXX
    Threads::Threads
    Boost::system
    Boost::json
)


enable_testing()
add_subdirectory(tests)


