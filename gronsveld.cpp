#include "encryption_interface.h"
#include <vector>
#include <string>
#include <cctype>
#include <random>
#include <stdexcept>

using namespace std;

class GronsveldPlugin : public IEncryptionPlugin {
private:
    vector<int> parse_key(const string& key) {
        vector<int> digits;
        for (unsigned char c : key) {
            if (isdigit(c)) {
                digits.push_back(c - '0');
            }
        }
        if (digits.empty()) {
            throw invalid_argument("Ключ должен содержать хотя бы одну цифру!");
        }
        return digits;
    }
    
public:
    PluginInfo getInfo() override {
        PluginInfo info;
        info.name = "Gronsveld";
        info.extension = ".grn";
        info.description = "Шифр Гронсфельда - сдвиг байта на цифру ключа";
        return info;
    }
    
    // Генерация случайного числового ключа длиной 6 цифр
    string generateKey() override {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(0, 9);
        
        string key;
        // Первая цифра не 0, чтобы ключ был значимым
        uniform_int_distribution<int> firstDist(1, 9);
        key += to_string(firstDist(gen));
        for (int i = 1; i < 6; ++i) {
            key += to_string(dist(gen));
        }
        return key;
    }
    
    vector<unsigned char> encrypt(const vector<unsigned char>& data, const string& key) override {
        vector<int> digits = parse_key(key);
        size_t keyLen = digits.size();
        vector<unsigned char> result(data.size());
        
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = (data[i] + digits[i % keyLen]) % 256;
        }
        return result;
    }
    
    vector<unsigned char> decrypt(const vector<unsigned char>& data, const string& key) override {
        vector<int> digits = parse_key(key);
        size_t keyLen = digits.size();
        vector<unsigned char> result(data.size());
        
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = (data[i] - digits[i % keyLen] + 256) % 256;
        }
        return result;
    }
};

extern "C" {
    IEncryptionPlugin* create_plugin() {
        return new GronsveldPlugin();
    }
    
    void destroy_plugin(IEncryptionPlugin* plugin) {
        delete plugin;
    }
}
