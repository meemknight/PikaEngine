//////////////////////////////////////////////////
//ph2d.h				0.0.1
//Copyright(c) 2024 Luta Vlad - Low Level Game Dev
// please credit me if you want, 
// and keep this in the code
//
//////////////////////////////////////////////////


//credits:
//https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331

//todo layers
//todo apply force,
//todo optimize collisions using some octrees maybe or something.


#include <ph2d/ph2d.h>
#include <iostream>

constexpr static float MAX_VELOCITY = 10'000;
constexpr static float MAX_ACCELERATION = 10'000;
constexpr static float MAX_AIR_DRAG = 100;

void normalizeSafe(glm::vec2 &v)
{
	float l = glm::length(v);

	if (l <= 0.00000001)
	{
		v = {1,0};
	}
	else
	{
		v /= l;
	}
}

namespace ph2d
{

	void AABB::rotateAroundCenter(float r)
	{
		glm::vec2 newCenter = ph2d::rotateAroundCenter(center(), r);
		pos = newCenter - size / 2.f;
	}

	void AABB::getMinMaxPointsRotated(glm::vec2 &outMin, glm::vec2 &outMax, float r)
	{
		glm::vec2 minPoint = min();
		glm::vec2 maxPoint = max();
		glm::vec2 point3 = {minPoint.x, maxPoint.y};
		glm::vec2 point4 = {maxPoint.x, minPoint.y};

		glm::vec2 centerP = center();

		minPoint = ph2d::rotateAroundPoint(minPoint, centerP, r);
		maxPoint = ph2d::rotateAroundPoint(maxPoint, centerP, r);
		point3 = ph2d::rotateAroundPoint(point3, centerP, r);
		point4 = ph2d::rotateAroundPoint(point4, centerP, r);

		outMin = minPoint;
		if (maxPoint.x < outMin.x) { outMin.x = maxPoint.x; }
		if (point3.x < outMin.x) { outMin.x = point3.x; }
		if (point4.x < outMin.x) { outMin.x = point4.x; }

		if (maxPoint.y < outMin.y) { outMin.y = maxPoint.y; }
		if (point3.y < outMin.y) { outMin.y = point3.y; }
		if (point4.y < outMin.y) { outMin.y = point4.y; }

		outMax = minPoint;
		if (maxPoint.x > outMax.x) { outMax.x = maxPoint.x; }
		if (point3.x > outMax.x) { outMax.x = point3.x; }
		if (point4.x > outMax.x) { outMax.x = point4.x; }

		if (maxPoint.y > outMax.y) { outMax.y = maxPoint.y; }
		if (point3.y > outMax.y) { outMax.y = point3.y; }
		if (point4.y > outMax.y) { outMax.y = point4.y; }

	}

	void AABB::getCornersRotated(glm::vec2 corners[4], float r)
	{
		corners[0] = min();
		corners[1] = max();
		corners[2] = {corners[0].x, corners[1].y};
		corners[3] = {corners[1].x, corners[0].y};

		glm::vec2 c = center();

		if (r)
		{
			corners[0] = rotateAroundPoint(corners[0], c, r);
			corners[1] = rotateAroundPoint(corners[1], c, r);
			corners[2] = rotateAroundPoint(corners[2], c, r);
			corners[3] = rotateAroundPoint(corners[3], c, r);
		}

	}

