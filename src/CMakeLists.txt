cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libhimawari8_service")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "himawari8_service")
endif()

#########################################################################
#INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/noncopyable.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/baseinclude.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helpmethods.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helpmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/logging.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/curlservice.h
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/curlservice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/download_server.h
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/download_server.cpp
	)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/noncopyable.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/baseinclude.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helpmethods.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helpmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/logging.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.cpp
	)
	
SOURCE_GROUP(himawari8 FILES
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/curlservice.h
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/curlservice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/download_server.h
	${CMAKE_CURRENT_SOURCE_DIR}/himawari8/download_server.cpp
	)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

 if(WIN32)
	 ADD_DEFINITIONS(
		 -D_CRT_SECURE_NO_WARNINGS
	 )
 else()
	 ADD_DEFINITIONS(
		 -std=c++11
	 )
 endif()
#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)