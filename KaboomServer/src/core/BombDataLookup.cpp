#include "BombDataLookup.h"

const BombDataLookup BombDataLookup::instance("data/bombs.xml");

BombDataLookup::BombDataLookup(const std::string &filename) {
    loadXMLFile(filename);
}

void BombDataLookup::loadXMLNode(osgDB::XmlNode *xmlRoot) {
    if (xmlRoot->type == osgDB::XmlNode::ROOT) {
        for (auto child : xmlRoot->children) {
            if (child->name == "bombs") {
                return loadXMLNode(child);
            }
        }

        return;
    }

    for (auto bombNode : xmlRoot->children) {
        if (bombNode->name != "bomb") {
            continue;
        }

        BombData data;
        memset(&data, 0, sizeof(BombData));

        for (auto dataNode : bombNode->children) {
            if (dataNode->name == "id") {
                loadUint(dataNode, data.id);
            } else if (dataNode->name == "name") {
                loadString(dataNode, data.name);
            } else if (dataNode->name == "size") {
                loadFloat(dataNode, data.size);
            } else if (dataNode->name == "mass") {
                loadFloat(dataNode, data.mass);
            } else if (dataNode->name == "explosion-radius") {
                loadFloat(dataNode, data.explosionRadius);
            } else if (dataNode->name == "cooldown") {
                loadInt(dataNode, data.cooldown);
            } else if (dataNode->name == "launch-speed") {
                loadInt(dataNode, data.launchSpeed);
            }
        }

        bombs[static_cast<EntityType>(data.id)] = data;
    }
}

const BombData &BombDataLookup::operator[](const EntityType &type) const {
    return bombs.at(type);
}
