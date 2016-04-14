# Compiler and linker
CXX            += -std=c++14
CPPFLAGS        = -DDEBUG
CXXFLAGS	= -O3 -g
CXXFLAGS       += -Wall
LDFLAGS	    	= 
LDLIBS          = 

#-------------------------------------------------------------------------------

# How to compile a C++ file, and generate automatic dependencies.
%.o:	    	    	%.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -c -o $@

# How to generate assember for C++ files.
%.s:	    	    	%.cc force
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -S -o $@

# How to link an executable. 
%:  	    	    	%.o
%:  	    	    	%.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

#-------------------------------------------------------------------------------

.PHONY: all
all:			dot

dot:	    	    	dot.o util.o json.o

# Use this target as a dependency to force another target to be rebuilt.
.PHONY: force
force: ;

