# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.1.1\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.1.1\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\61416\CLionProjects\TowerDefenseSDL

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/TowerDefenseSDL.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/TowerDefenseSDL.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TowerDefenseSDL.dir/flags.make

CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.obj: CMakeFiles/TowerDefenseSDL.dir/flags.make
CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.obj: ../Timer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.obj"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\TowerDefenseSDL.dir\Timer.cpp.obj -c C:\Users\61416\CLionProjects\TowerDefenseSDL\Timer.cpp

CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.i"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\61416\CLionProjects\TowerDefenseSDL\Timer.cpp > CMakeFiles\TowerDefenseSDL.dir\Timer.cpp.i

CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.s"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\61416\CLionProjects\TowerDefenseSDL\Timer.cpp -o CMakeFiles\TowerDefenseSDL.dir\Timer.cpp.s

CMakeFiles/TowerDefenseSDL.dir/main.cpp.obj: CMakeFiles/TowerDefenseSDL.dir/flags.make
CMakeFiles/TowerDefenseSDL.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/TowerDefenseSDL.dir/main.cpp.obj"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\TowerDefenseSDL.dir\main.cpp.obj -c C:\Users\61416\CLionProjects\TowerDefenseSDL\main.cpp

CMakeFiles/TowerDefenseSDL.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TowerDefenseSDL.dir/main.cpp.i"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\61416\CLionProjects\TowerDefenseSDL\main.cpp > CMakeFiles\TowerDefenseSDL.dir\main.cpp.i

CMakeFiles/TowerDefenseSDL.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TowerDefenseSDL.dir/main.cpp.s"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\61416\CLionProjects\TowerDefenseSDL\main.cpp -o CMakeFiles\TowerDefenseSDL.dir\main.cpp.s

CMakeFiles/TowerDefenseSDL.dir/shaders.c.obj: CMakeFiles/TowerDefenseSDL.dir/flags.make
CMakeFiles/TowerDefenseSDL.dir/shaders.c.obj: ../shaders.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/TowerDefenseSDL.dir/shaders.c.obj"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\TowerDefenseSDL.dir\shaders.c.obj   -c C:\Users\61416\CLionProjects\TowerDefenseSDL\shaders.c

CMakeFiles/TowerDefenseSDL.dir/shaders.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TowerDefenseSDL.dir/shaders.c.i"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\61416\CLionProjects\TowerDefenseSDL\shaders.c > CMakeFiles\TowerDefenseSDL.dir\shaders.c.i

CMakeFiles/TowerDefenseSDL.dir/shaders.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TowerDefenseSDL.dir/shaders.c.s"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\61416\CLionProjects\TowerDefenseSDL\shaders.c -o CMakeFiles\TowerDefenseSDL.dir\shaders.c.s

# Object files for target TowerDefenseSDL
TowerDefenseSDL_OBJECTS = \
"CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.obj" \
"CMakeFiles/TowerDefenseSDL.dir/main.cpp.obj" \
"CMakeFiles/TowerDefenseSDL.dir/shaders.c.obj"

# External object files for target TowerDefenseSDL
TowerDefenseSDL_EXTERNAL_OBJECTS =

TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/Timer.cpp.obj
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/main.cpp.obj
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/shaders.c.obj
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/build.make
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/linklibs.rsp
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/objects1.rsp
TowerDefenseSDL.exe: CMakeFiles/TowerDefenseSDL.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable TowerDefenseSDL.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\TowerDefenseSDL.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TowerDefenseSDL.dir/build: TowerDefenseSDL.exe

.PHONY : CMakeFiles/TowerDefenseSDL.dir/build

CMakeFiles/TowerDefenseSDL.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\TowerDefenseSDL.dir\cmake_clean.cmake
.PHONY : CMakeFiles/TowerDefenseSDL.dir/clean

CMakeFiles/TowerDefenseSDL.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\61416\CLionProjects\TowerDefenseSDL C:\Users\61416\CLionProjects\TowerDefenseSDL C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug C:\Users\61416\CLionProjects\TowerDefenseSDL\cmake-build-debug\CMakeFiles\TowerDefenseSDL.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TowerDefenseSDL.dir/depend

