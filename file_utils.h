#pragma once
#include <vector>
#include <string>

std::vector<unsigned char> readFile(const std::string& path);
void writeFile(const std::string& path, const std::vector<unsigned char>& data);
std::string getPassword(const std::string& prompt = "Введите ключ: ");
std::string toHex(const std::vector<unsigned char>& data);
