#include "Enemy.h"
#include "StateMachine.h"
#include "State.h";
#include "StateTransition.h"
#include "PhysicsObject.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

Enemy::Enemy() {
	grid = new NavigationGrid("TestGrid2.txt");
	startPoint = Vector3(10, 0, 180);
	endPoint = Vector3(310, 0, 260);
	FindPath();
	SetupPath();
	pathIndex = 0;
	
	stateMachine = new StateMachine();

	State* patrol = new State([&](float dt)->void {
		this->Patrol(dt);
	});

	State* chasePlayer = new State([&](float dt)->void{
		this->RunFromPlayer(dt);
	});

	stateMachine->AddState(patrol);
	stateMachine->AddState(chasePlayer);

	stateMachine->AddTransition(new StateTransition(patrol, chasePlayer, [&]()->bool {
		return hitByPlayer;
	}));

	stateMachine->AddTransition(new StateTransition(chasePlayer, patrol, [&]()->bool {
		return !hitByPlayer;
	}));
}

Enemy::~Enemy() {
	delete stateMachine;
	delete grid;
}

void Enemy::SetupPath() {
	Vector3 pos;
	while (outPath.PopWaypoint(pos))
	{
		pathNodes.push_back(pos);
	}
}

bool Enemy::FindPath() {
	return grid->FindPath(startPoint, endPoint, outPath);
}

void Enemy::Update(float dt) {
	stateMachine->Update(dt);
}

void Enemy::SwapStartAndEnd() {
	Vector3 temp;
	temp = endPoint;
	endPoint = startPoint;
	startPoint = temp;
}

void Enemy::Patrol(float dt) {
	if (pathIndex == pathNodes.size() - 1) {
		SwapStartAndEnd();
		FindPath();
		pathNodes.clear();
		SetupPath();
		pathIndex = 0;
	}
	if ((pathNodes[pathIndex] - this->GetTransform().GetPosition()).Length() > 7) {
		Vector3 dir = pathNodes[pathIndex] - this->GetTransform().GetPosition();
		GetPhysicsObject()->AddForce({ dir * 0.3});
	}
	else {
		if (pathIndex != pathNodes.size() - 1) {
			++pathIndex;
		}
	}
}

void Enemy::RunFromPlayer(float dt) {
	if (durationLeft > 0) {
		Vector3 dir = player->GetTransform().GetPosition() - this->GetTransform().GetPosition();
		GetPhysicsObject()->AddForce({ -dir * 0.5 });
		durationLeft -= dt;
	}
	else {
		hitByPlayer = false;
	}
}

void Enemy::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName().compare((std::string)"Player") == 0) {
		durationLeft = 7;
		player = (PlayerObject*)otherObject;
		hitByPlayer = true;
	}
}

void Enemy::OnCollisionEnd(GameObject* otherObject) {
	
}