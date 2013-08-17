if (WIN32)
  set( PLATFORM_SPECIFIC_LIBS WS2_32  )
else (WIN32)
  set( PLATFORM_SPECIFIC_LIBS )
endif(WIN32)

add_executable( blockchain_tests blockchain_tests.cpp )
target_link_libraries( blockchain_tests bshare fc leveldb ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${PLATFORM_SPECIFIC_LIBS} )

add_executable( pow_test pow_test.cpp )
target_link_libraries( pow_test bshare fc ${BOOST_LIBRARIES})

add_executable( timekeeper timekeeper.cpp )
target_link_libraries( timekeeper bshare fc ${BOOST_LIBRARIES}  ${PLATFORM_SPECIFIC_LIBS} )
