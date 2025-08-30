#include "GameObject.h"
#include "Collision/CollisionSystem.h"
#include <algorithm>
#include <Core/FrameProfiler.h>
#include <imgui.h>
#include <Graphics/ScreenRenderer.h>

// Tracking
std::vector<GameObject*> GameObject::s_allocatedObjects;
std::vector<GameObject*> GameObject::s_tickList;
std::vector<GameObject*> GameObject::s_drawList;
std::vector<GameObject*> GameObject::s_destroyList;
std::unordered_map<size_t, GameObject::DeserializeFuncPtr>* GameObject::s_objectDeserializers = nullptr;
std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>>* GameObject::s_objectConstructors = nullptr;

void GameObject::SetPosition(const Vector3f& newPos)
{
	if (m_position != newPos)
	{
		m_position = newPos;
		OnMoved.Broadcast(this);
	}
}

void GameObject::SetPosition(const Vector2f& position)
{
	if(m_position.x != position.x || m_position.y != position.y)
	{
		m_position.x = position.x;
		m_position.y = position.y;
		OnMoved.Broadcast(this);
	}
}

void GameObject::MovePosition(const Vector3f& moveDelta)
{
	if (moveDelta != Vector3f::ZeroVector)
	{
		m_position += moveDelta;
		OnMoved.Broadcast(this);
	}
}

void GameObject::MovePosition(const Vector2f& moveDelta)
{
	if(moveDelta != Vector2f::ZeroVector)
	{
		m_position.x += moveDelta.x;
		m_position.y += moveDelta.y;
		OnMoved.Broadcast(this);
	}
}

void GameObject::GlobalTick(float deltaTime)
{	
	START_PROFILE("GameObject Update");
	for (int i = s_tickList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_tickList[i];
		if (!obj->IsTickEnabled())
		{
			DisableTick_Internal(obj, i);
		}
		else
		{
			obj->OnTick_Internal(deltaTime);
		}
	}

	Collision::CollisionSystem2D::Get().Tick();

	DestroyObjects_Internal();

	END_PROFILE("GameObject Update");
}

void GameObject::GlobalDraw()
{
	START_PROFILE("GameObject Draw");
	for (int i = s_drawList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_drawList[i];
		if (!obj->IsDrawEnabled())
		{
			DisableDraw_Internal(obj, i);
		}
		else
		{
			obj->OnDraw_Internal();
		}
	}
	END_PROFILE("GameObject Draw");
}

void GameObject::GlobalDestroy()
{
	s_tickList.clear();
	s_drawList.clear();
	s_destroyList.clear();
	while (s_allocatedObjects.size())
	{
		GameObject* obj = s_allocatedObjects.back();
		obj->OnDestroy_Internal();
		delete obj;
		s_allocatedObjects.pop_back();
	}
}

void GameObject::FlushPendingDestroys()
{
	for (int i = s_drawList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_drawList[i];
		if(obj->IsPendingDestroy() && obj->IsDrawEnabled())
		{
			DisableDraw_Internal(obj, i);
		}
	}

	for (int i = s_tickList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_tickList[i];
		if(obj->IsPendingDestroy() && obj->IsTickEnabled())
		{
			DisableTick_Internal(obj, i);
		}
	}

	DestroyObjects_Internal();
}

void GameObject::DestroyObjects_Internal()
{
	while (s_destroyList.size())
	{
		GameObject* obj = s_destroyList.back();
		ASSERT(obj->IsSafeToDestroy());
		
		const int cachedInternalId = obj->internalId;
		obj->OnDestroy_Internal();

		while (!obj->trackedPtrs.empty())
		{
			GameObjectPtrBase* ptr = *obj->trackedPtrs.begin();
			ptr->Invalidate();
		}

		std::iter_swap(s_allocatedObjects.begin() + cachedInternalId, s_allocatedObjects.end() - 1);
		GameObject* swappedObj = s_allocatedObjects[cachedInternalId];
		swappedObj->internalId = cachedInternalId;
		s_allocatedObjects.pop_back();
		delete obj;

		s_destroyList.pop_back();
	}
}

