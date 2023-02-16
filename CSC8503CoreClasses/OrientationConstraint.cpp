#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b, Vector3 x)
{
	objectA = a;
	objectB = b;
	axis = x;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {
	Vector3 aRotation = objectA->GetTransform().GetOrientation() * axis;
	Vector3 bRotation = objectB->GetTransform().GetOrientation() * axis;
	Vector3 up = Vector3(0, 1, 0);

	float offset = Vector3::Dot(aRotation, bRotation);
	Vector3 offsetDir = Vector3::Cross(aRotation, bRotation);

	if (abs(offset) > 0.0f) {

		PhysicsObject* physA = objectA->GetPhysicsObject();
		PhysicsObject* physB = objectB->GetPhysicsObject();

		Vector3 relativeVelocity = physA->GetAngularVelocity() - physB->GetAngularVelocity();

		float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

		if (constraintMass > 0.0f) {
			float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);
			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 aImpulse = offsetDir * lambda;
			Vector3 bImpulse = -offsetDir * lambda;

			physA->ApplyAngularImpulse(aImpulse);
			physB->ApplyAngularImpulse(bImpulse);
		}
	}
}