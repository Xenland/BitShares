
set( sources main.cpp )
add_executable( bitchat ${sources} )
target_link_libraries( bitchat bshare fc leveldb ${BOOST_LIBRARIES} )
