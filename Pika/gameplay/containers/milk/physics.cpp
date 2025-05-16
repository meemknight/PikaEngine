#include "physics.h"

PhysicsObject createBall(glm::vec3 pos, float r)
{
	PhysicsObject ret;
	ret.type = TYPE_CIRCLE;

	ret.position = pos;
	ret.shape.x = r;
	ret.mass = 4.f * 3.141592 * r * r * r / 3.f;

	return ret;
}

PhysicsObject createBox(glm::vec3 pos, glm::vec3 size)
{
	PhysicsObject ret;
	ret.type = TYPE_BOX;

	ret.position = pos;
	ret.shape = size;
	ret.mass = size.x * size.y * size.z;

	return ret;
}

PhysicsObject createCilindru(glm::vec3 pos, float r, float h)
{
	PhysicsObject ret;
	ret.type = TYPE_CILINDRU;

	ret.position = pos;
	ret.shape.x = r;
	ret.shape.y = h;
	ret.mass = 3.14159 * r * r * h;

	return ret;
}

void applyDrag(PhysicsObject &object)
{
	glm::vec3 dragForce = 0.1f * -object.velocity * glm::abs(object.velocity) / 2.f;
	float length = glm::length(dragForce);
	if (length)
	{
		if (length > MAX_AIR_DRAG)
		{
			dragForce /= length;
			dragForce *= MAX_AIR_DRAG;
		}

		object.acceleration += dragForce;
	}
}

void Simulator::updateForces(PhysicsObject &object, float deltaTime)
{

	if (object.mass >= INFINITY || object.mass <= 0)
	{
		object.velocity = {};
		object.acceleration = {};
		return;
	}

	object.acceleration = glm::clamp(object.acceleration,
		glm::vec3(-MAX_ACCELERATION), glm::vec3(MAX_ACCELERATION));

	//Symplectic Euler
	object.velocity += object.acceleration * deltaTime * 0.5f;
	object.velocity = glm::clamp(object.velocity, glm::vec3(-MAX_VELOCITY), glm::vec3(MAX_VELOCITY));

	object.position += object.velocity * deltaTime;

	object.velocity += object.acceleration * deltaTime * 0.5f;
	object.velocity = glm::clamp(object.velocity, glm::vec3(-MAX_VELOCITY), glm::vec3(MAX_VELOCITY));

	if (std::fabs(object.velocity.x) < 0.00001) { object.velocity.x = 0; }
	if (std::fabs(object.velocity.y) < 0.00001) { object.velocity.y = 0; }

	object.acceleration = {};
}

void normalizeSafe(glm::vec3 &v)
{
	float l = glm::length(v);

	if (l <= 0.00000001)
	{
		v = {1,0,0};
	}
	else
	{
		v /= l;
	}
}

