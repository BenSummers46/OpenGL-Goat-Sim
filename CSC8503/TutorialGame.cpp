#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "NavigationGrid.h"
#include "NetworkObject.h"

#include "PushdownState.h"
#include "PushdownMachine.h"

using namespace NCL;
using namespace CSC8503;

class Game : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		Debug::Print("Game Started!", Vector2(70, 30));
		Debug::Print("CNTRL to Pause!", Vector2(70, 40));
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::CONTROL)) {
			return PushdownState::Pop;
		}
		return PushdownState::NoChange;
	}

	void OnAwake() override {
		Debug::Print("Game Started!", Vector2(30, 50));
	}
};

class MainMenu : public PushdownState {
	
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		Debug::Print("Welcome to Goat Imitator", Vector2(30, 50), Debug::BLACK);
		Debug::Print("Space to start the game or Control to quit!", Vector2(15, 60), Debug::BLACK);
		Debug::Print("Press F1 to restart the game!", Vector2(25, 70), Debug::BLACK);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
			*newState = new Game();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::CONTROL)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		Debug::Print("Welcome to Goat Imitator", Vector2(50, 50));
		Debug::Print("Space to start the game or Control to quit!", Vector2(50, 60));
	}
};

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	//machine = new PushdownMachine(new MainMenu());
	
	InitialiseAssets();
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(5, 2, 10);
	Vector3 staticCubeSize = Vector3(5, 5, 5); //default 8, 8, 8

	float invCubeMass = 3;
	int numLinks = 10;
	float maxDistance = 7; //default 30
	float cubeDistance = 6; //default 20

	Vector3 startPos = Vector3(222, -22, -35) + Vector3(175, 13, 175);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), staticCubeSize, "Floor", 0);
	NetworkObject* s = new NetworkObject(*start, bridgeID++);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), staticCubeSize,"Floor", 0);
	NetworkObject* e = new NetworkObject(*end, bridgeID++);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, "Floor", invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		OrientationConstraint* orConst = new OrientationConstraint(previous, block, Vector3(0, 1, 0));
		OrientationConstraint* orConst2 = new OrientationConstraint(previous, block, Vector3(1, 0, 0));
		world->AddConstraint(constraint);
		world->AddConstraint(orConst);
		world->AddConstraint(orConst2);
		previous = block;
		NetworkObject* b = new NetworkObject(*block, bridgeID++);
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	OrientationConstraint* orConst = new OrientationConstraint(previous, end, Vector3(0, 1, 0));
	OrientationConstraint* orConst2 = new OrientationConstraint(previous, end, Vector3(1, 0, 0));
	world->AddConstraint(constraint);
	world->AddConstraint(orConst);
	world->AddConstraint(orConst2);
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	gooseMesh = renderer->LoadMesh("goose.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld2();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
	delete machine;
}

void TutorialGame::UpdateGame(float dt) {
	if (destructList.size() == 0) {
		destructList.clear();
		powerupList.clear();
		InitWorld2();
		selectionObject = nullptr;
		machine->Reset();
	}
	
	if (machine != nullptr) {
		if (!machine->Update(dt)) return;
		PushdownState* state = machine->GetActiveState();
		if (dynamic_cast<MainMenu*>(state)) {
			lockedObject = nullptr;
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
				destructList.clear();
				powerupList.clear();
				InitWorld2();
				selectionObject = nullptr;
			}
		}
		else if (dynamic_cast<Game*>(state)) {
			lockedObject = player;
		}
	}
	else {
		machine = new PushdownMachine(new MainMenu());
	}
	
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		
		LockedObjectMovement();
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Quaternion objRot = lockedObject->GetTransform().GetOrientation();
		Vector3 camPos = objPos + objRot * lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x); 
		world->GetMainCamera()->SetYaw(angles.y); 
	}

	UpdateKeys();

	if (testStateObject) {
		testStateObject->Update(dt);
	}

	if (enemy) {
		enemy->Update(dt);
	}

	if (player->GetDead()) {
		player->GetTransform().SetPosition(Vector3(50, 10, 50));
		player->SetDead(false);
	}
	
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	Debug::Print("Players Score: " + std::to_string(player->GetScore()), Vector2(5, 10), Debug::BLACK);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	for (DestructibleObject* i : destructList) {
		if (i->IsActive() == false) {
			world->RemoveGameObject(i);
			destructList.erase(std::remove(destructList.begin(), destructList.end(), i), destructList.end());
		}
	}
	for (Powerup* i : powerupList) {
		if (i->IsActive() == false) {
			world->RemoveGameObject(i);
			powerupList.erase(std::remove(powerupList.begin(), powerupList.end(), i), powerupList.end());
		}
	}
	
	for (Powerup* i : player->GetBuffs()) {
		if (i->GetTime() <= 0.0f) {
			player->SetJumpForce(player->GetJumpForce() / i->GetJumpForce());
			player->SetWalkSpeed(player->GetWalkSpeed() / i->GetSpeed());
			player->RemoveBuff(i);
			break;
		}
		else {
			float timeRemaining = i->GetTime();
			i->SetTime(timeRemaining - dt);
			Debug::Print("Buff Time Remaining: " + std::to_string(i->GetTime()), Vector2(5, 15), Debug::BLACK);
			break;
		}
		break;
	}
	Debug::Print( "Destructibles Left: " + std::to_string(destructList.size()), Vector2(5, 20), Debug::BLACK);

	BehaviourState state;
	state = goose->GetRootSequence()->Execute(dt);

	if (state == Failure || state == Success) {
		goose->GetRootSequence()->Reset();
	}

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
	//	InitWorld2(); //We can reset the simulation at any time with F1
	//	selectionObject = nullptr;
	//}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		//LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	float yaw = Window::GetMouse()->GetRelativePosition().x;
	lockedObject->GetPhysicsObject()->AddTorque(Vector3(0, -yaw, 0));
	
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * player->GetWalkSpeed());
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * player->GetWalkSpeed());
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * player->GetWalkSpeed());
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * player->GetWalkSpeed());
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE) && player->GetGrounded()) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, player->GetJumpForce(), 0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	InitGameExamples();
	InitDefaultFloor();

	testStateObject = AddStateObjectToWorld(Vector3(0, 50, 0));
}

