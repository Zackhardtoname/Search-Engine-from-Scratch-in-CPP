CXXFLAGS = -std=c++17 -Wall -Werror -pedantic -Ofast -funsafe-math-optimizations -funroll-loops 
LIBS = -ldl -lssl -lcrypto

bf: main.cpp
	g++ $(CXXFLAGS) $^ $(LIBS) ../../../stl/include/Unique_Pointer.h ../../../stl/include/C20_PTR_FUNCTIONALITIES.h -o $@ 

clean:
	rm -rf bf