bool AABBvsAABB(PhysicsObject &a, PhysicsObject &b, float &penetration, glm::vec3 &normal)
{
	// Vector from A to B
	glm::vec3 n = b.position - a.position;

	auto aMax = a.getMax();
	auto aMin = a.getMin();
	auto bMax = b.getMax();
	auto bMin = b.getMin();

	// Calculate half extents along each axis for each object
	float a_extent_x = (aMax.x - aMin.x) / 2.0f;
	float b_extent_x = (bMax.x - bMin.x) / 2.0f;
	float a_extent_y = (aMax.y - aMin.y) / 2.0f;
	float b_extent_y = (bMax.y - bMin.y) / 2.0f;
	float a_extent_z = (aMax.z - aMin.z) / 2.0f;
	float b_extent_z = (bMax.z - bMin.z) / 2.0f;

	// Calculate overlaps on each axis
	float x_overlap = a_extent_x + b_extent_x - abs(n.x);
	float y_overlap = a_extent_y + b_extent_y - abs(n.y);
	float z_overlap = a_extent_z + b_extent_z - abs(n.z);

	// SAT test on x, y, and z axes
	if (x_overlap > 0 && y_overlap > 0 && z_overlap > 0)
	{
		// Determine the axis of least penetration
		if (x_overlap < y_overlap && x_overlap < z_overlap)
		{
			normal = (n.x < 0) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
			penetration = x_overlap;
		}
		else if (y_overlap < z_overlap)
		{
			normal = (n.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
			penetration = y_overlap;
		}
		else
		{
			normal = (n.z < 0) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
			penetration = z_overlap;
		}
		return true;
	}

	return false;
}


bool CirclevsCircle(PhysicsObject &a, PhysicsObject &b,
	float &penetration,
	glm::vec3 &normal
)
{


	float r = a.shape.r + b.shape.r;
	float rSquared = r * r;
	float distanceSquared = ((a.position.x - b.position.x) * (a.position.x - b.position.x)
		+ (a.position.y - b.position.y) * (a.position.y - b.position.y)
		+ (a.position.z - b.position.z) * (a.position.z - b.position.z));

	bool rez = rSquared > distanceSquared;

	if (rez)
	{
		normal = b.position - a.position;
		normalizeSafe(normal);
		penetration = r - sqrt(distanceSquared);
	}

	return rez;
}

void positionalCorrection(PhysicsObject &A, PhysicsObject &B, glm::vec3 n,
	float penetrationDepth, float aInverseMass, float bInverseMass)
{

	const float percent = 0.2; // usually 20% to 80%
	const float slop = 0.01; // usually 0.01 to 0.1 

	glm::vec3 correction = (glm::max(penetrationDepth - slop, 0.0f) / (aInverseMass + bInverseMass)) * percent * n;

	if(aInverseMass) A.position -= aInverseMass * correction;
	if(bInverseMass) B.position += bInverseMass * correction;
};

float pythagoreanSolve(float fA, float fB)
{
	return std::sqrt(fA * fA + fB * fB);
}

void applyFriction(PhysicsObject &A, PhysicsObject &B, glm::vec3 tangent, glm::vec3 rv,
	float aInverseMass, float bInverseMass, float j)
{
	// Solve for magnitude to apply along the friction vector
	float jt = -glm::dot(rv, tangent);
	jt = jt / (aInverseMass + bInverseMass);

	// PythagoreanSolve = A^2 + B^2 = C^2, solving for C given A and B
	// Use to approximate mu given friction coefficients of each body
	float mu = pythagoreanSolve(A.staticFriction, B.staticFriction);

	// Clamp magnitude of friction and create impulse vector
	//(Coulomb's Law) Ff<=Fn
	glm::vec3 frictionImpulse = {};
	if (abs(jt) < j * mu)
	{
		frictionImpulse = jt * tangent;
	}
	else
	{
		float dynamicFriction = pythagoreanSolve(A.dynamicFriction, B.dynamicFriction);
		frictionImpulse = -j * tangent * dynamicFriction;
	}

	// Apply
	if(aInverseMass) A.velocity -= (aInverseMass)*frictionImpulse;
	if(bInverseMass) B.velocity += (bInverseMass)*frictionImpulse;

};


void impulseResolution(PhysicsObject &A, PhysicsObject &B, glm::vec3 normal,
	float velAlongNormal, float penetrationDepth)
{

	//calculate elasticity
	float e = std::min(A.bouncyness, B.bouncyness);
	//float e = 0.9;

	float massInverseA = 1.f / A.mass;
	float massInverseB = 1.f / B.mass;

	if (A.mass == 0 || A.mass == INFINITY) { massInverseA = 0; }
	if (B.mass == 0 || B.mass == INFINITY) { massInverseB = 0; }

	// Calculate impulse scalar
	float j = -(1.f + e) * velAlongNormal;
	j /= massInverseA + massInverseB;

	// Apply impulse
	glm::vec3 impulse = j * normal;
	if(massInverseA) A.velocity -= massInverseA * impulse;
	if(massInverseB) B.velocity += massInverseB * impulse;

	positionalCorrection(A, B, normal, penetrationDepth, massInverseA, massInverseB);

	{

		// Re-calculate relative velocity after normal impulse
		// is applied (impulse from first article, this code comes
		// directly thereafter in the same resolve function)

		glm::vec3 rv = B.velocity - A.velocity;

		// Solve for the tangent vector
		glm::vec3 tangent = rv - glm::dot(rv, normal) * normal;

		normalizeSafe(tangent);

		applyFriction(A, B, tangent, rv, massInverseA, massInverseB, j);
	}
};

bool AABBvsSphere(PhysicsObject abox, PhysicsObject bsphere, float &penetration, glm::vec3 &normal)
{
	// Vector from A to B
	glm::vec3 n = bsphere.position - abox.position;

	// Closest point on A to center of B
	glm::vec3 closest = n;

	// Calculate half extents along each axis for the AABB
	float x_extent = (abox.getMax().x - abox.getMin().x) / 2.0f;
	float y_extent = (abox.getMax().y - abox.getMin().y) / 2.0f;
	float z_extent = (abox.getMax().z - abox.getMin().z) / 2.0f;

	// Clamp point to edges of the AABB
	closest.x = glm::clamp(closest.x, -x_extent, x_extent);
	closest.y = glm::clamp(closest.y, -y_extent, y_extent);
	closest.z = glm::clamp(closest.z, -z_extent, z_extent);

	bool inside = false;

	// Check if sphere center is inside the AABB
	if (n == closest)
	{
		inside = true;

		// Clamp to closest extent along the axis with the largest component
		if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z))
		{
			closest.x = (closest.x > 0) ? x_extent : -x_extent;
		}
		else if (abs(n.y) > abs(n.z))
		{
			closest.y = (closest.y > 0) ? y_extent : -y_extent;
		}
		else
		{
			closest.z = (closest.z > 0) ? z_extent : -z_extent;
		}
	}

	// Calculate the vector from the closest point on A to the center of B
	glm::vec3 normal3D = n - closest;
	float d = glm::dot(normal3D, normal3D);
	float r = bsphere.shape.r;

	// Early out if the distance to the closest point is greater than the sphere's radius and the sphere is not inside the AABB
	if (d > r * r && !inside)
	{
		return false;
	}

	// Calculate the actual distance if needed
	d = sqrt(d);

	// Set the collision normal and penetration depth
	if (inside)
	{
		normal = -glm::normalize(normal3D);
		penetration = r - d;
	}
	else
	{
		normal = glm::normalize(normal3D);
		penetration = r - d;
	}

	return true;
}


