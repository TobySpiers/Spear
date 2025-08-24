#pragma once
#include "Core/Core.h"
#include <list>
#include <unordered_map>

// Implementation based on the explanation by javidx9 YouTube videos "Quirky Quad Trees" (Parts 1 & 2), with some adjustments

// Struct representing an axis-aligned quad for use in QuadTrees
struct QuadTreeRect
{
	Vector2f min{Vector2f::ZeroVector};
	Vector2f max{Vector2f::ZeroVector};

	QuadTreeRect(const Vector2f originTopLeft = Vector2f::ZeroVector, const Vector2f extent = Vector2f::OneVector);

	bool Contains(const Vector2f& point) const;
	bool Contains(const QuadTreeRect& otherRect) const;
	bool Overlaps(const QuadTreeRect& otherRect) const;
};

// Struct storing information about the position of an item inside a QuadTree
template <typename T>
struct QuadTreeItemLocation
{
	typename std::list<std::pair<QuadTreeRect, T>>* container;
	typename std::list<std::pair<QuadTreeRect, T>>::iterator iterator;
};

// Struct representing and item currently stored in a QuadTree
template <typename T>
struct QuadTreeItem
{
	QuadTreeItem(const T& inItem) : item(inItem) {}

	// Stored item itself
	T item;

	// Position of item inside the QuadTree
	QuadTreeItemLocation<typename std::list<QuadTreeItem<T>>::iterator> pItem;
};

// Class representing a single layer of a quad tree
template <typename T, size_t MaxLayers>
class QuadTreeLayer
{
public:
	QuadTreeLayer(const QuadTreeRect& area, int layer) : m_layer(layer) { Resize(area); }

	// Changes the size of this layer and all children, emptying self and child layers in the process
	void Resize(const QuadTreeRect& newArea)
	{
		Clear();

		m_area = newArea;
		const Vector2f halfArea = (newArea.max - newArea.min) / 2.f;
		
		m_childAreas[0] = QuadTreeRect(newArea.min, halfArea);
		m_childAreas[1] = QuadTreeRect({newArea.min.x + halfArea.x, newArea.min.y}, halfArea);
		m_childAreas[1] = QuadTreeRect({newArea.min.x, newArea.min.y + halfArea.y}, halfArea);
		m_childAreas[1] = QuadTreeRect({ newArea.min.x + halfArea.x, newArea.min.y + halfArea.y}, halfArea);
	}

	// Empties this layer and all child layers
	void Clear()
	{
		m_items.clear();

		for (int i = 0; i < 4; i++)
		{
			if (m_childLayers[i])
			{
				m_childLayers[i]->Clear();
				m_childLayers[i].reset();
			}
		}
	}

	// Adds a new item either to this layer or a child layer, based on area
	QuadTreeItemLocation<T> Insert(const T& item, const QuadTreeRect& itemArea)
	{
		for (int i = 0; i < 4; i++)
		{
			// If itemArea fits wholly inside a childArea, pass it to childLayer
			if (m_childAreas[i].Contains(itemArea))
			{
				if (m_layer + 1 < MaxLayers)
				{
					if (!m_childLayers[i])
					{
						m_childLayers[i] = std::make_shared<QuadTreeLayer<T, MaxLayers>>(m_childAreas[i], m_layer + 1);
					}

					return m_childLayers[i]->Insert(item, itemArea);
				}
			}
		}

		// itemArea did not fit wholly within a childArea, store it within our own layer
		m_items.push_back({itemArea, item});
		return {&m_items, std::prev(m_items.end())};
	}

