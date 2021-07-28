//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FrameBufferObject_H__
#define __FrameBufferObject_H__

namespace fw {

class FrameBufferObject
{
protected:
    bool m_HasValidResources;
    bool m_FullyLoaded;

    bool m_FailedToInit;
    bool m_OnlyFreeOnShutdown;

    GLuint m_ColorTextureID;
    GLuint m_DepthTextureID;
    GLuint m_FrameBufferID;

    unsigned int m_RequestedWidth; // Size requested, mainly used by glViewport call.
    unsigned int m_RequestedHeight;

    unsigned int m_TextureWidth; // Will be power of 2 bigger than requested width/height.
    unsigned int m_TextureHeight;

    int m_MinFilter;
    int m_MagFilter;

    bool m_NeedColorTexture;
    int m_DepthBits;
    bool m_DepthIsTexture;

protected:
    void Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, bool depthReadable);
    void Invalidate(bool cleanGLAllocs);
    bool Create();

public:
    FrameBufferObject(unsigned int width, unsigned int height, int minFilter = GL_NEAREST, int magFilter = GL_NEAREST, bool depthReadable = true);
    virtual ~FrameBufferObject();

    bool IsFullyLoaded() { return m_FullyLoaded; }
    bool Resize(unsigned int width, unsigned int height);

    void Bind();
    void Unbind();

    GLuint GetFramebufferHandle() { return m_FrameBufferID; }
    GLuint GetColorTextureHandle() { return m_ColorTextureID; }
    GLuint GetDepthTextureHandle() { return m_DepthTextureID; }

    unsigned int GetRequestedWidth() { return m_RequestedWidth; }
    unsigned int GetRequestedHeight() { return m_RequestedHeight; }

    unsigned int GetTextureWidth() { return m_TextureWidth; }
    unsigned int GetTextureHeight() { return m_TextureHeight; }

    float GetWidthRatio() { return (float)m_RequestedWidth / m_TextureWidth; }
    float GetHeightRatio() { return (float)m_RequestedHeight / m_TextureHeight; }
};

} // namespace fw

#endif //__FrameBufferObject_H__