bool CylindervsCylinder(PhysicsObject a, PhysicsObject b, float &penetration, glm::vec3 &normal)
{
	// Vector from A to B
	glm::vec3 n = b.position - a.position;

	auto aMax = a.getMax();
	auto aMin = a.getMin();
	auto bMax = b.getMax();
	auto bMin = b.getMin();

	// Calculate half extents along each axis for each object
	float a_extent_x = (aMax.x - aMin.x) / 2.0f;
	float b_extent_x = (bMax.x - bMin.x) / 2.0f;
	float a_extent_y = (aMax.y - aMin.y) / 2.0f;
	float b_extent_y = (bMax.y - bMin.y) / 2.0f;
	float a_extent_z = (aMax.z - aMin.z) / 2.0f;
	float b_extent_z = (bMax.z - bMin.z) / 2.0f;

	// Calculate overlaps on each axis
	float x_overlap = a_extent_x + b_extent_x - abs(n.x);
	float y_overlap = a_extent_y + b_extent_y - abs(n.y);
	float z_overlap = a_extent_z + b_extent_z - abs(n.z);

	glm::vec2 distantaXZ(n.x, n.z);

	float r = a.shape.r + b.shape.r;
	float rSquared = r * r;
	float distanceSquared = ((a.position.x - b.position.x) * (a.position.x - b.position.x)
		+ (a.position.z - b.position.z) * (a.position.z - b.position.z));
	bool overlapXZ = rSquared > distanceSquared;


	// SAT test on x, y, and z axes
	if (y_overlap > 0 && overlapXZ)
	{
		float XZdist = std::sqrt(distanceSquared);
		float xzOverlap = r - XZdist;

		// Determine the axis of least penetration
		if (y_overlap < xzOverlap)
		{
			normal = (n.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
			penetration = y_overlap;
		}
		else
		{

			if (distantaXZ.x == 0 && distantaXZ.y == 0)
			{
				normal = glm::vec3(-1, 0, 0);
				penetration = r;
			}
			else
			{


				distantaXZ /= XZdist; //normalize
				normal.x = distantaXZ.x;
				normal.y = 0;
				normal.z = distantaXZ.y;

				penetration = xzOverlap;
			}
		}

		return true;
	}

	return false;

}


//a is cube
//b is cylinder
bool AABBvsCylinder(PhysicsObject a, PhysicsObject b, float &penetration, glm::vec3 &normal)
{
	// Vector from A to B
	glm::vec3 n = b.position - a.position;

	auto aMax = a.getMax();
	auto aMin = a.getMin();
	auto bMax = b.getMax();
	auto bMin = b.getMin();

	// Calculate half extents along each axis for each object
	float a_extent_x = (aMax.x - aMin.x) / 2.0f;
	float b_extent_x = (bMax.x - bMin.x) / 2.0f;
	float a_extent_y = (aMax.y - aMin.y) / 2.0f;
	float b_extent_y = (bMax.y - bMin.y) / 2.0f;
	float a_extent_z = (aMax.z - aMin.z) / 2.0f;
	float b_extent_z = (bMax.z - bMin.z) / 2.0f;

	// Calculate overlaps on each axis
	float x_overlap = a_extent_x + b_extent_x - abs(n.x);
	float y_overlap = a_extent_y + b_extent_y - abs(n.y);
	float z_overlap = a_extent_z + b_extent_z - abs(n.z);


	// Closest point on A to center of B
	glm::vec3 closest = n;

	// Calculate half extents along each axis for the AABB
	float x_extent = (aMax.x - aMin.x) / 2.0f;
	float y_extent = (aMax.y - aMin.y) / 2.0f;
	float z_extent = (aMax.z - aMin.z) / 2.0f;

	// Clamp point to edges of the AABB
	closest.x = glm::clamp(closest.x, -x_extent, x_extent);
	closest.y = glm::clamp(closest.y, -y_extent, y_extent);
	closest.z = glm::clamp(closest.z, -z_extent, z_extent);

	bool inside = false;

	// Check if sphere center is inside the AABB
	if (n == closest)
	{
		inside = true;

		// Clamp to closest extent along the axis with the largest component
		if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z))
		{
			closest.x = (closest.x > 0) ? x_extent : -x_extent;
		}
		else if (abs(n.y) > abs(n.z))
		{
			closest.y = (closest.y > 0) ? y_extent : -y_extent;
		}
		else
		{
			closest.z = (closest.z > 0) ? z_extent : -z_extent;
		}
	}

	glm::vec3 closestWorldPos = closest + a.position;


	float distantaClosestPoint = glm::distance(glm::vec3{closestWorldPos.x, 0.f, closestWorldPos.z}, {b.position.x, 0.f, b.position.z});
	float overlapXZ = b.shape.x - distantaClosestPoint;

	// SAT test on x, y, and z axes
	if (inside || (y_overlap > 0 && overlapXZ > 0))
	{

		// Determine the axis of least penetration
		if (y_overlap < overlapXZ)
		{
			normal = (n.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
			penetration = y_overlap;
		}
		else
		{
			//cerc patrat

			// Calculate the vector from the closest point on A to the center of B
			glm::vec3 normal3D = n - closest; normal3D.y = 0;
			float d = glm::dot(normal3D, normal3D);
			float r = b.shape.r;

			if (normal3D.x == 0 && normal3D.z == 0)
			{
				normal = -glm::vec3({1,0,0});
				penetration = r;
				return true;
			}

			d = sqrt(d);

			// Set the collision normal and penetration depth
			if (inside)
			{
				normal = -glm::normalize(normal3D);
				penetration = r - d;
			}
			else
			{
				normal = glm::normalize(normal3D);
				penetration = r - d;
			}
		}

		return true;
	}

	return false;

}


