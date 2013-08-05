project( bitshares )
cmake_minimum_required( VERSION 2.8.0 )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/fc/CMakeModules;${CMAKE_MODULE_PATH}" )
FIND_PACKAGE( OpenSSL )
SET( ALL_OPENSSL_LIBRARIES
       ${OPENSSL_LIBRARIES}
       ${SSL_EAY_RELEASE}
       ${LIB_EAY_RELEASE}
   )

include( VersionMacros )
include( SetupTargetMacros )

IF( WIN32 )
    ADD_DEFINITIONS( -DWIN32 )
	ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
	ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB -DBOOST_THREAD_BUILD_LIB)
	ADD_DEFINITIONS( -DWIN32)
	# Activate C++ exception handling
	IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")    
	ENDIF()
ELSE(WIN32)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs -fmax-errors=3 " ) 
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs -fmax-errors=3 " )
ENDIF(WIN32)


set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.51 COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context ) 

include_directories( ${CMAKE_SOURCE_DIR}/fc/include )
include_directories( ${CMAKE_SOURCE_DIR}/include )
include_directories( ${CMAKE_SOURCE_DIR}/fc/vendor/boost_1.51/include )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/SFMT-src-1.4 )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/leveldb-1.12.0/include )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/miniupnp )
include_directories( ${Boost_INCLUDE_DIR} )
   
#link_directories( ${Boost_LIBRARY_DIRS} )

add_subdirectory( fc )
add_subdirectory( vendor/leveldb-1.12.0 )
add_subdirectory( vendor/miniupnp/miniupnpc )
#add_subdirectory( vendor/libzerocoin )
set( BOOST_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_CHRONO_LIBRARY} ${ALL_OPENSSL_LIBRARIES})


set( sources 
     src/network/stcp_socket.cpp
     src/network/connection.cpp
     src/network/server.cpp
     src/network/get_public_ip.cpp
     src/network/upnp.cpp

     src/peer/peer_channel.cpp
     src/peer/peer_messages.cpp

     src/bitname/name_miner.cpp
     src/bitname/name_block.cpp
     src/bitname/name_db.cpp
     src/bitname/name_channel.cpp
     src/bitname/name_hash.cpp

     src/bitchat/bitchat_private_message.cpp
     src/bitchat/bitchat_messages.cpp
     src/bitchat/bitchat_channel.cpp
     src/bitchat/bitchat_client.cpp 

     src/blockchain/asset.cpp
     src/blockchain/block.cpp
     src/blockchain/transaction.cpp
     src/blockchain/trx_validation_state.cpp
     src/blockchain/blockchain_db.cpp
     src/blockchain/blockchain_printer.cpp

     src/db.cpp
     src/small_hash.cpp
     src/mini_pow.cpp 
     src/merkle_tree.cpp
     src/address.cpp
     src/wallet.cpp
     src/bitmessage.cpp

     vendor/SFMT-src-1.4/SFMT.c
     src/proof_of_work.cpp )

add_library( bshare ${sources} )

add_subdirectory( bitchat )


#add_executable( wallet_test tests/wallet_test.cpp )
#target_link_libraries( wallet_test bshare fc ${BOOST_LIBRARIES})
#setup_library( bshare SOURCES ${sources} LIBRARY_TYPE STATIC )

#add_executable( unit_tests tests/unit_tests.cpp )
#target_link_libraries( unit_tests bshare fc ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  )

#add_executable( bitshares src/main.cpp )
#target_link_libraries( bitshares bshare fc ${BOOST_LIBRARIES})

#add_executable( bitchatc bitchat/main.cpp )
#target_link_libraries( bitchatc bshare fc ${BOOST_LIBRARIES} upnpc-static )

#add_executable( bitshared bitshared.cpp )
#target_link_libraries( bitshared bshare fc ${BOOST_LIBRARIES} )

add_subdirectory( tests )
