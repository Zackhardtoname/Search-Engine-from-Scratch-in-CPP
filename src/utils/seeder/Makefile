## EECS 440 Makefile
## Author: Emerson Pfeiffer (epfeiffe)

#######################
#######################
# Change 'executable' to match the command for the given component
EXECUTABLE  = seeder

# Change 'source_files' to match the component source files needed
SOURCE_FILES = Seeder.cpp ../../stl/String.cpp
#######################
#######################

# designate which compiler to use
CXX         = g++

# list of objects used in project
OBJECTS =  $(SOURCE_FILES:.cpp=.o)

# Default Flags
CXXFLAGS = -std=c++1z -pedantic #-Werror -Wconversion -Wall -Wextra

# Optimization Flag
OPT = -Ofast

# Libararies needed to link, including ssl and mqueue
LINK_FLAGS = -lssl -lrt -lcrypto

# make all - set as default command run
all: $(SOURCE_FILES) $(EXECUTABLE)

# make debug - will compile sources with $(CXXFLAGS) and the -g3 flag
#              also defines DEBUG, so "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(EXECUTABLE)_debug $(LINK_FLAGS)


$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile.
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJECTS) -o $@ $(LINK_FLAGS)
endif

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OPT) -c $*.cpp -o $*.o

# make clean - remove .o files, executables, tarball
clean:
	rm -f *.o $(EXECUTABLE) $(EXECUTABLE)_debug
	rm -Rf *.dSYM


# these targets do not create any files
.PHONY: all debug clean

# disable built-in rules
.SUFFIXES:
