
add_executable(test_thread_safe_queue
    test_thread_safe_queue.cpp
)
target_include_directories(test_thread_safe_queue PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(test_thread_safe_queue PRIVATE Threads::Threads)
add_test(NAME ThreadSafeQueueTest COMMAND test_thread_safe_queue)


add_executable(test_url_utils
    test_url_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/http/url_utils.cpp
)
target_include_directories(test_url_utils PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
add_test(NAME UrlUtilsTest COMMAND test_url_utils)


add_executable(test_json_serializer
    test_json_serializer.cpp
    ${CMAKE_SOURCE_DIR}/src/http/json_serializer.cpp
)
target_include_directories(test_json_serializer PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(test_json_serializer PRIVATE Boost::json)
add_test(NAME JsonSerializerTest COMMAND test_json_serializer)


add_executable(test_journal_repository
    test_journal_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/db/journal_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/db/db_connection_pool.cpp
)
target_include_directories(test_journal_repository PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(test_journal_repository PRIVATE PkgConfig::LIBPQXX Threads::Threads)
add_test(NAME JournalRepositoryTest COMMAND test_journal_repository)
