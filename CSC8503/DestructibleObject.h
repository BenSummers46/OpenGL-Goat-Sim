#pragma once
#include "GameObject.h"
#include <string>

using namespace NCL;
using namespace CSC8503;
namespace NCL::CSC8503 {
	class DestructibleObject : public GameObject {
	public:
		DestructibleObject(std::string objName, bool special, bool key);
		~DestructibleObject();
		
		void OnCollisionBegin(GameObject* otherObject);
		void OnCollisionEnd(GameObject* otherObject);
	protected:
		bool special;
		bool key;
	};
}
