CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -fPIC
LDFLAGS = -ldl

all: libgronsveld.so libkeyword.so liba51.so main

libgronsveld.so: gronsveld.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

libkeyword.so: keyword.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

liba51.so: a51.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

main: main.cpp module_manager.cpp file_utils.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) main.cpp module_manager.cpp file_utils.cpp -o main $(LDFLAGS)

clean:
	rm -f *.so main *.grn *.kwrd *.a51 decrypted_*

install: all
	@echo "Готово! Запустите ./main"
	@echo ""
	@echo "Примеры:"
	@echo "  ./main list"
	@echo "  ./main encrypt test.txt Gronsveld"
	@echo "  ./main decrypt test.txt.grn"
	@echo "  ./main text encrypt Gronsveld"
	@echo "  ./main keygen Gronsveld"

.PHONY: all clean install