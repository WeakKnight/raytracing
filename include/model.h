#pragma once

#include "mesh.h"
#include <float.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <spdlog/spdlog.h>

#include "utils.h"

class BVHNode;

class Model : public Object
{
public:
	Model(Mesh* meshes, unsigned int meshesNum)
		:meshes(meshes),
		meshesNum(meshesNum)
	{
		aabb.Init();

		for (int i = 0; i < meshesNum; i++)
		{
			auto mesh = meshes[i];

			aabb += mesh.aabb;
		}

		cdf = new CDF();
	}

	~Model()
	{
		if (meshes)
		{
			delete meshes;
		}

		delete cdf;
	}

	CDF* cdf;

	void BuildCDFAndArea()
	{
		for (int i = 0; i < meshesNum; i++)
		{
			auto mesh = meshes[i];
			mesh.CalculateAreaAndCDF();
			cdf->Add(mesh.area);
		}
	}

	virtual float Area() const 
	{

		Vec3f zero = parent->TransformPointToWorld(Vec3f(0.0f, 0.0f, 0.0f));
		Vec3f width = parent->TransformPointToWorld(Vec3f(1.0f, 0.0f, 0.0f));
		Vec3f height = parent->TransformPointToWorld(Vec3f(0.0f, 1.0f, 0.0f));

		float scaler = (width - zero).Length() * (height - zero).Length();

		return scaler * cdf->total;
	}

	virtual Interaction Sample() const
	{
		int meshId = cdf->Sample();
		auto mesh = meshes[meshId];
		Interaction it = mesh.Sample();
		TransformInteractionToWorld(it);
		return it;
	}

	bool TraceBVHNode(Ray const& ray, HitInfo& hInfo, int hitSide, Mesh& mesh, BVHNode* node) const;
	bool TraceBVHNode(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide, Mesh& mesh, BVHNode* node) const;

	virtual bool IntersectRay(Ray const& ray, HitInfo& hInfo, int hitSide = HIT_FRONT) const;

