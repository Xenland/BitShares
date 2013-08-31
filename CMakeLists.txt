project( bitshares )
cmake_minimum_required( VERSION 2.8.11 )

set( CMAKE_AUTOMOC ON )

set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";${CMAKE_SOURCE_DIR}/vendor/qt.5.1/5.1.0/msvc2012/lib/cmake/Qt5Widgets" )
find_package( Qt5Widgets )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/fc/CMakeModules" ${CMAKE_MODULE_PATH} )
FIND_PACKAGE( OpenSSL )
SET( ALL_OPENSSL_LIBRARIES
       ${OPENSSL_LIBRARIES}
       ${SSL_EAY_RELEASE}
       ${LIB_EAY_RELEASE}
   )

#include( VersionMacros )
#include( SetupTargetMacros )

IF( WIN32 )
  MESSAGE(status "Compiling on WIN32")
  ADD_DEFINITIONS( -DWIN32 )
  ADD_DEFINITIONS( -DNOMINMAX )
  #ADD_DEFINITIONS( -DBOOST_CONTEXT_NO_LIB )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  #ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB -DBOOST_THREAD_BUILD_LIB)
  ADD_DEFINITIONS( -DWIN32)
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -DLEVELDB_PLATFORM_WINDOWS )
  # Activate C++ exception handling
  IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")    
  ENDIF()
  #boost
  SET( Boost_INCLUDE_DIR $ENV{BOOST_ROOT} )
  #openssl
  set( OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openssl" )
  set( LIB_EAY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openssl/out32dll/libeay32.lib" )
  set( SSL_EAY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openssl/out32dll/ssleay32.lib" )
  include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openssl/include" )

  include_directories( ${CMAKE_SOURCE_DIR}/vendor/leveldb-win/include )
  add_subdirectory( vendor/leveldb-win )
ELSE(WIN32)
  include_directories( ${CMAKE_SOURCE_DIR}/vendor/leveldb-1.12.0/include )
  add_subdirectory( vendor/leveldb-1.12.0 )

  MESSAGE(status "Compiling on UNIX")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs -fmax-errors=3 -msse4.2" ) 
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs -fmax-errors=3 -msse4.2" )
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs" ) 
  #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs" )
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context coroutine) 

 IF( NOT APPLE )
      SET( pthread pthread )
 ENDIF( NOT APPLE )
ENDIF(WIN32)



include_directories( ${CMAKE_SOURCE_DIR}/fc/include )
include_directories( ${CMAKE_SOURCE_DIR}/include )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/SFMT-src-1.4 )
include_directories( ${CMAKE_SOURCE_DIR}/vendor/miniupnp )
include_directories( ${Boost_INCLUDE_DIR} )
#need this for boost::process (not an official boost library yet)
include_directories( ${CMAKE_SOURCE_DIR}/fc/vendor/boost_1.51/include )

   
#link_directories( ${Boost_LIBRARY_DIRS} )

add_subdirectory( fc )
add_subdirectory( vendor/miniupnp/miniupnpc )
#add_subdirectory( vendor/libzerocoin )
set( BOOST_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_CHRONO_LIBRARY} ${ALL_OPENSSL_LIBRARIES} ${Boost_COROUTINE_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${pthread})



set( sources 
     src/network/stcp_socket.cpp
     src/network/connection.cpp
     src/network/server.cpp
     src/network/get_public_ip.cpp
     src/network/upnp.cpp

     src/peer/peer_channel.cpp
     src/peer/peer_messages.cpp

     src/bitname/bitname_block.cpp
     src/bitname/bitname_hash.cpp
     src/bitname/bitname_miner.cpp
     src/bitname/bitname_db.cpp
     src/bitname/bitname_fork_db.cpp
     src/bitname/bitname_messages.cpp
     src/bitname/bitname_channel.cpp
     src/bitname/bitname_client.cpp
     src/bitname/bitname_record.cpp

     src/bitchat/bitchat_private_message.cpp
     src/bitchat/bitchat_messages.cpp
     src/bitchat/bitchat_channel.cpp
     src/bitchat/bitchat_client.cpp 
     src/bitchat/bitchat_message_cache.cpp

     src/addressbook/addressbook.cpp 

     src/blockchain/asset.cpp
     src/blockchain/block.cpp
     src/blockchain/transaction.cpp
     src/blockchain/trx_validation_state.cpp
     src/blockchain/blockchain_outputs.cpp
     src/blockchain/blockchain_db.cpp
     src/blockchain/blockchain_market_db.cpp
     src/blockchain/blockchain_printer.cpp
     src/blockchain/blockchain_messages.cpp
     src/blockchain/blockchain_channel.cpp
     src/blockchain/blockchain_client.cpp
     src/blockchain/blockchain_time_keeper.cpp

     src/rpc/rpc_server.cpp

     src/application.cpp
     src/difficulty.cpp 
     src/profile.cpp
     src/extended_address.cpp
     src/small_hash.cpp
     src/merkle_tree.cpp
     src/address.cpp
     src/wallet.cpp
     src/keychain.cpp
     src/wallet_cache.cpp
     src/bitmessage.cpp

     vendor/SFMT-src-1.4/SFMT.c
     src/proof_of_work.cpp )

add_library( bshare ${sources} )

add_subdirectory( bitchat )
IF( NOT Qt5Widgets_DIR MATCHES Qt5Widgets_DIR-NOTFOUND )
add_subdirectory( src/gui )
ENDIF()


#add_executable( wallet_test tests/wallet_test.cpp )
#target_link_libraries( wallet_test bshare fc ${BOOST_LIBRARIES})
#setup_library( bshare SOURCES ${sources} LIBRARY_TYPE STATIC )

#add_executable( unit_tests tests/unit_tests.cpp )
#target_link_libraries( unit_tests bshare fc ${BOOST_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}  )

#add_executable( bitshares src/main.cpp )
#target_link_libraries( bitshares bshare fc ${BOOST_LIBRARIES})

#add_executable( bitchatc bitchat/main.cpp )
#target_link_libraries( bitchatc upnpc-static bshare fc ${BOOST_LIBRARIES} upnpc-static )

#add_executable( bitshared bitshared.cpp )
#target_link_libraries( bitshared bshare fc ${BOOST_LIBRARIES} )

add_subdirectory( tests )