void GameObject::DisableTick_Internal(GameObject* obj, int ticklistIndex)
{
	std::iter_swap(s_tickList.begin() + ticklistIndex, s_tickList.end());
	s_tickList.pop_back();
	obj->tickState = eGameObjectTickState::TickDisabled;
	if (obj->IsPendingDestroy() && obj->IsSafeToDestroy())
	{
		s_destroyList.push_back(obj);
	}
}

void GameObject::DisableDraw_Internal(GameObject* obj, int drawlistIndex)
{
	std::iter_swap(s_drawList.begin() + drawlistIndex, s_drawList.end());
	s_drawList.pop_back();
	obj->drawState = eGameObjectTickState::TickDisabled;
	if (obj->IsPendingDestroy() && obj->IsSafeToDestroy())
	{
		s_destroyList.push_back(obj);
	}
}

void GameObject::RegisterTickingObject(GameObject* obj)
{
	// make sure nothing is trying to add duplicates to the ticklist
	// this check isn't very performant but will be stripped out in Release builds
	ASSERT(std::find(s_tickList.begin(), s_tickList.end(), obj) == s_tickList.end()); 
	s_tickList.push_back(obj);
}

void GameObject::RegisterDrawingObject(GameObject* obj)
{
	// make sure nothing is trying to add duplicates to the ticklist
	// this check isn't very performant but will be stripped out in Release builds
	ASSERT(std::find(s_drawList.begin(), s_drawList.end(), obj) == s_drawList.end());
	s_drawList.push_back(obj);
}

void GameObject::RegisterPtr(GameObjectPtrBase* ptr)
{
	ASSERT(ptr);
	trackedPtrs.insert(ptr);
}

void GameObject::DeregisterPtr(GameObjectPtrBase* ptr)
{
	ASSERT(ptr);
	trackedPtrs.erase(ptr);
}

void GameObject::PopulateEditorPanel(ExposedPropertyData& propertyData)
{
	EXPOSE_PROPERTIES(propertyData, m_position);

	ImGui::SeparatorText("Components");
	for (int c = 0; c < m_components.size(); c++)
	{
		Serializer::ExposeInEditor(propertyData, "##Components", i++, *m_components[c].get());
	}
	ImGui::SeparatorText("Object");
}

void GameObject::SetProperty(const void* value, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step)
{
	const int propertyId = propertyChain[propertyChain.size() - step];
	SET_PROPERTY(value, propertyChain, outPropertyData, step + 1, propertyId, m_position);

	for (int c = 0; c < m_components.size(); c++)
	{
		if (i++ == propertyId)
		{
			Serializer::SetProperty(value, propertyChain, outPropertyData, step + 1, *m_components[c].get());
		}
	}
}

void GameObject::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step) const
{
	const int propertyId = propertyChain[propertyChain.size() - step];
	DELETE_PROPERTY_DATA(allocatedData, propertyChain, step, propertyId, m_position);

	for (int c = 0; c < m_components.size(); c++)
	{
		if (i++ == propertyId)
		{
			Serializer::DeletePropertyData(allocatedData, propertyChain, step, *m_components[c].get());
		}
	}
}

void GameObject::DrawInEditor(const Vector3f& position, float zoom, float mapSpacing, bool bSelected, bool bHovered)
{
	// Check object is not pending destroy/editor destroyed
	if (IsValid())
	{
		OnEditorDraw_Internal(position, zoom, mapSpacing);
		if (bHovered || bSelected)
		{
			Spear::Renderer::LinePolyData icon;
			icon.segments = 4;
			icon.colour = bSelected ? Colour4f::Cyan() : Colour4f::White();
			icon.pos = position.XY();
			icon.radius = GetEditorHoverRadius(zoom);
			icon.depth = position.z;
			icon.rotation = TO_RADIANS(45.f);
			Spear::Renderer::Get().AddLinePoly(icon);
		}
	}
}

