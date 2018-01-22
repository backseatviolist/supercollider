set(tests
    plugins_test.cpp
)

foreach(test ${tests})
    string(REPLACE .cpp "" test_name ${test})
    add_executable(${test_name} ${test})
    target_include_directories(
        ${test_name}
        PUBLIC ${CMAKE_SOURCE_DIR}/common
        PUBLIC ${CMAKE_SOURCE_DIR}/include/common
        PUBLIC ${CMAKE_SOURCE_DIR}/include/server
        PUBLIC ${CMAKE_SOURCE_DIR}/include/plugin_interface
        PUBLIC ${CMAKE_SOURCE_DIR}/server/scsynth
    )
    target_link_libraries(${test_name} libscsynth boost_test)
    if (PTHREADS_FOUND)
        target_link_libraries(${test_name} ${PTHREADS_LIBRARIES})
    endif()
    add_test(${test_name}_run ${EXECUTABLE_OUTPUT_PATH}/${test_name})
endforeach(test)
