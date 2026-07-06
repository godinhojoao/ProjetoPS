CXX = g++
CXXFLAGS = -std=c++17 -O2 -I src

SRC = src/main_cli.cpp \
      src/shared/shared.cpp \
      src/vm/vm.cpp \
      src/cpu/cpu.cpp \
      src/instruction_set/decoder.cpp \
      src/loader/binary_loader.cpp \
      src/linker/linker.cpp \
      src/macro_processor/macro_processor.cpp \
      src/memory/memory.cpp

OUT = vm

TEST_LOADER_SRC = tests/loader_test.cpp \
                  src/loader/binary_loader.cpp \
                  src/memory/memory.cpp
TEST_LOADER_OUT = test_loader

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT) $(ARGS)

test-loader:
	$(CXX) $(CXXFLAGS) $(TEST_LOADER_SRC) -o $(TEST_LOADER_OUT)
	./$(TEST_LOADER_OUT)

clean:
	rm -f $(OUT) $(TEST_LOADER_OUT)
