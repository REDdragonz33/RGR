#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

using namespace std;
namespace fs = filesystem;

vector<unsigned char> readFile(const string& path) {
    ifstream file(path, ios::binary);
    if (!file) {
        throw runtime_error("Не удалось открыть файл: " + path);
    }
    return vector<unsigned char>((istreambuf_iterator<char>(file)),
                                  istreambuf_iterator<char>());
}

void writeFile(const string& path, const vector<unsigned char>& data) {
    // Создаём директории если их нет
    fs::path filePath(path);
    if (filePath.has_parent_path() && !fs::exists(filePath.parent_path())) {
        cout << "Директория '" << filePath.parent_path().string()
             << "' не существует. Создать? (y/n): ";
        char answer;
        cin >> answer;
        cin.ignore();
        if (answer == 'y' || answer == 'Y') {
            fs::create_directories(filePath.parent_path());
        } else {
            throw runtime_error("Запись отменена пользователем.");
        }
    }
    
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

// Перевод байт в hex-строку для вывода в консоль
string toHex(const vector<unsigned char>& data) {
    ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        oss << hex << setw(2) << setfill('0') << (int)data[i];
        if (i + 1 < data.size()) oss << " ";
    }
    return oss.str();
}
