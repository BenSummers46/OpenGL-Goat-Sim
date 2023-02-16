#include "DestructibleObject.h"
#include "PlayerObject.h"
#include "BehaviourTreeObject.h"

using namespace NCL;
using namespace CSC8503;

DestructibleObject::DestructibleObject(std::string objName, bool special, bool key) {
	name = objName;
	this->special = special;
	this->key = key;
}

DestructibleObject::~DestructibleObject() {

}

void DestructibleObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Player") == 0 && !this->key) {
		this->isActive = false;
		PlayerObject* p = (PlayerObject*)otherObject;
		if (this->special) {
			p->SetScore(20);
		}
		else {
			p->SetScore((rand() % 5) + 1);
		}
	}

	if (otherObject->GetName().compare((std::string)"Goose") == 0 && this->key) {
		BehaviourTreeObject* p = (BehaviourTreeObject*)otherObject;
		p->collectKey();
		this->isActive = false;
	}
}

void DestructibleObject::OnCollisionEnd(GameObject* otherObject) {

}