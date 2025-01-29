# Compiler
CXX = g++

# Compiler flags (added -I for include directory)
CXXFLAGS = -std=c++23 -Wall -Wextra -fopenmp -I.
# Use '-g' to generate debug information, or '-O3' for optimization
# CXXFLAGS += -g
CXXFLAGS += -O3
# Output executable name
TARGET = solar_optimiser

# Source files (all in the Solar-Collector-Shape-Optimiser folder)
SOURCES = Solar-Collector-Shape-Optimiser/main.cpp \
		  Solar-Collector-Shape-Optimiser/mesh3d.cpp \
          Solar-Collector-Shape-Optimiser/specimen.cpp \
          Solar-Collector-Shape-Optimiser/solarcollector.cpp 
		  

# Object files (will be placed in the current directory)
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Rule to build object files (note the -I flag for header files)
Solar-Collector-Shape-Optimiser/%.o: Solar-Collector-Shape-Optimiser/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean