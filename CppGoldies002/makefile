COMPILE = c++
LINK = $(COMPILE)
CFLAGS = -std=c++14
PROGRAM = CppGoldies002
OUTFOLDER = Debug

ouput: CppGoldies002.o
	$(LINK) $(OUTFOLDER)/CppGoldies002.o -o $(OUTFOLDER)/$(PROGRAM)

CppGoldies002.o: CppGoldies002.cpp
	$(COMPILE) CppGoldies002.cpp -c $(CFLAGS)

clean:
	rm -rf $(OUTFOLDER)