void TutorialGame::InitWorld2() {
	world->ClearAndErase();
	physics->Clear();

	NavigationGrid grid("TestGrid2.txt");
	
	LockCameraToObject(AddPlayerToWorld(Vector3(50, 0, 50)));
	player = (PlayerObject*)lockedObject;
	CreateWorld(grid);
	InitDefaultFloor();
}

void TutorialGame::CreateWorld(NavigationGrid& grid) {
	GridNode* nodes = grid.GetAllNodes();
	for (int x = 0; x < 40; ++x) {
		for (int y = 0; y < 40; ++y) {
			GridNode& n = nodes[(40 * x) + y];
			switch (n.type) {
			case 'x':
				AddCubeToWorld(n.position, Vector3(10, 10, 10), "walls", 0); 
				wallID++;
				break;
			case 'd':
				AddDestructibleToWorld(n.position, Vector3(5, 5, 5), false, false);
				boxID++;
				break;
			case 'p':
				AddPowerupToWorld(n.position, Vector3(3, 3, 3), 10, 10, 5);
				powerupID++;
				break;
			case 'c':
				AddCubeToWorld(n.position, Vector3(6, 6, 6), "walls", 1);
				wallID++;
				break;
			default:
				break;
			}
		}
	}
	enemy = AddEnemyStateToWorld(nodes[(40 * 18) + 2].position - Vector3(0, 3, 0));
	AddDestructibleToWorld(Vector3(482, -4, 140), Vector3(5, 5, 5), true, false); boxID++;
	AddDestructibleToWorld(nodes[(40 * 1) + 33].position - Vector3(0, 3, 0), Vector3(5, 5, 5), false, true); boxID++;
	killBox(Vector3(0, -100, 0));
	/*AddCapsuleToWorld(Vector3(40, -1, 50));
	AddCapsuleToWorld(Vector3(35, -1, 50));*/
	BridgeConstraintTest();
	goose = AddBehaviourTreeToWorld(nodes[(40 * 1) + 18].position - Vector3(0, 3, 0));
	return;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position) {
	GameObject* cap = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(2, 0.5);
	cap->SetBoundingVolume((CollisionVolume*)volume);
	cap->GetTransform()
		.SetScale(Vector3(1, 1, 1))
		.SetPosition(position);

	cap->SetRenderObject(new RenderObject(&cap->GetTransform(), capsuleMesh, basicTex, basicShader));
	cap->SetPhysicsObject(new PhysicsObject(&cap->GetTransform(), cap->GetBoundingVolume()));

	cap->GetPhysicsObject()->SetInverseMass(1.0f);
	cap->GetPhysicsObject()->InitSphereInertia();
	cap->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	NetworkObject* n = new NetworkObject(*cap, 3);

	world->AddGameObject(cap);

	return cap;
}

