cmake_minimum_required (VERSION 3.4)

project ("udr-nanodbc")

set(SOURCE ${CMAKE_SOURCE_DIR}/nano)

if(UNIX)

    set(ODBC ${ODBCLIB})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")

elseif(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    set(ODBC odbc32.lib odbccp32.lib Ws2_32.lib)

    set(NANODBC ${CMAKE_SOURCE_DIR}/nanodbc-master)
    add_library(nanodbc SHARED IMPORTED)
    set_target_properties(nanodbc 
        PROPERTIES
            IMPORTED_IMPLIB ${NANODBC}/out/build/x64-Debug/nanodbc.lib
            INTERFACE_INCLUDE_DIRECTORIES ${NANODBC}/nanodbc
    )

    set(FIREBIRD $ENV{ProgramW6432}/firebird/firebird_3_0)
    add_library(firebird SHARED IMPORTED)
    set_target_properties(firebird 
        PROPERTIES
            IMPORTED_IMPLIB ${FIREBIRD}/lib/fbclient_ms.lib
            INTERFACE_INCLUDE_DIRECTORIES "${FIREBIRD}/include;${FIREBIRD}/include/firebird"
    )

endif() 

file(GLOB files ${SOURCE}/*) 

add_library(nano SHARED ${files})

set_property(TARGET nano PROPERTY CXX_STANDARD 17)
set_property(TARGET nano PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(nano PUBLIC ${ODBC} nanodbc firebird)