	//The second is the circle
	bool AABBvsCircle(AABB abox, AABB bbox, float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint)
	{
		normal = {};
		// Vector from A to B
		glm::vec2 n = bbox.center() - abox.center();

		// Closest point on A to center of B
		glm::vec2 closest = n;

		// Calculate half extents along each axis
		float x_extent = (abox.max().x - abox.min().x) / 2;
		float y_extent = (abox.max().y - abox.min().y) / 2;

		// Clamp point to edges of the AABB
		closest.x = glm::clamp(closest.x, - x_extent, x_extent);
		closest.y = glm::clamp(closest.y, -y_extent, y_extent);

		bool inside = false;
		// Circle is inside the AABB, so we need to clamp the circle's center
		// to the closest edge
		if (n == closest)
		{

			inside = true;

			// Find closest axis
			if (abs(n.x) > abs(n.y))
			{

				// Clamp to closest extent
				if (closest.x > 0)
				{
					closest.x = x_extent;
				}
				else
				{
					closest.x = -x_extent;
				}
			}
			// y axis is shorter
			else
			{
				// Clamp to closest extent
				if (closest.y > 0)
				{
					closest.y = y_extent;
				}
				else
				{
					closest.y = -y_extent;
				};
			}

		}

		glm::vec2 normal2 = n - closest;
		float d = glm::dot(normal2, normal2);
		float r = bbox.size.x/2.f;

		// Early out of the radius is shorter than distance to closest point and
		// Circle not inside the AABB
		if (d > r * r && !inside)
		{
			return false;
		}


		// Avoided sqrt until we needed
		d = sqrt(d);

		// Collision normal needs to be flipped to point outside if circle was
		// inside the AABB
		if (inside)
		{
			normal = -normal2;
			penetration = r - d;
			normalizeSafe(normal);

			glm::vec2 midpoint = (abox.center() + bbox.center()) * 0.5f;
			//contactPoint = midpoint - (normal * (penetration * 0.5f));
			contactPoint = bbox.center() + (-normal * (r - std::min(penetration, r) * 0.5f));
			//contactPoint = bbox.center();
		}
		else
		{
			normal = normal2;
			penetration = r - d;
			normalizeSafe(normal);
			contactPoint = bbox.center() + (-normal * (r - penetration * 0.5f));
		}



		return true;
	}

	bool HalfSpaceVSCircle(LineEquation line, AABB circle, float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint)
	{
		float r = circle.size.x / 2.f;
		
		line.normalize();

		normal = line.getNormal();

		glm::vec2 center = circle.center();
		float distance = line.computeEquation(center);

		penetration = r + distance;

		if (penetration > 0)
		{
			contactPoint = center + (normal * (r - penetration * 0.5f));
			return 1;
		}

		return 0;
	}

	//The second is the circle
	bool OBBvsCircle(AABB abox, float ar, AABB bbox, float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint)
	{
		
		if(ar == 0){ return AABBvsCircle(abox, bbox, penetration, normal, contactPoint); }

		glm::vec2 centerA = abox.center();

		abox.pos -= centerA;
		bbox.pos -= centerA;

		bbox.rotateAroundCenter(-ar);

		bool rez = AABBvsCircle(abox, bbox, penetration, normal, contactPoint);

		normal = rotateAroundCenter(normal, ar);
		normalizeSafe(normal);
		contactPoint = rotateAroundCenter(contactPoint, ar);
		contactPoint += centerA;

		return rez;
	}

	bool AABBvsAABB(AABB abox, AABB bbox, float &penetration,
		glm::vec2 &normal)
	{
		// Vector from A to B
		glm::vec2 n = bbox.center() - abox.center();

		// Calculate half extents along x axis for each object
		float a_extent = (abox.max().x - abox.min().x) / 2;
		float b_extent = (bbox.max().x - bbox.min().x) / 2;

		auto aMax = abox.max();
		auto aMin = abox.min();
		auto bMax = bbox.max();
		auto bMin = bbox.min();

		// Calculate overlap on x axis
		float x_overlap = a_extent + b_extent - abs(n.x);

		// SAT test on x axis
		if (x_overlap > 0)
		{

			// Calculate half extents along x axis for each object
			float a_extent = (abox.max().y - abox.min().y) / 2.f;
			float b_extent = (bbox.max().y - bbox.min().y) / 2.f;

			// Calculate overlap on y axis
			float y_overlap = a_extent + b_extent - abs(n.y);

			// SAT test on y axis
			if (y_overlap > 0)
			{

				// Find out which axis is axis of least penetration
				if (x_overlap < y_overlap)
				{

					// Point towards B knowing that n points from A to B
					if (n.x < 0)
					{
						normal = glm::vec2(-1, 0);
					}
					else
					{
						normal = glm::vec2(1, 0);
					}

					penetration = x_overlap;
					return true;
				}
				else
				{

					// Point toward B knowing that n points from A to B
					if (n.y < 0)
					{
						normal = glm::vec2(0, -1);
					}
					else
					{
						normal = glm::vec2(0, 1);
					}

					penetration = y_overlap;
					return true;
				}

			}

		}

		return false;
	}

