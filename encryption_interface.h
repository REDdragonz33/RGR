#pragma once
#include <vector>
#include <string>

// Информация о плагине
struct PluginInfo {
    std::string name;        // "Gronsveld", "Keyword", "A5/1"
    std::string extension;   // ".grn", ".kwrd", ".a51"
    std::string description; // Описание шифра
};

// Интерфейс всех плагинов шифрования
class IEncryptionPlugin {
public:
    virtual ~IEncryptionPlugin() {}
    
    // Получить информацию о плагине
    virtual PluginInfo getInfo() = 0;
    
    // Зашифровать данные (key - строковый ключ)
    virtual std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data,
                                               const std::string& key) = 0;
    
    // Расшифровать данные
    virtual std::vector<unsigned char> decrypt(const std::vector<unsigned char>& data,
                                               const std::string& key) = 0;
    
    // Сгенерировать случайный ключ для данного алгоритма
    virtual std::string generateKey() = 0;
};

// Функции для динамической загрузки
extern "C" {
    typedef IEncryptionPlugin* (*create_plugin_t)();
    typedef void (*destroy_plugin_t)(IEncryptionPlugin*);
}