void GameObject::OnEditorDraw(const Vector3f& position, float zoom, float mapSpacing)
{
	Spear::Renderer::LinePolyData icon;
	icon.segments = 8;
	icon.colour = Colour4f::White();
	icon.pos = position.XY();
	icon.radius = zoom * 10.f;
	icon.depth = position.z;
	Spear::Renderer::Get().AddLinePoly(icon);
}

float GameObject::GetEditorHoverRadius(float zoom)
{
	return zoom * 20;
}

const char* GameObject::GetClassName() const
{
	return "UnknownObject";
}

void GameObject::GlobalSerialize(const char* filename)
{
	std::ofstream file(filename);
	GlobalSerialize(file);
}

void GameObject::GlobalSerialize(std::ofstream& file)
{
	// Save out num objects to expect on deserialization
	int totalObjects = 0;
	for (const GameObject* obj : s_allocatedObjects)
	{
		if (obj->IsValid())
		{
			totalObjects++;
		}
	}

	file << totalObjects << std::endl;

	// Prep all GameObjects
	for (GameObject* obj : s_allocatedObjects)
	{
		if (obj->IsValid())
		{
			obj->OnPreSerialize();
		}
	}

	// Serialize each object
	for (const GameObject* obj : s_allocatedObjects)
	{
		if (obj->IsValid())
		{
			obj->Serialize(file);
		}
	}

	// Restore all GameObjects
	for (GameObject* obj : s_allocatedObjects)
	{
		if (obj->IsValid())
		{
			obj->OnPostSerialize();
		}
	}
}

void GameObject::GlobalDeserialize(const char* filename)
{
	std::ifstream file(filename);
	GlobalDeserialize(file);
}

void GameObject::GlobalDeserialize(std::ifstream& file)
{
	// Load in num objects to expect
	std::string objectTotalStr;
	std::getline(file, objectTotalStr);
	int objectTotal = std::stoi(objectTotalStr);

	// Deserialize each object based on registered typeHash->deserializer map
	std::string typeHashStr;
	for (int i = 0; i < objectTotal; i++)
	{
		typeHashStr = "";
		std::getline(file, typeHashStr);
		size_t typeHash = std::stoull(typeHashStr);
		DeserializeFuncPtr deserializeType = ObjectDeserializers()->at(typeHash); // if this FAILS, try setting Linker>UseLibraryDependencyInputs to TRUE

		GameObject* obj = deserializeType(file);
		obj->internalId = s_allocatedObjects.size();
		s_allocatedObjects.push_back(obj);

		ASSERT(obj->IsValid());

		if (obj->IsTickEnabled())
		{
			RegisterTickingObject(obj);
		}
		if (obj->IsDrawEnabled())
		{
			RegisterDrawingObject(obj);
		}
		obj->OnDeserialize();
		obj->OnCreated_Internal();
	}
}

void GameObject::SetTickEnabled(bool bShouldTick)
{
	ASSERT(!IsPendingDestroy());
	
	if (bShouldTick)
	{
		if (tickState == eGameObjectTickState::TickDisabled)
		{
			RegisterTickingObject(this);
		}
		tickState = eGameObjectTickState::TickEnabled;
	}
	else
	{
		if (tickState == eGameObjectTickState::TickEnabled)
		{
			tickState = eGameObjectTickState::TickPendingDisable;
		}
	}
}
bool GameObject::IsTickEnabled() const
{
	return tickState == eGameObjectTickState::TickEnabled;
}

void GameObject::SetDrawEnabled(bool bShouldDraw)
{
	ASSERT(!IsPendingDestroy());
	
	if (bShouldDraw)
	{
		if (drawState == eGameObjectTickState::TickDisabled)
		{
			RegisterDrawingObject(this);
		}
		drawState = eGameObjectTickState::TickEnabled;
	}
	else
	{
		if (drawState == eGameObjectTickState::TickEnabled)
		{
			drawState = eGameObjectTickState::TickPendingDisable;
		}
	}
}
bool GameObject::IsDrawEnabled() const
{
	return drawState == eGameObjectTickState::TickEnabled;
}

