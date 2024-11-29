# Python
VENV := "/Users/nikhilg/Documents/Virtual Environments/Python/defaulter"
PYTHON := $(VENV)/bin/python

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Source files and targets
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

SERVER_BIN = server_exe
CLIENT_BIN = client_exe

.PHONY: all clean server client plot

# Build all targets
build: $(SERVER_BIN) $(CLIENT_BIN)

# Build server binary
$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build client binary
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Run server
server: $(SERVER_BIN)
	./$(SERVER_BIN)

# Run client
client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

# Plot graph
plot:
	$(PYTHON) ./p1_plot.py

# Clean up binaries
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)