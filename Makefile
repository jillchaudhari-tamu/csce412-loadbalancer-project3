# Compiler
CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17 -Iinclude

# Executable name
TARGET = loadbalancer

# Object files
OBJ = src/main.o src/LoadBalancer.o src/WebServer.o src/Simulation.o src/Logger.o src/ConfigLoader.o

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Compile main
src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o

# Compile LoadBalancer
src/LoadBalancer.o: src/LoadBalancer.cpp
	$(CXX) $(CXXFLAGS) -c src/LoadBalancer.cpp -o src/LoadBalancer.o

# Compile WebServer
src/WebServer.o: src/WebServer.cpp
	$(CXX) $(CXXFLAGS) -c src/WebServer.cpp -o src/WebServer.o

# Compile Simulation
src/Simulation.o: src/Simulation.cpp
	$(CXX) $(CXXFLAGS) -c src/Simulation.cpp -o src/Simulation.o

# Compile Logger
src/Logger.o: src/Logger.cpp
	$(CXX) $(CXXFLAGS) -c src/Logger.cpp -o src/Logger.o

# Compile ConfigLoader
src/ConfigLoader.o: src/ConfigLoader.cpp
	$(CXX) $(CXXFLAGS) -c src/ConfigLoader.cpp -o src/ConfigLoader.o

# Clean
clean:
	rm -f $(TARGET) src/*.o