#include "MathVector.h"
#include "Serializer.h"
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

template<typename T>
Vector2<T>& Vector2<T>::operator<<(ExposedPropertyData& propertyData)
{
	const T cachedX = x;
	const T cachedY = y;

	bool bModified{false};
	if constexpr (std::is_same_v<T, int>)
	{
		bModified = ImGui::InputInt2(propertyData.propertyName.c_str(), &x);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		bModified = ImGui::InputFloat2(propertyData.propertyName.c_str(), &x);
	}
	else
	{
		ImGui::Text(propertyData.propertyName.c_str());
		ImGui::SameLine();
		ImGui::Text(" | ERROR: Unhandled Vector2<T> type");
	}

	if (bModified)
	{
		propertyData.modifiedPropertyName = propertyData.propertyName;
		if (cachedX != x)
		{
			propertyData.propertyChain.emplace_back(0);
			propertyData.SetOldValue(cachedX);
			propertyData.SetNewValue(x);
		}
		else
		{
			propertyData.propertyChain.emplace_back(1);
			propertyData.SetOldValue(cachedY);
			propertyData.SetNewValue(y);
		}
	}

	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator<<(PropertyManipulator& setter)
{
	switch ((*setter.propertyChain)[0])
	{
	case 0: Serializer::SetPropertyInternal<T>(setter.value, x, setter.outPropertyData); break;
	case 1: Serializer::SetPropertyInternal<T>(setter.value, y, setter.outPropertyData);; break;
	default: ASSERT(false); break;
	}
	return *this;
}

template<typename T>
const Vector2<T>& Vector2<T>::operator>>(PropertyManipulator& deleter) const
{
	Serializer::DeletePropertyDataInternal<T>(*deleter.allocatedValue);
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator<<(ExposedPropertyData& propertyData)
{
	const T cachedX = x;
	const T cachedY = y;
	const T cachedZ = z;

	bool bModified{false};
	if constexpr (std::is_same_v<T, int>)
	{
		bModified = ImGui::InputInt3(propertyData.propertyName.c_str(), &x);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		bModified = ImGui::InputFloat3(propertyData.propertyName.c_str(), &x);
	}
	else
	{
		ImGui::Text(propertyData.propertyName.c_str());
		ImGui::SameLine();
		ImGui::Text(" | ERROR: Unhandled Vector3<T> type");
	}

	if (bModified)
	{
		propertyData.modifiedPropertyName = propertyData.propertyName;
		if (cachedX != x)
		{
			propertyData.propertyChain.emplace_back(0);
			propertyData.SetOldValue(cachedX);
			propertyData.SetNewValue(x);
		}
		else if(cachedY != y)
		{
			propertyData.propertyChain.emplace_back(1);
			propertyData.SetOldValue(cachedY);
			propertyData.SetNewValue(y);
		}
		else
		{
			propertyData.propertyChain.emplace_back(2);
			propertyData.SetOldValue(cachedZ);
			propertyData.SetNewValue(z);
		}
	}

	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator<<(PropertyManipulator& setter)
{
	switch ((*setter.propertyChain)[0])
	{
	case 0: Serializer::SetPropertyInternal<T>(setter.value, x, setter.outPropertyData); break;
	case 1: Serializer::SetPropertyInternal<T>(setter.value, y, setter.outPropertyData); break;
	case 2: Serializer::SetPropertyInternal<T>(setter.value, z, setter.outPropertyData); break;
	default: ASSERT(false); break;
	}
	return *this;
}

template<typename T>
const Vector3<T>& Vector3<T>::operator>>(PropertyManipulator& deleter) const
{
	Serializer::DeletePropertyDataInternal<T>(*deleter.allocatedValue);
	return *this;
}

template<typename T>
bool Vector2<T>::IsBetween(const Vector2<T>& A, const Vector2<T> B)
{
	T xMin;
	T xMax;
	if (A.x < B.x)
	{
		xMin = A.x;
		xMax = B.x;
	}
	else
	{
		xMin = B.x;
		xMax = A.x;
	}

	T yMin;
	T yMax;
	if (A.y < B.y)
	{
		yMin = A.y;
		yMax = B.y;
	}
	else
	{
		yMin = B.y;
		yMax = A.y;
	}


	return x > xMin && x < xMax && y > yMin && y < yMax;
}

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