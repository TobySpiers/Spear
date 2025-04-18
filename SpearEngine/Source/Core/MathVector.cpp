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
Vector2<T>& Vector2<T>::operator<<(EditorExposer& editor)
{
	const T cachedX = x;
	const T cachedY = y;

	if constexpr (std::is_same_v<T, int>)
	{
		editor.modified = ImGui::InputInt2(editor.propertyName.c_str(), &x);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		editor.modified = ImGui::InputFloat2(editor.propertyName.c_str(), &x);
	}
	else
	{
		ImGui::Text(editor.propertyName.c_str());
		ImGui::SameLine();
		ImGui::Text(" | ERROR: Unhandled Vector2<T> type");
	}

	if (editor.modified)
	{
		if (cachedX != x)
		{
			editor.outPropertyChain->emplace_back(0);
		}
		else
		{
			editor.outPropertyChain->emplace_back(1);
		}
	}

	return *this;
}

template<typename T>
const Vector2<T>& Vector2<T>::operator>>(EditorManipulator& extractor) const
{
	switch ((*extractor.propertyChain)[0])
	{
		case 0: extractor.value = &x; break;
		case 1: extractor.value = &y; break;
		default: ASSERT(false); break;
	}
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator<<(EditorManipulator& inserter)
{
	switch ((*inserter.propertyChain)[0])
	{
	case 0: x = *static_cast<const T*>(inserter.value); break;
	case 1: y = *static_cast<const T*>(inserter.value); break;
	default: ASSERT(false); break;
	}
	return *this;
}

template<typename T>
inline Vector3<T>& Vector3<T>::operator<<(EditorExposer& editor)
{
	const T cachedX = x;
	const T cachedY = y;
	const T cachedZ = z;

	if constexpr (std::is_same_v<T, int>)
	{
		editor.modified = ImGui::InputInt3(editor.propertyName.c_str(), &x);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		editor.modified = ImGui::InputFloat3(editor.propertyName.c_str(), &x);
	}
	else
	{
		ImGui::Text(editor.propertyName.c_str());
		ImGui::SameLine();
		ImGui::Text(" | ERROR: Unhandled Vector3<T> type");
	}

	if (editor.modified)
	{
		if (cachedX != x)
		{
			editor.outPropertyChain->emplace_back(0);
		}
		else if(cachedY != y)
		{
			editor.outPropertyChain->emplace_back(1);
		}
		else
		{
			editor.outPropertyChain->emplace_back(2);
		}
	}

	return *this;
}

template<typename T>
const Vector3<T>& Vector3<T>::operator>>(EditorManipulator& extractor) const
{
	switch ((*extractor.propertyChain)[0])
	{
	case 0: extractor.value = &x; break;
	case 1: extractor.value = &y; break;
	case 2: extractor.value = &z; break;
	default: ASSERT(false); break;
	}
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator<<(EditorManipulator& inserter)
{
	switch ((*inserter.propertyChain)[0])
	{
	case 0: x = *static_cast<const T*>(inserter.value); break;
	case 1: y = *static_cast<const T*>(inserter.value); break;
	case 2: z = *static_cast<const T*>(inserter.value); break;
	default: ASSERT(false); break;
	}
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