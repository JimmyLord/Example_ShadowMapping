#include "PCH.h"

#include "../../Libraries/imgui/imgui.h"
#include "Core/FWCore.h"
#include "GL/GLExtensions.h"
#include "GLHelpers/FrameBufferObject.h"
#include "GLHelpers/ShaderProgram.h"
#include "UI/ImGuiManager.h"
#include "Math/MyMatrix.h"

#include "ShadowMappingRunnable.h"

// Small demo app for the basics of shadow mapping.
// This doesn't actually calculate the light intensity
//     so produces many artifacts in places light wouldn't normally reach.

ShadowMappingRunnable::ShadowMappingRunnable(fw::FWCore* pFramework)
    : Runnable( pFramework )
{
    m_pImGuiManager = nullptr;
}

ShadowMappingRunnable::~ShadowMappingRunnable()
{
    delete m_pSceneFBO;
    delete m_pDepthFBO;
    delete m_pDepthShader;
    delete m_pWhiteWithShadowShader;

    delete m_pImGuiManager;
}

void ShadowMappingRunnable::Init()
{
    m_pImGuiManager = new fw::ImGuiManager( m_pFramework );
    m_pImGuiManager->Init();

    glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
    glEnable( GL_DEPTH_TEST );

    // Init our FBOs and Shaders.
    m_pSceneFBO = new fw::FrameBufferObject( 640, 360 );
    m_pDepthFBO = new fw::FrameBufferObject( 256, 256 );
    m_pDepthShader = new fw::ShaderProgram( "Data/DepthOnly.vert", "Data/DepthOnly.frag" );
    m_pWhiteWithShadowShader = new fw::ShaderProgram( "Data/WhiteWithShadow.vert", "Data/WhiteWithShadow.frag" );

    // Create a simple cube VBO. (only front and top - 6 triangles).
    int numTriangles = 4;
    m_NumVerts = numTriangles * 3;
    float cubeVerts[] = {
        -1,-1,-1,   -1, 1,-1,   1,-1,-1,   // Front BL
        -1, 1,-1,    1, 1,-1,   1,-1,-1,   // Front TR
        -1, 1,-1,   -1, 1, 1,   1, 1,-1,   // Top BL
        -1, 1, 1,    1, 1, 1,   1, 1,-1,   // Top TR
    };
    glGenBuffers( 1, &m_ShapeVBO );
    glBindBuffer( GL_ARRAY_BUFFER, m_ShapeVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float)*3*m_NumVerts, cubeVerts, GL_STATIC_DRAW );

    // Setup the light.
    m_LightPosition.Set( 0, 0, 0 );
    m_LightOrbitRadius = 5.0f;
    m_LightAngle = 0.0f;
    m_LightSpeed = 1.0f;
    
    // Setup the camera.
    m_CameraPosition.Set( 0, 0, 0 );
    m_CameraOrbitRadius = 15.0f;
    m_CameraAngle = PI / 4;
    m_CameraSpeed = 0.0f;
}

void ShadowMappingRunnable::StartFrame(float deltaTime)
{
    m_pImGuiManager->StartFrame( deltaTime );
}

void ShadowMappingRunnable::Update(float deltaTime)
{
    m_LightAngle += m_LightSpeed * deltaTime;
    m_CameraAngle += m_CameraSpeed * deltaTime;

    m_LightPosition.Set( cosf(m_LightAngle)*m_LightOrbitRadius, 3.5f, sinf(m_LightAngle)*m_LightOrbitRadius );
    m_CameraPosition.Set( cosf(m_CameraAngle)*m_CameraOrbitRadius, 8.0f, sinf(m_CameraAngle)*m_CameraOrbitRadius );

    ImGui::Begin( "Controls" );
    ImGui::SliderFloat( "Camera Angle", &m_CameraAngle, 0, PI*2 );
    ImGui::SliderFloat( "Camera Speed", &m_CameraSpeed, 0, -5 );

    ImGui::SliderFloat( "Shadow Caster Angle", &m_LightAngle, 0, PI*2 );
    ImGui::SliderFloat( "Shadow Caster Speed", &m_LightSpeed, 0, 5 );
    ImGui::End();
}