	virtual bool IntersectRay(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const;

	virtual Box  GetBoundBox() const
	{
		return aabb;
	}

	bool IntersectRayWithFace(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide, Mesh& mesh, Face& face) const
	{
		HitInfo& hInfo = hInfoContext.mainHitInfo;

		if (IntersectRayWithFace(rayContext.cameraRay, hInfo, hitSide, mesh, face))
		{
			const glm::vec3& glmV0 = mesh.vertices[face.indices[0]];
			const glm::vec3& glmV1 = mesh.vertices[face.indices[1]];
			const glm::vec3& glmV2 = mesh.vertices[face.indices[2]];

			Vec3f v0(glmV0.x, glmV0.y, glmV0.z);
			Vec3f v1(glmV1.x, glmV1.y, glmV1.z);
			Vec3f v2(glmV2.x, glmV2.y, glmV2.z);

			const glm::vec3& glmT0 = mesh.textureCoords[face.indices[0]];
			const glm::vec3& glmT1 = mesh.textureCoords[face.indices[1]];
			const glm::vec3& glmT2 = mesh.textureCoords[face.indices[2]];

			Vec3f t0(glmT0.x, glmT0.y, glmT0.z);
			Vec3f t1(glmT1.x, glmT1.y, glmT1.z);
			Vec3f t2(glmT2.x, glmT2.y, glmT2.z);

			Vec3f v01 = v1 - v0;
			Vec3f v02 = v2 - v0;

			const Ray& rightRay = rayContext.rightRay;
			const Ray& topRay = rayContext.topRay;

			HitInfo& rightInfo = hInfoContext.rightHitInfo;
			HitInfo& topInfo = hInfoContext.topHitInfo;

			const auto N = hInfo.N.GetNormalized();

			const auto zRight = (rightRay.p - hInfo.p).Dot(N);
			const auto tRight = zRight / (rightRay.dir.Dot(N));
			const auto pRight = rightRay.p + tRight * rightRay.dir;
			const auto nRight = N;

			const auto zTop = (topRay.p - hInfo.p).Dot(N);
			const auto tTop = zTop / (topRay.dir.Dot(N));
			const auto pTop = topRay.p + tTop * topRay.dir;
			const auto nTop = N;

			assert(!isnan(nTop.x));
			assert(!isnan(nRight.x));

			rightInfo.N = nRight;
			rightInfo.z = tRight;
			rightInfo.p = pRight;

			topInfo.N = nTop;
			topInfo.z = tTop;
			topInfo.p = pTop;

			Vec3f texRight;
			{
				Matrix3<float> mmRight = Matrix3<float>(-rightRay.dir, v01, v02);
				mmRight.Invert();

				Vec3f vv0 = mmRight * v0;
				Vec3f vv1 = mmRight * v1;
				Vec3f vv2 = mmRight * v2;
				Vec3f pp = mmRight * pRight;

				Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
				Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
				Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
				Vec2f p_2d = Vec2f(pp.y, pp.z);

				Vec2f pv0 = v0_2d - p_2d;
				Vec2f pv1 = v1_2d - p_2d;
				Vec2f pv2 = v2_2d - p_2d;

				float two_a0 = pv1.Cross(pv2);
				float two_a1 = pv2.Cross(pv0);
				float two_a2 = pv0.Cross(pv1);

				float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);

				float beta0 = two_a0 / two_a;
				float beta1 = two_a1 / two_a;
				float beta2 = two_a2 / two_a;

				texRight = beta0 * t0 + beta1 * t1 + beta2 * t2;
			}

			Vec3f texTop;
			{
				Matrix3<float> mmTop = Matrix3<float>(-topRay.dir, v01, v02);
				mmTop.Invert();

				Vec3f vv0 = mmTop * v0;
				Vec3f vv1 = mmTop * v1;
				Vec3f vv2 = mmTop * v2;
				Vec3f pp = mmTop * pTop;

				Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
				Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
				Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
				Vec2f p_2d = Vec2f(pp.y, pp.z);

				Vec2f pv0 = v0_2d - p_2d;
				Vec2f pv1 = v1_2d - p_2d;
				Vec2f pv2 = v2_2d - p_2d;

				float two_a0 = pv1.Cross(pv2);
				float two_a1 = pv2.Cross(pv0);
				float two_a2 = pv0.Cross(pv1);

				float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);

				float beta0 = two_a0 / two_a;
				float beta1 = two_a1 / two_a;
				float beta2 = two_a2 / two_a;

				texTop = beta0 * t0 + beta1 * t1 + beta2 * t2;
			}

			if (rayContext.hasDiff)
			{
				hInfo.duvw[0] = (texRight - hInfo.uvw) / rayContext.delta;
				hInfo.duvw[1] = (texTop - hInfo.uvw) / rayContext.delta;
			}
			else
			{
				hInfo.duvw[0] = Vec3f(0.0f, 0.0f, 0.0f);
				hInfo.duvw[1] = Vec3f(0.0f, 0.0f, 0.0f);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool IntersectRayWithFace(Ray const& ray, HitInfo& hInfo, int hitSide, Mesh& mesh, Face& face) const
	{
		const glm::vec3& v0 = mesh.vertices[face.indices[0]];
		const glm::vec3& v1 = mesh.vertices[face.indices[1]];
		const glm::vec3& v2 = mesh.vertices[face.indices[2]];

		glm::vec3 v01 = v1 - v0;
		glm::vec3 v02 = v2 - v0;

		// unnormalized
		glm::vec3 n = glm::cross(v01, v02);

		glm::vec3 rayDir = glm::vec3(ray.dir.x, ray.dir.y, ray.dir.z);
		glm::vec3 rayPos = glm::vec3(ray.p.x, ray.p.y, ray.p.z);

		// (ray.origin + ray.dir * t - v0).dot(n) = 0
		float dirDotN = glm::dot(rayDir, n);

		if (abs(dirDotN) <= FLT_EPSILON)
		{
			return false;
		}

		float originDotN = glm::dot(rayPos, n);
		float v0DotN = glm::dot(v0, n);
		float t = (v0DotN - originDotN) / dirDotN;

		if (t < 0.0f)
		{
			return false;
		}

		bool isFront = (dirDotN < 0.0f);

		if (hitSide == HIT_FRONT && !isFront)
		{
			return false;
		}

		if (hitSide == HIT_BACK && isFront)
		{
			return false;
		}

		glm::vec3 p = rayPos + rayDir * t;

		glm::mat3x3 mm(-rayDir, v01, v02);
		mm = glm::inverse(mm);

		glm::vec3 vv0 = mm * v0;
		glm::vec3 vv1 = mm * v1;
		glm::vec3 vv2 = mm * v2;
		glm::vec3 pp = mm * p;

		Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
		Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
		Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
		Vec2f p_2d = Vec2f(pp.y, pp.z);

		Vec2f pv0 = v0_2d - p_2d;
		Vec2f pv1 = v1_2d - p_2d;
		Vec2f pv2 = v2_2d - p_2d;

		float two_a0 = pv1.Cross(pv2);
		if (two_a0 <= 0.0f)
		{
			return false;
		}
		float two_a1 = pv2.Cross(pv0);
		if (two_a1 <= 0.0f)
		{
			return false;
		}
		float two_a2 = pv0.Cross(pv1);
		if (two_a2 <= 0.0f)
		{
			return false;
		}

		float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);

		float beta0 = two_a0 / two_a;
		float beta1 = two_a1 / two_a;
		float beta2 = two_a2 / two_a;

		const glm::vec3& n0 = mesh.normals[face.indices[0]];
		const glm::vec3& n1 = mesh.normals[face.indices[1]];
		const glm::vec3& n2 = mesh.normals[face.indices[2]];

		glm::vec3 normal = glm::normalize(beta0 * n0 + beta1 * n1 + beta2 * n2);
		float normalSum = normal.x + normal.y + normal.z;
		if (isnan(normalSum) || isinf(normalSum))
		{
			normal = glm::normalize(n);
		}

		const glm::vec3& t0 = mesh.textureCoords[face.indices[0]];
		const glm::vec3& t1 = mesh.textureCoords[face.indices[1]];
		const glm::vec3& t2 = mesh.textureCoords[face.indices[2]];

		glm::vec3 tangent = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 biTangent = glm::vec3(0.0f, 0.0f, 0.0f);

		if (face.indices.size() >= 4)
		{
			auto faceId = face.indices[3];
			tangent = mesh.tangents[faceId];
			biTangent = mesh.bitangents[faceId];
		}
		glm::vec3 tex = beta0 * t0 + beta1 * t1 + beta2 * t2;

		hInfo.p = Vec3f(p.x, p.y, p.z);
		hInfo.z = t;
		hInfo.N = isFront ? Vec3f(normal.x, normal.y, normal.z) : -1.0f * Vec3f(normal.x, normal.y, normal.z);
		hInfo.front = isFront;
		hInfo.uvw = Vec3f(tex.x, tex.y, tex.z);
		hInfo.Tangent = Vec3f(tangent.x, tangent.y, tangent.z);
		hInfo.Bitangent = Vec3f(biTangent.x, biTangent.y, biTangent.z);

		return true;
	}
	std::string path;

private:
	Box aabb;
	Mesh* meshes;
	unsigned int meshesNum;
};

class ModelLoader
{
public:
	Node* Load(const std::string& path, LightComponent* lightFromParent)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path,
			aiProcess_CalcTangentSpace
			| aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_SortByPType
			| aiProcess_GenBoundingBoxes
			| aiProcess_GenNormals
			//|aiProcess_MakeLeftHanded
		);

