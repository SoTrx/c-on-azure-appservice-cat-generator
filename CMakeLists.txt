cmake_minimum_required(VERSION 3.18)
project(c_on_azure_cat_api C)
set(CMAKE_C_STANDARD 11)

include_directories(include/)


# Building MJson for JSON support
add_library(mjson third-party/mjson/mjson.c)

# Add TLS support with OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
# Imports all certs.
# This could (and should) be handled/overrided by the underlying OS
# Still useful in debug
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ca.pem ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

# Building mongoose lib
add_library(mongoose third-party/mongoose/mongoose.c)
IF (WIN32)
    target_compile_options(mongoose PRIVATE /DMG_ENABLE_OPENSSL=1)
    target_compile_options(mongoose PRIVATE /DMG_ENABLE_LOG=1)
    # On windows, an additional Winsock lib is needed
    target_link_libraries(mongoose Ws2_32)
ELSE ()
    target_compile_options(mongoose PRIVATE -DMG_ENABLE_OPENSSL=1)
    target_compile_options(mongoose PRIVATE -DMG_ENABLE_LOG=1)
ENDIF ()

target_link_libraries(mongoose OpenSSL::SSL OpenSSL::Crypto OpenSSL::applink mjson)
# Build the cat API
file(GLOB_RECURSE SRCS src/*)
add_executable(c_on_azure_cat_api ${SRCS})
message(${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O1")
target_link_libraries(c_on_azure_cat_api mongoose)
