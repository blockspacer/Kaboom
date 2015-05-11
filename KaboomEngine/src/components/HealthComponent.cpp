#include "HealthComponent.h"

HealthComponent::HealthComponent(int _healthAmoumt, int _healthCap)
:   healthAmount(_healthAmoumt),
    healthCap(_healthCap),
	shieldAmount(0),
	shieldCap(0){
}


HealthComponent::~HealthComponent() {
}

void HealthComponent::addToHealthAmount(int healthAmount) {
	if (this->healthAmount + healthAmount > healthCap) {
		this->healthAmount = healthCap;
	} else {
		this->healthAmount += healthAmount;
	}
}
void HealthComponent::subtractFromHealthAmount(int healthAmount) {
	if (this->healthAmount - healthAmount < 0) {
		this->healthAmount = 0;
	} else {
		this->healthAmount -= healthAmount;
	}
}


int HealthComponent::getHealthAmount() const {
	return healthAmount;
}
int HealthComponent::getShieldAmount() const {
	return shieldAmount;
}

void HealthComponent::setHealthAmount(int healthAmount) {
	this->healthAmount = healthAmount;
}


