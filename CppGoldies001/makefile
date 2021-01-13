COMPILE = $(CXX)
LINK = $(COMPILE)
CFLAGS = -std=c++14
PROGRAM = runnable
BUILD_DIR = Debug

ouput: checkdirs $(BUILD_DIR)/CppGoldies001.o
	$(LINK) $(BUILD_DIR)/CppGoldies001.o -o $(BUILD_DIR)/$(PROGRAM)

$(BUILD_DIR)/CppGoldies001.o: CppGoldies001.cpp
	$(COMPILE) CppGoldies001.cpp -c $(CFLAGS) -o $@

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	$(shell mkdir -p $@)

clean:
	rm -rf $(BUILD_DIR)
