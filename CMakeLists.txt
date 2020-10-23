# Copyright (c) 2017 - 2020 LiteSpeed Technologies Inc.  See LICENSE.
cmake_minimum_required(VERSION 2.8)


PROJECT(lsquic C)

OPTION(LSQUIC_FIU "Use Fault Injection in Userspace (FIU)" OFF)
OPTION(LSQUIC_BIN "Compile example binaries that use the library" ON)
OPTION(LSQUIC_TESTS "Compile library unit tests" ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)

IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # If using older glibc, need to link with -lrt.  See clock_getres(2).
    EXECUTE_PROCESS(
        COMMAND ${PROJECT_SOURCE_DIR}/print-glibc-version.sh ${CMAKE_C_COMPILER}
        OUTPUT_VARIABLE GLIBC_VERSION)
    IF(NOT GLIBC_VERSION EQUAL "" AND GLIBC_VERSION VERSION_LESS 2.17)
        SET(LIBS ${LIBS} rt)
    ENDIF()
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # for android-ndk >= r19b
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY "BOTH")
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE "BOTH")
    set(CMAKE_FIND_ROOT_PATH_MODE_PATH "BOTH")
ENDIF()

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

IF (NOT "$ENV{EXTRA_CFLAGS}" MATCHES "-DLSQUIC_DEBUG_NEXT_ADV_TICK")
    SET(MY_CMAKE_FLAGS "-DLSQUIC_DEBUG_NEXT_ADV_TICK=1")
ENDIF()

IF (NOT MSVC)

SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Wall -Wextra -Wno-unused-parameter")
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -fno-omit-frame-pointer")
INCLUDE(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG(-Wno-implicit-fallthrough HAS_NO_IMPLICIT_FALLTHROUGH)
IF (HAS_NO_IMPLICIT_FALLTHROUGH)
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Wno-implicit-fallthrough")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9.3)
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Wno-missing-field-initializers")
ENDIF()

IF(LSQUIC_FIU)
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -DFIU_ENABLE=1")
    SET(LIBS ${LIBS} fiu)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -O0 -g3")
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Werror")
    IF(CMAKE_C_COMPILER MATCHES "clang" AND
                        NOT "$ENV{TRAVIS}" MATCHES "^true$" AND
                        NOT "$ENV{EXTRA_CFLAGS}" MATCHES "-fsanitize")
        SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -fsanitize=address")
        SET(LIBS ${LIBS} -fsanitize=address)
    ENDIF()
    # Uncomment to enable cleartext protocol mode (no crypto):
    #SET (MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -DLSQUIC_ENABLE_HANDSHAKE_DISABLE=1")
ELSE()
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -O3 -g0")
    # Comment out the following line to compile out debug messages:
    #SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -DLSQUIC_LOWEST_LOG_LEVEL=LSQ_LOG_INFO")
ENDIF()

IF(LSQUIC_PROFILE EQUAL 1)
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -g -pg")
ENDIF()

IF(LSQUIC_COVERAGE EQUAL 1)
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

IF(MY_CMAKE_FLAGS MATCHES "fsanitize=address")
    MESSAGE(STATUS "AddressSanitizer is ON")
ELSE()
    MESSAGE(STATUS "AddressSanitizer is OFF")
ENDIF()

#MSVC
ELSE()
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4100")	# unreferenced formal parameter
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4115")	# unnamed type definition in parentheses
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4116")	# named type definition in parentheses
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4146")	# unary minus operator applied to unsigned type, result still unsigned
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4132")	# const initialization
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4200")	# zero-sized array
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4204")	# non-constant aggregate initializer
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4244")	# integer conversion
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4245")	# conversion from 'int' to 'unsigned int', signed/unsigned mismatch
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4267")	# integer conversion
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4214")	# nonstandard extension used: bit field types other than int
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4295")	# array is too small to include a terminating null character
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4334")	# result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4456")	# hide previous local declaration
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4459")	# hide global declaration
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} /wd4706")	# assignment within conditional expression
SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -W4 -WX -Zi -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -I${CMAKE_CURRENT_SOURCE_DIR}/wincompat")
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Od")
    #SET (MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -DFIU_ENABLE=1")
    #SET(LIBS ${LIBS} fiu)
ELSE()
    SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -Ox")
    # Comment out the following line to compile out debug messages:
    #SET(MY_CMAKE_FLAGS "${MY_CMAKE_FLAGS} -DLSQUIC_LOWEST_LOG_LEVEL=LSQ_LOG_INFO")
