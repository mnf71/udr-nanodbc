###############################################################################
#
# This file is CMake configuration for NANOUDR library
#
###############################################################################
# General settings
############

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project (NANOUDR)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

###############################################################################
# CMake standard options
############

option(BUILD_SHARED_LIBS "Build shared library" ON)

###############################################################################
# Specific options
############

option(NANOUDR_ENABLE_LTO "Enable link time optimization" OFF)
option(NANOUDR_ENABLE_ASAN "Enable address sanitizer on GCC v4.8+/Clang v 3.1+" OFF)
option(NANOUDR_HIDING_VISIBILITY "Enable hiding private symbol using ELF visibility" ON)
option(NANOUDR_DISABLE_LIBCXX "Do not use libc++, if available" ON)
# in the future maybe
# option(NANOUDR_ENABLE_UNICODE "Enable Unicode support" OFF)
# --------

###############################################################################
# Version
############

find_path(UDR_VERSION VERSION.txt ${CMAKE_SOURCE_DIR})
if(UDR_VERSION)
	file(STRINGS VERSION.udr NANOUDR_VERSION REGEX "[0-9]+\\.[0-9]+\\.[0-9]+")
	string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" NANOUDR_VERSION_MAJOR "${NANOUDR_VERSION}")
	string(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" NANOUDR_VERSION_MINOR "${NANOUDR_VERSION}")
	string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" NANOUDR_VERSION_PATCH "${NANOUDR_VERSION}")
	message(STATUS "nanoudr version: ${NANOUDR_VERSION}")
endif()

###############################################################################
# Require and enable C++ 0x/11/14/17
############

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "nanoudr compile: C++${CMAKE_CXX_STANDARD}")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wnarrowing -Werror")
	include(CheckCXXCompilerFlag)
	if(NOT NANOUDR_DISABLE_LIBCXX)
		check_cxx_compiler_flag("-stdlib=libc++" CXX_SUPPORTS_STDLIB)
		if(CXX_SUPPORTS_STDLIB)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++")
		endif()
		set(NANOUDR_DISABLE_LIBCXX ${NANOUDR_DISABLE_LIBCXX} CACHE BOOL "Do not use libc++, if available." FORCE)
	endif()
	message(STATUS "nanoudr build: Disable linking libc++ - ${NANOUDR_DISABLE_LIBCXX}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	string(REGEX REPLACE "[/-]W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	if (NOT (CMAKE_VERSION VERSION_LESS 3.6.0)) # Compiler features for Intel in CMake 3.6+
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++17")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /QaxCORE-AVX2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise")
	set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Qipo")
elseif(MSVC)
	string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	if(MSVC_VERSION LESS 1900)
		message(FATAL_ERROR "nanoudr build: Build requires C++17-compliant compiler")
	endif()
endif()

###############################################################################
# Check and enable ASAN support
############

if(NANOUDR_ENABLE_ASAN)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		if(NOT CMAKE_CXX_COMPILER_VERSION LESS 3.1)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
		endif()
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		if(NOT CMAKE_CXX_COMPILER_VERSION LESS 4.8 AND SOCI_ASAN)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
		endif()
	else()
		message(STATUS "nanoudr build: Status ASAN not determined and not enable")
		set(NANOUDR_ENABLE_ASAN OFF)
	endif()
endif()

###############################################################################
# Check and enable LTO support
############

if (NANOUDR_ENABLE_LTO)
    cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
	include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if (NOT ipo_supported)
        message(STATUS "nanoudr build: IPO/LTO not supported")
	else()
		if (NOT NANOUDR_ENABLE_ASAN)
			message(STATUS "nanoudr build: IPO/LTO enabled")
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
			if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
				# Check for lld as Clang LTO works best with its own linker
				include(CheckCXXCompilerFlag)
				check_cxx_compiler_flag("-fuse-ld=lld" CLANG_LLD)
				if (CLANG_LLD)
					set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
					set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
					set(CMAKE_MODULE_LINKER_FLAGS  "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")
				endif()
			endif()
		else()
			message(STATUS "nanoudr build: IPO/LTO conflicts with ASAN and not enabled")
		endif()
    endif()
endif()

###############################################################################
# Check whether visibility is supported
############

