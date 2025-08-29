# Makefile
CXX = x86_64-w64-mingw32-g++
WINDRES = x86_64-w64-mingw32-windres
STRIP = x86_64-w64-mingw32-strip

# Compiler Flags
CXXFLAGS = -std=c++17 -O2 -Wall -mwindows -static-libgcc -static-libstdc++
LDFLAGS = -lgdi32 -lgdiplus -luser32 -lole32 -lshell32 -lshlwapi -static

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/chatwheel.exe

# Source files
SOURCES = $(SRC_DIR)/main.cpp
OBJECTS = $(BUILD_DIR)/main.o

# Build rules
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)
	$(STRIP) $(TARGET)
	@echo "Build complete: $(TARGET)"
	@echo "Size: $$(du -h $(TARGET) | cut -f1)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

test:
	wine $(TARGET)

.PHONY: all clean test
