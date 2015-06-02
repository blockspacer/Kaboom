#pragma once

#include <string>
#include <unordered_map>

class Configuration;
enum EntityType;

class EntityConfigLookup {
public:
    static const Configuration &get(EntityType type);
    static void load();

private:
    static bool initialized;
    static EntityConfigLookup instance;

    static void initialize();

    std::unordered_map<EntityType, Configuration> config;

    EntityConfigLookup() {}

    void loadBombConfig(const std::string &filename);
    void loadCharacterConfig(const std::string &filename);
};
