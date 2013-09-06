add_executable( cli_client main.cpp )
target_link_libraries( cli_client upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} )
