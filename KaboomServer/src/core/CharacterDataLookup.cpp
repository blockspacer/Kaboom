#include "CharacterDataLookup.h"

const CharacterDataLookup CharacterDataLookup::instance("data/characters.xml");

CharacterDataLookup::CharacterDataLookup(const std::string &filename) {
    loadXMLFile(filename);
}

void CharacterDataLookup::loadXMLNode(osgDB::XmlNode *xmlRoot) {
    if (xmlRoot->type == osgDB::XmlNode::ROOT) {
        for (auto child : xmlRoot->children) {
            if (child->name == "characters") {
                return loadXMLNode(child);
            }
        }

        return;
    }

    for (auto characterNode : xmlRoot->children) {
        if (characterNode->name != "character") {
            continue;
        }

        CharacterData data;
        data.id = 0;
        data.name = std::string();
        data.inventory = std::unordered_map<EntityType, int>();

        for (auto dataNode : characterNode->children) {
            if (dataNode->name == "id") {
                loadUint(dataNode, data.id);
            } else if (dataNode->name == "name") {
                loadString(dataNode, data.name);
            } else if (dataNode->name == "mass") {
                loadFloat(dataNode, data.mass);
            } else if (dataNode->name == "inventory") {
                for (auto itemNode : dataNode->children) {
                    unsigned int itemId = 0;
                    int amount = 0;

                    for (auto itemDataNode : itemNode->children) {
                        if (itemDataNode->name == "id") {
                            loadUint(itemDataNode, itemId);
                        } else if (itemDataNode->name == "amount") {
                            loadInt(itemDataNode, amount);
                        }
                    }

                    data.inventory[static_cast<EntityType>(itemId)] = amount;
                }
            }
        }

        characters[static_cast<EntityType>(data.id)] = data;
    }
}

const CharacterData &CharacterDataLookup::operator[](const EntityType &type) const {
    return characters.at(type);
}
