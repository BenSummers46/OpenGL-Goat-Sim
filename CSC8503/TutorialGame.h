#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "PlayerObject.h"
#include "DestructibleObject.h"
#include "Powerup.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"
#include "Enemy.h"
#include "BehaviourTreeObject.h"
#include "PushdownMachine.h"
#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void InitWorld2();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void CreateWorld(NavigationGrid& grid);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void BridgeConstraintTest();

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			Enemy* AddEnemyStateToWorld(const Vector3& position);
			Enemy* enemy;

			BehaviourTreeObject* AddBehaviourTreeToWorld(const Vector3& position);
			BehaviourTreeObject* goose;

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, std::string name, float inverseMass = 10.0f);

			PlayerObject* AddPlayerToWorld(const Vector3& position);
			DestructibleObject* AddDestructibleToWorld(const Vector3& position, const Vector3& dimensions, bool special, bool key);
			Powerup* AddPowerupToWorld(const Vector3& position, const Vector3& dimensions, float speed, float jump, float time);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* killBox(const Vector3& position);
			GameObject* AddCapsuleToWorld(const Vector3& position);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;
			PlayerObject* player = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;
			MeshGeometry* gooseMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 4, 12);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			std::vector<DestructibleObject*> destructList;
			std::vector<Powerup*> powerupList;

			int boxID = 4;
			int powerupID = 50;
			int wallID = 100;
			int bridgeID = 500;

			PushdownMachine* machine = nullptr;
		};
	}
}

