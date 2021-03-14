## EECS 281 Advanced Makefile

# How to use this Makefile...
###################
###################
##               ##
##  $ make help  ##
##               ##
###################
###################

#######################
# TODO (begin) #
#######################
# Change 'executable' to match the command for the given component
EXECUTABLE  = worker

# Change 'source_files' to match the project source files
SOURCE_FILES = Worker.cpp ../crawler/Crawler.cpp ../crawler/ParsedUrl.cpp ../stl/src/io/utils.cpp ../stl/src/String.cpp ../crawler/ParsedHttpResponse.cpp ../parser/HtmlParser.cpp ../parser/HtmlLexer.cpp

# Change 'header_files' to match the project source files
HEADER_FILES = Worker.h ../crawler/Crawler.h ../crawler/ParsedUrl.h ../parser/HtmlParser.h ../parser/HtmlLexer.h

#######################
# TODO (end) #
#######################

# enables c++17 on CAEN or 281 autograder
PATH := /usr/um/gcc-6.2.0/bin:$(PATH)
LD_LIBRARY_PATH := /usr/um/gcc-6.2.0/lib64
LD_RUN_PATH := /usr/um/gcc-6.2.0/lib64

# designate which compiler to use
CXX         = g++

# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%)

# list of sources used in project
SOURCES     = $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS     = $(SOURCES:%.cpp=%.o)

# name of the perf data file, only used by the clean target
PERF_FILE = perf.data*

#Default Flags (we prefer -std=c++17 but Mac/Xcode/Clang doesn't support)
CXXFLAGS = -std=c++1z -pedantic#-Werror -Wconversion -Wall -Wextra

LINK_FLAGS = -lssl

# make release - will compile "all" with $(CXXFLAGS) and the -O3 flag
#                also defines NDEBUG so that asserts will not check
release: CXXFLAGS += -Ofast
release:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(EXECUTABLE) $(LINK_FLAGS)

# make debug - will compile sources with $(CXXFLAGS) and the -g3 flag
#              also defines DEBUG, so "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(EXECUTABLE)_debug $(LINK_FLAGS)

# make profile - will compile "all" with $(CXXFLAGS) and the -g3 and -O3 flags
profile: CXXFLAGS += -g3 -Ofast
profile:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(EXECUTABLE)_profile $(LINK_FLAGS)
	
# make gprof - will compile "all" with $(CXXFLAGS) and the -pg (for gprof)
gprof: CXXFLAGS += -pg
gprof:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(EXECUTABLE)_profile $(LINK_FLAGS)

# make static - will perform static analysis in the matter currently used
#               on the autograder
static:
	cppcheck --enable=all --suppress=missingIncludeSystem \
      $(SOURCE_FILES) *.h *.hpp


# Build all executables
all: release debug profile

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile.
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)
endif

# rule for creating objects
%.o: %.cpp ${SOURCE_FILES}
	$(CXX) $(CXXFLAGS) -c $<
%.o: %.cpp ${HEADER_FILES}
	$(CXX) $(CXXFLAGS) -c $<

# make clean - remove .o files, executables, tarball
clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(EXECUTABLE)_debug $(EXECUTABLE)_profile \
      $(TESTS) $(PERF_FILE)
	rm -Rf *.dSYM



# these targets do not create any files
.PHONY: all release debug profile static clean alltests \
        ungraded help identifier
# disable built-in rules
.SUFFIXES:
