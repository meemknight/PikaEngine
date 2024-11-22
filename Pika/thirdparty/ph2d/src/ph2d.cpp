#include "ph2d/ph2d.h"
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
#include <algorithm>


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


	LineEquation Body::getLineEquationForHalfPlaneColliders()
	{
		if (collider.type != ColliderHalfSpace) { return {}; }

		LineEquation lineEquation;
		lineEquation.createFromRotationAndPoint(motionState.rotation,
			motionState.pos);

		return lineEquation;
	}


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
		corners[2] = max();
		corners[1] = {corners[0].x, corners[2].y};
		corners[3] = {corners[2].x, corners[0].y};

		glm::vec2 c = center();

		if (r)
		{
			corners[0] = rotateAroundPoint(corners[0], c, r);
			corners[1] = rotateAroundPoint(corners[1], c, r);
			corners[2] = rotateAroundPoint(corners[2], c, r);
			corners[3] = rotateAroundPoint(corners[3], c, r);
		}

	}

	void ConvexPolygon::getCornersRotated(glm::vec2 corners[PH2D_MAX_CONVEX_SHAPE_POINTS],
		float angle) const
	{
		int c = std::min((unsigned char)vertexCount, (unsigned char)PH2D_MAX_CONVEX_SHAPE_POINTS);
		for (int i = 0; i < c; i++)
		{
			corners[i] = vertexesObjectSpace[i];
			corners[i] = rotateAroundCenter(corners[i], angle);
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


	bool CirclevsConvexPolygon(AABB circle, const ConvexPolygon &convexPolygon,
		glm::vec2 convexPolygonCenter,
		float rotation,
		float &penetration, glm::vec2 &normal, glm::vec2 &contactPoint)
	{
		penetration = 0;
		normal = {0, 0};

		glm::vec2 corners[PH2D_MAX_CONVEX_SHAPE_POINTS] = {};
		convexPolygon.getCornersRotated(corners, rotation);
		int vertexCount = convexPolygon.vertexCount;

		glm::vec2 circleCenter = circle.center();
		float circleRadius = circle.size.x/2.f;
		

		return true;
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


	bool HalfSpaceVsOBB(LineEquation line, AABB bbox, float rotation,
		float &penetration, glm::vec2 &normal, glm::vec2 &contactPoint,
		glm::vec2 &tangentA, glm::vec2 &tangentB)
	{
		glm::vec2 corners[4];
		bbox.getCornersRotated(corners, rotation);

		//todo optimize this wtf
		std::vector<glm::vec2> intersectionPoints;
		line.normalize();
		normal = line.getNormal();


		// Clip edges of the OBB against the half-plane
		for (int i = 0; i < 4; ++i)
		{
			glm::vec2 start = corners[i];
			glm::vec2 end = corners[(i + 1) % 4];

			float startDist = line.computeEquation(start);
			float endDist = line.computeEquation(end);

			// Include points inside the half-plane
			if (startDist >= 0) intersectionPoints.push_back(start);

			// Check if the edge intersects the plane
			if ((startDist >= 0 && endDist < 0) || (startDist < 0 && endDist >= 0))
			{
				float t = startDist / (startDist - endDist); // Interpolation factor
				glm::vec2 intersection = start + t * (end - start);
				intersectionPoints.push_back(intersection);
			}
		}


		// No intersection if there are no points
		if (intersectionPoints.empty())
			return false;

		//used to calculate the tangent along the obb
		glm::vec2 bestEdgeStart = {};
		glm::vec2 bestEdgeEnd = {};
		tangentB = {};

		if (intersectionPoints.size() == 1)
		{

		}
		if (intersectionPoints.size() == 2)
		{
			bestEdgeStart = intersectionPoints[0];
			bestEdgeEnd = intersectionPoints[1];
			tangentB = glm::normalize(bestEdgeEnd - bestEdgeStart);
		}
		else if (intersectionPoints.size() == 3)
		{
			//the OBB is entering the plane and forms a triangle, use the biggest edge
			//we first determine the "pointy" end of the tirangle that enters the triangle
			
			//determine deepest center point
			float biggestPenetration = -1000000000;
			int centerPoint = 0;
			for (int i = 0; i < 3; i++)
			{
				float dist = line.computeEquation(intersectionPoints[i]);
				if (dist > biggestPenetration)
				{
					biggestPenetration = dist;
					centerPoint = i;
				}
			}

			glm::vec2 first = intersectionPoints[centerPoint] - intersectionPoints[(centerPoint +1) % 3];
			glm::vec2 second = intersectionPoints[centerPoint] - intersectionPoints[(centerPoint + 2) %3];
			
			float firstL = glm::length(first);
			float secondL = glm::length(second);

			if (firstL > secondL)
			{
				if(firstL)
					tangentB = first / firstL;
			}
			else
			{
				if(secondL)
					tangentB = second / secondL;
			}
		}
		else
		{
			float biggestPenetration = -1000000000;
			for (int i = 0; i < intersectionPoints.size(); i++)
			{
				glm::vec2 start = intersectionPoints[i];
				glm::vec2 end = intersectionPoints[(i + 1) % 4];

				float startDist = line.computeEquation(start);
				float endDist = line.computeEquation(end);

				if (startDist + endDist > biggestPenetration)
				{
					biggestPenetration = startDist + endDist;
					bestEdgeStart = start;
					bestEdgeEnd = end;
				}
			}
			tangentB = glm::normalize(bestEdgeEnd - bestEdgeStart);
		}



		// Calculate centroid of the intersection polygon
		glm::vec2 centroid = {0.0f, 0.0f};
		for (const auto &point : intersectionPoints)
			centroid += point;
		centroid /= static_cast<float>(intersectionPoints.size());

		contactPoint = centroid;

		// Penetration is the maximum distance of included points from the plane
		penetration = 0.0f;
		for (const auto &point : intersectionPoints)
		{
			float dist = line.computeEquation(point);
			if (dist > penetration)
				penetration = dist;
		}

		tangentA = line.getLineVector();

		return true;
	}

	bool HalfSpaceVsConvexPolygon(LineEquation line, const ConvexPolygon &convexPolygon, 
		glm::vec2 convexPolygonCenter,
		float rotation,
		float &penetration, glm::vec2 &normal, glm::vec2 &contactPoint,
		glm::vec2 &tangentA, glm::vec2 &tangentB)
	{
		line.normalize();
		normal = line.getNormal();

		glm::vec2 corners[PH2D_MAX_CONVEX_SHAPE_POINTS] = {};
		convexPolygon.getCornersRotated(corners, rotation);
		int vertexCount = convexPolygon.vertexCount;

		for (int i = 0; i < vertexCount; i++)
		{
			corners[i] += convexPolygonCenter;
		}

		if (vertexCount < 3) { return 0; }

		std::vector<glm::vec2> intersectionPoints;

		// Clip edges of the OBB against the half-plane
		for (int i = 0; i < vertexCount; ++i)
		{
			glm::vec2 start = corners[i];
			glm::vec2 end = corners[(i + 1) % vertexCount];

			float startDist = line.computeEquation(start);
			float endDist = line.computeEquation(end);

			// Include points inside the half-plane
			if (startDist >= 0) intersectionPoints.push_back(start);

			// Check if the edge intersects the plane
			if ((startDist >= 0 && endDist < 0) || (startDist < 0 && endDist >= 0))
			{
				float t = startDist / (startDist - endDist); // Interpolation factor
				glm::vec2 intersection = start + t * (end - start);
				intersectionPoints.push_back(intersection);
			}
		}

		// No intersection if there are no points
		if (intersectionPoints.empty())
			return false;

		//used to calculate the tangent along the obb
		glm::vec2 bestEdgeStart = {};
		glm::vec2 bestEdgeEnd = {};
		tangentB = {};

		if (intersectionPoints.size() == 1)
		{

		}
		if (intersectionPoints.size() == 2)
		{
			bestEdgeStart = intersectionPoints[0];
			bestEdgeEnd = intersectionPoints[1];
			tangentB = glm::normalize(bestEdgeEnd - bestEdgeStart);
		}
		else if (intersectionPoints.size() == 3)
		{
			//the OBB is entering the plane and forms a triangle, use the biggest edge
			//we first determine the "pointy" end of the tirangle that enters the triangle

			//determine deepest center point
			float biggestPenetration = -1000000000;
			int centerPoint = 0;
			for (int i = 0; i < 3; i++)
			{
				float dist = line.computeEquation(intersectionPoints[i]);
				if (dist > biggestPenetration)
				{
					biggestPenetration = dist;
					centerPoint = i;
				}
			}

			glm::vec2 first = intersectionPoints[centerPoint] - intersectionPoints[(centerPoint + 1) % 3];
			glm::vec2 second = intersectionPoints[centerPoint] - intersectionPoints[(centerPoint + 2) % 3];

			float firstL = glm::length(first);
			float secondL = glm::length(second);

			if (firstL > secondL)
			{
				if (firstL)
					tangentB = first / firstL;
			}
			else
			{
				if (secondL)
					tangentB = second / secondL;
			}
		}
		else
		{
			float biggestPenetration = -1000000000;
			for (int i = 0; i < intersectionPoints.size(); i++)
			{
				glm::vec2 start = intersectionPoints[i];
				glm::vec2 end = intersectionPoints[(i + 1) % 4];

				float startDist = line.computeEquation(start);
				float endDist = line.computeEquation(end);

				if (startDist + endDist > biggestPenetration)
				{
					biggestPenetration = startDist + endDist;
					bestEdgeStart = start;
					bestEdgeEnd = end;
				}
			}
			tangentB = glm::normalize(bestEdgeEnd - bestEdgeStart);
		}

		glm::vec2 centroid = {0.0f, 0.0f};
		for (const auto &point : intersectionPoints)
			centroid += point;
		centroid /= static_cast<float>(intersectionPoints.size());

		contactPoint = centroid;

		// Penetration is the maximum distance of included points from the plane
		penetration = 0.0f;
		for (const auto &point : intersectionPoints)
		{
			float dist = line.computeEquation(point);
			if (dist > penetration)
				penetration = dist;
		}
		
		tangentA = line.getLineVector();

		return true;
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

	glm::vec2 findClosestEdge(const glm::vec2 *corners, int count, glm::vec2 point)
	{
		float minDist = FLT_MAX;
		glm::vec2 closestEdge = {0, 0};

		for (int i = 0; i < count; ++i)
		{
			glm::vec2 edgeStart = corners[i];
			glm::vec2 edgeEnd = corners[(i + 1) % count];
			glm::vec2 edge = edgeEnd - edgeStart;
			glm::vec2 edgeNormal = glm::normalize(glm::vec2(-edge.y, edge.x));

			float dist = glm::abs(glm::dot(point - edgeStart, edgeNormal));
			if (dist < minDist)
			{
				minDist = dist;
				closestEdge = edge;
			}
		}

		return glm::normalize(closestEdge);
	}


	void clipPolygon(const glm::vec2 *cornersA, int countA, const glm::vec2 &normal,
		const glm::vec2 *cornersB, int countB, std::vector<glm::vec2> &intersectionPoints)
	{
		std::vector<glm::vec2> inputPolygon(cornersA, cornersA + countA);

		for (int i = 0; i < countB; ++i)
		{
			glm::vec2 edgeStart = cornersB[i];
			glm::vec2 edgeEnd = cornersB[(i + 1) % countB];
			glm::vec2 edgeNormal = glm::normalize(glm::vec2(edgeEnd.y - edgeStart.y, edgeStart.x - edgeEnd.x));

			std::vector<glm::vec2> outputPolygon;

			for (size_t j = 0; j < inputPolygon.size(); ++j)
			{
				glm::vec2 current = inputPolygon[j];
				glm::vec2 next = inputPolygon[(j + 1) % inputPolygon.size()];

				float currentDist = glm::dot(current - edgeStart, edgeNormal);
				float nextDist = glm::dot(next - edgeStart, edgeNormal);

				if (currentDist >= 0) outputPolygon.push_back(current);

				if (currentDist * nextDist < 0)
				{
					float t = currentDist / (currentDist - nextDist);
					glm::vec2 intersection = current + t * (next - current);
					outputPolygon.push_back(intersection);
				}
			}

			inputPolygon = outputPolygon;
			if (inputPolygon.empty()) break;
		}

		intersectionPoints = inputPolygon;
	}


	bool OBBvsOBB(AABB a, float ar, AABB b, float br,
		float &penetration, glm::vec2 &normal, glm::vec2 &contactPoint, 
		glm::vec2 &tangentA, glm::vec2 &tangentB)
	{
		penetration = 0;
		normal = {0, 0};

		glm::vec2 cornersA[4] = {};
		glm::vec2 cornersB[4] = {};
		a.getCornersRotated(cornersA, ar);
		b.getCornersRotated(cornersB, br);

		//std::swap(cornersA[1], cornersA[2]);
		//std::swap(cornersB[1], cornersB[2]);

		glm::vec2 axes[4] = {
			rotateAroundCenter({0, 1}, ar), // Normal of A's first edge
			rotateAroundCenter({1, 0}, ar), // Normal of A's second edge
			rotateAroundCenter({0, 1}, br), // Normal of B's first edge
			rotateAroundCenter({1, 0}, br)  // Normal of B's second edge
		};

		// SAT collision test
		float minPenetration = FLT_MAX;
		glm::vec2 minPenetrationAxis = {};
		bool flipSign = false;

		for (int i = 0; i < 4; ++i)
		{
			bool flip = false;
			float penetrationDepth = calculatePenetrationAlongOneAxe(
				cornersA, 4, cornersB, 4, axes[i], &flip);

			if (penetrationDepth < 0.0f)
			{
				return false; // No collision
			}

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



		// Clip the polygons to find the intersection
		std::vector<glm::vec2> intersectionPoints;
		clipPolygon(cornersA, 4, normal, cornersB, 4, intersectionPoints);

		if (intersectionPoints.empty())
		{
			return false; // No intersection points
		}

		// Compute centroid of the intersection polygon
		contactPoint = glm::vec2(0);
		for (const auto &point : intersectionPoints)
		{
			contactPoint += point;
		}
		contactPoint /= static_cast<float>(intersectionPoints.size());

		// Calculate tangents based on the closest edges
		tangentA = findClosestEdge(cornersA, 4, contactPoint);
		tangentB = findClosestEdge(cornersB, 4, contactPoint);

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
		glm::vec2 &normal, glm::vec2 &contactPoint, glm::vec2 &tangentA, glm::vec2 &tangentB, bool *reverseOrder)
	{
		tangentA = {};
		tangentB = {};
		if (reverseOrder) { *reverseOrder = false; }

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
				penetration, normal, contactPoint, tangentA, tangentB);

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

			bool rez = ph2d::OBBvsCircle(
				bbox, B.motionState.rotation, abox, penetration, normal, contactPoint);
			normal = -normal;
			return rez;

		}


		else if (A.collider.type == ph2d::ColliderHalfSpace &&
			B.collider.type == ph2d::ColliderCircle)
		{

			auto bbox = B.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(A.motionState.rotation,
				A.motionState.pos);

			bool rez = ph2d::HalfSpaceVSCircle(
				lineEquation, bbox, penetration, normal, contactPoint);
			normal = -normal;
			return rez;
		}
		else if (A.collider.type == ph2d::ColliderCircle &&
			B.collider.type == ph2d::ColliderHalfSpace)
		{

			auto abox = A.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(B.motionState.rotation,
				B.motionState.pos);
			
			if (reverseOrder) { *reverseOrder = true; }

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

			bool rez = ph2d::HalfSpaceVsOBB(
				lineEquation, bbox, B.motionState.rotation,
				penetration, normal, contactPoint, tangentA, tangentB);
			normal = -normal;
			return rez;
		}
		else if (A.collider.type == ph2d::ColliderBox &&
			B.collider.type == ph2d::ColliderHalfSpace)
		{
			auto abox = A.getAABB();

			LineEquation lineEquation;
			lineEquation.createFromRotationAndPoint(B.motionState.rotation,
				B.motionState.pos);

			if (reverseOrder) { *reverseOrder = true; }
			return ph2d::HalfSpaceVsOBB(
				lineEquation, abox, A.motionState.rotation, penetration, normal, contactPoint,
			tangentB, tangentA);

		}


		else if (A.collider.type == ph2d::ColliderHalfSpace &&
			B.collider.type == ph2d::ColliderConvexPolygon)
			{

				LineEquation lineEquation;
				lineEquation.createFromRotationAndPoint(A.motionState.rotation,
					A.motionState.pos);

				bool rez = ph2d::HalfSpaceVsConvexPolygon(
					lineEquation, B.collider.collider.convexPolygon, B.motionState.pos, B.motionState.rotation,
					penetration, normal, contactPoint, tangentA, tangentB);
				normal = -normal;
				return rez;
				}
		else if (A.collider.type == ph2d::ColliderConvexPolygon &&
			B.collider.type == ph2d::ColliderHalfSpace)
			{

				LineEquation lineEquation;
				lineEquation.createFromRotationAndPoint(B.motionState.rotation,
					B.motionState.pos);

				return ph2d::HalfSpaceVsConvexPolygon(
					lineEquation, A.collider.collider.convexPolygon, A.motionState.pos, A.motionState.rotation, penetration, normal, contactPoint,
					tangentB, tangentA);
			}

		else if (A.collider.type == ph2d::ColliderCircle
			&& B.collider.type == ph2d::ColliderConvexPolygon
			)
			{
				return CirclevsConvexPolygon(A.getAABB(), B.collider.collider.convexPolygon,
					B.motionState.pos, B.motionState.rotation, penetration, normal, contactPoint);
			}
		else if (A.collider.type == ph2d::ColliderConvexPolygon
			&& B.collider.type == ph2d::ColliderCircle
			)
			{
				bool rez = CirclevsConvexPolygon(B.getAABB(), A.collider.collider.convexPolygon,
					A.motionState.pos, A.motionState.rotation, penetration, normal, contactPoint);
				normal = -normal;
				return rez;
			}


		return 0;
	}

	//todo make sure drag can't be stronger than speed!
	void applyDrag(MotionState &motionState, float scale, SimulationPhysicsSettings &s)
	{
		glm::vec2 dragForce = scale * s.airDragCoeficient * -motionState.velocity * glm::abs(motionState.velocity) / 2.f;

		float length = glm::length(dragForce);
		if (length)
		{
			if (length > s.maxAirDrag)
			{
				dragForce /= length;
				dragForce *= s.maxAirDrag;
			}
		
			motionState.acceleration += dragForce;
		}
	}

	//todo delta time
	void applyAngularDrag(MotionState &motionState, float scale,
		SimulationPhysicsSettings &s)
	{
		float dragForce = scale * s.rotationalDragCoeficient * -motionState.angularVelocity;
		if (dragForce > s.maxAngularDrag)
		{
			dragForce = s.maxAngularDrag;
		}
		if (dragForce < -s.maxAngularDrag)
		{
			dragForce = -s.maxAngularDrag;
		}
		motionState.angularVelocity += dragForce;
	}

	void integrateForces(MotionState &motionState, float deltaTime,
		SimulationPhysicsSettings &s, BodyFlags flags)
	{

		bool frezeX = flags.isFreezeX();
		bool frezeY = flags.isFreezeY();

		if (motionState.mass == 0 || motionState.mass == INFINITY
			|| (frezeX && frezeY)
			)
		{
			motionState.acceleration = {};
			motionState.velocity = {};
		}
		else
		{

			//if (glm::length(motionState.velocity) < MIN_VELOCITY)
			//{
			//	motionState.velocity = glm::vec2(0.0f);
			//}
			//
			//if (std::abs(motionState.angularVelocity) < MIN_ANGULAR_VELOCITY)
			//{
			//	motionState.angularVelocity = 0.0f;
			//}

			//linear motion
			motionState.acceleration = glm::clamp(motionState.acceleration,
				glm::vec2(-s.maxAcceleration), glm::vec2(s.maxAcceleration));

			//Symplectic Euler
			motionState.velocity += motionState.acceleration * deltaTime * 0.5f;
			motionState.velocity = glm::clamp(motionState.velocity, glm::vec2(-s.maxVelocity), glm::vec2(s.maxVelocity));

			glm::vec2 toAdd = motionState.velocity * deltaTime;

			if (frezeX) { toAdd.x = 0; motionState.velocity.x = 0; }
			if (frezeY) { toAdd.y = 0; motionState.velocity.y = 0; }

			motionState.pos += toAdd;

			motionState.velocity += motionState.acceleration * deltaTime * 0.5f;
			motionState.velocity = glm::clamp(motionState.velocity, glm::vec2(-s.maxVelocity), glm::vec2(s.maxVelocity));

			if (std::fabs(motionState.velocity.x) < 0.00001) { motionState.velocity.x = 0; }
			if (std::fabs(motionState.velocity.y) < 0.00001) { motionState.velocity.y = 0; }

			motionState.acceleration = {};
		}

		if (motionState.momentOfInertia == 0 || motionState.momentOfInertia == INFINITY
			|| flags.isFreezeRotation())
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

	float cross(glm::vec2 a, glm::vec2 b)
	{
		return a.x * b.y - a.y * b.x;
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

	Collider createConvexPolygonCollider(glm::vec2 *shape, unsigned char count)
	{
		Collider c;
		c.type = ColliderConvexPolygon;

		int countMax = std::min(count, (unsigned char)PH2D_MAX_CONVEX_SHAPE_POINTS);

		memcpy(c.collider.convexPolygon.vertexesObjectSpace, shape, countMax * sizeof(glm::vec2));
		c.collider.convexPolygon.vertexCount = count;

		return c;
	}

	float Collider::computeMass()
	{
		switch (type)
		{
			case ColliderCircle:
			return (collider.circle.radius * collider.circle.radius * 3.1415926);
			
			case ColliderBox:
			return collider.box.size.x * collider.box.size.y;

			case ColliderConvexPolygon:
			{
				float mass = 0;
				unsigned char c = collider.convexPolygon.vertexCount;

				for (int i = 0; i < c; i++)
				{
					glm::vec2 a = collider.convexPolygon.vertexesObjectSpace[i];
					glm::vec2 b = collider.convexPolygon.vertexesObjectSpace[(i + 1) % c];
					mass += cross(a, b) / 2.f;
				}
			}
			break;

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

		case ColliderConvexPolygon:
		{
			float momentOfInertia = 0;
			unsigned char c = collider.convexPolygon.vertexCount;

			for (int i = 0; i < c; i++)
			{
				glm::vec2 a = collider.convexPolygon.vertexesObjectSpace[i];
				glm::vec2 b = collider.convexPolygon.vertexesObjectSpace[(i + 1) % c];
				float crossProduct = cross(a, b);

				// Sum the contributions from the triangle
				momentOfInertia += (glm::dot(a, a) + glm::dot(a, b) + glm::dot(b, b)) * 1;

			}

			return momentOfInertia * mass / 6.f;
		}
		break;

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




void ph2d::MotionState::applyImpulseObjectPosition(glm::vec2 impulse, glm::vec2 contactVector, BodyFlags flags)
{


	if (mass != 0 && mass != INFINITY )
	{
		glm::vec2 toAdd = (1.0f / mass) * impulse;
		if (flags.isFreezeX()) { toAdd.x = 0; }
		if (flags.isFreezeY()) { toAdd.y = 0; }

		velocity += toAdd;
	}

	if (momentOfInertia != 0 && momentOfInertia != INFINITY && !flags.isFreezeRotation())
	{
		angularVelocity -= (1.0f / momentOfInertia) * cross(contactVector, impulse) * 0.95f;
	}
}

void ph2d::MotionState::applyImpulseWorldPosition(glm::vec2 impulse, glm::vec2 contactVectorWorldPos, BodyFlags flags)
{
	glm::vec2 relVector = contactVectorWorldPos - pos;
	applyImpulseObjectPosition(impulse, relVector, flags);
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
			bool dontMoveA = A.flags.isFreezeX() || A.flags.isFreezeY();
			bool dontMoveB = B.flags.isFreezeX() || B.flags.isFreezeY();

			float massInverseA = 1.f / A.motionState.mass;
			float massInverseB = 1.f / B.motionState.mass;

			if (dontMoveA) { massInverseA = 0; }
			if (dontMoveB) { massInverseB = 0; }

			if (dontMoveA == 0 && massInverseB == 0) { return; }

			if (A.motionState.mass == 0 || A.motionState.mass == INFINITY) { massInverseA = 0; }
			if (B.motionState.mass == 0 || B.motionState.mass == INFINITY) { massInverseB = 0; }

			const float percent = 0.20; // usually 20% to 80%
			const float slop = 0.01; // usually 0.01 to 0.1 

			glm::vec2 correction = (glm::max(penetrationDepth - slop, 0.0f) / (massInverseA + massInverseB)) * percent * n;

			A.motionState.pos -= massInverseA * correction;
			B.motionState.pos += massInverseB * correction;
		};

		auto rotationalCorrection = [&](Body &A, Body &B,
			glm::vec2 tangentA, glm::vec2 tangentB)
		{

			if (tangentA.x == 0 && tangentA.y == 0) { return; }
			if (tangentB.x == 0 && tangentB.y == 0) { return; }
			bool dontRotateA = A.flags.isFreezeRotation();
			bool dontRotateB = B.flags.isFreezeRotation();

			float inertiaInverseA = 1.f / A.motionState.momentOfInertia;
			float inertiaInverseB = 1.f / B.motionState.momentOfInertia;

			if (A.motionState.momentOfInertia == 0 || A.motionState.momentOfInertia == INFINITY) { inertiaInverseA = 0; }
			if (B.motionState.momentOfInertia == 0 || B.motionState.momentOfInertia == INFINITY) { inertiaInverseB = 0; }
			if (dontRotateA) { inertiaInverseA = 0; }
			if (dontRotateB) { inertiaInverseB = 0; }

			if (inertiaInverseA == 0 && inertiaInverseB == 0) { return; }

			const float PI = 3.1415926;

			const float percent = 0.17; //
			const float slop = 0.00; // usually 0.01 to 0.1 
			const float tresshold = glm::radians(5.f);

			int flipped = 1;

			if (glm::dot(tangentA, tangentB) < 0)
			{
				tangentB = -tangentB;
				//flipped = -1;
			}

			// Calculate the angle between the tangents
			float angleA = atan2(tangentA.y, tangentA.x);
			float angleB = atan2(tangentB.y, tangentB.x);
			float angleDifference = angleB - angleA;

			// Normalize the angle difference to [-pi, pi]
			if (angleDifference > PI) angleDifference -= 2 * PI;
			if (angleDifference < -PI) angleDifference += 2 * PI;

			// Apply correction only if the angle is not too big
			if (fabs(angleDifference) < tresshold && fabs(angleDifference) > slop)
			{
				// Correction angle scaled by the inertia and correction percent
				float correction = percent * angleDifference / (inertiaInverseA + inertiaInverseB);

				// Rotate object A towards B
				if (inertiaInverseA > 0)
				{
					//if (std::abs(A.motionState.angularVelocity) < 10)
					{
						A.motionState.rotation -= correction * inertiaInverseA * flipped;
					}
				}

				// Rotate object B towards A
				if (inertiaInverseB > 0)
				{
					//if (std::abs(A.motionState.angularVelocity) < 10)
					{
						B.motionState.rotation += correction * inertiaInverseB * flipped;
					}
				}
			}

			//glm::vec2 correction = (glm::max(penetrationDepth - slop, 0.0f) / (intertiaInverseA + intertiaInverseB)) * percent * n;
			//A.motionState.pos -= massInverseA * correction;
			//B.motionState.pos += massInverseB * correction;
		};

		auto applyFriction = [&](Body &A, Body &B, glm::vec2 tangent, glm::vec2 rv,
			float aInverseMass, float bInverseMass, float j, glm::vec2 rContactA,
			glm::vec2 rContactB, glm::vec2 contactPoint)
		{
			
			float momentOfInertiaInverseA = 1.f / A.motionState.momentOfInertia;
			float momentOfInertiaInverseB = 1.f / B.motionState.momentOfInertia;
			if (A.motionState.momentOfInertia == 0 || A.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseA = 0; }
			if (B.motionState.momentOfInertia == 0 || B.motionState.momentOfInertia == INFINITY) { momentOfInertiaInverseB = 0; }

			float jt = -glm::dot(rv, tangent);

			float inertiaDivisorA = std::pow(cross(rContactA, tangent), 2) * momentOfInertiaInverseA;
			float inertiaDivisorB = std::pow(cross(rContactB, tangent), 2) * momentOfInertiaInverseB;

			if (aInverseMass == 0 && bInverseMass == 0
				&& inertiaDivisorA == 0 && inertiaDivisorB == 0
				)
			{
				//nothing will move, no need to compute anything
				return;
			}

			//remove moment of inertia
			//momentOfInertiaInverseA = 0;
			//momentOfInertiaInverseB = 0;

			// Solve for magnitude to apply along the friction vector
			jt = jt / (aInverseMass + bInverseMass + inertiaDivisorA + inertiaDivisorB);

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

			A.motionState.applyImpulseWorldPosition(-frictionImpulse, contactPoint, A.flags);
			B.motionState.applyImpulseWorldPosition( frictionImpulse, contactPoint, B.flags);


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
			//inertiaDivisorA = 0;
			//inertiaDivisorB = 0;

			// Calculate impulse scalar
			float j = -(1.f + e) * velAlongNormal;
			j /= (massInverseA + massInverseB + 
				inertiaDivisorA +
				inertiaDivisorB);

			// Apply impulse
			glm::vec2 impulse = j * normal;
			//A.motionState.velocity -= massInverseA * impulse;
			//B.motionState.velocity += massInverseB * impulse;

			A.motionState.applyImpulseWorldPosition(-impulse, contactPoint, A.flags);
			B.motionState.applyImpulseWorldPosition( impulse, contactPoint, B.flags);

			{

				// Re-calculate relative velocity after normal impulse
				// is applied (impulse from first article, this code comes
				// directly thereafter in the same resolve function)
				// rv also holds the rotationalrotational velocity contribution
				glm::vec2 rv = (B.motionState.velocity + cross(-B.motionState.angularVelocity, rContactB)) -
					(A.motionState.velocity + cross(-A.motionState.angularVelocity, rContactA));

				// Solve for the tangent vector
				glm::vec2 tangent = (rv - glm::dot(rv, normal) * normal);

				float tangentSize = glm::length(tangent);
				
				if (tangentSize > 0.001)
				{
					tangent /= tangentSize;

					applyFriction(A, B, tangent, rv, massInverseA, massInverseB, j,
						rContactA, rContactB, contactPoint);
				}

			}


		};


		for (auto &it : bodies)
		{
			auto &b = it.second;

			//gravity
			//no need to check for freeze position on x y because I clear them later anyway.
			if (b.motionState.mass != 0 && b.motionState.mass != INFINITY
				&& !b.flags.isFreezePosition()
				)
			{
				b.motionState.acceleration += simulationphysicsSettings.gravity
					* b.gravityScale;
			}

			//make sure all objects use a valid mass
			if (b.motionState.mass <= 0)
			{
				b.motionState.mass = INFINITY;
			}

			if (b.motionState.momentOfInertia <= 0)
			{
				b.motionState.momentOfInertia = INFINITY;
			}

			applyDrag(b.motionState, b.dragScale, simulationphysicsSettings);
			applyAngularDrag(b.motionState, b.angularDragScale, simulationphysicsSettings);

		}


		for (int _ = 0; _ < collisionChecksCount; _++)
		{
			intersections.clear();

			for (auto &it1 : bodies)
			{
				//detect colisions
				for (auto &it2 : bodies)
				{
					//break;
					if (it1.first == it2.first) { break; }

					auto &A = it1.second;
					auto &B = it2.second;

					glm::vec2 normal = {};
					glm::vec2 contactPoint = {};
					float penetration = 0;

					glm::vec2 tangentA = {};
					glm::vec2 tangentB = {};

					bool reverseOrder = 0;

					if (BodyvsBody(A, B, penetration, normal, contactPoint, 
						tangentA, tangentB, &reverseOrder))
					{
						ManifoldIntersection intersection = {};

						if (reverseOrder)
						{
							intersection.A = it2.first;
							intersection.B = it1.first;
							intersection.tangentA = tangentB;
							intersection.tangentB = tangentA;
							intersection.massA = B.motionState.mass;
							intersection.massB = A.motionState.mass;
							intersection.normal = -normal;
						}
						else
						{
							intersection.A = it1.first;
							intersection.B = it2.first;
							intersection.tangentA = tangentA;
							intersection.tangentB = tangentB;
							intersection.massA = A.motionState.mass;
							intersection.massB = B.motionState.mass;
							intersection.normal = normal;
						}

						intersection.contactPoint = contactPoint;
						intersection.penetration = penetration;

						intersections.push_back(intersection);
					}

				}

			}

			std::sort(intersections.begin(), intersections.end(), [&](auto &a, auto &b)
			{
				// Retrieve the mass of the objects from the indices A and B
				float massA = a.massA;
				float massB = a.massB;

				// Sort first by penetration depth (deeper penetration first)
				if (a.penetration != b.penetration)
					return a.penetration > b.penetration;  // Sort by deeper penetration first

				//TODO this aint right!
				// If penetrations are equal, sort by mass (lighter mass first)
				return massA < massB;  // Lighter mass first (this avoids smaller objects being "swallowed")
			});

			for (auto &m : intersections)
			{
				auto &A = bodies[m.A];
				auto &B = bodies[m.B];

				glm::vec2 relativeVelocity = B.motionState.velocity -
					A.motionState.velocity;
				float velAlongNormal = glm::dot(relativeVelocity, m.normal);

				// Do not resolve if velocities are separating
				if (velAlongNormal > 0)
				{

				}
				else
				{
					impulseResolution(A, B, m.normal, velAlongNormal, m.penetration, m.contactPoint);
				}

				if (_ == collisionChecksCount - 1 || velAlongNormal <= 0)
				{
					positionalCorrection(A, B, m.normal, m.penetration);
					rotationalCorrection(A, B, m.tangentA, m.tangentB);
				}
			}

		}
		
		for (auto &it : bodies)
		{
			auto &b = it.second;

			integrateForces(b.motionState, deltaTime, simulationphysicsSettings, 
				b.flags);
			b.motionState.lastPos = b.motionState.pos;
		};
	

	};

}


ph2d::ph2dBodyId ph2d::PhysicsEngine::addBody(glm::vec2 centerPos, Collider collider)
{
	Body body;
	body.motionState.setPos(centerPos);
	body.collider = collider;
	body.motionState.mass = collider.computeMass();
	body.motionState.momentOfInertia = collider.computeMomentOfInertia(body.motionState.mass);

	bodies.emplace(++idCounter, body);
	return idCounter;
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

ph2d::ph2dBodyId ph2d::PhysicsEngine::addHalfSpaceStaticObject(glm::vec2 position, glm::vec2 normal)
{
	Body body;
	body.motionState.setPos(position);
	body.collider.type = ColliderType::ColliderHalfSpace;
	body.motionState.mass = 0;
	body.motionState.momentOfInertia = 0;

	body.motionState.rotation = vectorToRotation(normal);
	bodies.emplace(++idCounter ,body);
	return idCounter;
}


void ph2d::Body::applyImpulseObjectPosition(glm::vec2 impulse, glm::vec2 contactVector)
{
	motionState.applyImpulseObjectPosition(impulse, contactVector, flags);
}

void ph2d::Body::applyImpulseWorldPosition(glm::vec2 impulse, glm::vec2 contactVectorWorldPos)
{
	motionState.applyImpulseWorldPosition(impulse, contactVectorWorldPos, flags);
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

	case ColliderConvexPolygon:
	{
		unsigned char c = collider.collider.convexPolygon.vertexCount;
		auto v = collider.collider.convexPolygon.vertexesObjectSpace;

		if (c == 0) { return {}; }

		glm::vec2 minPos = v[0];
		glm::vec2 maxPos = v[0];

		for (int i = 0; i < c; i++)
		{
			if (v[i].x < minPos.x) { minPos.x = v[i].x; }
			if (v[i].y < minPos.y) { minPos.y = v[i].y; }

			if (v[i].x > maxPos.x) { maxPos.x = v[i].x; }
			if (v[i].y > maxPos.y) { maxPos.y = v[i].y; }
		}

		glm::vec4 rez(minPos + motionState.pos, maxPos - minPos);
		return rez;
	}
	break;

	}

	return {};

}

bool ph2d::Body::isHalfPlane()
{
	return collider.type == ColliderHalfSpace;
}

//https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
bool pointInTriangle(glm::vec2 pt, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3)
{
	auto sign = [](glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	};

	float d1 = sign(pt, v1, v2);
	float d2 = sign(pt, v2, v3);
	float d3 = sign(pt, v3, v1);

	float has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	float has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
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

	case ColliderHalfSpace:
	{
		LineEquation lineEquation;
		lineEquation.createFromRotationAndPoint(motionState.rotation,
			motionState.pos);

		return (lineEquation.computeEquation(p) + delta) >= 0;
	}
	break;

	case ColliderConvexPolygon:
	{
		//todo apply delta
		auto &c = collider.collider.convexPolygon;

		for (int i = 0; i < c.vertexCount; i++)
		{
			glm::vec2 p1 = c.vertexesObjectSpace[i] + motionState.pos;
			glm::vec2 p2 = c.vertexesObjectSpace[(i + 1) % c.vertexCount] + motionState.pos;
			glm::vec2 p3 = motionState.pos;
				
			if (pointInTriangle(p, p1, p2, p3))
			{
				return true;
			}
		}
		return false;
	}
	break;

	}

	return {};
}

