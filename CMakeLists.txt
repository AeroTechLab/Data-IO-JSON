cmake_minimum_required( VERSION 2.8 )
project( Data-IO-JSON )

set( CMAKE_C_STANDARD 99 )
set( CMAKE_C_STANDARD_REQUIRED ON )

set( LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "Relative or absolute path to directory where built shared libraries will be placed" )

include( ${CMAKE_CURRENT_LIST_DIR}/json/CMakeLists.txt )

add_library( DataIOJSON SHARED ${CMAKE_CURRENT_LIST_DIR}/data_io_json.c )
set_target_properties( DataIOJSON PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR} )
target_include_directories( DataIOJSON PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/interface )
target_compile_definitions( DataIOJSON PUBLIC -DDEBUG )
target_link_libraries( DataIOJSON SimpleJSON )
