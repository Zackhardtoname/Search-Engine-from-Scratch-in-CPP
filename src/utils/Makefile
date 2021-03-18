## EECS 440 Makefile
## Auther: Emerson Pfeiffer (epfeiffe)

#######################
# TODO (begin) #
#######################
# Change 'executable' to match the command for the given component
EXECUTABLE  = create-mq

# Change 'source_files' to match the component source files needed
SOURCE_FILES = CreateMQs.cpp
#######################
# TODO (end) #
#######################

# designate which compiler to use
CXX         = g++

# list of objects used in project
OBJECTS =  $(SOURCE_FILES:.cpp=.o)

# Default Flags
CXXFLAGS = -std=c++1z -pedantic -Ofast #-Werror -Wconversion -Wall -Wextra

# Libararies needed to link, including ssl and mqueue
LINK_FLAGS = -lssl -lrt -lcrypto

# make create - set as default command run
create: $(SOURCE_FILES) $(EXECUTABLE)

# make delete
delete: EXECUTABLE = delete-mq
delete:
	$(CXX) $(CXXFLAGS) DeleteMQs.cpp -o $(EXECUTABLE) $(LINK_FLAGS)

# make check
check: EXECUTABLE = check-mq
check:
	$(CXX) $(CXXFLAGS) CheckMQs.cpp -o $(EXECUTABLE) $(LINK_FLAGS)

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
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

# make clean - remove .o files, executables, tarball
clean:
	rm -f *.o $(EXECUTABLE) check-mq delete-mq $(EXECUTABLE)_debug
	rm -Rf *.dSYM


# these targets do not create any files
.PHONY: create delete debug clean

# disable built-in rules
.SUFFIXES:
