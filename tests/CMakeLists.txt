if(ENABLE_TESTING)
    include_directories(
        ${ArffFiles_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    set(TEST_ARFFILES "unit_tests_arffFiles")
    add_executable(${TEST_ARFFILES} TestArffFiles.cc)
    target_link_libraries(${TEST_ARFFILES} PUBLIC Catch2::Catch2WithMain)
    add_test(NAME ${TEST_ARFFILES} COMMAND ${TEST_ARFFILES})
endif(ENABLE_TESTING)
