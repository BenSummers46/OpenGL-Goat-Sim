#pragma once
#include "GameObject.h"


using namespace NCL;
using namespace CSC8503;
namespace NCL::CSC8503 {
	class Powerup : public GameObject {
	public:
		Powerup(std::string name, float speedIncrease, float jumpIncrease, float time);
		~Powerup();

		void OnCollisionBegin(GameObject* otherObject);
		void OnCollisionEnd(GameObject* otherObject);

		void SetTime(float dt) { time = dt; }
		float GetTime() { return time; }

		float GetSpeed() { return speedIncrease; }
		float GetJumpForce() { return jumpIncrease; }
	protected:
		float speedIncrease, jumpIncrease, time;
	};
}
