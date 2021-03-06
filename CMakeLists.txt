cmake_minimum_required(VERSION 2.6)

include(CheckIncludeFileCXX)

include_directories(BEFORE "${CMAKE_BINARY_DIR}")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/utils/qxt")

project(wt-comments)

set(PROJECT_NAME "Wt Comments")
set(PROJECT_SHORT_NAME "WtComments")
set(PROJECT_URL "http://mupuf.org/project/wt-comments/")
set(PROJECT_AUTHORS "Martin Peres")
set(PROJECT_ORGANIZATION "MuPuF.org")
set(PROJECT_LICENSE "GPLv2 or later")
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_STATE "alpha")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}-${PROJECT_VERSION_STATE}")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DPROJECT_SHORT_NAME="${PROJECT_SHORT_NAME}")
add_definitions(-DPROJECT_URL="${PROJECT_URL}")
add_definitions(-DPROJECT_AUTHORS="${PROJECT_AUTHORS}")
add_definitions(-DPROJECT_ORGANIZATION="${PROJECT_ORGANIZATION}")
add_definitions(-DPROJECT_LICENSE="${PROJECT_LICENSE}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

# Needed from cmake version 3.0
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/share/cmake-2.8/Modules/")

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

# Boost
option(BOOST_STATIC_LINK "Use boost static linkage" OFF)
SET(Boost_ADDITIONAL_VERSIONS
  "1.35" "1.36" "1.37" "1.38" "1.39"
  "1.40" "1.41" "1.42" "1.43" "1.44" "1.45" "1.46" "1.47" "1.48" "1.49"
  "1.50" "1.51" "1.52" "1.53" "1.54" "1.55" "1.56" "1.57" "1.58" "1.59")
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ${BOOST_STATIC_LINK})
find_package(Boost 1.35 COMPONENTS signals thread system REQUIRED)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

# PThread
find_package (Threads REQUIRED)

# Wt (I would advise to compile a minimal Wt to lower dependencies)
include(FindWt REQUIRED)
include_directories(${Wt_INCLUDE_DIR})

# Select the connector (FCGI by default)
option(USE_CON_HTTP "Build wt-comments with an HTTP connector instead of an FCGI one" OFF)
if (USE_CON_HTTP)
	SET (Wt_CONNECTOR "${Wt_HTTP_LIBRARY}")
else (USE_CON_HTTP)
	SET (Wt_CONNECTOR "${Wt_FCGI_LIBRARY}")
endif (USE_CON_HTTP)

# Use a simple TextArea for comment edition
option(USE_SIMPLE_EDITOR "Use a simple text editor instead of TinyMCE" OFF)
if (USE_SIMPLE_EDITOR)
	add_definitions("-DUSE_SIMPLE_EDITOR")
endif (USE_SIMPLE_EDITOR)

# Select the connector (FCGI by default)
option(SEND_EMAIL "Email support in Wt-comments" OFF)
if (SEND_EMAIL)
	find_package(PkgConfig)
	pkg_check_modules(LIBCURL libcurl)
	if (LIBCURL_FOUND)
		add_definitions("-DSEND_EMAIL")
		include_directories(${LIBCURL_INCLUDE_DIR})
	else (LIBCURL_FOUND)
		message ("Libcurl was not found. Disable SEND_MAIL\n")
		set(SEND_EMAIL "OFF")
	endif (LIBCURL_FOUND)
endif (SEND_EMAIL)

# Output binary
file(GLOB wtcomments_SOURCES "*.cpp" "*.h")
add_executable(wtcomments ${wtcomments_SOURCES})
target_link_libraries(wtcomments ${Wt_LIBRARY} ${Wt_CONNECTOR} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBCURL_LDFLAGS})

message("")
message ("-- Summary --")
message("	Build type                               : ${CMAKE_BUILD_TYPE} (-DCMAKE_BUILD_TYPE)")
message("	Boost static linkage                     : ${Boost_USE_STATIC_LIBS} (-DBOOST_STATIC_LINK)")
message("")
message("	Options:")
message("	Wt connector, use HTTP instead of FCGI   : ${USE_CON_HTTP} (-DUSE_CON_HTTP)")
message("	Using a simple editor                    : ${USE_SIMPLE_EDITOR} (-DUSE_SIMPLE_EDITOR)")
message("	Send emails when receiving a new comment : ${SEND_EMAIL} (-DSEND_EMAIL)")
message ("")
