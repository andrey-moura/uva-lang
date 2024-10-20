#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva)

include(include/uva/CMakeLists.txt)

uva_library(string)
uva_library(core)
uva_library(json)
uva_library(file)
uva_library(console)

if(WIN32)
	if(DEFINED ENV{SDK})		
		message(STATUS "Found SDK folder: $ENV{SDK}")
		
		#wxWidgets
		if(EXISTS "$ENV{SDK}\\wxWidgets")
			set(wxWidgets_PATH "$ENV{SDK}\\wxWidgets")
			message(STATUS "Found wxWidgets: ${wxWidgets_PATH}")
			
			# wxWidgets include directory
			include_directories("${wxWidgets_PATH}\\include")
			include_directories("${wxWidgets_PATH}\\include\\msvc")
			# wxWidgets library folder
			if(${CMAKE_CL_64})
				link_directories("${wxWidgets_PATH}\\lib\\vc_x64_lib")
			else()
				link_directories("${wxWidgets_PATH}\\lib\\vc_lib")
			endif()			
		endif()
	endif()
	# Set characters to Unicode
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
			
	# Group ALL_BUILD and ZERO_CHECK in one folder
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	# Set Uva Engine as the startup project
	set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "UvaEngine")
	# Disable these annoying warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	set_property(GLOBAL APPEND PROPERTY LINK_FLAGS /MACHINE:X86)

else()
	#wxWidgets
	find_package(wxWidgets REQUIRED COMPONENTS net core base xml)
	include(${wxWidgets_USE_FILE})
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(include/fmt/include)

add_library(uva-parser
    parser/parser.cpp
)

target_link_libraries(uva-parser uva-lexer)

add_library(uva-extension
    extension/src/extension.cpp
)

if(DEFINED LANGUANIZER_ENABLED)
	include("languanizer/CMakeLists.txt")
endif()

include("vm/CMakeLists.txt")
include("lexer/CMakeLists.txt")

if(DEFINED WX_ENABLED)
	include("uva-wx/CMakeLists.txt")
endif()

add_executable(uva
    uva/uva.cpp
)

target_link_libraries(uva uva-vm uva-core uva-console uva-file uva-string ${wxWidgets_LIBRARIES})

install(TARGETS uva DESTINATION bin)