set(tests
    plugins_test.cpp
)

foreach(test ${tests})
  string(REPLACE .cpp "" test_name ${test})
  add_executable(${test_name} ${test})
  target_link_libraries(${test_name} libscsynth boost_test)
  add_test(${test_name}_run ${EXECUTABLE_OUTPUT_PATH}/${test_name})
endforeach(test)
