# Compiler
CXX = g++

# --- Common Compiler Flags ---
COMMON_CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -I.
COMMON_CXXFLAGS += -O3 -march=native
# COMMON_CXXFLAGS += -ffast-math  # Consider if you really need this
# COMMON_CXXFLAGS += -pg -g
COMMON_LDFLAGS = -ltbb

# --- Default (x86-64) Target ---
CXXFLAGS = $(COMMON_CXXFLAGS)

# --- ARMv7 Target ---
ARMV7_CXX = arm-linux-androideabi-g++  # Use your ARM cross-compiler!  IMPORTANT!
ARMV7_CXXFLAGS = $(COMMON_CXXFLAGS) -DNO_STD_EXECUTION -fopenmp
ARMV7_LDFLAGS = $(COMMON_LDFLAGS) 
ARMV7_TARGET = solar_optimiser_armv7

# Source files (all in the Solar-Collector-Shape-Optimiser folder)
SOURCES = Solar-Collector-Shape-Optimiser/main.cpp \
		  Solar-Collector-Shape-Optimiser/mesh3d.cpp \
          Solar-Collector-Shape-Optimiser/genome.cpp \
          Solar-Collector-Shape-Optimiser/solarcollector.cpp \
          Solar-Collector-Shape-Optimiser/config.cpp 

		  

# Object files (will be placed in the current directory)
OBJECTS = $(SOURCES:.cpp=.o)
ARMV7_OBJECTS = $(SOURCES:.cpp=.armv7.o)


# --- Default target (x86-64) ---
TARGET = solar_optimiser

all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(COMMON_LDFLAGS)

# Rule to build object files (note the -I flag for header files)
Solar-Collector-Shape-Optimiser/%.o: Solar-Collector-Shape-Optimiser/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# --- ARMv7 Target ---
armv7: $(ARMV7_TARGET)

$(ARMV7_TARGET): $(ARMV7_OBJECTS)
	$(ARMV7_CXX) $(ARMV7_CXXFLAGS) $(ARMV7_OBJECTS) -o $(ARMV7_TARGET) $(ARMV7_LDFLAGS)

# Rule to build ARMv7 object files
Solar-Collector-Shape-Optimiser/%.armv7.o: Solar-Collector-Shape-Optimiser/%.cpp
	$(ARMV7_CXX) $(ARMV7_CXXFLAGS) -c $< -o $@



# Clean rule
clean:
	rm -f $(OBJECTS) $(ARMV7_OBJECTS) $(TARGET) $(ARMV7_TARGET) gmon.out

# Phony targets
.PHONY: all clean armv7