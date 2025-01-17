![Image](https://github.com/WeakKnight/PathTracer/blob/master/imgs/readme/bullet.gif?raw=true)
![Image](https://github.com/WeakKnight/PathTracer/blob/master/imgs/readme/editor.gif?raw=true)
![Image](https://github.com/WeakKnight/PathTracer/blob/master/imgs/readme/whiteblack.png?raw=true)

# Feature
1. Unidirectional PathTracer
2. Texture Mapping (Albedo, Metallic, Roughness, Normal maps)
3. Rectangular Area Lights
4. Progressive Renderer
5. BVH Acceleration Structures
6. Tone Mapping
7. Debug GUI

# Build Instruction
## macOS
1. mkdir build
2. cd build
3. mkdir macos
4. cd macos
5. cmake -G  "Xcode" ../../

## Windows
1. mkdir build
2. cd build
3. mkdir windows
4. cd windows
5. cmake -G "Visual Studio 14 Win64" ../../

## Unix(Not Tested)
1. mkdir build
2. cd build
3. mkdir unix
4. cd unix
5. cmake ../../

## Thirdparty Library

Library                                     | Functionality         
------------------------------------------  | -------------
[assimp](https://github.com/assimp/assimp)  | Mesh Loading And Pre Processing
[glfw](https://github.com/glfw/glfw)        | Windowing And Input Handling
[glm](https://github.com/g-truc/glm)        | Mathematics
[imgui](https://github.com/ocornut/imgui)    | GUI
[spdlog](https://github.com/gabime/spdlog)   | Debug Logging
[glad](https://github.com/Dav1dde/glad)   | OpenGL Loader





