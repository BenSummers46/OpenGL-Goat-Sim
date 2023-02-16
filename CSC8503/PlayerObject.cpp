#include "PlayerObject.h"
#include "BehaviourTreeObject.h"

using namespace NCL;
using namespace CSC8503;

PlayerObject::PlayerObject(std::string objectName) {
	name = objectName;
}

PlayerObject::~PlayerObject() {

}

void PlayerObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Floor") == 0) {
		isGrounded = true;
	}
}

void PlayerObject::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Floor") == 0) {
		isGrounded = false;
	}

	if (otherObject->GetName().compare((std::string)"Goose") == 0) {
		BehaviourTreeObject* goose = (BehaviourTreeObject*)otherObject;
		goose->SetAttacked();
		this->dead = true;
		score -= 5;
	}

	if (otherObject->GetName().compare((std::string)"killBox") == 0) {
		this->dead = true;
		score -= 5;
	}
}