		// If the import failed, report it
		if (!scene)
		{
			spdlog::info("assimp: {}", importer.GetErrorString());
			return nullptr;
		}

		auto node = ProcessNode(scene, scene->mRootNode, path, 0, lightFromParent);

		// scene->mMeshes[scene->mRootNode->];
		return node;
	}

	Node* ProcessNode(const aiScene* scene, aiNode* node, const std::string& path, int order, LightComponent* lightFromParent)
	{
		Node* result = new Node();
		result->Init();

		result->SetLight(lightFromParent);

		int meshCount = node->mNumMeshes;
		int childCount = node->mNumChildren;
		auto transformation = node->mTransformation;

		cy::Matrix3<float> trans = Matrix3<float>(transformation.a1, transformation.a2, transformation.a3, transformation.b1, transformation.b2, transformation.b3, transformation.c1, transformation.c2, transformation.c3);
		result->Transform(trans);

		if (node->mNumMeshes > 0)
		{
			Mesh* myMeshes = new Mesh[meshCount];
			for (int i = 0; i < meshCount; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				Mesh myMesh;
				myMesh.path = StringUtils::Format("%s,order%d:%d", path.c_str(), order, i);
				myMesh.ProcessAssimpData(mesh);
				myMeshes[i] = myMesh;
			}

			Model* model = new Model(myMeshes, meshCount);
			model->path = StringUtils::Format("%s,order%d", path.c_str(), order);
			result->SetNodeObj(model);
		}

		for (int i = 0; i < childCount; i++)
		{
			aiNode* child = node->mChildren[i];
			Node* childNode = ProcessNode(scene, child, path, order + 1, lightFromParent);
			result->AppendChild(childNode);
		}

		return result;
	}
};
