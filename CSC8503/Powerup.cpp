#include "Powerup.h"
#include "PlayerObject.h"

using namespace NCL;
using namespace CSC8503;

Powerup::Powerup(std::string name, float speedIncrease, float jumpIncrease, float time) {
	this->name = name;
	this->speedIncrease = speedIncrease;
	this->jumpIncrease = jumpIncrease;
	this->time = time;
}

Powerup::~Powerup() {

}

void Powerup::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Player") == 0) {
		PlayerObject* p = (PlayerObject*)otherObject;
		p->SetWalkSpeed(p->GetWalkSpeed() * 10);
		p->SetJumpForce(p->GetJumpForce() * 10);
		p->AddBuff(this);
		this->isActive = false;
	}
}

void Powerup::OnCollisionEnd(GameObject* otherObject) {

}