	// Appends all items overlapping the search area in self and children to outItems
	void Search(const QuadTreeRect& searchArea, std::vector<T>& outItems) const
	{
		// First, add any overlapping items owned by this layer (those which did not fit wholly inside a child area)
		for (const auto& it : m_items)
		{
			if (searchArea.Overlaps(it.first))
			{
				outItems.push_back(it.second);
			}
		}

		// Next, add any overlapping items owned by child layers
		for (int i = 0; i < 4; i++)
		{
			if (m_childLayers[i])
			{
				// If our child layer fits wholly within searchArea, simply append all items from child
				if (searchArea.Contains(m_childAreas[i]))
				{
					m_childLayers[i]->AppendItems(outItems);
				}
				else if (searchArea.Overlaps(m_childAreas[i])) // otherwise if child layer overlaps our searchArea, continue search within child
				{
					m_childLayers[i]->Search(searchArea, outItems);
				}
			}
		}
	}

	// Appends all items in self and children to outItems
	void AppendItems(std::vector<T>& outItems)
	{
		for (const auto& it : m_items)
		{
			outItems.push_back(it.second);
		}

		for (int i = 0; i < 4; i++)
		{
			if (m_childLayers[i])
			{
				m_childLayers[i]->AppendItems(outItems);
			}
		}
	}

	size_t GetMaxLayers() const
	{
		return MaxLayers;
	}

private:
	// Depth within tree
	int m_layer{0};

	// Area of this layer 
	QuadTreeRect m_area;

	// Area of child layers
	QuadTreeRect m_childAreas[4];

	// Child layers
	std::shared_ptr<QuadTreeLayer<T, MaxLayers>> m_childLayers[4];

	// Items which belong to this layer (items which could not fit inside a child layer)
	std::list<std::pair<QuadTreeRect, T>> m_items;
};

// Class representing a complete Quad Tree, comprised of MaxLayers layers (default 8)
template <typename T, size_t MaxLayers = 8>
class QuadTree
{
public:
	using QuadTreeContainer = std::list<QuadTreeItem<T>>;

	QuadTree(const QuadTreeRect& totalArea = {{0.f, 0.f},{50, 50}}) : m_root(totalArea, 0) {}

	// Resizes the QuadTree and empties contents
	void Resize(const QuadTreeRect& newArea)
	{
		m_root.Resize(newArea);
	}

	// Adds a new item to the tree
	typename QuadTreeContainer::iterator Insert(const T& item, const QuadTreeRect& itemArea)
	{
		m_items.emplace_back(item);
		QuadTreeItem<T>& newItem = m_items.back();
		newItem.pItem = m_root.Insert(std::prev(m_items.end()), itemArea);
		return std::prev(m_items.end());
	}

	// Moves an existing item into the appropriate tree node for itemArea
	void Relocate(typename QuadTreeContainer::iterator& item, const QuadTreeRect& itemArea)
	{
		item->pItem.container->erase(item->pItem.iterator);
		item->pItem = m_root.Insert(item, itemArea);
	}

	// Removes an existing item from the tree
	void Remove(typename QuadTreeContainer::iterator& item)
	{
		item->pItem.container->erase(item->pItem.iterator);
		m_items.erase(item);
	}

	// Returns a list of all items in tree overlapping the search area
	void Search(const QuadTreeRect& searchArea, std::vector<typename QuadTreeContainer::iterator>& outItems) const
	{
		m_root.Search(searchArea, outItems);
	}

	// Removes all items from tree
	bool Clear()
	{
		m_root.Clear();
		m_items.clear();
	}

	// Returns current number of items stored in tree
	size_t Size() const
	{
		return m_items.size();
	}

	// Convenience functions for iterators
	typename QuadTreeContainer::iterator Begin()
	{
		return m_items.begin();
	}
	typename QuadTreeContainer::iterator End()
	{
		return m_items.end();
	}
	typename QuadTreeContainer::const_iterator CBegin()
	{
		return m_items.cbegin();
	}
	typename QuadTreeContainer::const_iterator CEnd()
	{
		return m_items.cend();
	}
	

private:
	// Lists all items currently in tree
	QuadTreeContainer m_items;

	// Top layer of tree
	QuadTreeLayer<typename QuadTreeContainer::iterator, MaxLayers> m_root;
};