//a is sphere
//b is cylinder
bool SpherevsCylinder(PhysicsObject a, PhysicsObject b, float &penetration, glm::vec3 &normal)
{
	// Vector from A to B
	glm::vec3 n = b.position - a.position;

	auto aMax = a.getMax();
	auto aMin = a.getMin();
	auto bMax = b.getMax();
	auto bMin = b.getMin();

	// Calculate half extents along each axis for each object
	float a_extent_x = (aMax.x - aMin.x) / 2.0f;
	float b_extent_x = (bMax.x - bMin.x) / 2.0f;
	float a_extent_y = (aMax.y - aMin.y) / 2.0f;
	float b_extent_y = (bMax.y - bMin.y) / 2.0f;
	float a_extent_z = (aMax.z - aMin.z) / 2.0f;
	float b_extent_z = (bMax.z - bMin.z) / 2.0f;

	// Calculate overlaps on each axis
	//float x_overlap = a_extent_x + b_extent_x - abs(n.x);
	//float y_overlap = a_extent_y + b_extent_y - abs(n.y);
	//float z_overlap = a_extent_z + b_extent_z - abs(n.z);

	float y_overlap = 0;
	bool inside = false;

	// Closest point on B to center of A sphere
	glm::vec3 closest = -n;
	{

		// Calculate half extents along each axis for the AABB
		float x_extent = (bMax.x - bMin.x) / 2.0f;
		float y_extent = (bMax.y - bMin.y) / 2.0f;
		float z_extent = (bMax.z - bMin.z) / 2.0f;

		// Clamp point to edges of the AABB
		closest.x = glm::clamp(closest.x, -x_extent, x_extent);
		closest.y = glm::clamp(closest.y, -y_extent, y_extent);
		closest.z = glm::clamp(closest.z, -z_extent, z_extent);


		// Check if sphere center is inside the AABB
		if (n == closest)
		{
			inside = true;

			// Clamp to closest extent along the axis with the largest component
			if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z))
			{
				closest.x = (closest.x > 0) ? x_extent : -x_extent;
			}
			else if (abs(n.y) > abs(n.z))
			{
				closest.y = (closest.y > 0) ? y_extent : -y_extent;
			}
			else
			{
				closest.z = (closest.z > 0) ? z_extent : -z_extent;
			}
		}

		glm::vec3 closestWorldPos = closest + b.position;

		y_overlap = a.shape.x - std::abs(a.position.y - closestWorldPos.y);
	}

	float overlapXZ = 0;
	float r = a.shape.r + b.shape.r;
	float distanceSquared = 0;
	{
		glm::vec2 distantaXZ(n.x, n.z);
		float rSquared = r * r;
		float distanceSquared = ((a.position.x - b.position.x) * (a.position.x - b.position.x)
			+ (a.position.z - b.position.z) * (a.position.z - b.position.z));
		float XZdist = std::sqrt(distanceSquared);
		overlapXZ = r - XZdist;
	}


	// SAT test on x, y, and z axes
	if (inside || (y_overlap > 0 && overlapXZ > 0))
	{

		// Determine the axis of least penetration
		if (y_overlap < overlapXZ)
		{
			normal = (n.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
			penetration = y_overlap;

			//cerc patrat

			//// Calculate the vector from the closest point on A to the center of B
			//glm::vec3 normal3D = a.position - (b.position + closest);
			//float d = glm::dot(normal3D, normal3D);
			//float r = a.shape.r;
			//
			//if (normal3D.x == 0 && normal3D.z == 0)
			//{
			//	normal = -glm::vec3({1,0,0});
			//	penetration = r;
			//	return true;
			//}
			//
			//d = sqrt(d);
			//
			//// Set the collision normal and penetration depth
			//if (inside)
			//{
			//	normal = -glm::normalize(normal3D);
			//	penetration = r - d;
			//}
			//else
			//{
			//	normal = glm::normalize(normal3D);
			//	penetration = r - d;
			//}
		}
		else
		{
			//cerc cerc pe XZ
			normal = b.position - a.position;
			normal.y = 0;
			normalizeSafe(normal);
			penetration = overlapXZ;

		}

		return true;
	}

	return false;

}



