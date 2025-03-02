#include "MathVector.h"
#include "imgui.h"
#include "Core.h"

// Explicit instantiations
template struct Vector2<int>;
template struct Vector2<float>;
template struct Vector2<double>;
template struct Vector2<int32_t>;
template struct Vector3<int>;
template struct Vector3<float>;
template struct Vector3<double>;

// Imgui helper
template <typename T> 
Vector2<T>::operator ImVec2() const
{
	return ImVec2(static_cast<float>(x), static_cast<float>(y));
}

float Lerp(const float lerpKey, const Vector2f* keyValueArray, int arraySize)
{
	int lutKey{ arraySize - 2 };
	for (int i = 1; i < arraySize; i++)
	{
		if (lerpKey < keyValueArray[i].x)
		{
			lutKey = i - 1;
			break;
		}
	}

    return Lerp(lerpKey, keyValueArray[lutKey], keyValueArray[lutKey + 1]);
}

float Lerp(float lerpKey, const Vector2f& keyValueA, const Vector2f& keyValueB)
{
    const float keyRange = keyValueB.x - keyValueA.x;
    lerpKey -= keyValueA.x;
    float percentage = lerpKey / keyRange;
    const float valueRange = keyValueB.y - keyValueA.y;
    return keyValueA.y + (percentage * valueRange);
}

HashableVector2i::HashableVector2i(const Vector2i& inVal) : Vector2<int32_t>(inVal.x, inVal.y) 
{ 
	ASSERT(inVal.x <= std::numeric_limits<int32_t>::max() && inVal.y >= std::numeric_limits<int32_t>::min());
};