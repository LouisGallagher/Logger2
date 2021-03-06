cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(ZLIB REQUIRED)
find_package(Qt4 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost COMPONENTS date_time REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(LibRealSense REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LCM REQUIRED lcm)

file(GLOB lcmtypes lcmtypes/*.lcm)
execute_process(COMMAND lcm-gen -x ${lcmtypes} --cpp-hpath ${CMAKE_CURRENT_SOURCE_DIR}/lcmtypes/)

include(${QT_USE_FILE})
 
qt4_wrap_cpp(main_moc_SRCS
             GUI.h)

IF(UNIX)
    set(CMAKE_CXX_FLAGS "-msse2 -msse3 -pg -std=c++11")#-O3
ENDIF (UNIX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lcmtypes/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/RealSense/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/Include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LIBREALSENSE_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${LCM_INCLUDE_DIRS})


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ALL_DYN_LINK ON)   # force dynamic linking for all libraries

set(CMAKE_CXX_FLAGS "-msse2 -msse3 -std=c++11 -pg")#-O3

#Magic hack to check if host is 32 or 64 bit
if(APPLE)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2.dylib ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.dylib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFile.dylib ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.dylib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080.dylib ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.dylib COPYONLY)
elseif(UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2x64.so ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libDummyDevicex64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libDummyDevice.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFilex64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080x64.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.so COPYONLY)
    else()
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOpenNI2x86.so ${CMAKE_CURRENT_BINARY_DIR}/libOpenNI2.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libDummyDevicex86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libDummyDevice.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libOniFilex86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libOniFile.so COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/libPS1080x86.so ${CMAKE_CURRENT_BINARY_DIR}/OpenNI2/Drivers/libPS1080.so COPYONLY)
    endif()
endif()

set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,. -L${CMAKE_CURRENT_BINARY_DIR}")

set(srcs main.cpp
         MainController.cpp
         GUI.cpp
         Logger2.cpp
         Communicator.cpp
         Streamer.cpp
         VideoSourceFactory.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/OpenNI2/OpenNI2Interface.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/RealSense/RSInterface.cpp)

add_executable(Logger2 ${main_moc_SRCS} ${srcs})

target_link_libraries(Logger2                  
                      ${ZLIB_LIBRARY}
                      ${Boost_LIBRARIES}
                      ${OpenCV_LIBS} 
                      ${QT_LIBRARIES}
                      boost_system
                      boost_filesystem
                      boost_thread
                      OpenNI2
                      ${LIBREALSENSE_LIBRARIES}
                      ${LCM_LIBRARIES})

