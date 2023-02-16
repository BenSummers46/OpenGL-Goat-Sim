#pragma once
#include "GameObject.h"
#include "BehaviourAction.h"
#include "BehaviourSequence.h"
#include "BehaviourSelector.h"
#include "Ray.h"
#include "PlayerObject.h"
#include "Enemy.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"

using namespace NCL;
using namespace CSC8503;

namespace NCL::CSC8503 {
	class BehaviourTreeObject : public GameObject {
	public:
		BehaviourTreeObject(std::string nameInput, PlayerObject* playerChar, Enemy* enemy);
		~BehaviourTreeObject();

		void Update(float dt);
		void OnCollisionBegin(GameObject* otherObject);
		void OnCollisionEnd(GameObject* otherObject);

		void SetDetectionRadius(float r) { detectionRadius = r; }
		float GetDetectionRadius() { return detectionRadius; }

		void collectKey() { keyCollected = !keyCollected; }
		void SetAttacked() { attackedPlayer = !attackedPlayer; }

		BehaviourSequence* GetRootSequence() { return rootSequence; }
	protected:
		bool PlayerInRadius();
		void MoveToLocation();
		bool FindPath();
		void SetupPath();
		bool CheckLocation(Vector3 endLocation);

		bool attackedPlayer;

		PlayerObject* player;
		Enemy* enemyObject;
		float detectionRadius;

		Vector3 startPoint;
		Vector3 endPoint;

		NavigationGrid* grid;
		NavigationPath outPath;
		std::vector<Vector3> pathNodes;
		int pathIndex;

		bool keyCollected = false;
		bool roomOpened = false;
		float guardDuration;

		BehaviourSequence* rootSequence;
		BehaviourSelector* activitySelection;
		BehaviourSequence* playerSequence;
		BehaviourSequence* powerupSequence;
	};
}