	float calculatePenetrationAlongOneAxe(glm::vec2 *aPoints, size_t aPointsCount,
		glm::vec2 *bPoints, size_t bPointsCount, glm::vec2 axeDirection, bool *flipSign)
	{

		if (aPointsCount <= 0 || bPointsCount <= 0 || !aPoints || !bPoints) { return 0; }

		float d0a = glm::dot(aPoints[0], axeDirection);
		float d0b = glm::dot(bPoints[0], axeDirection);

		float aMin = d0a;
		float aMax = d0a;
		float bMin = d0b;
		float bMax = d0b;

		for (int i = 1; i < aPointsCount; i++)
		{
			glm::vec2 p = aPoints[i];
			float d = glm::dot(p, axeDirection);
			if (d > aMax) { aMax = d; }
			if (d < aMin) { aMin = d; }
		}

		for (int i = 1; i < bPointsCount; i++)
		{
			glm::vec2 p = bPoints[i];
			float d = glm::dot(p, axeDirection);
			if (d > bMax) { bMax = d; }
			if (d < bMin) { bMin = d; }
		}

		// Calculate overlaps
		float overlapA = aMax - bMin; // Overlap from A to B
		float overlapB = bMax - aMin; // Overlap from B to A

		if (overlapA < overlapB)
		{
			if (flipSign) { *flipSign = 0; }
			return overlapA;
		}
		else
		{
			if (flipSign) { *flipSign = 1; }
			return overlapB;
		}
	}


	bool HalfSpaceVSOBB(LineEquation line, AABB bbox, float br, float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint)
	{

		glm::vec2 corners[4] = {};
		bbox.getCornersRotated(corners, br);

		float intersections[4] = {};
		int intersectioncCount = 0;
		float biggestPenetration = -100000;
		glm::vec2 intersectedCrners[4] = {};


		for (int i = 0; i < 4; i++)
		{
			float rez = 0;
			rez = line.computeEquation(corners[i]);

			if (rez >= 0)
			{
				intersections[intersectioncCount] = rez;
				intersectedCrners[intersectioncCount] = corners[i];
				intersectioncCount++;
				
				if (rez > biggestPenetration)
				{
					biggestPenetration = rez;
				}
			}
		}

		if (intersectioncCount)
		{
			line.normalize();
			normal = line.getNormal();
			penetration = biggestPenetration;

			contactPoint = {};

			for (int i = 0; i < intersectioncCount; i++)
			{
				contactPoint += intersectedCrners[i];
			}

			contactPoint /= intersectioncCount;

			if (intersectioncCount != 4)
			{
				contactPoint += normal * (-penetration / 2.f);
			}

			return true;
		}

		return false;
	}

	//a is aabb and b has a rotation
	bool AABBvsOBB(AABB a, AABB b, float br)
	{

		glm::vec2 aMin = a.min();
		glm::vec2 aMax = a.max();

		glm::vec2 bMin = {};
		glm::vec2 bMax = {};
		b.getMinMaxPointsRotated(bMin, bMax, br);


		if (aMax.x < bMin.x || aMin.x > bMax.x) return false;
		if (aMax.y < bMin.y || aMin.y > bMax.y) return false;

		//return true;

		//passed first axis test, try the other one
		{
			ph2d::AABB newA = a;
			ph2d::AABB newB = b;
			float newRotationA = -br;

			glm::vec2 bCenter = b.center();
			newA.pos -= bCenter;
			newB.pos -= bCenter;

			newA.rotateAroundCenter(newRotationA);

			glm::vec2 aMin = {};
			glm::vec2 aMax = {};
			glm::vec2 bMin = newB.min();
			glm::vec2 bMax = newB.max();

			newA.getMinMaxPointsRotated(aMin, aMax, newRotationA);

			if (aMax.x < bMin.x || aMin.x > bMax.x) return false;
			if (aMax.y < bMin.y || aMin.y > bMax.y) return false;
		}

		return true;

	}

	bool OBBvsOBB(AABB a, float ar, AABB b, float br)
	{
		//move A in center.
		glm::vec2 aPos = a.center();
		a.pos -= aPos;
		b.pos -= aPos;

		//we rotate both cubes so now a is axis aligned
		b.rotateAroundCenter(-ar);
		br -= ar;

		return AABBvsOBB(a, b, br);
	}

