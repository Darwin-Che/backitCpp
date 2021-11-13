cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Backit_Client VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
    src/app.cpp
    src/util.cpp
    src/load.cpp
)

add_executable(init exec/init.cpp ${SOURCES})


target_include_directories(init
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

add_executable(update exec/update.cpp ${SOURCES})

target_include_directories(update
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)
