#include "objects.h"
#include <vector>
#include "bvh.h"
#include "GLFW/glfw3.h"
#include "float.h"

extern float buildTime;
extern Camera camera;

bool TriObj::TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, BVHNode* node) const
{
    if(node->IsLeaf())
    {
        bool result = false;
        for(unsigned i = 0; i < node->faceList.size(); i++)
        {
            unsigned faceId = node->faceList[i];
            HitInfo currentHitInfo;
            if(IntersectTriangle(ray, currentHitInfo, hitSide, faceId))
            {
                if(currentHitInfo.z < hInfo.z)
                {
                    result = true;
                    
                    hInfo.N = currentHitInfo.N;
                    hInfo.p = currentHitInfo.p;
                    hInfo.z = currentHitInfo.z;
                    hInfo.front = currentHitInfo.front;
                    hInfo.uvw = currentHitInfo.uvw;
                    hInfo.mtlID = currentHitInfo.mtlID;
                    hInfo.duvw[0] = currentHitInfo.duvw[0];
                    hInfo.duvw[1] = currentHitInfo.duvw[1];
                }
            }
        }
        return result;
    }
    else
    {
        if(node->bound.IntersectRay(ray))
        {
            HitInfo leftHitInfo;
            HitInfo rightHitInfo;
            bool hitLeft = TraceBVHNode(ray, leftHitInfo, hitSide, node->left);
            bool hitRight = TraceBVHNode(ray, rightHitInfo, hitSide, node->right);
            
            if (hitLeft && hitRight) {
                if (leftHitInfo.z < rightHitInfo.z)
                {
                    hInfo.N = leftHitInfo.N;
                    hInfo.p = leftHitInfo.p;
                    hInfo.z = leftHitInfo.z;
                    hInfo.front = leftHitInfo.front;
                    hInfo.uvw = leftHitInfo.uvw;
                    hInfo.mtlID = leftHitInfo.mtlID;
                    hInfo.duvw[0] = leftHitInfo.duvw[0];
                    hInfo.duvw[1] = leftHitInfo.duvw[1];
                }
                else
                {
                    hInfo.N = rightHitInfo.N;
                    hInfo.p = rightHitInfo.p;
                    hInfo.z = rightHitInfo.z;
                    hInfo.front = rightHitInfo.front;
                    hInfo.uvw = rightHitInfo.uvw;
                    hInfo.mtlID = rightHitInfo.mtlID;
                    hInfo.duvw[0] = rightHitInfo.duvw[0];
                    hInfo.duvw[1] = rightHitInfo.duvw[1];
                }
                return true;
            }
            else if (hitLeft)
            {
                hInfo.N = leftHitInfo.N;
                hInfo.p = leftHitInfo.p;
                hInfo.z = leftHitInfo.z;
                hInfo.front = leftHitInfo.front;
                hInfo.uvw = leftHitInfo.uvw;
                hInfo.mtlID = leftHitInfo.mtlID;
                hInfo.duvw[0] = leftHitInfo.duvw[0];
                hInfo.duvw[1] = leftHitInfo.duvw[1];
                return true;
            }
            else if (hitRight)
            {
                hInfo.N = rightHitInfo.N;
                hInfo.p = rightHitInfo.p;
                hInfo.z = rightHitInfo.z;
                hInfo.front = rightHitInfo.front;
                hInfo.uvw = rightHitInfo.uvw;
                hInfo.mtlID = rightHitInfo.mtlID;
                hInfo.duvw[0] = rightHitInfo.duvw[0];
                hInfo.duvw[1] = rightHitInfo.duvw[1];
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}


bool TriObj::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    if(!GetBoundBox().IntersectRay(ray, BIGFLOAT))
    {
        return false;
    }

    return TraceBVHNode(ray, hInfo, hitSide, bvh->GetRoot());
}

bool TriObj::Load(char const *filename, bool loadMtl)
{
    if(! LoadFromFileObj(filename, loadMtl))
    {
        if(! LoadFromFileObj( StringUtils::Format("assets/%s", filename).c_str() , loadMtl))
        {
            return false;
        }
    }
    if ( ! HasNormals() ) ComputeNormals();
    ComputeBoundingBox();
    
    if(bvh != nullptr)
    {
        delete bvh;
    }
    float now = glfwGetTime();
    bvh = new MeshBVH(this);
    float then = glfwGetTime();
    buildTime += (then - now);
   
    return true;
}

bool TriObj::IntersectTriangle( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int faceID) const
{
    const TriFace& face = F(faceID);
    
    const Vec3f& v0 = V(face.v[0]);
    const Vec3f& v1 = V(face.v[1]);
    const Vec3f& v2 = V(face.v[2]);
    
    Vec3f v01 = v1 - v0;
    Vec3f v02 = v2 - v0;
    
    // unnormalized
    Vec3f n = v01.Cross(v02);
    
    // (ray.origin + ray.dir * t - v0).dot(n) = 0
    float dirDotN = ray.dir.Dot(n);
    
    if(abs(dirDotN) <= FLT_EPSILON)
    {
        return false;
    }
    
    float originDotN = ray.p.Dot(n);
    float v0DotN = v0.Dot(n);
    float t = (v0DotN - originDotN) / dirDotN;
    
    if(t < 0.0f)
    {
        return false;
    }
    
    bool isFront = (dirDotN < 0.0f);
    
    if(hitSide == HIT_FRONT && !isFront)
    {
        return false;
    }
    
    if(hitSide == HIT_BACK && isFront)
    {
        return false;
    }
    
    Vec3f p = ray.p + ray.dir * t;
    
    Matrix3<float> mm = Matrix3<float>(-ray.dir, v01, v02);
    mm.Invert();
    
    Vec3f vv0 = mm * v0;
    Vec3f vv1 = mm * v1;
    Vec3f vv2 = mm * v2;
    Vec3f pp = mm * p;
    
    Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
    Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
    Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
    Vec2f p_2d = Vec2f(pp.y, pp.z);
    
    Vec2f pv0 = v0_2d - p_2d;
    Vec2f pv1 = v1_2d - p_2d;
    Vec2f pv2 = v2_2d - p_2d;
    
    float two_a0 = pv1.Cross(pv2);
    if(two_a0 <= 0.0f)
    {
        return false;
    }
    float two_a1 = pv2.Cross(pv0);
    if(two_a1 <= 0.0f)
    {
        return false;
    }
    float two_a2 = pv0.Cross(pv1);
    if(two_a2 <= 0.0f)
    {
        return false;
    }
    
    float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);
    
    float beta0 = two_a0 / two_a;
    float beta1 = two_a1 / two_a;
    float beta2 = two_a2 / two_a;
    
    const TriFace& normalFace = FN(faceID);
    
    const Vec3f& n0 = VN(normalFace.v[0]);
    const Vec3f& n1 = VN(normalFace.v[1]);
    const Vec3f& n2 = VN(normalFace.v[2]);
    
    Vec3f normal = (beta0 * n0 + beta1 * n1 + beta2 * n2).GetNormalized();
    
    const TriFace& texFace = FT(faceID);
    
    const Vec3f& t0 = VT(texFace.v[0]);
    const Vec3f& t1 = VT(texFace.v[1]);
    const Vec3f& t2 = VT(texFace.v[2]);
    
    Vec3f tex = beta0 * t0 + beta1 * t1 + beta2 * t2;
    
    hInfo.p = p;
    hInfo.z = t;
    hInfo.N = isFront? normal: -1.0f * normal;
    hInfo.front = isFront;
    hInfo.uvw = tex;
    
    return true;
}

void TriObj::ViewportDisplay(const Material *mtl) const
{
    
}

Vec3f CalculatePlaneTexCoord(const Vec3f &p){
    return p * 0.5f + Vec3f(0.5f, 0.5f, 0.0f);
};

bool Plane::IntersectRay(RayContext const &rayContext, HitInfo& hInfo, int hitSide) const
{
    if(IntersectRay(rayContext.cameraRay, hInfo, hitSide))
    {
        // RAY DIFF
    
        const Ray& rightRay = rayContext.rightRay;
        const Ray& topRay = rayContext.topRay;
        HitInfo rightHitInfo;
        HitInfo topHitInfo;
        if(IntersectRay(rightRay, rightHitInfo, hitSide)
           && IntersectRay(topRay, topHitInfo, hitSide))
        {
            hInfo.duvw[0] = (CalculatePlaneTexCoord(rightHitInfo.p) - hInfo.uvw) / rayContext.delta;
            hInfo.duvw[1] = (CalculatePlaneTexCoord(topHitInfo.p) - hInfo.uvw) / rayContext.delta;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool Plane::IntersectRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
{
    Vec3f n = Vec3f(0.0f, 0.0f, 1.0f);
    float dirDotN = ray.dir.Dot(n);
    
    if(abs(dirDotN - 0.0f) < 0.000001f)
    {
        return false;
    }
    
    float originDotN = ray.p.Dot(n);
    float t = -1.0f * (originDotN / dirDotN);
    
    if(t <= 0.0f)
    {
        return false;
    }
    
    Vec3f p = ray.p + ray.dir * t;
    
    if(abs(p.x) >= 1.0f)
    {
        return false;
    }
    
    if(abs(p.y) >= 1.0f)
    {
        return false;
    }
    
    bool isFront = (dirDotN < 0.0f);
    
    if(hitSide == HIT_FRONT && !isFront)
    {
        return false;
    }
    
    if(hitSide == HIT_BACK && isFront)
    {
        return false;
    }
    
    hInfo.front = isFront;
    hInfo.N = isFront? n : -1.0f * n;
    hInfo.p = p;
    hInfo.z = t;
    
    hInfo.uvw = CalculatePlaneTexCoord(p);

    return true;
}

void Plane::ViewportDisplay(const Material *mtl) const
{
    
}

bool Sphere::IntersectRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
{ 
    // |ray.p + ray.dir * l| = 1
    // ray.dir.LengthSquared() * l * l + 2 * ray.p.Dot(ray.dir) * l + ray.p.LengthSquared() - 1 = 0
    float a = ray.dir.LengthSquared();
    float b = 2 * ray.p.Dot(ray.dir);
    float c = ray.p.LengthSquared() - 1;
    
    float delta = b*b - 4 * a * c;
    
    if(delta < 0)
    {
        return false;
    }
    else
    {
        if(delta == 0)
        {
            float dist = -1.0f * b / (2.0f * a);
            Vec3f intersectPoint = ray.p + ray.dir * dist;
            
            // if hit in front of the eye, it should be transparent, just like lights go through
            if(dist < 0.0f)
            {
                return false;
            }
            else
            {
                if(hitSide == HIT_FRONT || hitSide == HIT_FRONT_AND_BACK)
                {
                    if(dist < hInfo.z)
                    {
                        hInfo.z = dist;
                        hInfo.N = intersectPoint; // obj space
//                        hInfo.N.Normalize();
                        hInfo.p = intersectPoint; // obj space
                        // behind the image plane and only one root, it is in the tangent plane, must be front
                        hInfo.front = true;
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        // Delta > 0, double roots
        else
        {
            float dist1 = (-1.0f * b - sqrtf(delta))/(2.0f * a);
            float dist2 = (-1.0f * b + sqrtf(delta))/(2.0f * a);
            
            Vec3f intersectPoint1 = ray.p + ray.dir * dist1;
            Vec3f intersectPoint2 = ray.p + ray.dir * dist2;
            
            if(dist1 < 0.0f)
            {
                if(dist2 < 0.0f)
                {
                    return false;
                }
                else
                {
                    if(hitSide == HIT_BACK || hitSide == HIT_FRONT_AND_BACK)
                    {
                        if(dist2 < hInfo.z)
                        {
                            hInfo.z = dist2;
                            hInfo.N = -1.0f * intersectPoint2;
//                            hInfo.N.Normalize();
                            hInfo.p = intersectPoint2;
                            hInfo.front = false;
                        }

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                if(hitSide == HIT_FRONT || hitSide == HIT_FRONT_AND_BACK)
                {
                    if(dist1 < hInfo.z)
                    {
                        hInfo.z = dist1;
                        hInfo.N = intersectPoint1;
//                        hInfo.N.Normalize();
                        hInfo.p = intersectPoint1;
                        hInfo.front = true;
                    }

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        return true;
    }
}

void Sphere::ViewportDisplay(const Material *mtl) const
{
}
    
