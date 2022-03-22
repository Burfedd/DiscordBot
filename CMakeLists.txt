# CMakeList.txt : CMake project for DiscordBot, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


project ("DiscordBot")

# DPP sub-directory
add_subdirectory(libs/DPP)

# Add source to this project's executable.
add_executable (DiscordBot "src/DiscordBot.cpp" "src/DiscordBot.h")

# Linking libraries
target_link_libraries(${PROJECT_NAME}
	dpp
)

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE
	libs/DPP/include
)

# C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
