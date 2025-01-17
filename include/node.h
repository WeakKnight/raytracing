#pragma once

#include "scene.h"
#include "transformation.h"
#include "itembase.h"
#include "hitinfo.h"
#include "box.h"
#include "constants.h"
#include "objbase.h"

using namespace cy;

class Material;
class LightComponent;

struct HitInfo;
struct HitInfoContext;

class Node;


class Node : public ItemBase, public Transformation
{
private:
	Node** child;               // Child nodes
	int numChild;               // The number of child nodes
	Object* obj;                // Object reference (merely points to the object, but does not own the object, so it doesn't get deleted automatically)
	Material* mtl;              // Material used for shading the object
	Box childBoundBox;          // Bounding box of the child nodes, which does not include the object of this node, but includes the objects of the child nodes
	LightComponent* light = nullptr;

public:
	std::vector<Node*> chain;

	Vec3f TransformPointToLocal(const Vec3f& p)
	{
		Vec4f result = worldToLocal * Vec4f(p.x, p.y, p.z, 1.0f);
		return Vec3f(result.x, result.y, result.z);
	}

	Vec3f TransformPointToWorld(const Vec3f& p)
	{
		Vec4f result = localToWorld * Vec4f(p.x, p.y, p.z, 1.0f);
		return Vec3f(result.x, result.y, result.z);
	}

	void InitWorldMatrix()
	{
		// first get node chain
		Node* current = this;
		while (current != nullptr)
		{
			chain.push_back(current);
			current = current->GetParent();
		}
		// from self to ancestor
		// for (int i = chain.size() - 1; i >= 0; i--)
		for (int i = 0; i < chain.size(); i++)
		{
			auto item = chain[i];
			localToWorld = item->localToParent * localToWorld;
		}
		worldToLocal = localToWorld.GetInverse();
	}

	Node() : child(nullptr), numChild(0), obj(nullptr), mtl(nullptr), parent(nullptr) {}
	virtual ~Node() { DeleteAllChildNodes(); }

	void Init() { DeleteAllChildNodes(); obj = nullptr; mtl = nullptr; childBoundBox.Init(); SetName(nullptr); InitTransform(); } // Initialize the node deleting all child nodes

	Node* parent;

	Node CopyNodeWithSameMaterialAndChild()
	{
		Node result;
		result.numChild = numChild;
		result.child = child;
		result.mtl = mtl;
		result.parent = nullptr;
		result.obj = obj;

		return result;
	}

	// 
	Node* GetParent()
	{
		return parent;
	}
	void SetParent(Node* node)
	{
		parent = node;
	}

	// Hierarchy management
	int  GetNumChild() const { return numChild; }
	void SetNumChild(int n, int keepOld = false)
	{
		if (n < 0) n = 0;    // just to be sure
		Node** nc = nullptr;    // new child pointer
		if (n > 0) nc = new Node * [n];
		for (int i = 0; i < n; i++) nc[i] = nullptr;
		if (keepOld) {
			int sn = Min(n, numChild);
			for (int i = 0; i < sn; i++) nc[i] = child[i];
		}
		if (child) delete[] child;
		child = nc;
		numChild = n;
	}
	Node const* GetChild(int i) const { return child[i]; }
	Node* GetChild(int i) { return child[i]; }

	void        SetChild(int i, Node* node)
	{
		child[i] = node;
	}

	void        AppendChild(Node* node)
	{ 
		SetNumChild(numChild + 1, true);
		SetChild(numChild - 1, node);
		node->SetParent(this);
	}

	void        RemoveChild(int i) { for (int j = i; j < numChild - 1; j++) child[j] = child[j + 1]; SetNumChild(numChild - 1); }
	void        DeleteAllChildNodes() { for (int i = 0; i < numChild; i++) { child[i]->DeleteAllChildNodes(); delete child[i]; } SetNumChild(0); }

	// Bounding Box
	const Box& ComputeChildBoundBox()
	{
		childBoundBox.Init();
		for (int i = 0; i < numChild; i++) {
			Box childBox = child[i]->ComputeChildBoundBox();
			Object* cobj = child[i]->GetNodeObj();
			if (cobj) childBox += cobj->GetBoundBox();
			if (!childBox.IsEmpty()) {
				// transform the box from child coordinates
				for (int j = 0; j < 8; j++) childBoundBox += child[i]->TransformFrom(childBox.Corner(j));
			}
		}
		return childBoundBox;
	}
	const Box& GetChildBoundBox() const { return childBoundBox; }

	// Object management
	Object const* GetNodeObj() const { return obj; }
	Object* GetNodeObj() { return obj; }
	void           SetNodeObj(Object* object) 
	{ 
		obj = object;
		obj->SetParent(this);
	}

	// Material management
	Material* GetMaterial()
	{
		if (mtl == nullptr && parent != nullptr)
		{
			return parent->GetMaterial();
		}

		return mtl;
	}
	void            SetMaterial(Material* material) { mtl = material; }

	LightComponent* GetLight()
	{
		return light;
	}
	void SetLight(LightComponent* com);

	// Transformations
	Ray ToNodeCoords(Ray const& ray) const
	{
		Ray r;
		r.p = TransformTo(ray.p);
		r.dir = TransformTo(ray.p + ray.dir) - r.p;
		return r;
	}

	RayContext ToNodeCoords(RayContext const& rayContext) const
	{
		RayContext result;

		result.cameraRay = ToNodeCoords(rayContext.cameraRay);
		result.rightRay = ToNodeCoords(rayContext.rightRay);
		result.topRay = ToNodeCoords(rayContext.topRay);
		result.delta = rayContext.delta;
		result.hasDiff = rayContext.hasDiff;

		return result;
	}

	void FromNodeCoords(HitInfo& hInfo) const
	{
		hInfo.p = TransformFrom(hInfo.p);
		hInfo.N = VectorTransformFrom(hInfo.N).GetNormalized();
		hInfo.Tangent = VectorTransformFrom(hInfo.Tangent).GetNormalized();
		hInfo.Bitangent = VectorTransformFrom(hInfo.Bitangent).GetNormalized();
	}

	void FromNodeCoords(HitInfoContext& hInfoContext) const
	{
		FromNodeCoords(hInfoContext.mainHitInfo);
		FromNodeCoords(hInfoContext.rightHitInfo);
		// assert(!isnan(hInfoContext.rightHitInfo.N.x));
		FromNodeCoords(hInfoContext.topHitInfo);
		// assert(!isnan(hInfoContext.topHitInfo.N.x));
	}
};