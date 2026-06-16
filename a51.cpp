#include "encryption_interface.h"
#include <vector>
#include <string>
#include <cstdint>
#include <random>
#include <stdexcept>

using namespace std;

struct A51State {
    uint32_t r1, r2, r3;
};

class A51Plugin : public IEncryptionPlugin {
private:
    // Обратная связь для R1 (19 бит): полином x^19 + x^18 + x^17 + x^14 + 1
    uint32_t r1_feedback(uint32_t r) {
        return ((r >> 18) ^ (r >> 17) ^ (r >> 16) ^ (r >> 13)) & 1u;
    }
    
    // Обратная связь для R2 (22 бита): полином x^22 + x^21 + 1
    uint32_t r2_feedback(uint32_t r) {
        return ((r >> 21) ^ (r >> 20)) & 1u;
    }
    
    // Обратная связь для R3 (23 бита): полином x^23 + x^22 + x^21 + x^8 + 1
    uint32_t r3_feedback(uint32_t r) {
        return ((r >> 22) ^ (r >> 21) ^ (r >> 20) ^ (r >> 7)) & 1u;
    }
    
    // Мажоритарная функция
    uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
        return (a & b) | (b & c) | (a & c);
    }
    
    // Один такт A5/1 с мажоритарным управлением
    uint32_t a51_clock(A51State& s) {
        uint32_t clk1 = (s.r1 >> 8)  & 1u;
        uint32_t clk2 = (s.r2 >> 10) & 1u;
        uint32_t clk3 = (s.r3 >> 10) & 1u;
        uint32_t maj = majority(clk1, clk2, clk3);
        
        if (clk1 == maj) {
            s.r1 = ((s.r1 << 1) | r1_feedback(s.r1)) & 0x7FFFFu;
        }
        if (clk2 == maj) {
            s.r2 = ((s.r2 << 1) | r2_feedback(s.r2)) & 0x3FFFFFu;
        }
        if (clk3 == maj) {
            s.r3 = ((s.r3 << 1) | r3_feedback(s.r3)) & 0x7FFFFFu;
        }
        
        return ((s.r1 >> 18) ^ (s.r2 >> 21) ^ (s.r3 >> 22)) & 1u;
    }
    
    // Инициализация A5/1 по стандарту GSM:
    // Фаза 1: принудительное тактирование всех регистров, XOR бита ключа в LSB
    // Фаза 2: то же с битами номера фрейма
    // Фаза 3: 100 тактов прогрева с мажоритарным управлением
    A51State init(uint64_t key, uint32_t frameNum) {
        A51State s = {0, 0, 0};
        
        // Фаза 1: загрузка 64 бит ключа
        for (int i = 0; i < 64; ++i) {
            // Принудительный такт всех регистров
            s.r1 = ((s.r1 << 1) | r1_feedback(s.r1)) & 0x7FFFFu;
            s.r2 = ((s.r2 << 1) | r2_feedback(s.r2)) & 0x3FFFFFu;
            s.r3 = ((s.r3 << 1) | r3_feedback(s.r3)) & 0x7FFFFFu;
            
            // XOR бита ключа в LSB (бит 0) каждого регистра
            uint32_t bit = (key >> i) & 1u;
            s.r1 ^= bit;
            s.r2 ^= bit;
            s.r3 ^= bit;
        }
        
        // Фаза 2: загрузка 22 бит номера фрейма
        for (int i = 0; i < 22; ++i) {
            // Принудительный такт всех регистров
            s.r1 = ((s.r1 << 1) | r1_feedback(s.r1)) & 0x7FFFFu;
            s.r2 = ((s.r2 << 1) | r2_feedback(s.r2)) & 0x3FFFFFu;
            s.r3 = ((s.r3 << 1) | r3_feedback(s.r3)) & 0x7FFFFFu;
            
            // XOR бита фрейма в LSB каждого регистра
            uint32_t bit = (frameNum >> i) & 1u;
            s.r1 ^= bit;
            s.r2 ^= bit;
            s.r3 ^= bit;
        }
        
        // Фаза 3: 100 тактов прогрева с мажоритарным управлением (вывод не берём)
        for (int i = 0; i < 100; ++i) {
            a51_clock(s);
        }
        return s;
    }
    
    // Генерация одного байта гаммы (LSB первым внутри байта)
    uint8_t next_byte(A51State& s) {
        uint8_t byte = 0;
        for (int i = 0; i < 8; ++i) {
            byte |= (a51_clock(s) << i);
        }
        return byte;
    }
    
    // Парсинг ключа: десятичное или hex с префиксом 0x
    uint64_t parse_key(const string& key) {
        if (key.empty()) {
            throw invalid_argument("Ключ не может быть пустым!");
        }
        try {
            size_t idx = 0;
            uint64_t result = stoull(key, &idx, 0);
            if (idx != key.size()) {
                throw invalid_argument("");
            }
            return result;
        } catch (...) {
            throw invalid_argument("Ключ должен быть 64-битным числом (десятичным или hex с 0x)");
        }
    }
    
public:
    PluginInfo getInfo() override {
        PluginInfo info;
        info.name = "A5/1";
        info.extension = ".a51";
        info.description = "Потоковый шифр A5/1 (GSM)";
        return info;
    }
    
    // Генерация случайного 64-битного числового ключа
    string generateKey() override {
        random_device rd;
        uint64_t key = ((uint64_t)rd() << 32) | rd();
        return to_string(key);
    }
    
    vector<unsigned char> encrypt(const vector<unsigned char>& data, const string& key) override {
        uint64_t k = parse_key(key);
        A51State state = init(k, 0);
        vector<unsigned char> result(data.size());
        
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = data[i] ^ next_byte(state);
        }
        return result;
    }
    
    vector<unsigned char> decrypt(const vector<unsigned char>& data, const string& key) override {
        // A5/1 симметричен: расшифровка = шифрование
        return encrypt(data, key);
    }
};

extern "C" {
    IEncryptionPlugin* create_plugin() {
        return new A51Plugin();
    }
    
    void destroy_plugin(IEncryptionPlugin* plugin) {
        delete plugin;
    }
}
