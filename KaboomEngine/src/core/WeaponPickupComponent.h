#pragma once

#include <ostream>

#include "Component.h"
#include "EntityType.h"

class WeaponPickupComponent : public Component {
public:
    WeaponPickupComponent(
            const EntityType &type = KABOOM_V2,
            unsigned int amount = 1);

    const EntityType &getBombType() const;
    unsigned int getAmount() const;

    void setBombType(const EntityType &type);
    void setAmount(unsigned int amount);

    friend std::ostream &operator<<(std::ostream &os, const WeaponPickupComponent &o) {
        os << "WeaponPickupComponent: {" << std::endl;
        os << "    bombType: " << o.bombType << std::endl;
        os << "    amount:   " << o.amount << std::endl;
        os << "}";

        return os;
    }

private:
    EntityType bombType;
    unsigned int amount;
};