	bool OBBvsOBB(AABB a, float ar, AABB b, float br,
		float &penetration, glm::vec2 &normal, glm::vec2 &contactPoint)
	{
		penetration = 0;
		normal = {0,0};

		glm::vec2 cornersA[4] = {};
		glm::vec2 cornersB[4] = {};
		a.getCornersRotated(cornersA, ar);
		b.getCornersRotated(cornersB, br);

		glm::vec2 axes[4] = {
			 rotateAroundCenter({0, 1}, ar), // Normal of A's first edge
			 rotateAroundCenter({1, 0}, ar), // Normal of A's second edge
			 rotateAroundCenter({0, 1}, br), // Normal of B's first edge
			 rotateAroundCenter({1, 0}, br)  // Normal of B's second edge
		};

		// Initialize the minimum penetration depth
		float minPenetration = FLT_MAX;
		glm::vec2 minPenetrationAxis = {};
		bool flipSign = 0;

		// Test each axis
		for (int i = 0; i < 4; ++i)
		{
			bool flip = 0;
			float penetrationDepth = calculatePenetrationAlongOneAxe(
				cornersA, 4, cornersB, 4, axes[i], &flip);

			// If there's no overlap along this axis, shapes are not colliding
			if (penetrationDepth < 0.0f)
			{
				return false; // No collision
			}

			// Find the axis of least penetration
			if (penetrationDepth < minPenetration)
			{
				minPenetration = penetrationDepth;
				minPenetrationAxis = axes[i];
				flipSign = flip;
			}
		}

		penetration = minPenetration;
		normal = glm::normalize(minPenetrationAxis);
		if (flipSign) { normal = -normal; }


		//contactPoint
		// Midpoint between the two centers
		glm::vec2 midpoint = (a.center() + b.center()) * 0.5f;
		// Adjust by half of the penetration depth along the collision normal
		contactPoint = midpoint - (normal * (penetration * 0.5f));

		return true;
	}


	bool AABBvsAABB(AABB a, AABB b, float delta)
	{
		glm::vec2 aMax = a.max() + glm::vec2(delta, delta);
		glm::vec2 bMax = b.max() + glm::vec2(delta, delta);

		glm::vec2 aMin = a.min() - glm::vec2(delta, delta);
		glm::vec2 bMin = b.min() - glm::vec2(delta, delta);

		if (aMax.x < bMin.x || aMin.x > bMax.x) return false;
		if (aMax.y < bMin.y || aMin.y > bMax.y) return false;

		return true;
	}

	bool AABBvsPoint(AABB a, glm::vec2 b, float delta)
	{
		glm::vec2 aMin = a.min() - glm::vec2(delta, delta);
		glm::vec2 aMax = a.max() + glm::vec2(delta, delta);

		if (
			aMin.x < b.x && aMax.x > b.x &&
			aMin.y < b.y && aMax.y > b.y)
		{
			return true;
		}

		return false;
	}

	glm::vec2 rotateAroundCenter(glm::vec2 in, float r)
	{
		float c = std::cos(-r);
		float s = std::sin(-r);
		return glm::vec2(in.x * c - in.y * s, in.x * s + in.y * c);
	}

	glm::vec2 rotateAroundPoint(glm::vec2 in, glm::vec2 centerReff, float r)
	{
		in -= centerReff;
		in = rotateAroundCenter(in, r);
		in += centerReff;

		return in;
	}

	bool OBBvsPoint(AABB a, float rotation, glm::vec2 b, float delta)
	{
		if (rotation == 0)
		{
			return AABBvsPoint(a, b, delta);
		}

		//moved the cube in the center of the screen.
		glm::vec2 pos = a.center();
		a.pos -= pos;
		b -= pos;

		b = rotateAroundCenter(b, -rotation);

		return AABBvsPoint(a, b, delta);
	}

	bool CircleVsPoint(glm::vec2 pos, float r, glm::vec2 p, float delta)
	{
		glm::vec2 dist = pos - p;

		float rSquared = (r + delta) * (r + delta);
		float distSquared = glm::dot(dist, dist);

		return distSquared < rSquared;
	}

	bool CirclevsCircle(Circle a, Circle b,
		float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint
		)
	{
		float r = a.r + b.r;
		float rSquared = r * r;
		float distanceSquared = ((a.center.x - b.center.x) * (a.center.x - b.center.x)
			+ (a.center.y - b.center.y) * (a.center.y - b.center.y));

		bool rez = rSquared > distanceSquared;

		if(rez)
		{
			normal = b.center - a.center;
			normalizeSafe(normal);
			penetration = r - sqrt(distanceSquared);

			contactPoint = a.center + normal * (a.r - penetration / 2.f);
		}

		return rez;
	}

