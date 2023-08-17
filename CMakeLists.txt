cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


project(elevationmanager)

include_directories(deps)

message("Configuring elevationmanager library...")
add_library(elevationmanager STATIC)
target_include_directories(elevationmanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_sources(elevationmanager PRIVATE src/curlutil.cpp)
target_sources(elevationmanager PRIVATE src/elevationcache.cpp)
target_sources(elevationmanager PRIVATE src/elevationdata.cpp)
target_sources(elevationmanager PRIVATE src/elevationexception.cpp)
target_sources(elevationmanager PRIVATE src/elevationdownloader.cpp)
target_sources(elevationmanager PRIVATE src/elevationio.cpp)
target_sources(elevationmanager PRIVATE src/elevationmanager.cpp)
target_sources(elevationmanager PRIVATE src/elevationregion.cpp)
target_sources(elevationmanager PRIVATE src/elevationutils.cpp)
target_sources(elevationmanager PRIVATE src/elevationvendor.cpp)
target_sources(elevationmanager PRIVATE src/elevationvendor_impl.cpp)

# ADD CURL LIBRARY
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(elevationmanager ${CURL_LIBRARIES})


### DEMO
message("Configuring demo application...")
add_executable(demo demo.cpp)
target_link_libraries(demo elevationmanager)
target_include_directories(demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

if(EXISTS ${ROOT}/sandbox.cpp)
	### SANDBOX
	message("Configuring sandbox application...")
	add_executable(sandbox sandbox.cpp)
	target_link_libraries(sandbox elevationmanager)
target_include_directories(sandbox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
endif()

#install(TARGETS elevationmanager RUNTIME DESTINATION bin)