GameObject* TutorialGame::killBox(const Vector3& position) {
	GameObject* floor = new GameObject("killBox");

	//Vector3 floorSize = Vector3(220, 2, 220);
	AABBVolume* volume = new AABBVolume(Vector3(1000, 5, 1000));
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(Vector3(1000, 5, 1000) * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	floor->GetRenderObject()->SetColour(Vector4(0, 0, 0, 0.5));

	world->AddGameObject(floor);

	return floor;
}

BehaviourTreeObject* TutorialGame::AddBehaviourTreeToWorld(const Vector3& position) {
	BehaviourTreeObject* cap = new BehaviourTreeObject("Goose", player, enemy);

	SphereVolume* volume = new SphereVolume(1.5f);
	cap->SetBoundingVolume((CollisionVolume*)volume);
	cap->GetTransform()
		.SetScale(Vector3(1, 1, 1))
		.SetPosition(position);

	cap->SetRenderObject(new RenderObject(&cap->GetTransform(), gooseMesh, basicTex, basicShader));
	cap->SetPhysicsObject(new PhysicsObject(&cap->GetTransform(), cap->GetBoundingVolume()));

	cap->GetPhysicsObject()->SetInverseMass(1.0f);
	cap->GetPhysicsObject()->InitSphereInertia();
	cap->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	NetworkObject* n = new NetworkObject(*cap, 2);

	world->AddGameObject(cap);

	return cap;
}

Enemy* TutorialGame::AddEnemyStateToWorld(const Vector3& position) {
	Enemy* cap = new Enemy();

	AABBVolume* volume = new AABBVolume(Vector3(0.2, 0.9, 0.2) * 3);
	cap->SetBoundingVolume((CollisionVolume*)volume);
	cap->GetTransform()
		.SetScale(Vector3(3, 3, 3))
		.SetPosition(position);

	cap->SetRenderObject(new RenderObject(&cap->GetTransform(), enemyMesh, basicTex, basicShader));
	cap->SetPhysicsObject(new PhysicsObject(&cap->GetTransform(), cap->GetBoundingVolume()));

	cap->GetPhysicsObject()->SetInverseMass(1.0f);
	cap->GetPhysicsObject()->InitSphereInertia();
	cap->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	NetworkObject* n = new NetworkObject(*cap, 3);

	world->AddGameObject(cap);

	return cap;
}

Powerup* TutorialGame::AddPowerupToWorld(const Vector3& position, const Vector3& dimensions, float speed, float jump, float time) {
	Powerup* cube = new Powerup("Powerup", speed, jump, time);

	SphereVolume* volume = new SphereVolume(2.5);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions); //was *2

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), sphereMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(5);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));

	NetworkObject* n = new NetworkObject(*cube, powerupID);
	
	world->AddGameObject(cube);
	powerupList.emplace_back(cube);

	return cube;
}

DestructibleObject* TutorialGame::AddDestructibleToWorld(const Vector3& position, const Vector3& dimensions, bool special, bool key) {
	DestructibleObject* cube = new DestructibleObject("Destructible", special, key);

	AABBVolume* volume = new AABBVolume(dimensions * 0.45);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions); //was *2

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(5);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	NetworkObject* n = new NetworkObject(*cube, boxID);
	
	world->AddGameObject(cube);
	destructList.emplace_back(cube);

	return cube;
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size) {
	GameObject* floor = new GameObject("Floor");

	//Vector3 floorSize = Vector3(220, 2, 220);
	AABBVolume* volume = new AABBVolume(size);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("Sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, std::string name, float inverseMass) {
	GameObject* cube = new GameObject(name);

	AABBVolume* volume = new AABBVolume(dimensions * 0.45);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions); //was *2

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	NetworkObject* n = new NetworkObject(*cube, wallID);
	
	world->AddGameObject(cube);

	return cube;
}

PlayerObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	PlayerObject* character = new PlayerObject("Player");
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, basicTex, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetPhysicsObject()->SetElasticity(0.0f);

	NetworkObject* n = new NetworkObject(*character, 1);

	character->GetRenderObject()->SetColour(Vector4(0.40, 1.0, 0.50, 1));

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	//CapsuleVolume* volume = new CapsuleVolume(1.5f, 0.5f);
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(50, 50, 50))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), enemyMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0) + Vector3(175, 13, 175), Vector3(220, 2, 220));
	AddFloorToWorld(Vector3(490, -8, 140), Vector3(20, 1, 20));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
	BridgeConstraintTest();
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(220, 2, 220));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, "name");
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, "cube", 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q) && player->GetScore() >= 5) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		if (player->GetScore() >= 5) {
			Debug::Print("Force Gun Unlocked!", Vector2(60, 10), Debug::BLACK);
			Debug::Print("Press Q to use force gun!", Vector2(5, 85));
		}
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}


