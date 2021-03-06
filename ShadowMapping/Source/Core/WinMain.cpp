//
// Copyright (c) 2021 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "PCH.h"

#include <Windows.h>
#include "Core/FWCore.h"
#include "Core/Runnable.h"
#include "ShadowMapping/ShadowMappingRunnable.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    fw::FWCore* pFramework = new fw::FWCore();
    pFramework->Init( 1100, 600 );

    ShadowMappingRunnable* pRunnable = new ShadowMappingRunnable( pFramework );
    pRunnable->Init();

    pFramework->Run( pRunnable );
    pFramework->Shutdown();

    delete pRunnable;
    delete pFramework;
}