	bool BodyvsBody(Body &A, Body &B, float &penetration,
		glm::vec2 &normal, glm::vec2 &contactPoint)
	{

		if (A.collider.type == ph2d::ColliderCircle &&
			B.collider.type == ph2d::ColliderCircle
			)
		{

			return ph2d::CirclevsCircle(
				glm::vec3(A.motionState.pos, A.collider.collider.circle.radius),
				glm::vec3(B.motionState.pos, B.collider.collider.circle.radius),
				penetration, normal, contactPoint);
			
		}
		else if (A.collider.type == ph2d::ColliderBox &&
			B.collider.type == ph2d::ColliderBox)
		{
			
			auto abox = A.getAABB();
			auto bbox = B.getAABB();

			return ph2d::OBBvsOBB(
				abox, A.motionState.rotation, bbox,
				B.motionState.rotation,
				penetration, normal, contactPoint);

		}

		else if (A.collider.type == ph2d::ColliderBox &&
			B.collider.type == ph2d::ColliderCircle)
		{

			auto abox = A.getAABB();
			auto bbox = B.getAABB();

			return ph2d::OBBvsCircle(
				abox, A.motionState.rotation, bbox, penetration, normal, contactPoint);

		}
		else if (A.collider.type == ph2d::ColliderCircle &&
			B.collider.type == ph2d::ColliderBox)
		{

			auto abox = A.getAABB();
			auto bbox = B.getAABB();

			return ph2d::OBBvsCircle(
				bbox, B.motionState.rotation, abox, penetration, normal, contactPoint);

		}

		else if (A.collider.type == ph2d::ColliderHalfSpace &&
			B.collider.type == ph2d::ColliderCircle)
		{

			auto bbox = B.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(A.motionState.rotation,
				A.motionState.pos);

			return ph2d::HalfSpaceVSCircle(
				lineEquation, bbox, penetration, normal, contactPoint);
		}
		else if (A.collider.type == ph2d::ColliderCircle &&
			B.collider.type == ph2d::ColliderHalfSpace)
		{

			auto abox = A.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(B.motionState.rotation,
				B.motionState.pos);

			return ph2d::HalfSpaceVSCircle(
				lineEquation, abox, penetration, normal, contactPoint);
		}

		else if (A.collider.type == ph2d::ColliderHalfSpace &&
			B.collider.type == ph2d::ColliderBox)
		{
			auto bbox = B.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(A.motionState.rotation,
				A.motionState.pos);

			return ph2d::HalfSpaceVSOBB(
				lineEquation, bbox, B.motionState.rotation,
				penetration, normal, contactPoint);
		}
		else if (A.collider.type == ph2d::ColliderBox &&
			B.collider.type == ph2d::ColliderHalfSpace)
		{
			auto abox = A.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(B.motionState.rotation,
				B.motionState.pos);

			return ph2d::HalfSpaceVSOBB(
				lineEquation, abox, A.motionState.rotation, penetration, normal, contactPoint);
		}

		return 0;
	}

	//todo make sure drag can't be stronger than speed!
	void applyDrag(MotionState &motionState)
	{
		glm::vec2 dragForce = 0.1f * -motionState.velocity * glm::abs(motionState.velocity) / 2.f;
		float length = glm::length(dragForce);
		if (length)
		{
			if (length > MAX_AIR_DRAG)
			{
				dragForce /= length;
				dragForce *= MAX_AIR_DRAG;
			}
		
			motionState.acceleration += dragForce;
		}
	}

	void integrateForces(MotionState &motionState, float deltaTime)
	{

		if (motionState.mass == 0 || motionState.mass == INFINITY)
		{
			motionState.acceleration = {};
			motionState.velocity = {};
		}
		else
		{
			//linear motion
			motionState.acceleration = glm::clamp(motionState.acceleration,
				glm::vec2(-MAX_ACCELERATION), glm::vec2(MAX_ACCELERATION));

			//Symplectic Euler
			motionState.velocity += motionState.acceleration * deltaTime * 0.5f;
			motionState.velocity = glm::clamp(motionState.velocity, glm::vec2(-MAX_VELOCITY), glm::vec2(MAX_VELOCITY));

			motionState.pos += motionState.velocity * deltaTime;

			motionState.velocity += motionState.acceleration * deltaTime * 0.5f;
			motionState.velocity = glm::clamp(motionState.velocity, glm::vec2(-MAX_VELOCITY), glm::vec2(MAX_VELOCITY));

			if (std::fabs(motionState.velocity.x) < 0.00001) { motionState.velocity.x = 0; }
			if (std::fabs(motionState.velocity.y) < 0.00001) { motionState.velocity.y = 0; }

			motionState.acceleration = {};
		}

		if (motionState.momentOfInertia == 0 || motionState.momentOfInertia == INFINITY)
		{
			motionState.angularVelocity = 0;
			motionState.torque = 0;
		}
		else
		{
			//rotation
			motionState.angularVelocity += motionState.torque * (1.f/motionState.momentOfInertia) * deltaTime;
			motionState.rotation += motionState.angularVelocity * deltaTime;
			motionState.torque = 0;
		}
		
	}


