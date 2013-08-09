add_executable( blockchain_tests blockchain_tests.cpp )
target_link_libraries( blockchain_tests bshare fc leveldb ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  )

add_executable( pow_test pow_test.cpp )
target_link_libraries( pow_test bshare fc ${BOOST_LIBRARIES})
