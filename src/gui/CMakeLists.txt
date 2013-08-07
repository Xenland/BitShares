add_executable( sexc gui_main.cpp )
target_link_libraries( sexc Qt5::Widgets fc bshare ${BOOST_LIBRARIES} )
