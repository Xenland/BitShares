
set( sources main.cpp )
add_executable( bitchat ${sources} )
target_link_libraries( bitchat upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} )