if (NANOUDR_HIDING_VISIBILITY)
    include(CheckCSourceCompiles)
    check_c_source_compiles(
        "
        __attribute__((visibility(\"default\"))) int _v0() { return 0; }
        __attribute__((visibility(\"hidden\")))  int _v1() { return 1; }
		int main(int argc, char* argv[]) { _v0(); _v1(); return 0; }
        "
        visibility_support
    )
    if (visibility_support)
        message(STATUS "nanoudr build: GCC/Clang visibility enabled")
        set(CMAKE_CXX_VISIBILITY_PRESET hidden)
        cmake_policy(SET CMP0063 NEW)
    endif()
endif()

###############################################################################
## Find unixODBC or iODBC config binary
############

if(UNIX)
	# Try to find unixODBC first via odbc_config program.
	find_program(ODBC_CONFIG odbc_config
		PATHS $ENV{ODBC_PATH}/bin /usr/bin /usr/local/bin)
	if(ODBC_CONFIG)
		message(STATUS "nanoudr build: ODBC on Unix - unixODBC")
		set(ODBCLIB odbc)
		execute_process(COMMAND ${ODBC_CONFIG} --include-prefix
			OUTPUT_VARIABLE ODBC_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
		set(ODBC_CFLAGS "-I${ODBC_INCLUDE_DIR}")
		set(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")
		execute_process(COMMAND ${ODBC_CONFIG} --libs
			OUTPUT_VARIABLE ODBC_LINK_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	endif()

	# Fallback to finding unixODBC via install paths
	if(NOT ODBC_CONFIG)
		find_path(UnixODBC_INCLUDE_DIR uodbc_stats.h
			/usr/include
			/usr/local/include
			/usr/include/odbc
			/usr/local/include/odbc
			/usr/include/libodbc
			/usr/local/include/libodbc)
		if(UnixODBC_INCLUDE_DIR)
			set(ODBC_CONFIG 1)
			message(STATUS "nanoudr build: ODBC on Unix - unixODBC")
			set(ODBCLIB odbc)
			set(ODBC_CFLAGS "-I${UnixODBC_INCLUDE_DIR} -DHAVE_UNISTD_H -DHAVE_PWD_H -DHAVE_SYS_TYPES_H -DHAVE_LONG_LONG -DSIZEOF_LONG_INT=8")
			endif()
	endif()

	# Fallback to using iODBC
	if(NOT ODBC_CONFIG)
		find_program(ODBC_CONFIG iodbc-config
			PATHS $ENV{ODBC_PATH}/bin /usr/bin /usr/local/bin)
		if(ODBC_CONFIG)
			message(STATUS "nanoudr build: ODBC on Unix - iODBC")
			set(ODBCLIB iodbc)
			execute_process(COMMAND ${ODBC_CONFIG} --cflags
				OUTPUT_VARIABLE ODBC_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
			set(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")
			execute_process(COMMAND ${ODBC_CONFIG} --libs
				OUTPUT_VARIABLE ODBC_LINK_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
# in the future maybe
#			if(NANOUDR_ENABLE_UNICODE)
#			endif()
# ------
		endif()
	endif()

	if(NOT ODBC_CONFIG)
		message(FATAL_ERROR "nanoudr build: Can not find a suitable ODBC driver manager")
	endif()

	message(STATUS "ODBC compile flags: ${ODBC_CFLAGS}")
	message(STATUS "ODBC link flags: ${ODBC_LINK_FLAGS}")
endif()

###############################################################################
# ODBC libraries to link
############

if(UNIX)
	set(ODBC_LIBRARIES ${ODBCLIB})
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
elseif(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	set(ODBC_LIBRARIES odbc32.lib odbccp32.lib Ws2_32.lib)
elseif(MINGW)
	set(ODBC_LIBRARIES odbc32 odbccp32)
endif()

message(STATUS "nanoudr link: odbc - ${ODBC_LIBRARIES}")

###############################################################################
# NANODBC libraries to link
############

find_path(NANODBC_INCLUDE_DIR nanodbc.h
	${CMAKE_SOURCE_DIR}/nanodbc/include
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	if(WIN32)
		find_library(NANODBC_LIB nanodbc
			${CMAKE_SOURCE_DIR}/nanodbc/lib/debug
		)
	endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	find_library(NANODBC_LIB nanodbc
		${CMAKE_SOURCE_DIR}/nanodbc/lib/release
	)
endif()

if(NOT NANODBC_INCLUDE_DIR OR NOT NANODBC_LIB)
	message(FATAL_ERROR "nanoudr build: Location of nanodbc library could not found")
endif()

add_library(nanodbc STATIC IMPORTED)
set_target_properties(nanodbc
	PROPERTIES
		IMPORTED_LOCATION ${NANODBC_LIB}
		INTERFACE_INCLUDE_DIRECTORIES ${NANODBC_INCLUDE_DIR}
)

message(STATUS "nanoudr link: nanodbc - ${NANODBC_LIB}")

###############################################################################
# Firebird libraries to link
############

find_path(FIREBIRD_INCLUDE_DIR ibase.h
	/usr/include
	/opt/firebird/include
	${CMAKE_SOURCE_DIR}/firebird/include
	$ENV{ProgramFiles}/Firebird/*/include
)

set(FIREBIRD_LIB_NAMES fbclient fbclient_ms)
find_library(FIREBIRD_LIB
	NAMES
		${FIREBIRD_LIB_NAMES}
	PATHS
		/usr/lib
		/opt/firebird/lib
		${CMAKE_SOURCE_DIR}/firebird/lib
		$ENV{ProgramFiles}/Firebird/*/lib
)

if(NOT FIREBIRD_INCLUDE_DIR OR NOT FIREBIRD_LIB)
	message(FATAL_ERROR "nanoudr build: Location of Firebird library could not found")
endif()

add_library(firebird SHARED IMPORTED)
if(UNIX)
	set_target_properties(firebird
		PROPERTIES
			IMPORTED_LOCATION ${FIREBIRD_LIB}
			INTERFACE_INCLUDE_DIRECTORIES "${FIREBIRD_INCLUDE_DIR};${FIREBIRD_INCLUDE_DIR}/firebird"
	)
else()
	set_target_properties(firebird
		PROPERTIES
			IMPORTED_IMPLIB ${FIREBIRD_LIB}
			INTERFACE_INCLUDE_DIRECTORIES "${FIREBIRD_INCLUDE_DIR};${FIREBIRD_INCLUDE_DIR}/firebird"
)
endif()

message(STATUS "nanoudr link: firebird - ${FIREBIRD_LIB}")

###############################################################################
# iConv libraries to link
############

if(WIN32)
	set(ICONV_LIB_PREFIX "lib")
endif()

find_path(ICONV_INCLUDE_DIR iconv.h
	/usr/include
	${CMAKE_SOURCE_DIR}/iconv/include
)

find_library(ICONV_LIB "${ICONV_LIB_PREFIX}iconv"
	PATHS
		/usr/lib
		${CMAKE_SOURCE_DIR}/iconv/lib
)

if(NOT ICONV_INCLUDE_DIR OR NOT ICONV_LIB)
	message(FATAL_ERROR "nanoudr build: Location of iConv library could not found")
endif()

add_library(iconv SHARED IMPORTED)
if(UNIX)
	set_target_properties(iconv
		PROPERTIES
			IMPORTED_LOCATION ${ICONV_LIB}
			INTERFACE_INCLUDE_DIRECTORIES ${ICONV_INCLUDE_DIR}
	)
else()
	set_target_properties(iconv
		PROPERTIES
			IMPORTED_IMPLIB ${ICONV_LIB}
			INTERFACE_INCLUDE_DIRECTORIES ${ICONV_INCLUDE_DIR}
	)
endif()

message(STATUS "nanoudr link: iconv - ${ICONV_LIB}")

###############################################################################
# Target library
############

file(GLOB files ${CMAKE_SOURCE_DIR}/src/*)

add_library(nano SHARED ${files})

set(def_file "udr_plugin") 
if(UNIX) 
	set_target_properties(nanodbc PROPERTIES COMPILE_FLAGS "${ODBC_CFLAGS}")
else(WIN32)
	set(def_file ${def_file}.def) 
	set_target_properties(nano PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_SOURCE_DIR}/firebird/defs/${def_file}\"")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	if(WIN32)
		find_path(FIREBIRD_PLUGINS udr_engine.dll
			$ENV{ProgramFiles}/Firebird/*/plugins
		)
		set_target_properties(nano
			PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY ${FIREBIRD_PLUGINS}/udr
		)
	endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	set_target_properties(nano
		PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x86_64-linux
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x86_64-win64
	)
endif()

if(BUILD_SHARED_LIBS AND NANOUDR_VERSION)
	set_target_properties(nano PROPERTIES VERSION ${NANOUDR_VERSION})
endif()

target_link_libraries(nano PUBLIC ${ODBC_LIBRARIES} nanodbc firebird iconv)

######

