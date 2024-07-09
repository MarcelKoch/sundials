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
include examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/progress.make

# Include the compile flags for this target's objects.
include examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/flags.make

examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o: examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/flags.make
examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o: ../examples/arkode/C_serial/ark_robertson_constraints.c
examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o: examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/maggul/STS/sundials/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o -MF CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o.d -o CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o -c /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_constraints.c

examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.i"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_constraints.c > CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.i

examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.s"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/maggul/STS/sundials/examples/arkode/C_serial/ark_robertson_constraints.c -o CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.s

# Object files for target ark_robertson_constraints
ark_robertson_constraints_OBJECTS = \
"CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o"

# External object files for target ark_robertson_constraints
ark_robertson_constraints_EXTERNAL_OBJECTS =

examples/arkode/C_serial/ark_robertson_constraints: examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/ark_robertson_constraints.c.o
examples/arkode/C_serial/ark_robertson_constraints: examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/build.make
examples/arkode/C_serial/ark_robertson_constraints: src/arkode/libsundials_arkode.so.6.0.0
examples/arkode/C_serial/ark_robertson_constraints: src/nvector/serial/libsundials_nvecserial.so.7.0.0
examples/arkode/C_serial/ark_robertson_constraints: src/sundials/libsundials_core.so.7.0.0
examples/arkode/C_serial/ark_robertson_constraints: examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/maggul/STS/sundials/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ark_robertson_constraints"
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ark_robertson_constraints.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/build: examples/arkode/C_serial/ark_robertson_constraints
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/build

examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/clean:
	cd /home/maggul/STS/sundials/build/examples/arkode/C_serial && $(CMAKE_COMMAND) -P CMakeFiles/ark_robertson_constraints.dir/cmake_clean.cmake
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/clean

examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/depend:
	cd /home/maggul/STS/sundials/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/maggul/STS/sundials /home/maggul/STS/sundials/examples/arkode/C_serial /home/maggul/STS/sundials/build /home/maggul/STS/sundials/build/examples/arkode/C_serial /home/maggul/STS/sundials/build/examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/arkode/C_serial/CMakeFiles/ark_robertson_constraints.dir/depend

