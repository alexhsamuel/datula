# Compiler and linker
CXX            += -std=c++14
CPPFLAGS        = -DDEBUG
CXXFLAGS	= -O3 -g
CXXFLAGS       += -Wall
LDFLAGS	    	= 
LDLIBS          = 
CXXFILT	    	= c++filt

#-------------------------------------------------------------------------------

# Compile a C++ file, and generate automatic dependencies.
%.o:	    	    	%.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -c -o $@

# Generate assembler for a C++ file.
%.s:	    	    	%.cc force
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -S -o $@

# Generate assembler for a C++ file, demangling C++ names.
%.ss:	    	    	%.cc force
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -S -o - | $(CXXFILT) > $@

# Link an executable. 
%:  	    	    	%.o
%:  	    	    	%.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

#-------------------------------------------------------------------------------

.PHONY: all
all:			dot

dot:	    	    	dot.o util.o json.o

timing_distribution:	timing_distribution.o util.o # -lpapi

# Use this target as a dependency to force another target to be rebuilt.
.PHONY: force
force: ;

