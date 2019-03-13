#include "Quadtree.h"
#include "GameObject.h"
#include <algorithm>

#define NE 0
#define NW 1
#define SE 2
#define SW 3

// QuadtreeNode --------------------------------------------------
QuadtreeNode::QuadtreeNode(const math::AABB& boundingBox) :boundingBox(boundingBox) 
{
	for (uint i = 0; i < 4; ++i)
		children[i] = nullptr;
}

QuadtreeNode::~QuadtreeNode() 
{
	if (!IsLeaf())
	{
		for (uint i = 0; i < 4; ++i)
			RELEASE(children[i]);
	}
}

bool QuadtreeNode::IsLeaf() const
{
	return children[0] == nullptr;
}

void QuadtreeNode::Insert(GameObject* gameObject)
{
	if (IsLeaf() && objects.size() < BUCKET_SIZE)
		objects.push_back(gameObject);
	else
	{
		objects.push_back(gameObject);

		if (subdivision < MAX_SUBDIVISIONS)
		{
			if (IsLeaf())
				Subdivide();

			RedistributeChildren();
		}
	}
}

void QuadtreeNode::Subdivide()
{
	const math::float3 size = boundingBox.Size();
	const math::float3 halfSize(size.x / 2.0f, size.y, size.z / 2.0f);
	const math::float3 quarterSize(halfSize.x / 2.0f, size.y, halfSize.z / 2.0f);

	const math::float3 center = boundingBox.CenterPoint();
	math::float3 quarterCenter;

	math::AABB quarterBoundingBox;

	// North East
	quarterCenter = { center.x + quarterSize.x, center.y, center.z - quarterSize.z };
	quarterBoundingBox.SetFromCenterAndSize(quarterCenter, halfSize);
	children[NE] = new QuadtreeNode(quarterBoundingBox);

	// North West
	quarterCenter = { center.x - quarterSize.x, center.y, center.z - quarterSize.z };
	quarterBoundingBox.SetFromCenterAndSize(quarterCenter, halfSize);
	children[NW] = new QuadtreeNode(quarterBoundingBox);

	// South East
	quarterCenter = { center.x + quarterSize.x, center.y, center.z + quarterSize.z };
	quarterBoundingBox.SetFromCenterAndSize(quarterCenter, halfSize);
	children[SE] = new QuadtreeNode(quarterBoundingBox);

	// South West
	quarterCenter = { center.x - quarterSize.x, center.y, center.z + quarterSize.z };
	quarterBoundingBox.SetFromCenterAndSize(quarterCenter, halfSize);
	children[SW] = new QuadtreeNode(quarterBoundingBox);

	children[NE]->subdivision = children[NW]->subdivision = children[SE]->subdivision = children[SW]->subdivision = subdivision + 1;
}

void QuadtreeNode::RedistributeChildren()
{
	std::list<GameObject*>::const_iterator it = objects.begin();

	while (it != objects.end())
	{
		uint intersections = 0;
		uint lastIntersection = 0;
		for (uint i = 0; i < 4; ++i)
		{
			if ((*it)->boundingBox.Intersects(children[i]->boundingBox))
			{
				++intersections;
				lastIntersection = i;
			}
		}

		if (intersections == 1)
		{
			children[lastIntersection]->Insert(*it);
			it = objects.erase(it);
		}
		else if (intersections == 4)
		{
			// Any time an object straddles the boundary between all 4 quads, it needs to be stored at the parent level
			++it;
		}
		else
		{
			for (uint i = 0; i < 4; ++i)
			{
				if ((*it)->boundingBox.Intersects(children[i]->boundingBox))
					children[i]->Insert(*it);
			}
			it = objects.erase(it);
		}
	}
}

void QuadtreeNode::GetGameObjects(std::vector<GameObject*>& object) const
{
	for (std::list<GameObject*>::const_iterator iterator = objects.begin(); iterator != objects.end(); ++iterator)
	{
		object.push_back(*iterator);
	}

	for (int i = 0; i < 4; i++)
	{
		if (children[i] != nullptr)
			children[i]->GetGameObjects(object);
		else
			break;
	}
}

// Quadtree --------------------------------------------------
Quadtree::Quadtree() {}

Quadtree::~Quadtree() 
{
	Clear();
}
void Quadtree::Create(const math::AABB& limits) //Internal Create
{
	if (root != nullptr)
		ReDoQuadtree(limits);

	root = new QuadtreeNode(limits);
}

void Quadtree::Create() //External Create
{
	if (root == nullptr)
		root = new QuadtreeNode(math::AABB());
}

void Quadtree::Clear()
{
	RELEASE(root);
}

void Quadtree::Insert(GameObject* gameObject)
{
	if (gameObject->boundingBox.Intersects(root->boundingBox))
		root->Insert(gameObject);
	else
		ReDoLimits(gameObject);
}

void Quadtree::ReDoQuadtree(std::vector<GameObject*> objects) // External Redo
{
	if (root != nullptr)
	{
		Clear();
		if (!objects.empty())
		{
			Create((*objects.begin())->boundingBox);

			for (std::vector<GameObject*>::iterator iterator = objects.begin(); iterator != objects.end(); ++iterator)
			{
				Insert(*iterator);
			}
		}
		else
			Create();
	}
}

void Quadtree::ReDoQuadtree(const math::AABB & limits) //Internal Redo
{
	if (root != nullptr)
	{
		std::vector<GameObject*> objects;
		GetGameObjects(objects);

		Clear();
		Create(limits);

		for (std::vector<GameObject*>::iterator iterator = objects.begin(); iterator != objects.end(); ++iterator)
		{
			Insert(*iterator);
		}
	}
}

void Quadtree::ReDoLimits(GameObject* newObject)
{
	if (root != nullptr)
	{
		math::float3 minPoint = root->boundingBox.minPoint;
		math::float3 maxPoint = root->boundingBox.maxPoint;
		math::float3 objectMin = newObject->boundingBox.minPoint;
		math::float3 objectMax = newObject->boundingBox.maxPoint;

		if (minPoint.x > objectMin.x)
			minPoint.x = objectMin.x;
		if (minPoint.y > objectMin.y)
			minPoint.y = objectMin.y;
		if (minPoint.z > objectMin.z)
			minPoint.z = objectMin.z;

		if (maxPoint.x < objectMax.x)
			maxPoint.x = objectMax.x;
		if (maxPoint.y < objectMax.y)
			maxPoint.y = objectMax.y;
		if (maxPoint.z < objectMax.z)
			maxPoint.z = objectMax.z;

		ReDoQuadtree(math::AABB(minPoint, maxPoint));
		Insert(newObject);
	}
}

void Quadtree::GetGameObjects(std::vector<GameObject*>& objects) const
{
	if (root != nullptr)
	{
		root->GetGameObjects(objects);
		UniqueObjects(objects);
	}
}

void Quadtree::UniqueObjects(std::vector<GameObject*>& objects) const
{
	if (!objects.empty())
	{
		std::sort(objects.begin(), objects.end()); //all equals objects together
		objects.erase(std::unique(objects.begin(), objects.end()), objects.end());
		//erase corrupt objects after unique(that returns the new last object position)
	}
}