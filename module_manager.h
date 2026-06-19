#pragma once
#include "encryption_interface.h"
#include <map>
#include <string>
#include <vector>

class ModuleManager {
private:
    struct LoadedModule {
        void* handle;
        IEncryptionPlugin* plugin;
    };
    
    std::map<std::string, LoadedModule> modules;
    std::map<std::string, std::string> extToName;
    
public:
    ModuleManager();
    ~ModuleManager();
    
    bool loadModule(const std::string& path);
    IEncryptionPlugin* getPlugin(const std::string& name);
    IEncryptionPlugin* getPluginByExtension(const std::string& ext);
    std::vector<std::string> getPluginNames();
    std::vector<std::string> getPluginExtensions();
    void unloadAll();
};