	glm::mat2 rotationMatrix(float angle)
	{
		float c = cos(angle);
		float s = sin(angle);

		return glm::mat2(
			{
				c, -s,
				s, c
			});
	}

	Collider createBoxCollider(glm::vec2 size)
	{
		Collider c;
		c.type = ColliderBox;

		c.collider.box.size = size;

		return c;
	}

	Collider createCircleCollider(float r)
	{
		Collider c;
		c.type = ColliderCircle;

		c.collider.circle.radius = r;

		return c;
	}

	float Collider::computeMass()
	{
		switch (type)
		{
			case ColliderCircle:
			return (collider.circle.radius * collider.circle.radius * 3.1415);
			
			case ColliderBox:
			return collider.box.size.x * collider.box.size.y;

			default:
			return 0;
		}
	}

	float Collider::computeMomentOfInertia(float mass)
	{
		switch (type)
		{

		case ColliderCircle:
		return (collider.circle.radius * collider.circle.radius * mass * 0.5f);

		case ColliderBox:
		return mass * (collider.box.size.x * collider.box.size.x + 
			collider.box.size.y * collider.box.size.y) * (1.f/12.f);

		default:
		return 0;
		}

	}


};

float PythagoreanSolve(float fA, float fB)
{
	return std::sqrt(fA * fA + fB * fB);
}

bool overlap(ph2d::Body &a, ph2d::Body &b)
{

	

	//todo
	return 0;
}

