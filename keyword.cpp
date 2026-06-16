#include "encryption_interface.h"
#include <vector>
#include <string>
#include <array>
#include <random>
#include <stdexcept>

using namespace std;

class KeywordPlugin : public IEncryptionPlugin {
private:
    array<unsigned char, 256> build_table(const string& key) {
        if (key.empty()) {
            throw invalid_argument("Ключ не может быть пустым!");
        }
        
        array<unsigned char, 256> table = {0};
        array<bool, 256> used = {false};
        size_t pos = 0;
        
        // Шаг 1: уникальные байты ключа в порядке появления
        for (unsigned char c : key) {
            if (!used[c]) {
                table[pos++] = c;
                used[c] = true;
            }
        }
        
        // Шаг 2: остальные байты в порядке возрастания
        for (int i = 0; i < 256; ++i) {
            if (!used[i]) {
                table[pos++] = static_cast<unsigned char>(i);
            }
        }
        return table;
    }
    
    array<unsigned char, 256> build_inverse_table(const array<unsigned char, 256>& table) {
        array<unsigned char, 256> inv = {0};
        for (int i = 0; i < 256; ++i) {
            inv[table[i]] = static_cast<unsigned char>(i);
        }
        return inv;
    }
    
public:
    PluginInfo getInfo() override {
        PluginInfo info;
        info.name = "Keyword";
        info.extension = ".kwrd";
        info.description = "Шифр кодового слова - подстановка по таблице 256x256";
        return info;
    }
    
    // Генерация случайного кодового слова из 8 латинских символов
    string generateKey() override {
        random_device rd;
        mt19937 gen(rd());
        // Используем printable ASCII (33-126) чтобы ключ был читаемым
        uniform_int_distribution<int> dist(33, 126);
        
        string key;
        for (int i = 0; i < 8; ++i) {
            key += static_cast<char>(dist(gen));
        }
        return key;
    }
    
    vector<unsigned char> encrypt(const vector<unsigned char>& data, const string& key) override {
        auto table = build_table(key);
        vector<unsigned char> result(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = table[data[i]];
        }
        return result;
    }
    
    vector<unsigned char> decrypt(const vector<unsigned char>& data, const string& key) override {
        auto table = build_table(key);
        auto invTable = build_inverse_table(table);
        vector<unsigned char> result(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = invTable[data[i]];
        }
        return result;
    }
};

extern "C" {
    IEncryptionPlugin* create_plugin() {
        return new KeywordPlugin();
    }
    
    void destroy_plugin(IEncryptionPlugin* plugin) {
        delete plugin;
    }
}
