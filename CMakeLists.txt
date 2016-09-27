cmake_minimum_required( VERSION 2.8 )

project(ai-test)
set(Project_NAME "ai-test")
set(Project_SHORTCUT "aiTest")
set(Project_PREFIX "AIT")

set(Project_BIN_DIR "bin")
set(Project_LIB_DIR "lib")
set(Project_MEDIA_DIR "media")
set(Project_DOC_DIR "doc")
set(Project_DOC_HEADER "doc.doxy")



################### set module path ###################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/extras/cmake/Modules/")


################### setup options ###################
set(Option_DEFAULT_MAKE_DOXYGEN_TARGET ON)

if(MSVC)
set(Boost_USE_STATIC_LIBS ON)
set(Option_DEFAULT_MAKE_DOXYGEN_TARGET OFF)
endif(MSVC)


option(Option_USE_PRE_COMPILED_HEADER		"Use precompiled header for compilation" ON)
option(Option_MAKE_DOXYGEN_TARGET			"Create a doxygen build target" ${Option_DEFAULT_MAKE_DOXYGEN_TARGET})
option(Option_RETHROW_THREAD_EXCEPTIONS		"Rethrow exceptions in worker threads, instead of catch them silently" ON)
option(Option_COPY_MEDIA					"Copies the media files for samples and tests into the target directories" ON)

################### add additional functions ###################
include("extras/cmake/copy_media.txt")
include("extras/cmake/src_scanner.txt")


################### setup pre compiled header macro ###################
if(Option_USE_PRE_COMPILED_HEADER)
	include("extras/cmake/pch_for_cmake.txt")
	add_definitions("-D${PROJECT_PREFIX}_USE_PRECOMPILED_HEADER")
else(Option_USE_PRE_COMPILED_HEADER)

	# simply set an empty macro
	macro(ADD_PRECOMPILED_HEADER _targetName _input)
	endmacro()

endif(Option_USE_PRE_COMPILED_HEADER)


################### add c++11 support ###################
if(NOT MSVC)
	add_definitions("-std=c++0x")
endif(NOT MSVC)



################### set definitions ###################
if(Option_RETHROW_THREAD_EXCEPTIONS)
	add_definitions("-D${PROJECT_PREFIX}_RETHROW_THREAD_EXCEPTIONS")
endif(Option_RETHROW_THREAD_EXCEPTIONS)


if(NOT Boost_USE_STATIC_LIBS)
	add_definitions("-DBOOST_TEST_DYN_LINK") 
endif(NOT Boost_USE_STATIC_LIBS)
add_definitions( -DBOOST_ALL_NO_LIB )


################### find packages ###################
find_package(Boost COMPONENTS system thread serialization regex unit_test_framework filesystem REQUIRED)
find_package(SFML COMPONENTS system window graphics audio REQUIRED)
#find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(GLM REQUIRED)


################### setup target directories ###################
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${Project_BIN_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${Project_LIB_DIR})


################### setup include directories ###################
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SFML_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

################### setup link directories ###################
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${BOOST_LIBRARYDIR})

################### set dependencies for tilenet-lib ###################
#set(project_dependency_libs ${Boost_LIBRARIES} ${SFML_LIBRARIES})



################### add directories with source###################


add_subdirectory("src")


################### setup documentation target ###################
# add a target to generate API documentation with Doxygen
if(Option_MAKE_DOXYGEN_TARGET)
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${Project_DOC_HEADER} ${CMAKE_CURRENT_BINARY_DIR}/${Project_DOC_HEADER} @ONLY)
		add_custom_target(documentation ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${Project_DOC_HEADER}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
	endif(DOXYGEN_FOUND)
endif(Option_MAKE_DOXYGEN_TARGET)
