cmake_minimum_required(VERSION 2.8)

project(dovo)

set(VERSION "1.0.4")

add_definitions(-DDOVO_VERSION="${VERSION}")

if(WIN32)
	#VC++ stuff
	add_definitions(-D_UNICODE)
	add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd -DDEBUG")
endif()

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)
find_package(Boost REQUIRED thread filesystem system)

set(wxWidgets_USE_STATIC   ON)
#define this so that wxWidgets doesn't define it. Conflicts w/ dcmtk's declaration
add_definitions(-DHAVE_SSIZE_T)
find_package(wxWidgets COMPONENTS core base adv REQUIRED)
include(${wxWidgets_USE_FILE})

#enable dcmtk filename to have wide version, only affects WIN32.  http://utf8everywhere.org/
add_definitions(-DDCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS=1)
find_package(dcmtk REQUIRED CONFIG HINTS ${DCMTK_DIR})

#include zlib
find_package(zlib REQUIRED ${ZLIB_ROOT})

#include openjpeg
find_package(openjpeg REQUIRED HINTS ${OPENJPEG})
add_definitions(-DOPJ_STATIC)

#include fmjpeg2k
find_package(fmjpeg2k REQUIRED HINTS ${FMJPEG2K})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/sqlite3 ${Boost_INCLUDE_DIRS} ${FMJPEG2K_INCLUDE_DIRS} ${DCMTK_INCLUDE_DIRS} )
LINK_DIRECTORIES(${FMJPEG2K}/lib ${Boost_LIBRARY_DIRS})

set(dovo_HEADERS
	src/gui.h
	src/dovo_about.h
	src/dovo_changePatientInfo.h
	src/dovo_destination.h
	src/dovo_mainFrame.h
	src/dovo_searchStatus.h
	src/dovo_sendStatus.h
	src/dovo_changePatientInfo.h
	sqlite3/sqlite3.h
	src/engine.h
	src/destinationentry.h
	src/dicomscanner.h
	src/dicomsender.h
	src/dcm2img.h
	src/update.h
	src/alphanum.hpp
)

set(dovo_SRCS
  ${dovo_HEADERS}
	src/main.cpp
	src/gui.cpp
	src/dovo_about.cpp
	src/dovo_changePatientInfo.cpp
	src/dovo_destination.cpp
	src/dovo_mainFrame.cpp
	src/dovo_searchStatus.cpp
	src/dovo_sendStatus.cpp
	src/dovo_changePatientInfo.cpp
	sqlite3/sqlite3.c
	sqlite3/sqlite3_exec_stmt.c
	src/engine.cpp
	src/dicomscanner.cpp
	src/dicomsender.cpp
	src/dcm2img.cpp
	src/update.cpp
)

if(WIN32)
  set(dovo_SRCS ${dovo_SRCS} src/win/dovo.rc)
endif(WIN32)

if(APPLE)
  set(dovo_SRCS ${dovo_SRCS} src/osx/dovo.icns)
endif(APPLE)

ADD_EXECUTABLE(dovo WIN32 MACOSX_BUNDLE ${dovo_SRCS})

IF(WIN32)
  SET(OS_SPECIFIC_LIBS netapi32 wsock32 odbc32 Rpcrt4)
ELSE(WIN32)
  SET(OS_SPECIFIC_LIBS )
ENDIF(WIN32)

if(APPLE)
  set_target_properties(dovo PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/src/osx/Info.plist)
  set_target_properties(dovo PROPERTIES MACOSX_BUNDLE_ICON_FILE dovo)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/osx/dovo.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF()

TARGET_LINK_LIBRARIES(dovo
	${FMJPEG2K_LIBRARIES}
	${OPENJPEG_LIBRARIES}
	${ZLIB_LIBRARIES}
	${DCMTK_LIBRARIES}
	${wxWidgets_LIBRARIES}
	${Boost_LIBRARIES}
	${OS_SPECIFIC_LIBS}
	)