bool GameObject::IsValid() const
{
	return !IsPendingDestroy() && !IsDestroyedInEditor();
}

void GameObject::SetDestroyedInEditor(bool bDestroyed)
{
	ASSERT(!IsPendingDestroy());
	lifeState = bDestroyed ? eGameObjectLifeState::EditorDestroy : eGameObjectLifeState::Active;
}

bool GameObject::IsDestroyedInEditor() const
{
	return lifeState == eGameObjectLifeState::EditorDestroy;
}

void GameObject::Destroy()
{
	if (!IsPendingDestroy())
	{
		if (IsSafeToDestroy())
		{
			s_destroyList.push_back(this);
		}
		else
		{
			SetTickEnabled(false);
			SetDrawEnabled(false);
		}
		
		lifeState = eGameObjectLifeState::PendingDestroy;
	}
}
bool GameObject::IsPendingDestroy() const
{
	return lifeState == eGameObjectLifeState::PendingDestroy;
}

bool GameObject::IsSafeToDestroy() const
{
	return tickState == eGameObjectTickState::TickDisabled && drawState == eGameObjectTickState::TickDisabled;
}

GameObject::~GameObject()
{
	// Invalidate any GameObjectPtrs referencing this object
	while(!trackedPtrs.empty())
	{
		GameObjectPtrBase* ptr = *trackedPtrs.begin();
		ptr->Invalidate();
	}
}

void GameObject::Serialize_Internal(std::ofstream& os) const
{
	SERIALIZE(os, m_position)

	for (int i = 0; i < m_components.size(); i++)
	{
		Serializer::Serialize(os, *m_components[i].get());
	}
}

void GameObject::Deserialize(std::ifstream& is)
{
	DESERIALIZE(is, m_position)

	for (int i = 0; i < m_components.size(); i++)
	{
		Serializer::Deserialize(is, *m_components[i].get());
	}
}

bool GameObject::RegisterFactoryFunctionsForClass(const char* className, size_t hashcode, DeserializeFuncPtr deserializeFunc, ConstructorFuncPtr constructorFunc)
{
	ObjectDeserializers()->insert({hashcode, deserializeFunc });
	ObjectConstructors()->emplace_back(std::pair<const char*, ConstructorFuncPtr>(className, constructorFunc));
	return true;
}

void GameObject::OnCreated_Internal()
{
	OnCreated();

	for (std::unique_ptr<GameObjectComponent>& comp : m_components)
	{
		comp->OnCreated();
	}
}

void GameObject::OnTick_Internal(float deltaTime)
{
	OnTick(deltaTime);

	for (std::unique_ptr<GameObjectComponent>& comp : m_components)
	{
		comp->OnTick(deltaTime);
	}
}

void GameObject::OnDraw_Internal() const
{
	OnDraw();

	for (const std::unique_ptr<GameObjectComponent>& comp : m_components)
	{
		comp->OnDraw();
	}
}

void GameObject::OnEditorDraw_Internal(const Vector3f& position, float zoom, float mapSpacing)
{
	OnEditorDraw(position, zoom, mapSpacing);

	for (std::unique_ptr<GameObjectComponent>& comp : m_components)
	{
		comp->OnEditorDraw(position, zoom, mapSpacing);
	}
}

void GameObject::OnDestroy_Internal()
{
	for (std::unique_ptr<GameObjectComponent>& comp : m_components)
	{
		comp->OnDestroy();
	}

	OnDestroy();
}

std::unordered_map<size_t, GameObject::DeserializeFuncPtr>* GameObject::ObjectDeserializers()
{
	if (!s_objectDeserializers)
	{
		s_objectDeserializers = new std::unordered_map<size_t, DeserializeFuncPtr>;
	}
	return s_objectDeserializers;
}

std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>> *GameObject::ObjectConstructors()
{
	if (!s_objectConstructors)
	{
		s_objectConstructors = new std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>>;
	}
	return s_objectConstructors;
}
