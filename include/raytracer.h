#pragma once
#include <memory>
#include "cyVector.h"
#include "scene.h"

class Node;

namespace RayTracing
{
    class TraceContext
    {
        public :
        std::vector<Node*> transformStack;
    };
    
    cyVec3f localToWorld(cyVec3f p, TraceContext* context);
    
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
        
    private:
        std::shared_ptr<Texture2D> zbufferTexture;
        std::shared_ptr<Texture2D> renderTexture;
    };
}
