#pragma once
#include "Constraint.h"
#include "Vector3.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		using Maths::Vector3;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, GameObject* b, Vector3 x);
			~OrientationConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;

			//float distance;
			Vector3 axis;
		};
	}
}

