#include "BehaviourTreeObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include <random>

using namespace NCL;
using namespace CSC8503;

const Vector3 STARTPOINT = Vector3(180, 0, 20);
const Vector3 KEYLOC = Vector3(330, 0, 20);
const Vector3 ROOMLOC = Vector3(20, 0, 280);
const Vector3 POWERUPLOC = Vector3(390, 0, 390);

BehaviourTreeObject::BehaviourTreeObject(std::string nameInput, PlayerObject* playerChar, Enemy* enemy) {
	attackedPlayer = false;
	player = playerChar;
	enemyObject = enemy;
	name = nameInput;
	grid = new NavigationGrid("TestGrid2.txt");
	rootSequence = new BehaviourSequence("Root Sequence");
	pathIndex = 0;
	detectionRadius = 10;
	guardDuration = 30;
	
	//POWERUP SEQUENCE
	BehaviourAction* FindKey = new BehaviourAction("Find key", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			startPoint = this->GetTransform().GetPosition(); //Was STARTPOINT
			endPoint = KEYLOC;
			pathIndex = 0;
			FindPath();
			SetupPath();
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (keyCollected) {
				pathNodes.clear();
				return Success;
			}
			MoveToLocation();
		}
		return state;
	});

	BehaviourAction* OpenRoom = new BehaviourAction("Open Room", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			startPoint = KEYLOC;
			endPoint = ROOMLOC;
			pathIndex = 0;
			FindPath();
			SetupPath();
			state = Ongoing;
		}
		else if (state == Ongoing) {
			MoveToLocation();
			if (CheckLocation(ROOMLOC)) {
				roomOpened = true;
			}
			if (roomOpened) {
				pathNodes.clear();
				return Success;
			}
		}
		return state;
	});

	BehaviourAction* CollectPowerup = new BehaviourAction("Collect Powerup", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			startPoint = ROOMLOC;
			endPoint = POWERUPLOC;
			pathIndex = 0;
			FindPath();
			SetupPath();
			state = Ongoing;
		}
		else if (state == Ongoing) {
			MoveToLocation();
			if (CheckLocation(POWERUPLOC)) {
				detectionRadius += 10;
				pathNodes.clear();
				return Success;
			}
		}
		return state;
	});

	powerupSequence = new BehaviourSequence("Powerup Sequence");
	powerupSequence->AddChild(FindKey);
	powerupSequence->AddChild(OpenRoom);
	powerupSequence->AddChild(CollectPowerup);
	
	//PLAYER SEQUENCE
	BehaviourAction* LookforPlayer = new BehaviourAction("Looking for Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
		}
		else if (state == Ongoing) {
			srand((unsigned)time(0));
			GetPhysicsObject()->AddForce(Vector3((rand() % 15) - (rand() % 40), 0, (rand() % 15) - (rand() % 30)) * 0.7);
			if (PlayerInRadius()) {
				return Success;
			}
		}
		return state;
	});

	BehaviourAction* ChasePlayer = new BehaviourAction("Chasing Players", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
		}
		else if (state == Ongoing) {
			Vector3 dir = player->GetTransform().GetPosition() - this->GetTransform().GetPosition();
			GetPhysicsObject()->AddForce(dir * 0.9);
			if (attackedPlayer) {
				std::cout << "Player has been attacked!" << std::endl;
				return Success;
			}
		}
		return state;
	});

	playerSequence = new BehaviourSequence("Player Sequence");
	playerSequence->AddChild(LookforPlayer);
	playerSequence->AddChild(ChasePlayer);

	//BODYGUARD THE ENEMY
	BehaviourAction* bodyguard = new BehaviourAction("Bodyguard Enemy", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			Vector3 offset = Vector3(3, 0, 3);
			this->GetTransform().SetPosition(enemyObject->GetTransform().GetPosition() + offset);
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (enemyObject->GetHitByPlayer()) {
				return Failure;
			}
			if (guardDuration < 0) {
				return Success;
			}
			Vector3 dir = enemyObject->GetTransform().GetPosition() - this->GetTransform().GetPosition();
			GetPhysicsObject()->AddForce(dir * 0.9);
			guardDuration -= dt;
		}
		return state;
	});

	activitySelection = new BehaviourSelector("Activity Selector");
	activitySelection->AddChild(bodyguard);
	activitySelection->AddChild(playerSequence);
	activitySelection->AddChild(powerupSequence);
	
	rootSequence->AddChild(activitySelection);
}

BehaviourTreeObject::~BehaviourTreeObject() {
	delete grid;
	delete rootSequence;
	delete activitySelection;
	delete powerupSequence;
	delete playerSequence;
}

void BehaviourTreeObject::Update(float dt) {
	
}

bool BehaviourTreeObject::PlayerInRadius() {
	return (player->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() < detectionRadius;
}

void BehaviourTreeObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Player") == 0) {
		attackedPlayer = true;
	}
}

void BehaviourTreeObject::OnCollisionEnd(GameObject* otherObject) {

}

bool BehaviourTreeObject::FindPath() {
	return grid->FindPath(startPoint, endPoint, outPath);
}

void BehaviourTreeObject::SetupPath() {
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathNodes.push_back(pos);
	}
}

void BehaviourTreeObject::MoveToLocation() {
	/*FindPath();
	SetupPath();*/
	if (pathIndex == pathNodes.size() - 1) {
		pathNodes.clear();
		pathIndex = 0;
		return;
	}else if ((pathNodes[pathIndex] - this->GetTransform().GetPosition()).Length() > 7) {
		Vector3 dir = pathNodes[pathIndex] - this->GetTransform().GetPosition();
		GetPhysicsObject()->AddForce({ dir * 0.3 });
	}
	else {
		if (pathIndex != pathNodes.size() - 1) {
			++pathIndex;
		}
	}
}

bool BehaviourTreeObject::CheckLocation(Vector3 endLocation) {
	return (endLocation - GetTransform().GetPosition()).Length() < 5;
}