add_executable( blockchain_tests blockchain_tests.cpp )
if (WIN32)
  target_link_libraries( blockchain_tests bshare fc leveldb ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} WS2_32 WSock32 )
else (WIN32)
  target_link_libraries( blockchain_tests bshare fc leveldb ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  )
endif(WIN32)

add_executable( pow_test pow_test.cpp )
target_link_libraries( pow_test bshare fc ${BOOST_LIBRARIES})

add_executable( timekeeper timekeeper.cpp )
target_link_libraries( timekeeper bshare fc ${BOOST_LIBRARIES})
