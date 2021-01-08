COMPILE =$(CXX)
LINK = $(COMPILE)
CFLAGS = -std=c++14
PROGRAM = runnable
BUILD_DIR = Debug

ouput: checkdirs $(BUILD_DIR)/CppGoldies002.o
	$(LINK) $(BUILD_DIR)/CppGoldies002.o -o $(BUILD_DIR)/$(PROGRAM)

$(BUILD_DIR)/CppGoldies002.o: CppGoldies002.cpp
	$(COMPILE) CppGoldies002.cpp -c $(CFLAGS) -o $@

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	$(shell mkdir -p $@)

clean:
	rm -rf $(BUILD_DIR)