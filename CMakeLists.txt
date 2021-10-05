cmake_minimum_required (VERSION 3.4)

project ("udr-nanodbc")

set(SOURCE ${CMAKE_SOURCE_DIR}/src)

if(UNIX)

    set(ODBC ${ODBCLIB})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")

elseif(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    set(ODBC odbc32.lib odbccp32.lib Ws2_32.lib)

    set(FIREBIRD $ENV{ProgramW6432}/firebird/firebird_3_0)
    add_library(firebird SHARED IMPORTED)
    set_target_properties(firebird  
        PROPERTIES
            IMPORTED_IMPLIB ${FIREBIRD}/lib/fbclient_ms.lib
            INTERFACE_INCLUDE_DIRECTORIES "${FIREBIRD}/include;${FIREBIRD}/include/firebird"
    )

    set(NANODBC ${CMAKE_SOURCE_DIR}/nanodbc)
    add_library(nanodbc SHARED IMPORTED)
    set_target_properties(nanodbc 
        PROPERTIES
            IMPORTED_IMPLIB ${NANODBC}/lib/nanodbc.lib
            INTERFACE_INCLUDE_DIRECTORIES ${NANODBC}/include
    )

    set(ICONV ${CMAKE_SOURCE_DIR}/iconv)
    add_library(iconv SHARED IMPORTED)
    set_target_properties(iconv
        PROPERTIES
            IMPORTED_IMPLIB ${ICONV}/lib/libiconv.lib
            INTERFACE_INCLUDE_DIRECTORIES ${ICONV}/include

    )

endif() 

file(GLOB files ${SOURCE}/*) 

add_library(nano SHARED ${files})

set_property(TARGET nano PROPERTY CXX_STANDARD 17)
set_property(TARGET nano PROPERTY CXX_STANDARD_REQUIRED ON)

if(UNIX)

elseif(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    set_target_properties(nano 
        PROPERTIES 
            LINK_FLAGS "/DEF:\"${CMAKE_SOURCE_DIR}/defs/udr_plugin.def\"") 

endif() 

target_link_libraries(nano PUBLIC ${ODBC} firebird nanodbc iconv)




