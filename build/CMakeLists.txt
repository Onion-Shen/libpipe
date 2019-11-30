cmake_minimum_required(VERSION 2.8)
project(pipe)

# debug/release
set(CMAKE_BUILD_TYPE "Release")

# cpp compiler flags
set(CMAKE_CXX_FLAGS "-std=c++14")

# library header file include path
set(PROJECT_INCLUDE_PATH ../include)
# openssl header file path on mac
set(OPENSSL_INCLUDE_PATH /usr/local/opt/openssl/include)

# head files
include_directories(
    ${PROJECT_INCLUDE_PATH} 
    ${OPENSSL_INCLUDE_PATH}
)

# openssl lib path on mac
set(OPENSSL_LIB_PATH /usr/local/opt/openssl/lib)
# add library path to linker
link_directories(
    ${OPENSSL_LIB_PATH}
)

# library files that need to be linked
link_libraries(
    ${OPENSSL_LIB_PATH}/libcrypto.a 
    ${OPENSSL_LIB_PATH}/libssl.a
)

# library output path (all:LIBRARY_OUTPUT_PATH;share:CMAKE_LIBRARY_OUTPUT_DIRECTORY;static:CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
set(LIB_OUTPUT_PATH ../lib)
set(LIBRARY_OUTPUT_PATH ${LIB_OUTPUT_PATH})

# library src file root path
set(LIB_SRC_PATH ../src)
set(LIB_SRC 
${LIB_SRC_PATH}/base/string_container.c 
${LIB_SRC_PATH}/base/U8String.cpp 
${LIB_SRC_PATH}/base/StringEncoder.cpp 
${LIB_SRC_PATH}/json/JSObject.cpp 
${LIB_SRC_PATH}/json/JSONLexer.cpp 
${LIB_SRC_PATH}/json/JSONParser.cpp 
${LIB_SRC_PATH}/json/JSONToken.cpp 
${LIB_SRC_PATH}/util/LexerReader.cpp 
${LIB_SRC_PATH}/util/ZlibWrapper.cpp 
${LIB_SRC_PATH}/xml/XMLDocument.cpp 
${LIB_SRC_PATH}/xml/XMLLexer.cpp 
${LIB_SRC_PATH}/xml/XMLParser.cpp 
${LIB_SRC_PATH}/xml/XMLTok.cpp 
${LIB_SRC_PATH}/HTTPMsgParser/HTTPRecvMsgParser.cpp 
${LIB_SRC_PATH}/HTTPMsgParser/HTTPReqMsgParser.cpp 
${LIB_SRC_PATH}/HTTPMsgParser/HTTPRequest.cpp 
${LIB_SRC_PATH}/HTTPMsgParser/HTTPResponse.cpp 
${LIB_SRC_PATH}/net/BasicSocket.cpp 
${LIB_SRC_PATH}/net/BIOSocket.cpp 
${LIB_SRC_PATH}/net/SocketConfig.cpp 
${LIB_SRC_PATH}/net/TCPSocket.cpp 
${LIB_SRC_PATH}/net/UDPSocket.cpp 
)

# create library (STATIC/SHARED)
set(LIB_NAME pipe)
add_library(${LIB_NAME} SHARED ${LIB_SRC})

# link the library that the project is needed
# libiconv
target_link_libraries(${LIB_NAME} iconv)
# libz
target_link_libraries(${LIB_NAME} z)