float cross(glm::vec2 a, glm::vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

// More exotic (but necessary) forms of the cross product
// with a vector a and scalar s, both returning a vector
glm::vec2 cross(glm::vec2 a, float s)
{
	return glm::vec2(s * a.y, -s * a.x);
}

glm::vec2 cross(float s, glm::vec2 a)
{
	return glm::vec2(-s * a.y, s * a.x);
}


void ph2d::MotionState::applyImpulseObjectPosition(glm::vec2 impulse, glm::vec2 contactVector)
{
	if (mass != 0 && mass != INFINITY )
	{
		velocity += (1.0f / mass) * impulse;
	}

	//if (momentOfInertia != 0 && momentOfInertia != INFINITY)
	//{
	//	angularVelocity -= (1.0f / momentOfInertia) * cross(contactVector, impulse);
	//}
}

void ph2d::MotionState::applyImpulseWorldPosition(glm::vec2 impulse, glm::vec2 contactVectorWorldPos)
{
	glm::vec2 relVector = contactVectorWorldPos - pos;
	applyImpulseObjectPosition(impulse, relVector);
}



void ph2d::PhysicsEngine::runSimulation(float deltaTime)
{

	int counter = 0;
	if (setFixedTimeStamp <= 0)
	{
		_fixedTimeAccumulated = 0;
		counter = 1;
	}
	else
	{
		_fixedTimeAccumulated += deltaTime;

		_fixedTimeAccumulated = std::min(_fixedTimeAccumulated, maxAccumulated);
		

		while (_fixedTimeAccumulated > setFixedTimeStamp)
		{
			_fixedTimeAccumulated -= setFixedTimeStamp;
			counter++;
		}

		deltaTime = setFixedTimeStamp;
	}

	for (int currentIteration = 0; currentIteration < counter; currentIteration++)
	{
		auto positionalCorrection = [&](Body &A, Body &B, glm::vec2 n,
			float penetrationDepth)
		{

			float massInverseA = 1.f / A.motionState.mass;
			float massInverseB = 1.f / B.motionState.mass;

			if (A.motionState.mass == 0 || A.motionState.mass == INFINITY) { massInverseA = 0; }
			if (B.motionState.mass == 0 || B.motionState.mass == INFINITY) { massInverseB = 0; }

			const float percent = 0.40; // usually 20% to 80%
			const float slop = 0.01; // usually 0.01 to 0.1 

			glm::vec2 correction = (glm::max(penetrationDepth - slop, 0.0f) / (massInverseA + massInverseB)) * percent * n;

			A.motionState.pos -= massInverseA * correction;
			B.motionState.pos += massInverseB * correction;
		};

		auto applyFriction = [&](Body &A, Body &B, glm::vec2 tangent, glm::vec2 rv,
			float aInverseMass, float bInverseMass, float j, glm::vec2 rContactA,
			glm::vec2 rContactB, glm::vec2 contactPoint)
		{

			float momentOfInertiaInverseA = 1.f / A.motionState.momentOfInertia;
			float momentOfInertiaInverseB = 1.f / B.motionState.momentOfInertia;
			if (A.motionState.momentOfInertia == 0 || A.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseA = 0; }
			if (B.motionState.momentOfInertia == 0 || B.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseB = 0; }

			//remove moment of inertia
			momentOfInertiaInverseA = 0;
			momentOfInertiaInverseB = 0;

			// Solve for magnitude to apply along the friction vector
			float jt = -glm::dot(rv, tangent);
			jt = jt / (aInverseMass + bInverseMass)
				+
				(std::pow(cross(rContactA, rv), 2) * momentOfInertiaInverseA) +
				(std::pow(cross(rContactB, rv), 2) * momentOfInertiaInverseB)
				;

			// PythagoreanSolve = A^2 + B^2 = C^2, solving for C given A and B
			// Use to approximate mu given friction coefficients of each body
			float mu = PythagoreanSolve(A.staticFriction, B.staticFriction);

			// Clamp magnitude of friction and create impulse vector
			//(Coulomb's Law) Ff<=Fn
			glm::vec2 frictionImpulse = {};
			if (abs(jt) < j * mu)
			{
				frictionImpulse = jt * tangent;
			}
			else
			{
				float dynamicFriction = PythagoreanSolve(A.dynamicFriction, B.dynamicFriction);
				frictionImpulse = -j * tangent * dynamicFriction;
			}

			// Apply
			//A.motionState.velocity -= (aInverseMass)*frictionImpulse;
			//B.motionState.velocity += (bInverseMass)*frictionImpulse;

			//A.motionState.applyImpulseWorldPosition(-frictionImpulse, contactPoint);
			//B.motionState.applyImpulseWorldPosition( frictionImpulse, contactPoint);


		};

		auto impulseResolution = [&](Body &A, Body &B, glm::vec2 normal,
			float velAlongNormal, float penetrationDepth, glm::vec2 contactPoint)
		{

			//calculate elasticity
			float e = std::min(A.elasticity, B.elasticity);

			float massInverseA = 1.f / A.motionState.mass;
			float massInverseB = 1.f / B.motionState.mass;
			if (A.motionState.mass == 0 || A.motionState.mass == INFINITY) { massInverseA = 0; }
			if (B.motionState.mass == 0 || B.motionState.mass == INFINITY) { massInverseB = 0; }

			float momentOfInertiaInverseA = 1.f / A.motionState.momentOfInertia;
			float momentOfInertiaInverseB = 1.f / B.motionState.momentOfInertia;
			if (A.motionState.momentOfInertia == 0 || A.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseA = 0; }
			if (B.motionState.momentOfInertia == 0 || B.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseB = 0; }


			glm::vec2 rContactA = contactPoint - A.motionState.pos;
			glm::vec2 rContactB = contactPoint - B.motionState.pos;

			
			//float inertiaDivisorA = 0;
			//float inertiaDivisorB = 0;

			float inertiaDivisorA = std::pow(cross(rContactA, normal), 2) * momentOfInertiaInverseA;
			float inertiaDivisorB = std::pow(cross(rContactB, normal), 2) * momentOfInertiaInverseB;

			if (massInverseA == 0 && massInverseB == 0
				&& inertiaDivisorA == 0 && inertiaDivisorB == 0
				)
			{
				//nothing will move, no need to compute anything
				return;
			}

			//remove moment of inertia
			inertiaDivisorA = 0;
			inertiaDivisorB = 0;

			// Calculate impulse scalar
			float j = -(1.f + e) * velAlongNormal;
			j /= (massInverseA + massInverseB + 
				inertiaDivisorA +
				inertiaDivisorB);

			// Apply impulse
			glm::vec2 impulse = j * normal;
			//A.motionState.velocity -= massInverseA * impulse;
			//B.motionState.velocity += massInverseB * impulse;

			A.motionState.applyImpulseWorldPosition(-impulse, contactPoint);
			B.motionState.applyImpulseWorldPosition( impulse, contactPoint);

			{

				// Re-calculate relative velocity after normal impulse
				// is applied (impulse from first article, this code comes
				// directly thereafter in the same resolve function)

				glm::vec2 rv = B.motionState.velocity - A.motionState.velocity;

				// Solve for the tangent vector
				glm::vec2 tangent = rv - glm::dot(rv, normal) * normal;

				float rangentSize = glm::length(tangent);
				
				if (rangentSize > 0.001)
				{
					tangent /= rangentSize;

					applyFriction(A, B, tangent, rv, massInverseA, massInverseB, j,
						rContactA, rContactB, contactPoint);
				}

			}


		};

		size_t bodiesSize = bodies.size();
		for (int i = 0; i < bodiesSize; i++)
		{

			//applyDrag(bodies[i].motionState);

			//detect colisions
			for(int _ = 0; _ < collisionChecksCount; _++)
			for (int j = 0; j < bodiesSize; j++)
			{
				//break;
				if (i == j) { continue; }

				auto &A = bodies[i];
				auto &B = bodies[j];

				glm::vec2 normal = {};
				glm::vec2 contactPoint = {};
				float penetration = 0;

				if (BodyvsBody(A, B, penetration, normal, contactPoint))
				{
					glm::vec2 relativeVelocity = B.motionState.velocity -
						A.motionState.velocity;
					float velAlongNormal = glm::dot(relativeVelocity, normal);

					// Do not resolve if velocities are separating
					if (velAlongNormal > 0)
					{

					}
					else
					{
						impulseResolution(A, B, normal, velAlongNormal, penetration, contactPoint);
					}

					if (_ == collisionChecksCount - 1
						)
					{
						positionalCorrection(A, B, normal, penetration);
					}
				}

			}

			integrateForces(bodies[i].motionState, deltaTime);
			bodies[i].motionState.lastPos = bodies[i].motionState.pos;
		}

	};

}


void ph2d::PhysicsEngine::addBody(glm::vec2 centerPos, Collider collider)
{

	Body body;
	body.motionState.setPos(centerPos);
	body.collider = collider;
	body.motionState.mass = collider.computeMass();
	body.motionState.momentOfInertia = collider.computeMomentOfInertia(body.motionState.mass);

	bodies.push_back(body);

}


float ph2d::vectorToRotation(const glm::vec2 &vector)
{
	// Handle zero vector by defaulting to 0 radians (pointing up)
	if (glm::length(vector) == 0.0f)
	{
		return 0.0f;
	}
	
	// Calculate angle using atan2
	float angle = atan2(-vector.x, vector.y);

	// Adjust to ensure 0 radians corresponds to the up vector (0, 1)
	return angle;
}

glm::vec2 ph2d::rotationToVector(float rotation)
{
	// Calculate direction vector using sine and cosine
	float x = sin(rotation);
	float y = cos(rotation);
	return glm::normalize(glm::vec2(-x, y));
}

void ph2d::PhysicsEngine::addHalfSpaceStaticObject(glm::vec2 position, glm::vec2 normal)
{
	Body body;
	body.motionState.setPos(position);
	body.collider.type = ColliderType::ColliderHalfSpace;
	body.motionState.mass = 0;
	body.motionState.momentOfInertia = 0;

	body.motionState.rotation = vectorToRotation(normal);
	bodies.push_back(body);

}


ph2d::AABB ph2d::Body::getAABB()
{

	switch (collider.type)
	{

	case ColliderCircle:
	{
		glm::vec4 rez;
		rez.x = motionState.pos.x;
		rez.y = motionState.pos.y;

		rez.x -= collider.collider.circle.radius;
		rez.y -= collider.collider.circle.radius;

		rez.z = collider.collider.circle.radius * 2.f;
		rez.w = collider.collider.circle.radius * 2.f;

		return rez;
	};
	break;

	case ColliderBox:
	{
		glm::vec4 rez;
		rez.x = motionState.pos.x;
		rez.y = motionState.pos.y;

		rez.x -= collider.collider.box.size.x / 2.f;
		rez.y -= collider.collider.box.size.y / 2.f;

		rez.z = collider.collider.box.size.x;
		rez.w = collider.collider.box.size.y;

		return rez;
	}
	break;

	}

	return {};

}

bool ph2d::Body::intersectPoint(glm::vec2 p, float delta)
{
	switch (collider.type)
	{

	case ColliderCircle:
	{
		return CircleVsPoint(motionState.pos, collider.collider.circle.radius, p, delta);
	};
	break;

	case ColliderBox:
	{
		return OBBvsPoint(getAABB(), motionState.rotation, p, delta);
	}
	break;

	}

	return {};
}

