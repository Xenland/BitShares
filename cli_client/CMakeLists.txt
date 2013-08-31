add_executable( cli_client main.cpp )
target_link_libraries( cli_client bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} )
