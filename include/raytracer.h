#pragma once
#include <memory>
#include <string>
#include "cyVector.h"
#include "scene.h"

class Node;

class Texture2D;

class RayTracer
{
    public:

    void Init();
    void Run();
    void UpdateRenderResult();
    void WriteToFile();
    
    std::shared_ptr<Texture2D> GetZBufferTexture(){return zbufferTexture;}
    std::shared_ptr<Texture2D> GetRenderTexture(){return renderTexture;}
    std::shared_ptr<Texture2D> GetTimeTexture(){return timeTexture;}
    char scene_path[256] = "assets/project2.xml";
    
private:
    std::shared_ptr<Texture2D> zbufferTexture;
    std::shared_ptr<Texture2D> renderTexture;
    std::shared_ptr<Texture2D> timeTexture;
};

