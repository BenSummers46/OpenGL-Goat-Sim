# OpenGL-Goat-Sim
Small physics based goat sim game made in C++ and OpenGL to test understanding of game physics, AI and networking.\
YouTube link: https://youtu.be/qD831YfhpR8

## Gameplay
-	Certain green cubes are destructible objects that can be destroyed to gain points. Point values for these crates are random.
-	Blue spheres are powerups and will give the player a speed and jump buff for a set amount of time.
-	White/grey cubes are interactable physics objects that can be pushed around and interacted with.
-	The human patrols the maze and will run away from the player if they get hit.
-	The goose has several different behaviours that all include trying to find and attack the player.
-	Client/server gameplay can be enabled.
-	Game will end when all destructible objects have been picked up.

## Physics Features
-	Angular and linear motion applied to physics objects.
-	Torque used to rotate objects.
-	Player object in controlled entirely with forces.
-	Position constraints as well as orientation constraints for the bridge.
\
![image](https://user-images.githubusercontent.com/115077902/219667269-a72d5936-dc31-4cdd-b565-574dc5a813ae.png)

## Collision Features
-	AABB collisions
-	Sphere collisions
-	Capsule collisions
-	Ray casting collisions
-	Collision layers to stop AI interacting with player collectibles and vice versa.
-	Collisions for gameplay features such as picking up items and such.
-	Penalty box if you fall you respawn and lose points.
![image](https://user-images.githubusercontent.com/115077902/219667484-d75415b8-df5d-4127-9d35-2a519dc4c632.png)

## Optimisations
-	Broad phase/narrow phase.
-	If 2 items both have an inverse mass of 0, they are static and collision detection is not executed on them.

## Hierarchical State Machine
-	The Human enemy is an example of a hierarchical state machine.
-	States include navigating the maze with the use of grid-based pathfinding.
-	If the human encounters the player, he will run in the opposite direction of the player.
-	 The states also include the correct transitions between these states.

## Behaviour Tree
-	Goose is an example of a behaviour tree.
-	Has 3 activities to carry out; bodyguarding the human AI, hunting for the player and carrying out a task to get a buff that increases detection range.
-	Goose also uses grid-based pathfinding.
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219667957-20fdee9e-bf1a-4ba4-b30b-7f863b98c995.png)

## Pushdown Automata
-	Main menu system uses pushdown automata.
-	Player can switch between menu states and games states at will.
-	Game can also be reset from the main menu.
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219668209-3e5b8134-96cb-4025-bc9b-577d03e0fcdc.png)
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219668251-38958b16-0b8b-48c8-86ff-f810cfeca1ef.png)

## Networking
-	Clients can connect to server and packets can be exchanged between both.
-	Server controls the state of the game.
-	Requests sent from client to server where the server updates the game state and sends it back to the client.
-	Client cannot move if not connected to the server.
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219669378-29881f61-6ca6-42cb-9ca5-dce806d3f9d5.png)
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219669485-fd5f4ced-d79f-4c59-9a37-2ab477056e96.png)


