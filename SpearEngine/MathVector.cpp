#include "MathVector.h"
#include <cmath>

Vector2D::Vector2D(float inX, float inY)
	: x{inX}, y{inY}
{}

Vector2D Vector2D::operator*(const float& scalar) const
{
	return Vector2D(x * scalar, y * scalar);
}
Vector2D operator*(double scalar, const Vector2D& vec)
{
	return (vec * scalar);
}

Vector2D Vector2D::operator/(const float& scalar) const
{
	return Vector2D(x / scalar, y / scalar);
}

Vector2D Vector2D::operator+(const Vector2D& other) const
{
	return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator-(const Vector2D& other) const
{
	return Vector2D(x - other.x, y - other.y);
}

float Vector2D::Angle() const
{
	return atan2(x, y);
}

float Vector2D::Length() const
{
	return sqrt((x * x) + (y * y));
}

// faster for comparisons
float Vector2D::LengthSqr() const
{
	return (x * x) + (y * y);
}

Vector2D Vector2D::Normal() const
{
	return Vector2D(-y, x);
}

Vector2D Vector2D::Rotate(float angle) const
{
	return Vector2D(x * cos(angle) - y * sin(angle),
					y * cos(angle) + x * sin(angle));
}

float Dot(const Vector2D& a, const Vector2D& b)
{
	return (a.x * b.x) + (a.y * b.y);
}

float Cross(const Vector2D& a, const Vector2D& b)
{
	return (a.x * b.y) - (a.y * b.x);
}

Vector2D Normalize(const Vector2D& vec)
{
	return vec / vec.Length();
}

Vector2D Projection(const Vector2D& vecToProject, const Vector2D& vecTarget)
{
	return Normalize(vecTarget) * Dot(vecToProject, Normalize(vecTarget));
}

Vector2D Reflection(const Vector2D& vecToReflect, const Vector2D& vecSurface)
{
	return (vecToReflect - (2.f * Projection(vecToReflect, vecSurface.Normal())));
}

bool VectorIntersection(const Vector2D& posA, const Vector2D& vecA, const Vector2D& posB, const Vector2D& vecB, Vector2D& outIntersection)
{
	float abCross{Cross(vecA, vecB)};
	if (abCross == 0.f)
	{
		return false;
	}

	// calc scalars equal to intersection point on each line
	float intersectScalarA{ Cross(posB - posA, vecB) / abCross };
	float intersectScalarB{ Cross(posB - posA, vecA) / abCross };

	// output intersection position
	outIntersection = posA + (vecA * intersectScalarA);

	// if both scalars are within length of original vectors, return true
	return (   intersectScalarA >= 0 && intersectScalarA <= 1
			&& intersectScalarB >= 0 && intersectScalarB <= 1);
}