void Simulator::update(float deltaTime)
{

	size_t bodiesSize = bodies.size();
	for (auto &body: bodies)
	{
		auto &b = body.second;

		//gravity
		b.acceleration += glm::vec3{0, -9.81, 0};

		if(b.mass != 0 && b.mass != INFINITY)
		applyDrag(b);

		//detect colisions
		for (auto &body2 : bodies)
		{

			if (body.first == body2.first) { continue; }

			auto &A = b;
			auto &B = body2.second;

			if (A.type == TYPE_CIRCLE &&
				B.type == TYPE_CIRCLE
				)
			{
				glm::vec3 normal = {};
				float penetration = 0;

				if (CirclevsCircle(
					A, B,
					penetration, normal))
				{
					glm::vec3 relativeVelocity = B.velocity - A.velocity;
					float velAlongNormal = glm::dot(relativeVelocity, normal);

					// Do not resolve if velocities are separating
					if (velAlongNormal > 0)
					{

					}
					else
					{
						impulseResolution(A, B, normal, velAlongNormal, penetration);
					}
				}

			}
			else
				if (A.type == TYPE_BOX &&
					B.type == TYPE_BOX
					)
				{
					glm::vec3 normal = {};
					float penetration = 0;

					if (AABBvsAABB(
						A, B,
						penetration, normal))
					{
						glm::vec3 relativeVelocity = B.velocity - A.velocity;
						float velAlongNormal = glm::dot(relativeVelocity, normal);

						// Do not resolve if velocities are separating
						if (velAlongNormal > 0)
						{

						}
						else
						{
							impulseResolution(A, B, normal, velAlongNormal, penetration);
						}
					}

				}
				else
					if (A.type == TYPE_BOX &&
						B.type == TYPE_CIRCLE
						)
					{

						glm::vec3 normal = {};
						float penetration = 0;

						if (AABBvsSphere(
							A, B,
							penetration, normal))
						{
							glm::vec3 relativeVelocity = B.velocity - A.velocity;
							float velAlongNormal = glm::dot(relativeVelocity, normal);

							// Do not resolve if velocities are separating
							if (velAlongNormal > 0)
							{

							}
							else
							{
								impulseResolution(A, B, normal, velAlongNormal, penetration);
							}
						}
					}
					else
						if (A.type == TYPE_CILINDRU &&
							B.type == TYPE_CILINDRU
							)
						{

							glm::vec3 normal = {};
							float penetration = 0;

							if (CylindervsCylinder(
								A, B,
								penetration, normal))
							{
								glm::vec3 relativeVelocity = B.velocity - A.velocity;
								float velAlongNormal = glm::dot(relativeVelocity, normal);

								// Do not resolve if velocities are separating
								if (velAlongNormal > 0)
								{

								}
								else
								{
									impulseResolution(A, B, normal, velAlongNormal, penetration);
								}
							}
						}
						else if (A.type == TYPE_BOX && B.type == TYPE_CILINDRU)
						{
							glm::vec3 normal = {};
							float penetration = 0;

							if (AABBvsCylinder(
								A, B,
								penetration, normal))
							{
								glm::vec3 relativeVelocity = B.velocity - A.velocity;
								float velAlongNormal = glm::dot(relativeVelocity, normal);

								// Do not resolve if velocities are separating
								if (velAlongNormal > 0)
								{

								}
								else
								{
									impulseResolution(A, B, normal, velAlongNormal, penetration);
								}
							}
						}
						else if (A.type == TYPE_CIRCLE && B.type == TYPE_CILINDRU)
						{
							glm::vec3 normal = {};
							float penetration = 0;

							if (SpherevsCylinder(
								A, B,
								penetration, normal))
							{
								glm::vec3 relativeVelocity = B.velocity - A.velocity;
								float velAlongNormal = glm::dot(relativeVelocity, normal);

								// Do not resolve if velocities are separating
								if (velAlongNormal > 0)
								{

								}
								else
								{
									impulseResolution(A, B, normal, velAlongNormal, penetration);
								}
							}
						}



		}

		updateForces(b, deltaTime);

	#pragma region hit walls
		auto minPos = b.getMin();
		auto maxPos = b.getMax();

		float boxDown = -boxDimensions.y / 2;

		if (minPos.y < boxDown)
		{
			float extra = boxDown - minPos.y;
			b.position.y += extra;
			b.velocity.y *= -1;
		}

		if (maxPos.y > boxDimensions.y / 2)
		{
			float extra = maxPos.y - boxDimensions.y / 2;
			b.position.y -= extra;
			b.velocity.y *= -1;
		}

		if (minPos.x < -boxDimensions.x / 2)
		{
			float extra = -boxDimensions.x / 2 - minPos.x;
			b.position.x += extra;
			b.velocity.x *= -1;
		}

		if (minPos.z < -boxDimensions.z / 2)
		{
			float extra = -boxDimensions.z / 2 - minPos.z;
			b.position.z += extra;
			b.velocity.z *= -1;
		}

		if (maxPos.x > boxDimensions.x / 2)
		{
			float extra = maxPos.x - boxDimensions.x / 2;
			b.position.x -= extra;
			b.velocity.x *= -1;
		}

		if (maxPos.z > boxDimensions.z / 2)
		{
			float extra = maxPos.z - boxDimensions.z / 2;
			b.position.z -= extra;
			b.velocity.z *= -1;
		}
	#pragma endregion


	}
}