#pragma once
#include "GameObject.h"
#include "Powerup.h"
#include <string>

using namespace NCL;
using namespace CSC8503;
namespace NCL::CSC8503 {
	class PlayerObject : public GameObject {
	public:
		PlayerObject(std::string objectName);
		~PlayerObject();

		void SetScore(int value) { score += value; }
		int GetScore() { return score; }
		void SetWalkSpeed(float value) { walkSpeed = value; }
		float GetWalkSpeed() { return walkSpeed; }
		void SetJumpForce(float value) { jumpForce = value; }
		float GetJumpForce() { return jumpForce; }

		void AddBuff(Powerup* buff) { appliedBuffs.emplace_back(buff); }
		void RemoveBuff(Powerup* buff) { appliedBuffs.erase(std::remove(appliedBuffs.begin(), appliedBuffs.end(), buff), appliedBuffs.end()); }
		std::vector<Powerup*> GetBuffs() { return appliedBuffs; }

		void OnCollisionBegin(GameObject* otherObject);
		void OnCollisionEnd(GameObject* otherObject);

		bool GetGrounded() { return isGrounded; }
		void SetDead(bool inDead) { dead = inDead; }
		bool GetDead() { return dead; }
	protected:
		int score = 0;
		bool isGrounded = false;
		bool dead = false;

		float walkSpeed = 10.0f;
		float jumpForce = 100.0f;

		std::vector<Powerup*> appliedBuffs;
	};
}