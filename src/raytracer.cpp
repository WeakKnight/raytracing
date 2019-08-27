#include "raytracer.h"
#include "scene.h"

#include "texture2d.h"
#include "renderimagehelper.h"

#include "xmlload.h"

#include "objects.h"
#include <thread>
#include <vector>
#include <future>

#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"

namespace RayTracing
{
Node rootNode;
Camera camera;
RenderImage renderImage;
Sphere theSphere;

float imgPlaneHeight;
float imgPlaneWidth;

// pixel's world space size
float texelWdith;
float texelHeight;

cyVec3f cameraUp;
cyVec3f cameraFront;
cyVec3f cameraRight;

Color24 *myZImg = nullptr;


    Ray WorldToLocal(Ray r, TraceContext* context)
    {
        Ray result = r;
        for(size_t i = 0; i < context->transformStack.size(); i++)
        {
            Node* currentNode = context->transformStack[i];
            result = currentNode->ToNodeCoords(result);
        }
        return result;
    }
    
    cyVec3f localToWorld(cyVec3f p, TraceContext* context)
    {
        cyVec3f result = p;
        for(int i = context->transformStack.size() - 1; i >=0; i--)
        {
            Node* currentNode = context->transformStack[i];
            result = currentNode->TransformFrom(result);
        }
        return result;
    }
    
    bool TraceNode(HitInfo& hitInfo, Ray& ray, Node* node, TraceContext* context)
    {
        context->transformStack.push_back(node);
        
        bool result = false;
        
        
        
        Ray rayInNodeSpace = WorldToLocal(ray, context);
        
        HitInfo currentHitInfo;
        
        auto obj = node->GetNodeObj();
        
        if(obj != nullptr && obj->IntersectRay(rayInNodeSpace, currentHitInfo, context, HIT_FRONT))
        {
            result = true;
            if(currentHitInfo.z < hitInfo.z)
            {
                hitInfo.z = currentHitInfo.z;
                hitInfo.node = node;
                hitInfo.front = currentHitInfo.front;
            }
        }
        
        for(int index = 0; index < node->GetNumChild(); index++)
        {
            HitInfo currentHitInfo;
            Node* child = node->GetChild(index);
            if(TraceNode(currentHitInfo, ray, child, context))
            {
                result = true;
                
                if(currentHitInfo.z < hitInfo.z)
                {
                    hitInfo.z = currentHitInfo.z;
                    hitInfo.node = currentHitInfo.node;
                    hitInfo.front = currentHitInfo.front;
                }
            }
        }
        
        context->transformStack.pop_back();
        return result;
    }
    
    Ray GenCameraRay(int x, int y)
    {
        Ray result;
        
        result.p = camera.pos;
        result.dir =
        cameraRight * (-1.0f * imgPlaneWidth * 0.5f + x * texelWdith + 0.5f * texelWdith) +
        cameraUp * (imgPlaneHeight * 0.5f - y * texelHeight - 0.5f * texelHeight) +
        cameraFront
        - camera.pos;
        
        result.dir.Normalize();
        
        return result;
    }
    
    void RayTracer::Init()
    {
        renderTexture = std::make_shared<Texture2D>();
        zbufferTexture = std::make_shared<Texture2D>();
        
        // scene load, ini global variables
        LoadScene("assets/project1.xml");
        
        // make sure camera dir normalized
        camera.dir.Normalize();
        
        cameraUp = camera.up;
        cameraFront = camera.dir;
        cameraRight = cameraFront.Cross(cameraUp);
        
        imgPlaneHeight = 1.0f * tanf(camera.fov * 0.5f /180.0f * static_cast<float>(M_PI)) * 2.0f;
        imgPlaneWidth = imgPlaneHeight * static_cast<float>(camera.imgWidth) / static_cast<float>(camera.imgHeight);
        
        // pixel's world space size
        texelWdith = imgPlaneWidth / static_cast<float>(camera.imgWidth);
        texelHeight = imgPlaneHeight / static_cast<float>(camera.imgHeight);
    }

    void RayTracer::Run()
    {
        float now = glfwGetTime();
        
        std::size_t cores = std::thread::hardware_concurrency() - 1;
        std::vector<std::future<void>> threads;
        
        std::size_t size = renderImage.GetWidth() * renderImage.GetHeight();
        
        renderImage.ResetNumRenderedPixels();
        
        for(std::size_t i = 0; i < cores; i++)
        {
            threads.push_back(std::async([=](){
                auto traceContext = new TraceContext();
                
                for(std::size_t index = i; index < size; index+= cores)
                {
                    int y = index / renderImage.GetWidth();
                    int x = index - y * renderImage.GetWidth();
                    
                    HitInfo hitInfo;
                    Ray cameraRay = GenCameraRay(x, y);
                    bool sthTraced = TraceNode(hitInfo, cameraRay, &rootNode, traceContext);
                    if(sthTraced)
                    {
                        RenderImageHelper::SetPixel(renderImage, x, y, cyColor24::White());
                        RenderImageHelper::SetDepth(renderImage, x, y, hitInfo.z);
                    }
                    else
                    {
                        RenderImageHelper::SetPixel(renderImage, x, y, cyColor24::Black());
                        RenderImageHelper::SetDepth(renderImage, x, y, hitInfo.z);
                    }
                    
                    // done
                    if(index == size - 1)
                    {
                        renderImage.ComputeZBufferImage();
                        float finish = glfwGetTime();
                        spdlog::debug("time is {}", finish - now);
                    }
                }
                
                delete traceContext;
    //            for(int i = 0; i < renderImage.GetWidth(); i++)
    //            {
    //                for(int j = 0; j < renderImage.GetHeight(); j++)
    //                {
    //                    HitInfo hitInfo;
    //                    Ray cameraRay = GenCameraRay(i, j);
    //                    bool sthTraced = TraceNode(hitInfo, cameraRay, &rootNode);
    //                    if(sthTraced)
    //                    {
    //                        RenderImageHelper::SetPixel(renderImage, i, j, cyColor24::White());
    //                        RenderImageHelper::SetDepth(renderImage, i, j, hitInfo.z);
    //                    }
    //                    else
    //                    {
    //                        RenderImageHelper::SetPixel(renderImage, i, j, cyColor24::Black());
    //                        RenderImageHelper::SetDepth(renderImage, i, j, hitInfo.z);
    //                    }
    //                }
    //            }
            }));
        }
        
        
        
//        renderImage.ComputeZBufferImage();
        if(myZImg != nullptr)
        {
            delete [] myZImg;
        }
        
        myZImg = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
    }
    
    void RayTracer::UpdateRenderResult()
    {
        RenderImageHelper::CalculateMyDepthImg(myZImg, renderImage);

        zbufferTexture->SetData((unsigned char *)myZImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);

        renderTexture->SetData((unsigned char *)renderImage.GetPixels(), renderImage.GetWidth(), renderImage.GetHeight());
    }
    
    void RayTracer::WriteToFile()
    {
        renderImage.SaveImage("colorbuffer.png");
        renderImage.SaveZImage("zbuffer.png");
    }
}