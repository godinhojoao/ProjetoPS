CXX = g++
CXXFLAGS = -std=c++17 -O2 -I src

SRC = src/main_cli.cpp \
      src/shared/shared.cpp \
      src/vm/vm.cpp \
      src/cpu/cpu.cpp \
      src/instruction_set/decoder.cpp \
      src/loader/binary_loader.cpp \
      src/loader/object_loader.cpp \
      src/linker/linker.cpp \
      src/macro_processor/macro_processor.cpp \
      src/linker/global_symbol_table.cpp \
      src/linker/extref_resolver.cpp \
      src/memory/memory.cpp

OUT = vm

TEST_LOADER_SRC = tests/loader_test.cpp \
                  src/loader/binary_loader.cpp \
                  src/memory/memory.cpp
TEST_LOADER_OUT = test_loader

TEST_OBJECT_LOADER_SRC = tests/object_loader_test.cpp \
                         src/loader/object_loader.cpp \
                         src/shared/shared.cpp \
                         src/memory/memory.cpp
TEST_OBJECT_LOADER_OUT = test_object_loader

TEST_LINKER_GTS_SRC = tests/linker_gts_test.cpp \
                      src/linker/global_symbol_table.cpp \
                      src/shared/shared.cpp
TEST_LINKER_GTS_OUT = test_linker_gts

TEST_LINKER_EXTREF_SRC = tests/linker_extref_test.cpp \
                         src/linker/extref_resolver.cpp \
                         src/linker/global_symbol_table.cpp \
                         src/shared/shared.cpp
TEST_LINKER_EXTREF_OUT = test_linker_extref

TEST_LINKER_SRC = tests/linker_test.cpp \
                  src/linker/linker.cpp \
                  src/linker/extref_resolver.cpp \
                  src/linker/global_symbol_table.cpp \
                  src/shared/shared.cpp
TEST_LINKER_OUT = test_linker

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT) $(ARGS)

test-loader:
	$(CXX) $(CXXFLAGS) $(TEST_LOADER_SRC) -o $(TEST_LOADER_OUT)
	./$(TEST_LOADER_OUT)

test-object-loader:
	$(CXX) $(CXXFLAGS) $(TEST_OBJECT_LOADER_SRC) -o $(TEST_OBJECT_LOADER_OUT)
	./$(TEST_OBJECT_LOADER_OUT)

test-linker-gts:
	$(CXX) $(CXXFLAGS) $(TEST_LINKER_GTS_SRC) -o $(TEST_LINKER_GTS_OUT)
	./$(TEST_LINKER_GTS_OUT)

test-linker-extref:
	$(CXX) $(CXXFLAGS) $(TEST_LINKER_EXTREF_SRC) -o $(TEST_LINKER_EXTREF_OUT)
	./$(TEST_LINKER_EXTREF_OUT)

test-linker:
	$(CXX) $(CXXFLAGS) $(TEST_LINKER_SRC) -o $(TEST_LINKER_OUT)
	./$(TEST_LINKER_OUT)

clean:
	rm -f $(OUT) $(TEST_LOADER_OUT) $(TEST_OBJECT_LOADER_OUT) $(TEST_LINKER_GTS_OUT) $(TEST_LINKER_EXTREF_OUT) $(TEST_LINKER_OUT)
