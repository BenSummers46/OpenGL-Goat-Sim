#pragma once
#include "StateGameObject.h"
#include "NavigationPath.h"
#include "NavigationGrid.h"
#include "PlayerObject.h"

using namespace NCL;
using namespace CSC8503;

namespace NCL::CSC8503 {
	class StateMachine;
	class Enemy : public StateGameObject {
	public:
		Enemy();
		~Enemy();

		const Vector3& GetStartPoint() { return startPoint; }
		const Vector3& GetEndPoint() { return endPoint; }

		const bool& GetCanSeePlayer() { return hitByPlayer; }
		void SetCanSeePlayer() { hitByPlayer = !hitByPlayer; }

		void Update(float dt);
		void OnCollisionBegin(GameObject* otherObject);
		void OnCollisionEnd(GameObject* otherObject);
		bool GetHitByPlayer() { return hitByPlayer; }
	protected:
		void Patrol(float dt);
		void RunFromPlayer(float dt);
		bool FindPath();
		void SetupPath();
		void SwapStartAndEnd();
		
		Vector3 startPoint;
		Vector3 endPoint;
		std::vector<Vector3> pathNodes;

		NavigationGrid* grid;
		NavigationPath outPath;

		StateMachine* stateMachine;

		bool hitByPlayer = false;
		int pathIndex;
		float durationLeft;

		PlayerObject* player;
	};
}
