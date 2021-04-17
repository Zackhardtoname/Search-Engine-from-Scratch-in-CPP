## EECS 440 Makefile
## Auther: Emerson Pfeiffer (epfeiffe)

#######################
#######################
# Change 'executable' to match the command for the given component
EXECUTABLE  = ranker

# Change 'source_files' to match the component source files needed
SOURCE_FILES = main.cpp ../stl/String.cpp ../indexer/PorterStemming.cpp

#######################
#######################

# designate which compiler to use
CXX         = g++

# list of objects used in project
OBJECTS =  $(SOURCE_FILES:.cpp=.o)

# Default Flags
CXXFLAGS = -std=c++1z -pedantic#-Werror -Wconversion -Wall -Wextra

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
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LINK_FLAGS)
endif

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Ofast -c $*.cpp -o $*.o

# make clean - remove .o files, executables, tarball
clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(EXECUTABLE)_debug
	rm -Rf *.dSYM


# these targets do not create any files
.PHONY: all debug clean

# disable built-in rules
.SUFFIXES:
