#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

vector<unsigned char> readFile(const string& path) {
    ifstream file(path, ios::binary);
    if (!file) {
        throw runtime_error("Не удалось открыть файл: " + path);
    }

    file.seekg(0, ios::end);
    size_t size = (size_t)file.tellg();
    file.seekg(0, ios::beg);

    vector<unsigned char> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);

    return data;
}

void writeFile(const string& path, const vector<unsigned char>& data) {
    ofstream file(path, ios::binary);
    if (!file) {
        throw runtime_error("Не удалось создать файл: " + path);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

string getPassword(const string& prompt) {
    string pass;
    cout << prompt;
    getline(cin, pass);
    return pass;
}

string toHex(const vector<unsigned char>& data) {
    ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        oss << hex << setw(2) << setfill('0') << (int)data[i];
        if (i + 1 < data.size()) oss << " ";
    }
    return oss.str();
}