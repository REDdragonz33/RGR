#include "module_manager.h"
#include <iostream>
#include <dlfcn.h>

using namespace std;

ModuleManager::ModuleManager() {}

ModuleManager::~ModuleManager() {
    unloadAll();
}

bool ModuleManager::loadModule(const string& path) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        cerr << "Ошибка загрузки " << path << ": " << dlerror() << endl;
        return false;
    }
    
    dlerror();
    create_plugin_t create = (create_plugin_t)dlsym(handle, "create_plugin");
    const char* error = dlerror();
    if (error) {
        cerr << "Не найден create_plugin: " << error << endl;
        dlclose(handle);
        return false;
    }
    
    IEncryptionPlugin* plugin = create();
    if (!plugin) {
        cerr << "create_plugin вернул nullptr" << endl;
        dlclose(handle);
        return false;
    }
    
    PluginInfo info = plugin->getInfo();
    LoadedModule mod = {handle, plugin};
    modules[info.name] = mod;
    extToName[info.extension] = info.name;
    
    cout << "✓ Загружен: " << info.name << " (" << info.extension << ")" << endl;
    return true;
}

IEncryptionPlugin* ModuleManager::getPlugin(const string& name) {
    map<string, LoadedModule>::iterator it = modules.find(name);
    return (it != modules.end()) ? it->second.plugin : NULL;
}

IEncryptionPlugin* ModuleManager::getPluginByExtension(const string& ext) {
    map<string, string>::iterator it = extToName.find(ext);
    if (it == extToName.end()) return NULL;
    return getPlugin(it->second);
}

vector<string> ModuleManager::getPluginNames() {
    vector<string> names;
    for (map<string, LoadedModule>::iterator it = modules.begin(); it != modules.end(); ++it) {
        names.push_back(it->first);
    }
    return names;
}

vector<string> ModuleManager::getPluginExtensions() {
    vector<string> exts;
    for (map<string, string>::iterator it = extToName.begin(); it != extToName.end(); ++it) {
        exts.push_back(it->first);
    }
    return exts;
}

void ModuleManager::unloadAll() {
    for (map<string, LoadedModule>::iterator it = modules.begin(); it != modules.end(); ++it) {
        LoadedModule& mod = it->second;
        destroy_plugin_t destroy = (destroy_plugin_t)dlsym(mod.handle, "destroy_plugin");
        if (destroy && mod.plugin) {
            destroy(mod.plugin);
        }
        if (mod.handle) {
            dlclose(mod.handle);
        }
    }
    modules.clear();
    extToName.clear();
}