void ShadowMappingRunnable::Draw()
{
    fw::MyMatrix shadowViewProjMat;

    // Draw the scene from the light pov (depth only) into the DepthFBO.
    // Not really depth only since the FBO has a color buffer, but it's ignored.
    {
        m_pDepthFBO->Bind();

        glViewport( 0, 0, m_pDepthFBO->GetRequestedWidth(), m_pDepthFBO->GetRequestedHeight() );
        glClear( GL_DEPTH_BUFFER_BIT );

        fw::MyMatrix shadowViewMat;
        fw::MyMatrix shadowProjMat;
        
        // Setup a view and proj matrix for the light's pov.
        shadowViewMat.CreateLookAtView( m_LightPosition, vec3(0,1,0), vec3(0,0,0) );
        shadowProjMat.CreateOrtho( -10, 10, -10, 10, 0, 100 );
        shadowViewProjMat = shadowProjMat * shadowViewMat;
        
        // Draw the scene from the light pov.
        // 2nd param is null since this won't receive shadows.
        DrawScene( m_pDepthShader, shadowViewProjMat, nullptr );

        m_pDepthFBO->Unbind();
    }

    // Draw the scene into the SceneFBO.
    {
        m_pSceneFBO->Bind();

        glViewport( 0, 0, m_pSceneFBO->GetRequestedWidth(), m_pSceneFBO->GetRequestedHeight() );
        glClearColor( 0.0f, 0.2f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
        fw::MyMatrix viewMat;
        fw::MyMatrix projMat;
        
        // Setup the main camera.
        viewMat.CreateLookAtView( m_CameraPosition, vec3(0,1,0), vec3(0,0,0) );
        projMat.CreatePerspectiveVFoV( 45.0f, 640.0f/360.0f, 0.01f, 100.0f );
        
        // Draw from the main camera pov.
        // 2nd param is the light's viewProj matrix, needed for shadow test.
        DrawScene( m_pWhiteWithShadowShader, projMat * viewMat, &shadowViewProjMat );

        m_pSceneFBO->Unbind();
    }

    // Draw to the screen buffer, only imgui drawlist gets drawn here.
    {
        glViewport( 0, 0, m_pFramework->GetWindowWidth(), m_pFramework->GetWindowHeight() );
        glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        ImGui::Begin( "Scene" );
        ImVec2 uv0 = ImVec2(0, m_pSceneFBO->GetHeightRatio());
        ImVec2 uv1 = ImVec2(m_pSceneFBO->GetWidthRatio(), 0);
        ImGui::Image( (ImTextureID)(uintptr_t)m_pSceneFBO->GetColorTextureHandle(), ImVec2(640,360), uv0, uv1 );
        ImGui::End();

        ImGui::Begin( "Depth" );
        uv0 = ImVec2(0, m_pDepthFBO->GetHeightRatio());
        uv1 = ImVec2(m_pDepthFBO->GetWidthRatio(), 0);
        ImGui::Image( (ImTextureID)(uintptr_t)m_pDepthFBO->GetDepthTextureHandle(), ImVec2(256,256), uv0, uv1 );
        ImGui::End();

        m_pImGuiManager->EndFrame();
    }
}

void ShadowMappingRunnable::DrawScene(fw::ShaderProgram* pShader, fw::MyMatrix& cameraViewProj, fw::MyMatrix* pShadowViewProj)
{
    // Enable our shader.
    GLuint program = pShader->GetProgram();
    glUseProgram( program );

    // Get the attritube and uniform locations.
    GLint locPosition = glGetAttribLocation( program, "a_Position" );
    GLint locWVP = glGetUniformLocation( program, "u_WVPTransform" );
    GLint locColor = glGetUniformLocation( program, "u_Color" );

    GLint locShadowLightTransform = glGetUniformLocation( program, "u_ShadowLightTransform" );
    GLint locDepthTexture = glGetUniformLocation( program, "u_DepthTexture" );

    // Select our shape and setup attributes.
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_ShapeVBO );
        glEnableVertexAttribArray( locPosition );
        glVertexAttribPointer( locPosition, 3, GL_FLOAT, false, 0, 0 );
    }

    // Setup shadow depth texture uniform.
    {
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, m_pDepthFBO->GetDepthTextureHandle() );
        glUniform1i( locDepthTexture, 0 );
    }

    // Draw a large white floor cube.
    {
        // Enable our program and setup uniforms.
        fw::MyMatrix worldMat;
        worldMat.CreateSRT( vec3(8,1,5), 0, vec3(0,-3.0f,-2.5f) );
        fw::MyMatrix wvp = cameraViewProj * worldMat;

        glUniformMatrix4fv( locWVP, 1, false, &wvp.m11 );
        glUniform4f( locColor, 1,1,1,1 );

        // Setup shadow WVP.
        if( pShadowViewProj != nullptr )
        {
            fw::MyMatrix wvp = *pShadowViewProj * worldMat;
            glUniformMatrix4fv( locShadowLightTransform, 1, false, &wvp.m11 );
        }

        // Draw.
        glDrawArrays( GL_TRIANGLES, 0, m_NumVerts );
    }

    // Draw a small blue cube.
    {
        // Enable our program and setup uniforms.
        fw::MyMatrix worldMat;
        worldMat.CreateSRT( 1, 0, 0 );
        fw::MyMatrix wvp = cameraViewProj * worldMat;

        // Setup shadow WVP.
        if( pShadowViewProj != nullptr )
        {
            fw::MyMatrix wvp = *pShadowViewProj * worldMat;
            glUniformMatrix4fv( locShadowLightTransform, 1, false, &wvp.m11 );
        }

        glUniformMatrix4fv( locWVP, 1, false, &wvp.m11 );
        glUniform4f( locColor, 0,0,1,1 );

        // Draw.
        glDrawArrays( GL_TRIANGLES, 0, m_NumVerts );
    }
}