ENDIF()

ENDIF() #MSVC

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  ${MY_CMAKE_FLAGS} $ENV{EXTRA_CFLAGS}")

MESSAGE(STATUS "Compiler flags: ${CMAKE_C_FLAGS}")

find_package(Perl)
IF(NOT PERL_FOUND)
    MESSAGE(FATAL_ERROR "Perl not found -- need it to generate source code")
ENDIF()

IF (NOT DEFINED BORINGSSL_INCLUDE AND DEFINED BORINGSSL_DIR)
    FIND_PATH(BORINGSSL_INCLUDE NAMES openssl/ssl.h
                PATHS ${BORINGSSL_DIR}/include
                NO_DEFAULT_PATH)
ENDIF()
# This must be done before adding other include directories to take
# precedence over header files from other SSL installs.

IF (BORINGSSL_INCLUDE)
    MESSAGE(STATUS "BoringSSL include directory ${BORINGSSL_INCLUDE}")
    INCLUDE_DIRECTORIES(${BORINGSSL_INCLUDE})
ELSE()
    MESSAGE(FATAL_ERROR "BoringSSL headers not found")
ENDIF()

IF (NOT DEFINED BORINGSSL_LIB AND DEFINED BORINGSSL_DIR)
    FOREACH(LIB_NAME ssl crypto decrepit)
        IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
            FIND_LIBRARY(BORINGSSL_LIB_${LIB_NAME}
                NAMES ${LIB_NAME}
                PATHS ${BORINGSSL_DIR}/${LIB_NAME}
		PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                NO_DEFAULT_PATH)
        ELSE()
            FIND_LIBRARY(BORINGSSL_LIB_${LIB_NAME}
                NAMES lib${LIB_NAME}.a
                PATHS ${BORINGSSL_DIR}/${LIB_NAME}
                NO_DEFAULT_PATH)
        ENDIF()
        IF(BORINGSSL_LIB_${LIB_NAME})
            MESSAGE(STATUS "Found ${LIB_NAME} library: ${BORINGSSL_LIB_${LIB_NAME}}")
        ELSE()
            MESSAGE(STATUS "${LIB_NAME} library not found")
        ENDIF()
    ENDFOREACH()

ELSE()


    FOREACH(LIB_NAME ssl crypto decrepit)
        IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
            FIND_LIBRARY(BORINGSSL_LIB_${LIB_NAME}
                NAMES ${LIB_NAME}
                PATHS ${BORINGSSL_LIB}
                PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                NO_DEFAULT_PATH)
        ELSE()
            FIND_LIBRARY(BORINGSSL_LIB_${LIB_NAME}
                NAMES lib${LIB_NAME}.a
                PATHS ${BORINGSSL_LIB}
                PATH_SUFFIXES ${LIB_NAME}
                NO_DEFAULT_PATH)
        ENDIF()
        IF(BORINGSSL_LIB_${LIB_NAME})
            MESSAGE(STATUS "Found ${BORINGSSL_LIB} library: ${BORINGSSL_LIB_${LIB_NAME}}")
        ELSE()
            MESSAGE(STATUS "${BORINGSSL_LIB} library not found")
        ENDIF()
    ENDFOREACH()

ENDIF()

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(include)
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Find libevent on FreeBSD:
    include_directories( /usr/local/include )
    link_directories( /usr/local/lib )
ENDIF()

