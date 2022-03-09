cmake_minimum_required(VERSION 3.16)
project(Open_AlteryxYXDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

IF(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127 /wd4245 /wd4251 /wd4275 /wd4267 /wd4244 -EHa -J -MP -WX -W4")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob2 /Oi /Zi")
ELSE()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-unknown-pragmas -Wno-parentheses -fPIC -funsigned-char -pthread -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
ENDIF()

file(GLOB PROJECT_HEADERS include/Base/*.h include/RecordLib/*.h include/*.h)
file(GLOB PROJECT_SOURCES src/*.cpp src/sqlite3.c)
file(GLOB PROJECT_C_SOURCES src/lzf_c.c src/lzf_d.c)

set_source_files_properties(${PROJECT_C_SOURCES} PROPERTIES LANGUAGE C)

add_library(${PROJECT_NAME} SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${PROJECT_C_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE
	-DSRCLIB_REPLACEMENT
	-DOPEN_ALTERYX_EXPORTS
	-DUNICODE
	-DNOMINMAX
	)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/include/stdafx.h>")

add_executable(${PROJECT_NAME}Test test/main.cpp)
target_include_directories(${PROJECT_NAME}Test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(${PROJECT_NAME}Test PRIVATE -DUNICODE -DNOMINMAX)
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${PROJECT_NAME})
if (UNIX)
	target_link_libraries(${PROJECT_NAME}Test PRIVATE -ldl)
endif()
