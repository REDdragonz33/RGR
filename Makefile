CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -fPIC
LDFLAGS = -ldl

all: libgronsveld.so libkeyword.so liba51.so main

libgronsveld.so: gronsveld.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

libkeyword.so: keyword.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

liba51.so: a51.cpp encryption_interface.h
	$(CXX) $(CXXFLAGS) -shared $< -o $@

main: main.cpp module_manager.cpp file_utils.cpp encryption_interface.h module_manager.h file_utils.h
	$(CXX) $(CXXFLAGS) main.cpp module_manager.cpp file_utils.cpp -o main $(LDFLAGS)

clean:
	rm -f *.so main *.grn *.kwrd *.a51 decrypted_*

install: all
	@echo "Готово! Запустите ./main"

.PHONY: all clean install
