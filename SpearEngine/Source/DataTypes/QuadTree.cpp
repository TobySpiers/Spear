#include "QuadTree.h"

QuadTreeRect::QuadTreeRect(Vector2f originTopLeft, Vector2f extent)
	: min(originTopLeft), max(originTopLeft + extent)
{
}

bool QuadTreeRect::Contains(const Vector2f& point) const
{
	return point.x < max.x && point.x > min.x && point.y < max.y && point.y > min.y;
}

bool QuadTreeRect::Contains(const QuadTreeRect& otherRect) const
{
	return otherRect.max.x < max.x && otherRect.min.x > min.x && otherRect.max.y < max.y && otherRect.min.y > min.y;
}

bool QuadTreeRect::Overlaps(const QuadTreeRect& otherRect) const
{
	return otherRect.max.x > min.x && otherRect.min.x < max.x && otherRect.min.y < max.y && otherRect.max.y > min.y;
}