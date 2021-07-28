//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "PCH.h"
#include <assert.h>
#include "GL/GLExtensions.h"
#include "FrameBufferObject.h"

namespace fw {

FrameBufferObject::FrameBufferObject(unsigned int width, unsigned int height, int minFilter, int magFilter, bool depthReadable)
{
    m_HasValidResources = false;
    m_FullyLoaded = false;
    m_FailedToInit = false;
    m_OnlyFreeOnShutdown = false;

    m_ColorTextureID = 0;
    m_DepthTextureID = 0;
    m_FrameBufferID = 0;

    m_RequestedWidth = 0;
    m_RequestedHeight = 0;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    m_MinFilter = GL_LINEAR;
    m_MagFilter = GL_LINEAR;

    m_NeedColorTexture = true;
    m_DepthBits = 32;
    m_DepthIsTexture = false;

    Setup(width, height, minFilter, magFilter, depthReadable);
    Create();
}

FrameBufferObject::~FrameBufferObject()
{
    Invalidate( true );
}

void FrameBufferObject::Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, bool depthReadable)
{
    assert( width <= 8192 );
    assert( height <= 8192 );

    unsigned int newTextureWidth = 0;
    unsigned int newTextureHeight = 0;

    // Loop from 64 to 8192 and find appropriate size.
    for( unsigned int pow = 6; pow <= 13; pow++ )
    {
        unsigned int powsize = (unsigned int)(1 << pow);

        if( powsize >= width && newTextureWidth == 0 )
            newTextureWidth = powsize;
        if( powsize >= height && newTextureHeight == 0 )
            newTextureHeight = powsize;
    }

    m_RequestedWidth = width;
    m_RequestedHeight = height;

    m_TextureWidth = newTextureWidth;
    m_TextureHeight = newTextureHeight;

    m_MinFilter = minFilter;
    m_MagFilter = magFilter;

    m_DepthBits = 32;
    m_DepthIsTexture = depthReadable;
}

bool FrameBufferObject::Create()
{
    if( glGenFramebuffers == 0 )
    {
        return false;
    }

    GLint maxsize;

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxsize);

    if( m_TextureWidth > (unsigned int)maxsize || m_TextureHeight > (unsigned int)maxsize )
    {
        // Requested size is too big.
        return false;
    }

    assert( m_FrameBufferID == 0 );

    // Get a framebuffer, render buffer and a texture from opengl.
    glGenFramebuffers( 1, &m_FrameBufferID );
    m_HasValidResources = true;

    if( m_NeedColorTexture )
    {
        glGenTextures( 1, &m_ColorTextureID );
        m_HasValidResources = true;
    }

    if( m_DepthBits != 0 )
    {
        assert( m_DepthBits == 16 || m_DepthBits == 24 || m_DepthBits == 32 );

        if( m_DepthIsTexture )
        {
            glGenTextures( 1, &m_DepthTextureID );
            m_HasValidResources = true;
        }
        else
        {
            glGenRenderbuffers( 1, &m_DepthTextureID );
            m_HasValidResources = true;
        }
    }

    // Create the texture.
    if( m_ColorTextureID )
    {
        glBindTexture( GL_TEXTURE_2D, m_ColorTextureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_TextureWidth, m_TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_MinFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_MagFilter );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    // Create a depth renderbuffer.
    if( m_DepthTextureID )
    {
        GLint depthformat = GL_DEPTH_COMPONENT32;

        if( m_DepthIsTexture )
        {
            glBindTexture( GL_TEXTURE_2D, m_DepthTextureID );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexImage2D( GL_TEXTURE_2D, 0, depthformat, m_TextureWidth, m_TextureHeight, 0,
                          GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
        else
        {
            glBindRenderbuffer( GL_RENDERBUFFER, m_DepthTextureID );
            glRenderbufferStorage( GL_RENDERBUFFER, depthformat, m_TextureWidth, m_TextureHeight );
        }
    }

    // Attach everything to the FBO.
    {
        glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID );

        // Attach color texture.
        if( m_ColorTextureID )
        {
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTextureID, 0 );
        }

        // Attach depth renderbuffer.
        if( m_DepthTextureID )
        {
            if( m_DepthIsTexture )
            {
                glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureID, 0 );
            }
            else
            {
                glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthTextureID );
            }
        }

        // Any problems?
        GLint status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
        if( status != GL_FRAMEBUFFER_COMPLETE )
        {
            Invalidate( true );
            return false;
        }

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    }

    m_FullyLoaded = true;

    return true;
}

bool FrameBufferObject::Resize(unsigned int width, unsigned int height)
{
    if( width < m_TextureWidth && height < m_TextureHeight )
    {
        m_RequestedWidth = width;
        m_RequestedHeight = height;
        return false;
    }
    else
    {
        Invalidate( true );
        Setup( width, height, m_MinFilter, m_MagFilter, m_DepthIsTexture );
        Create();
        return true;
    }
}

void FrameBufferObject::Bind()
{
    glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBufferID );
}

void FrameBufferObject::Unbind()
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void FrameBufferObject::Invalidate(bool cleanGLAllocs)
{
    if( m_HasValidResources == false )
        return;

    if( cleanGLAllocs )
    {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );

        if( m_ColorTextureID != 0 )
        {
            glDeleteTextures( 1, &m_ColorTextureID );
            m_ColorTextureID = 0;
        }

        if( m_DepthTextureID != 0 )
        {
            if( m_DepthIsTexture )
                glDeleteTextures( 1, &m_DepthTextureID );
            else
                glDeleteRenderbuffers( 1, &m_DepthTextureID );

            m_DepthTextureID = 0;
        }

        if( m_FrameBufferID != 0 )
        {
            glDeleteFramebuffers( 1, &m_FrameBufferID );
            m_FrameBufferID = 0;
        }
    }
    else
    {
        m_ColorTextureID = 0;
        m_DepthTextureID = 0;

        m_FrameBufferID = 0;
    }

    m_FullyLoaded = false;
    m_HasValidResources = false;
}

} // namespace fw
