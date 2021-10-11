cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Backit_Client VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
    src/app.cpp
    src/sync.cpp
    src/load.cpp
)

add_executable(Test1 test/test1.cpp ${SOURCES})

target_include_directories(Test1
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)


