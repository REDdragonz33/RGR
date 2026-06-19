#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include "module_manager.h"
#include "file_utils.h"

using namespace std;

// Проверка существования файла (C++11 способ)
bool fileExists(const string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Получение расширения файла
string getExtension(const string& path) {
    size_t dot = path.rfind('.');
    if (dot == string::npos) return "";
    return path.substr(dot);
}

// Получение имени файла без расширения
string getStem(const string& path) {
    size_t dot = path.rfind('.');
    if (dot == string::npos) return path;
    return path.substr(0, dot);
}

void showHelp() {
    cout << "\n=== Криптографическая система ===" << endl;
    cout << "Команды:" << endl;
    cout << "  list                         - показать доступные шифры" << endl;
    cout << "  encrypt <файл> <шифр>        - зашифровать файл" << endl;
    cout << "  decrypt <файл>               - расшифровать файл" << endl;
    cout << "  text encrypt <шифр>          - зашифровать текст из консоли" << endl;
    cout << "  text decrypt <шифр>          - расшифровать hex-текст из консоли" << endl;
    cout << "  keygen <шифр>                - сгенерировать ключ" << endl;
    cout << "  help                         - показать эту справку" << endl;
    cout << "  exit                         - выход" << endl;
    cout << "\nДоступные шифры: Gronsveld, Keyword, A5/1" << endl;
    cout << "\nПримеры ключей:" << endl;
    cout << "  Gronsveld : цифры           (например: 31415)" << endl;
    cout << "  Keyword   : любая строка    (например: secret)" << endl;
    cout << "  A5/1      : 64-битное число (например: 123456789 или 0xDEADBEEF)" << endl;
    cout << "\nВсе файлы сохраняются в текущую папку." << endl;
}

void cmdList(ModuleManager& manager) {
    vector<string> names = manager.getPluginNames();
    if (names.empty()) {
        cout << "Нет загруженных шифров." << endl;
        return;
    }
    cout << "\nДоступные шифры:" << endl;
    for (size_t i = 0; i < names.size(); ++i) {
        const string& name = names[i];
        IEncryptionPlugin* plugin = manager.getPlugin(name);
        if (plugin) {
            PluginInfo info = plugin->getInfo();
            cout << "  • " << info.name << " - " << info.description << endl;
            cout << "    Расширение: " << info.extension << endl;
        }
    }
}

void cmdEncryptFile(ModuleManager& manager, const string& filePath, const string& pluginName) {
    if (!fileExists(filePath)) {
        cerr << "Ошибка: файл не найден - " << filePath << endl;
        return;
    }
    
    IEncryptionPlugin* plugin = manager.getPlugin(pluginName);
    if (!plugin) {
        cerr << "Ошибка: шифр не найден - " << pluginName << endl;
        cerr << "Доступные: Gronsveld, Keyword, A5/1" << endl;
        return;
    }
    
    string key = getPassword();
    PluginInfo info = plugin->getInfo();
    
    try {
        cout << "Шифрование с помощью " << info.name << "..." << endl;
        vector<unsigned char> data = readFile(filePath);
        vector<unsigned char> encrypted = plugin->encrypt(data, key);
        
        string outPath = filePath + info.extension;
        writeFile(outPath, encrypted);
        cout << "✓ Зашифровано: " << outPath << endl;
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void cmdDecryptFile(ModuleManager& manager, const string& filePath) {
    if (!fileExists(filePath)) {
        cerr << "Ошибка: файл не найден - " << filePath << endl;
        return;
    }
    
    string ext = getExtension(filePath);
    IEncryptionPlugin* plugin = manager.getPluginByExtension(ext);
    
    if (!plugin) {
        cerr << "Ошибка: не найден шифр для расширения " << ext << endl;
        return;
    }
    
    string key = getPassword();
    PluginInfo info = plugin->getInfo();
    
    try {
        cout << "Дешифрование с помощью " << info.name << "..." << endl;
        vector<unsigned char> data = readFile(filePath);
        vector<unsigned char> decrypted = plugin->decrypt(data, key);
        
        string outPath = getStem(filePath);
        writeFile(outPath, decrypted);
        cout << "✓ Расшифровано: " << outPath << endl;
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void cmdEncryptText(ModuleManager& manager, const string& pluginName) {
    IEncryptionPlugin* plugin = manager.getPlugin(pluginName);
    if (!plugin) {
        cerr << "Ошибка: шифр не найден - " << pluginName << endl;
        return;
    }
    
    string key = getPassword();
    
    string text;
    cout << "Введите текст: ";
    getline(cin, text);
    
    try {
        vector<unsigned char> data(text.begin(), text.end());
        vector<unsigned char> encrypted = plugin->encrypt(data, key);
        cout << "\nЗашифрованный текст (hex):" << endl;
        cout << toHex(encrypted) << endl;
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void cmdDecryptText(ModuleManager& manager, const string& pluginName) {
    IEncryptionPlugin* plugin = manager.getPlugin(pluginName);
    if (!plugin) {
        cerr << "Ошибка: шифр не найден - " << pluginName << endl;
        return;
    }
    
    string key = getPassword();
    
    string hexText;
    cout << "Введите hex-текст (байты через пробел, например: 41 42 43): ";
    getline(cin, hexText);
    
    vector<unsigned char> data;
    size_t pos = 0;
    while (pos < hexText.size()) {
        while (pos < hexText.size() && hexText[pos] == ' ') ++pos;
        if (pos + 2 > hexText.size()) break;
        try {
            unsigned char byte = static_cast<unsigned char>(stoul(hexText.substr(pos, 2), NULL, 16));
            data.push_back(byte);
        } catch (...) {
            cerr << "Ошибка: некорректный hex на позиции " << pos << endl;
            return;
        }
        pos += 2;
    }
    
    if (data.empty()) {
        cerr << "Ошибка: не удалось прочитать hex-данные." << endl;
        return;
    }
    
    try {
        vector<unsigned char> decrypted = plugin->decrypt(data, key);
        cout << "\nРасшифрованный текст:" << endl;
        cout << string(decrypted.begin(), decrypted.end()) << endl;
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void cmdKeygen(ModuleManager& manager, const string& pluginName) {
    IEncryptionPlugin* plugin = manager.getPlugin(pluginName);
    if (!plugin) {
        cerr << "Ошибка: шифр не найден - " << pluginName << endl;
        cerr << "Доступные: Gronsveld, Keyword, A5/1" << endl;
        return;
    }
    
    string key = plugin->generateKey();
    cout << "Сгенерированный ключ для " << pluginName << ": " << key << endl;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "RUS");
    
    ModuleManager manager;
    
    cout << "\nЗагрузка модулей..." << endl;
    manager.loadModule("./libgronsveld.so");
    manager.loadModule("./libkeyword.so");
    manager.loadModule("./liba51.so");
    
    if (argc > 1) {
        string cmd = argv[1];
        
        if (cmd == "list") {
            cmdList(manager);
            return 0;
        }
        
        if (cmd == "encrypt" && argc >= 4) {
            cmdEncryptFile(manager, argv[2], argv[3]);
            return 0;
        }
        
        if (cmd == "decrypt" && argc >= 3) {
            cmdDecryptFile(manager, argv[2]);
            return 0;
        }
        
        if (cmd == "text" && argc >= 4) {
            string mode = argv[2];
            string pluginName = argv[3];
            if (mode == "encrypt") {
                cmdEncryptText(manager, pluginName);
            } else if (mode == "decrypt") {
                cmdDecryptText(manager, pluginName);
            } else {
                cerr << "Использование: text encrypt|decrypt <шифр>" << endl;
            }
            return 0;
        }
        
        if (cmd == "keygen" && argc >= 3) {
            cmdKeygen(manager, argv[2]);
            return 0;
        }
        
        showHelp();
        return 0;
    }
    
    showHelp();
    
    string input;
    while (true) {
        cout << "\n>> ";
        if (!getline(cin, input)) break;
        if (input.empty()) continue;
        
        if (input == "exit" || input == "quit") {
            cout << "До свидания!" << endl;
            break;
        }
        
        if (input == "help") {
            showHelp();
            continue;
        }
        
        if (input == "list") {
            cmdList(manager);
            continue;
        }
        
        vector<string> tokens;
        size_t pos = 0;
        while (pos < input.size()) {
            while (pos < input.size() && input[pos] == ' ') ++pos;
            size_t end = input.find(' ', pos);
            if (end == string::npos) end = input.size();
            if (end > pos) tokens.push_back(input.substr(pos, end - pos));
            pos = end;
        }
        
        if (tokens.empty()) continue;
        
        if (tokens[0] == "encrypt" && tokens.size() >= 3) {
            cmdEncryptFile(manager, tokens[1], tokens[2]);
            continue;
        }
        
        if (tokens[0] == "decrypt" && tokens.size() >= 2) {
            cmdDecryptFile(manager, tokens[1]);
            continue;
        }
        
        if (tokens[0] == "text" && tokens.size() >= 3) {
            if (tokens[1] == "encrypt") {
                cmdEncryptText(manager, tokens[2]);
            } else if (tokens[1] == "decrypt") {
                cmdDecryptText(manager, tokens[2]);
            } else {
                cerr << "Использование: text encrypt|decrypt <шифр>" << endl;
            }
            continue;
        }
        
        if (tokens[0] == "keygen" && tokens.size() >= 2) {
            cmdKeygen(manager, tokens[1]);
            continue;
        }
        
        cout << "Неизвестная команда. Введите 'help' для справки." << endl;
    }
    
    return 0;
}