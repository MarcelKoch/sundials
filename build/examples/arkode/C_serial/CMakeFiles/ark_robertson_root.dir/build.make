# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/maggul/STS/sundials

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/maggul/STS/sundials/build

# Include any dependencies generated for this target.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/progress.make

# Include the compile flags for this target's objects.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/flags.make

examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o: examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/flags.make
examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o: ../examples/arkode/C_serial/ark_robertson_root.c
examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o: examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/maggul/STS/sundials/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o -MF CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o.d -o CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o -c /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_root.c

examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.i"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_root.c > CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.i

examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.s"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_root.c -o CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.s

# Object files for target ark_robertson_root
ark_robertson_root_OBJECTS = \
"CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o"

# External object files for target ark_robertson_root
ark_robertson_root_EXTERNAL_OBJECTS =

examples/arkode/C_serial/ark_robertson_root: examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/ark_robertson_root.c.o
examples/arkode/C_serial/ark_robertson_root: examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/build.make
examples/arkode/C_serial/ark_robertson_root: src/arkode/libsundials_arkode.so.6.0.0
examples/arkode/C_serial/ark_robertson_root: src/nvector/serial/libsundials_nvecserial.so.7.0.0
examples/arkode/C_serial/ark_robertson_root: src/sundials/libsundials_core.so.7.0.0
examples/arkode/C_serial/ark_robertson_root: examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/maggul/STS/sundials/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ark_robertson_root"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ark_robertson_root.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/build: examples/arkode/C_serial/ark_robertson_root
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/build

examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/clean:
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && $(CMAKE_COMMAND) -P CMakeFiles/ark_robertson_root.dir/cmake_clean.cmake
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/clean

examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/depend:
	cd /home/maggul/STS/sundials/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/maggul/STS/sundials /home/maggul/STS/sundials/examples/arkode/C_serial /home/maggul/STS/sundials/build /home/maggul/STS/sundials/build/examples/arkode/C_serial /home/maggul/STS/sundials/build/examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_root.dir/depend

