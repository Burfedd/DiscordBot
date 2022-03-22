# CMakeList.txt : CMake project for DiscordBot, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


project ("DiscordBot")

# Add source to this project's executable.
add_executable (DiscordBot "src/DiscordBot.cpp" "src/DiscordBot.h")

# TODO: Add tests and install targets if needed.
