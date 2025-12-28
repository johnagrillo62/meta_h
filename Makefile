CXX = g++
CXXFLAGS = -std=c++20
LDFLAGS = -L/usr/local/lib -lyaml-cpp

BIN_DIR = bin

SOURCES = $(wildcard example*.cpp)
EXES = $(SOURCES:.cpp=)
BIN_EXES = $(addprefix $(BIN_DIR)/, $(EXES))

.PHONY: all clean run

all: $(BIN_EXES)

# Ensure bin exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build executables into bin/
$(BIN_DIR)/%: %.cpp meta.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

run: all
	@for exe in $(BIN_EXES); do \
		echo ""; \
		echo "====== Running $$exe ======"; \
		echo ""; \
		./$$exe; \
	done

clean:
	rm -rf $(BIN_DIR)


