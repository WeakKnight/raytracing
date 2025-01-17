#ifndef _BVH_H_INCLUDED_
#define _BVH_H_INCLUDED_

#include "scene.h"
#include "cyVector.h"
#include <vector>
#include "cyTriMesh.h"
#include "objects.h"
#include "mesh.h"
#include <map>

class TriObj;

class BVHBound
{
public:
    BVHBound()
    {
        
    }
    
    BVHBound(const float* data)
    {
        this->data[0] = data[0];
        this->data[1] = data[1];
        this->data[2] = data[2];
        this->data[3] = data[3];
        this->data[4] = data[4];
        this->data[5] = data[5];
    }
    
    BVHBound(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        data[0] = minX;
        data[1] = minY;
        data[2] = minZ;
        data[3] = maxX;
        data[4] = maxY;
        data[5] = maxZ;
    }
    
    float data[6] = {BIGFLOAT, BIGFLOAT, BIGFLOAT, -BIGFLOAT, -BIGFLOAT, -BIGFLOAT};

    Vec3f GetMax()
    {
        return Vec3f(data[3], data[4], data[5]);
    }
    
    Vec3f GetMin()
    {
        return Vec3f(data[0], data[1], data[2]);
    }
    
    float SurfaceArea()
    {
        Vec3f lengthVector = GetMax() - GetMin();
        return lengthVector.x * lengthVector.y * 2.0f + lengthVector.x * lengthVector.z * 2.0f + lengthVector.y * lengthVector.z * 2.0f;
    }
    
    void UpdateByPoint(Vec3f point)
    {
        if(point.x > data[3])
        {
            data[3] = point.x;
        }
        if(point.x < data[0])
        {
            data[0] = point.x;
        }
        if(point.y > data[4])
        {
            data[4] = point.y;
        }
        if(point.y < data[1])
        {
            data[1] = point.y;
        }
        if(point.z > data[5])
        {
            data[5] = point.z;
        }
        if(point.z < data[2])
        {
            data[2] = point.z;
        }
    }
    
    bool IntersectRay(Ray const &r,
//                      float& t,
                      float t_max = BIGFLOAT) const{
        assert(data[3] - data[0] >= 0.0f);
        assert(data[4] - data[1] >= 0.0f);
        assert(data[5] - data[2] >= 0.0f);

		Vec3f starThis[2] = {};
		starThis[0] = Vec3f(data[0], data[1], data[2]);
		starThis[1] = Vec3f(data[3], data[4], data[5]);

		float tmin, tmax, tymin, tymax, tzmin, tzmax;

		Vec3f invertDir = Vec3f(1.0f, 1.0f, 1.0f) / r.dir;
		int sign0 = (invertDir.x < 0);
		int sign1 = (invertDir.y < 0);
		int sign2 = (invertDir.z < 0);

		tmin = (starThis[sign0].x - r.p.x) * invertDir.x;

		tmax = (starThis[1 - sign0].x - r.p.x) * invertDir.x;
		tymin = (starThis[sign1].y - r.p.y) * invertDir.y;
		tymax = (starThis[1 - sign1].y - r.p.y) * invertDir.y;

		if ((tmin > tymax) || (tymin > tmax))
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (starThis[sign2].z - r.p.z) * invertDir.z;
		tzmax = (starThis[1 - sign2].z - r.p.z) * invertDir.z;

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		return true;

//        
//        Vec3f invertDir =  Vec3f(1.0f, 1.0f, 1.0f) / r.dir;
//        
//        float tx0;
//        float tx1;
//        
//        if(invertDir.x >= 0.0f)
//        {
//            tx0 = (data[0] - r.p.x) * invertDir.x;
//            tx1 = (data[3] - r.p.x) * invertDir.x;
//        }
//        else
//        {
//            tx1 = (data[0] - r.p.x) * invertDir.x;
//            tx0 = (data[3] - r.p.x) * invertDir.x;
//        }
//        
//		// assert(tx0 <= tx1);
//        
//        float ty0;
//        float ty1;
//        
//        if(invertDir.y >= 0.0f)
//        {
//            ty0 = (data[1] - r.p.y) * invertDir.y;
//            ty1 = (data[4] - r.p.y) * invertDir.y;
//        }
//        else
//        {
//            ty1 = (data[1] - r.p.y) * invertDir.y;
//            ty0 = (data[4] - r.p.y) * invertDir.y;
//        }
//        
//        assert(ty0 <= ty1);
//        
//        if(ty1 < tx0)
//        {
//            return false;
//        }
//        
//        if(tx1 < ty0)
//        {
//            return false;
//        }
//        
//        float t0 = Max(tx0, ty0);
//        float t1 = Min(tx1, ty1);
//        
//        float tz0;
//        float tz1;
//        if(invertDir.z >= 0.0f)
//        {
//            tz0 = (data[2] - r.p.z) * invertDir.z;
//            tz1 = (data[5] - r.p.z) * invertDir.z;
//        }
//        else
//        {
//            tz1 = (data[2] - r.p.z) * invertDir.z;
//            tz0 = (data[5] - r.p.z) * invertDir.z;
//        }
//        
//        if(t1 < tz0)
//        {
//            return false;
//        }
//        
//        if(tz1 < t0)
//        {
//            return false;
//        }
//        
//        t0 = Max(t0, tz0);
//        t1 = Min(t1, tz1);
//        
//        assert(t0 <= t1);
//        
//        if(t0 < 0.0f)
//        {
//            if(t1 < 0.0f)
//            {
//                return false;
//            }
//            else
//            {
//                if(t1 > t_max)
//                {
//                    return false;
//                }
//                else
//                {
////                    t = t1;
//                    return true;
//                }
//            }
//        }
//        else
//        {
//            if(t0 > t_max)
//            {
//                return false;
//            }
//            else
//            {
////                t = t0;
//                return true;
//            }
//        }
//        
//        return false;
    }
};