# Find zlib and libevent header files and library files
# TODO: libevent is not strictly necessary to build the library.
FIND_PATH(ZLIB_INCLUDE_DIR NAMES zlib.h)
IF (ZLIB_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ELSE()
    MESSAGE(FATAL_ERROR "zlib.h was not found")
ENDIF()
IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
    FIND_LIBRARY(ZLIB_LIB zlib)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # XXX somehow FIND_LIBRARY() does not find zlib on Travis?
    SET(ZLIB_LIB z)
ELSE()
    FIND_LIBRARY(ZLIB_LIB libz.a)
ENDIF()
IF(ZLIB_LIB)
    MESSAGE(STATUS "Found zlib: ${ZLIB_LIB}")
ELSE()
    MESSAGE(STATUS "zlib not found")
ENDIF()

IF (LSQUIC_BIN OR LSQUIC_TESTS)
FIND_PATH(EVENT_INCLUDE_DIR NAMES event2/event.h)
IF (EVENT_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${EVENT_INCLUDE_DIR})
ELSE()
    MESSAGE(FATAL_ERROR "event2/event.h was not found")
ENDIF()
IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
    FIND_LIBRARY(EVENT_LIB event)
ELSE()
    FIND_LIBRARY(EVENT_LIB libevent.a)
    IF(NOT EVENT_LIB)
        FIND_LIBRARY(EVENT_LIB libevent.so)
    ENDIF()
ENDIF()
IF(EVENT_LIB)
    MESSAGE(STATUS "Found event: ${EVENT_LIB}")
ELSE()
    MESSAGE(STATUS "libevent not found")
ENDIF()
SET(LIBS lsquic ${EVENT_LIB} ${BORINGSSL_LIB_ssl} ${BORINGSSL_LIB_crypto} ${ZLIB_LIB} ${LIBS})
IF(MSVC)
FIND_LIBRARY(PCRE_LIB pcre)
IF(PCRE_LIB)
    MESSAGE(STATUS "Found pcre: ${PCRE_LIB}")
    LIST(APPEND LIBS ${PCRE_LIB})
ELSE()
    MESSAGE(STATUS "pcre not found: http_server won't work")
ENDIF()
FIND_LIBRARY(PCREPOSIX_LIB pcreposix)
IF(PCREPOSIX_LIB)
    MESSAGE(STATUS "Found pcreposix: ${PCREPOSIX_LIB}")
    LIST(APPEND LIBS ${PCREPOSIX_LIB})
ELSE()
    MESSAGE(STATUS "pcreposix not found: http_server won't work")
ENDIF()
LIST(APPEND LIBS ws2_32)
ENDIF()
ENDIF()	# LSQUIC_BIN OR LSQUIC_TESTS

IF(LSQUIC_BIN)
IF(MSVC)
    SET(GETOPT_C wincompat/getopt.c)
ENDIF()
add_executable(http_server /src/equic/bin/equic_user.o bin/http_server.c bin/prog.c bin/test_common.c bin/test_cert.c ${GETOPT_C})
IF(NOT MSVC)   #   TODO: port MD5 server and client to Windows
add_executable(md5_server bin/md5_server.c bin/prog.c bin/test_common.c bin/test_cert.c ${GETOPT_C})
add_executable(md5_client bin/md5_client.c bin/prog.c bin/test_common.c bin/test_cert.c ${GETOPT_C})
ENDIF()

# Modified to compile together with eQUIC library
add_executable(echo_server /src/equic/bin/equic_user.o bin/echo_server.c bin/prog.c bin/test_common.c bin/test_cert.c ${GETOPT_C})
add_executable(echo_client bin/echo_client.c bin/prog.c bin/test_common.c bin/test_cert.c ${GETOPT_C})


IF (NOT MSVC)

add_executable(http_client
    bin/http_client.c
    bin/prog.c
    bin/test_common.c
    bin/test_cert.c
)
LIST(APPEND LIBS pthread m)

#MSVC
ELSE()

add_executable(http_client
    bin/http_client.c
    bin/prog.c
    bin/test_common.c
    bin/test_cert.c
    wincompat/getopt.c
    wincompat/getopt1.c
)
LIST(APPEND LIBS ws2_32)

ENDIF()

TARGET_LINK_LIBRARIES(http_client ${LIBS})
# Links bpf library on the http_server implementation
TARGET_LINK_LIBRARIES(http_server ${LIBS} bpf elf /usr/lib/x86_64-linux-gnu/libz.a)
IF(NOT MSVC)
TARGET_LINK_LIBRARIES(md5_server  ${LIBS})
TARGET_LINK_LIBRARIES(md5_client  ${LIBS})
ENDIF()

# Links bpf library on the echo_server implementation
TARGET_LINK_LIBRARIES(echo_server ${LIBS} bpf elf /usr/lib/x86_64-linux-gnu/libz.a)
TARGET_LINK_LIBRARIES(echo_client ${LIBS})

add_subdirectory(bin)
ENDIF() # LSQUIC_BIN

add_subdirectory(src)

IF(LSQUIC_TESTS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Our test framework relies on assertions, only compile if assertions are
    # enabled.
    #
    enable_testing()
    add_subdirectory(tests)
ENDIF()


FIND_PROGRAM(SPHINX NAMES sphinx-build)
IF(SPHINX)
    ADD_CUSTOM_TARGET(docs
        ${SPHINX} -b html
        docs
        docs/_build
    )
ELSE()
    MESSAGE(STATUS "sphinx-build not found: docs won't be made")
ENDIF()