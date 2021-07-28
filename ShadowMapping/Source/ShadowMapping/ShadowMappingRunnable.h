#pragma once

#include "Core/Runnable.h"
#include "Math/Vector.h"

namespace fw
{
    class FrameBufferObject;
    class FWCore;
    class ImGuiManager;
    class MyMatrix;
    class ShaderProgram;
}

class ShadowMappingRunnable : public fw::Runnable
{
public:
    ShadowMappingRunnable(fw::FWCore* pFramework);
    virtual ~ShadowMappingRunnable();

    void Init();
    virtual void StartFrame(float deltaTime);
    virtual void Update(float deltaTime);
    virtual void Draw();

protected:
    void DrawScene(fw::ShaderProgram* pShader, fw::MyMatrix& cameraViewProj, fw::MyMatrix* pShadowViewProj);

protected:
    fw::ImGuiManager* m_pImGuiManager;

    // FBOs.
    fw::FrameBufferObject* m_pSceneFBO;
    fw::FrameBufferObject* m_pDepthFBO;

    // VBO.
    GLuint m_ShapeVBO;
    int m_NumVerts;

    // Shaders.
    fw::ShaderProgram* m_pDepthShader;
    fw::ShaderProgram* m_pWhiteWithShadowShader;

    // Movement.
    vec3 m_LightPosition;
    float m_LightOrbitRadius;
    float m_LightAngle;
    float m_LightSpeed;

    vec3 m_CameraPosition;
    float m_CameraOrbitRadius;    
    float m_CameraAngle;
    float m_CameraSpeed;
};
