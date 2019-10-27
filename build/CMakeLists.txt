cmake_minimum_required(VERSION 2.8)
project(pipe)

# cpp compiler flags
set(CMAKE_CXX_FLAGS "-std=c++14")

# library header file include path
set(PROJECT_INCLUDE_PATH ../include)

# head files
include_directories(
    ${PROJECT_INCLUDE_PATH}
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
${LIB_SRC_PATH}/xml/XMLDocument.cpp 
${LIB_SRC_PATH}/xml/XMLLexer.cpp 
${LIB_SRC_PATH}/xml/XMLParser.cpp 
${LIB_SRC_PATH}/xml/XMLTok.cpp 
)

# create library (STATIC/SHARED)
set(LIB_NAME pipe)
add_library(${LIB_NAME} SHARED ${LIB_SRC})

# link the library that the project is needed
target_link_libraries(${LIB_NAME} iconv)