class BVHNode
{
public:
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    BVHBound bound;
    std::vector<unsigned int> faceList;
    
    bool IsLeaf()
    {
        return (left == nullptr && right == nullptr);
    }
};

class MeshBVHNew
{
public:
	MeshBVHNew(Mesh* triObj)
	{
		mesh = triObj;
		BuildRoot();
	}

	BVHNode* GetRoot()
	{
		return root;
	}

private:

	void BuildRoot()
	{
		// divide by middle
		// choose widest side from x y z
		root = new BVHNode();
		const auto& meshBound = mesh->aabb;
		root->bound = BVHBound(meshBound.pmin.x, meshBound.pmin.y, meshBound.pmin.z, meshBound.pmax.x, meshBound.pmax.y, meshBound.pmax.z);

		// add all face to root node
		for (unsigned int i = 0; i < mesh->faces.size(); i++)
		{
			root->faceList.push_back(i);
		}

		BuildNode(root);
	}

	bool MiddleSplit(std::vector<unsigned int>& leftFaceList,
		std::vector<unsigned int>& rightFaceList,
		BVHBound& leftBound,
		BVHBound& rightBound,
		BVHNode* parent)
	{
		Vec3f lengthVec = (parent->bound.GetMax() - parent->bound.GetMin());
		int maxIndex = lengthVec.MaxIndex();
		float middle = (parent->bound.data[3 + maxIndex] + parent->bound.data[maxIndex]) * 0.5f;

		for (size_t i = 0; i < parent->faceList.size(); i++)
		{
			unsigned int faceId = parent->faceList[i];
			auto& face = mesh->faces[faceId];

			// should this face add into left node
			bool left = false;

			Vec3f sum = Vec3f(0.0f, 0.0f, 0.0f);

			for (int index = 0; index < 3; index++)
			{
				int verticeIndex = face.indices[index];
				const glm::vec3& vertex = mesh->vertices[verticeIndex];
				Vec3f pos =  Vec3f(vertex.x, vertex.y, vertex.z);
				sum += pos;
			}

			Vec3f avg = sum / 3.0f;

			if (avg[maxIndex] <= middle)
			{
				left = true;
			}

			for (int index = 0; index < 3; index++)
			{
				int verticeIndex = face.indices[index];
				const glm::vec3& vertex = mesh->vertices[verticeIndex];
				Vec3f pos = Vec3f(vertex.x, vertex.y, vertex.z);

				if (left)
				{
					leftBound.UpdateByPoint(pos);
				}
				else
				{
					rightBound.UpdateByPoint(pos);
				}
			}

			if (left)
			{
				leftFaceList.push_back(faceId);
			}
			else
			{
				rightFaceList.push_back(faceId);
			}
		}

		float totalSurfaceArea = parent->bound.SurfaceArea();
		// possibility to hit based on surface area size
		float leftSuraceArea = leftBound.SurfaceArea();
		float rightSurfaceArea = rightBound.SurfaceArea();
		float pLeft = leftSuraceArea / totalSurfaceArea;
		float pRight = rightSurfaceArea / totalSurfaceArea;

		if (leftFaceList.size() == 0)
		{
			pLeft = 0.0f;
		}

		if (rightFaceList.size() == 0)
		{
			pRight = 0.0f;
		}

		float parentAsLeafTime = parent->faceList.size() * 1.0f;
		float parentAsInternalTime = 1.0f + pLeft * leftFaceList.size() + pRight * rightFaceList.size();

		if (parentAsLeafTime > parentAsInternalTime)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ScanLineSplit(std::vector<unsigned int>& leftFaceList,
		std::vector<unsigned int>& rightFaceList,
		BVHBound& leftBound,
		BVHBound& rightBound,
		BVHNode* parent)
	{
		float parentAsLeafTime = parent->faceList.size() * 1.0f;
		float parentAsInternalTime = BIGFLOAT;

		Vec3f lengthVec = (parent->bound.GetMax() - parent->bound.GetMin());

		// segment length
		float step = 0.02;
		for (int axisIndex = 0; axisIndex <= 2; axisIndex++)
		{
			for (float splitFactor = 0.0f; splitFactor < 1.0f; splitFactor += step)
			{
				std::vector<unsigned int> currentLeftList;
				std::vector<unsigned int> currentRightList;
				BVHBound currentLeftBound;
				BVHBound currentRightBound;

				float splitPos = parent->bound.data[axisIndex] + lengthVec[axisIndex] * splitFactor;

				for (size_t i = 0; i < parent->faceList.size(); i++)
				{
					unsigned int faceId = parent->faceList[i];
					auto& face = mesh->faces[faceId];

					// should this face add into left node
					bool left = false;

					Vec3f sum = Vec3f(0.0f, 0.0f, 0.0f);

					for (int index = 0; index < 3; index++)
					{
						int verticeIndex = face.indices[index];
						const glm::vec3& vertex = mesh->vertices[verticeIndex];
						Vec3f pos = Vec3f(vertex.x, vertex.y, vertex.z);
						sum += pos;
					}

					Vec3f avg = sum / 3.0f;

					if (avg[axisIndex] <= splitPos)
					{
						left = true;
					}

					for (int index = 0; index < 3; index++)
					{
						int verticeIndex = face.indices[index];
						const glm::vec3& vertex = mesh->vertices[verticeIndex];
						Vec3f pos = Vec3f(vertex.x, vertex.y, vertex.z);

						if (left)
						{
							currentLeftBound.UpdateByPoint(pos);
						}
						else
						{
							currentRightBound.UpdateByPoint(pos);
						}
					}

					if (left)
					{
						currentLeftList.push_back(faceId);
					}
					else
					{
						currentRightList.push_back(faceId);
					}
				}

				float totalSurfaceArea = parent->bound.SurfaceArea();
				// possibility to hit based on surface area size
				float leftSuraceArea = currentLeftBound.SurfaceArea();
				float rightSurfaceArea = currentRightBound.SurfaceArea();
				float pLeft = leftSuraceArea / totalSurfaceArea;
				float pRight = rightSurfaceArea / totalSurfaceArea;

				if (currentLeftList.size() == 0)
				{
					pLeft = 0.0f;
				}

				if (currentRightList.size() == 0)
				{
					pRight = 0.0f;
				}

				float currentInternalTime = 1.0f + pLeft * currentLeftList.size() + pRight * currentRightList.size();

				if (currentInternalTime < parentAsInternalTime)
				{
					parentAsInternalTime = currentInternalTime;
					rightFaceList = currentRightList;
					leftFaceList = currentLeftList;
					leftBound = currentLeftBound;
					rightBound = currentRightBound;
				}
			}
		}

		if (parentAsLeafTime > parentAsInternalTime)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void BuildNode(BVHNode* parent)
	{
		if (parent->faceList.size() == 0 || parent->faceList.size() == 1)
		{
			assert(parent->IsLeaf());
			return;
		}

		std::vector<unsigned int> leftFaceList;
		std::vector<unsigned int> rightFaceList;

		BVHBound leftBound = BVHBound();
		BVHBound rightBound = BVHBound();

		// continue, divided into two leaves still could get better performance
		if (ScanLineSplit(leftFaceList, rightFaceList, leftBound, rightBound, parent))
			//        if(RandomSplit(leftFaceList, rightFaceList, leftBound, rightBound, parent))
		{
			assert(leftFaceList.size() + rightFaceList.size() == parent->faceList.size());
			parent->left = new BVHNode();
			parent->left->bound = leftBound;
			parent->left->faceList = leftFaceList;
			BuildNode(parent->left);

			parent->right = new BVHNode();
			parent->right->bound = rightBound;
			parent->right->faceList = rightFaceList;
			BuildNode(parent->right);
		}
	}

	Mesh* mesh;
	BVHNode* root;

};
















class MeshBVH
{
public:
    MeshBVH(TriObj* triObj)
    {
        mesh = triObj;
        BuildRoot();
    }
    
    BVHNode* GetRoot()
    {
        return root;
    }
    
private:
    
    void BuildRoot()
    {
        // divide by middle
        // choose widest side from x y z
        root = new BVHNode();
        auto meshBound = mesh->GetBoundBox();
        root->bound = BVHBound(meshBound.pmin.x, meshBound.pmin.y, meshBound.pmin.z, meshBound.pmax.x, meshBound.pmax.y, meshBound.pmax.z);
        
        // add all face to root node
        for(unsigned int i = 0; i < mesh->NF(); i++)
        {
            root->faceList.push_back(i);
        }
        
        BuildNode(root);
    }
    
    bool MiddleSplit(std::vector<unsigned int>& leftFaceList,
                        std::vector<unsigned int>& rightFaceList,
                        BVHBound& leftBound,
                        BVHBound& rightBound,
                        BVHNode* parent)
    {
        Vec3f lengthVec = (parent->bound.GetMax() - parent->bound.GetMin());
        int maxIndex = lengthVec.MaxIndex();
        float middle = (parent->bound.data[3 + maxIndex] + parent->bound.data[maxIndex]) * 0.5f;
        
        for(size_t i = 0; i < parent->faceList.size(); i++)
        {
            unsigned int faceId = parent->faceList[i];
            auto& face = mesh->F(faceId);
            
            // should this face add into left node
            bool left = false;
            
            Vec3f sum = Vec3f(0.0f, 0.0f, 0.0f);
            
            for(int index = 0; index < 3; index++)
            {
                int verticeIndex = face.v[index];
                Vec3f pos = mesh->V(verticeIndex);
                sum += pos;
            }
            
            Vec3f avg = sum/3.0f;
            
            if(avg[maxIndex] <= middle)
            {
                left = true;
            }
            
            for(int index = 0; index < 3; index++)
            {
                int verticeIndex = face.v[index];
                Vec3f pos = mesh->V(verticeIndex);
                
                if(left)
                {
                    leftBound.UpdateByPoint(pos);
                }
                else
                {
                    rightBound.UpdateByPoint(pos);
                }
            }
            
            if(left)
            {
                leftFaceList.push_back(faceId);
            }
            else
            {
                rightFaceList.push_back(faceId);
            }
        }
        
        float totalSurfaceArea = parent->bound.SurfaceArea();
        // possibility to hit based on surface area size
        float leftSuraceArea = leftBound.SurfaceArea();
        float rightSurfaceArea = rightBound.SurfaceArea();
        float pLeft = leftSuraceArea / totalSurfaceArea;
        float pRight = rightSurfaceArea / totalSurfaceArea;
        
        if(leftFaceList.size() == 0)
        {
            pLeft = 0.0f;
        }
        
        if(rightFaceList.size() == 0)
        {
            pRight = 0.0f;
        }
        
        float parentAsLeafTime = parent->faceList.size() * 1.0f;
        float parentAsInternalTime = 1.0f + pLeft * leftFaceList.size() + pRight * rightFaceList.size();
        
        if(parentAsLeafTime > parentAsInternalTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool ScanLineSplit(std::vector<unsigned int>& leftFaceList,
                        std::vector<unsigned int>& rightFaceList,
                        BVHBound& leftBound,
                        BVHBound& rightBound,
                        BVHNode* parent)
    {
        float parentAsLeafTime = parent->faceList.size() * 1.0f;
        float parentAsInternalTime = BIGFLOAT;
        
        Vec3f lengthVec = (parent->bound.GetMax() - parent->bound.GetMin());
        
        // segment length
        float step = 0.02;
        for(int axisIndex = 0; axisIndex <= 2; axisIndex++)
        {
            for(float splitFactor = 0.0f; splitFactor < 1.0f; splitFactor+=step)
            {
                std::vector<unsigned int> currentLeftList;
                std::vector<unsigned int> currentRightList;
                BVHBound currentLeftBound;
                BVHBound currentRightBound;
                
                float splitPos = parent->bound.data[axisIndex] + lengthVec[axisIndex] * splitFactor;
                
                for(size_t i = 0; i < parent->faceList.size(); i++)
                {
                    unsigned int faceId = parent->faceList[i];
                    auto& face = mesh->F(faceId);
                    
                    // should this face add into left node
                    bool left = false;
                    
                    Vec3f sum = Vec3f(0.0f, 0.0f, 0.0f);
                    
                    for(int index = 0; index < 3; index++)
                    {
                        int verticeIndex = face.v[index];
                        Vec3f pos = mesh->V(verticeIndex);
                        sum += pos;
                    }
                    
                    Vec3f avg = sum/3.0f;
                    
                    if(avg[axisIndex] <= splitPos)
                    {
                        left = true;
                    }
                    
                    for(int index = 0; index < 3; index++)
                    {
                        int verticeIndex = face.v[index];
                        Vec3f pos = mesh->V(verticeIndex);
                        
                        if(left)
                        {
                            currentLeftBound.UpdateByPoint(pos);
                        }
                        else
                        {
                            currentRightBound.UpdateByPoint(pos);
                        }
                    }
                    
                    if(left)
                    {
                        currentLeftList.push_back(faceId);
                    }
                    else
                    {
                        currentRightList.push_back(faceId);
                    }
                }
                
                float totalSurfaceArea = parent->bound.SurfaceArea();
                // possibility to hit based on surface area size
                float leftSuraceArea = currentLeftBound.SurfaceArea();
                float rightSurfaceArea = currentRightBound.SurfaceArea();
                float pLeft = leftSuraceArea / totalSurfaceArea;
                float pRight = rightSurfaceArea / totalSurfaceArea;
                
                if(currentLeftList.size() == 0)
                {
                    pLeft = 0.0f;
                }
                
                if(currentRightList.size() == 0)
                {
                    pRight = 0.0f;
                }
                
                float currentInternalTime = 1.0f + pLeft * currentLeftList.size() + pRight * currentRightList.size();
                
                if(currentInternalTime < parentAsInternalTime)
                {
                    parentAsInternalTime = currentInternalTime;
                    rightFaceList = currentRightList;
                    leftFaceList = currentLeftList;
                    leftBound = currentLeftBound;
                    rightBound = currentRightBound;
                }
            }
        }
        
        if(parentAsLeafTime > parentAsInternalTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    void BuildNode(BVHNode* parent)
    {
        if(parent->faceList.size() == 0 || parent->faceList.size() == 1)
        {
            assert(parent->IsLeaf());
            return;
        }
        
        std::vector<unsigned int> leftFaceList;
        std::vector<unsigned int> rightFaceList;
        
        BVHBound leftBound = BVHBound();
        BVHBound rightBound = BVHBound();
        
        // continue, divided into two leaves still could get better performance
        if(ScanLineSplit(leftFaceList, rightFaceList, leftBound, rightBound, parent))
//        if(RandomSplit(leftFaceList, rightFaceList, leftBound, rightBound, parent))
        {
            assert(leftFaceList.size() + rightFaceList.size() == parent->faceList.size());
            parent->left = new BVHNode();
            parent->left->bound = leftBound;
            parent->left->faceList = leftFaceList;
            BuildNode(parent->left);
            
            parent->right = new BVHNode();
            parent->right->bound = rightBound;
            parent->right->faceList = rightFaceList;
            BuildNode(parent->right);
        }
    }
    
    TriObj* mesh;
    BVHNode* root;
    
};

class BVHManager
{
public:
	MeshBVHNew* Get(const std::string& path)
	{
		auto it = bvhDic.find(path);

		if (it != bvhDic.end())
		{
			//element found;
			return it->second;
		}

		return nullptr;
	}

	void Set(const std::string& path, MeshBVHNew* item)
	{
		bvhDic[path] = item;
	}

private:
	std::map<std::string, MeshBVHNew*> bvhDic;
};

#endif
