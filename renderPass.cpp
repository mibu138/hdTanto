//
// Copyright 2020 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "renderPass.h"
#include <cstring>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/base/gf/matrix4f.h>

#include <iostream>

extern "C" {
#include "tanto/t_utils.h"
};

static Tanto_Timer timer;

PXR_NAMESPACE_OPEN_SCOPE
    

HdTantoPass::HdTantoPass(
    HdRenderIndex *index,
    HdRprimCollection const &collection,
    HdTantoRenderer& renderer)
    : HdRenderPass(index, collection),
    _renderer(renderer),
    _width(0), _height(0),
    _aovBindings(),
    _colorBuffer(SdfPath::EmptyPath())
{
    tanto_TimerInit(&timer);
}

HdTantoPass::~HdTantoPass()
{
    std::cout << "Destroying renderPass" << std::endl;
}

static bool initialized = false;

void
HdTantoPass::_Execute(
    HdRenderPassStateSharedPtr const& renderPassState,
    TfTokenVector const &renderTags)
{
    tanto_TimerStart(&timer);
    std::cout << "=> Execute RenderPass" << std::endl;

    GfVec4f vp = renderPassState->GetViewport();
    std::cout << "Viewport: " << vp << '\n';

    HdRenderPassAovBindingVector bindings =
        renderPassState->GetAovBindings();

    if (_width != vp[2] || _height != vp[3]) {
        _width = vp[2];
        _height = vp[3];

        printf("Viewport size changed.\n");

        HdTantoRenderBuffer* cb = static_cast<HdTantoRenderBuffer*>(bindings[0].renderBuffer);
        if (!initialized)
        {
            _renderer.Initialize(_width, _height);

            _renderer.UpdateRender(cb);

            initialized = true;
        }

        else
        {
            _renderer.UpdateViewport(_width, _height, cb);
        }
    }

    // Determine whether we need to update the renderer AOV bindings.
    //
    // It's possible for the passed in bindings to be empty, but that's
    // never a legal state for the renderer, so if that's the case we add
    // a color and depth aov.
    //
    // If the renderer AOV bindings are empty, force a bindings update so that
    // we always get a chance to add color/depth on the first time through.
    std::cout << "Bindings size: " << bindings.size() << '\n';

    const GfMatrix4f view = (GfMatrix4f)renderPassState->GetWorldToViewMatrix();
    const GfMatrix4f proj = (GfMatrix4f)renderPassState->GetProjectionMatrix();

    _renderer.SetCamera(view, proj);

    HdTantoRenderBuffer* rb = static_cast<HdTantoRenderBuffer*>(bindings[0].renderBuffer);
    rb->Map();
    _renderer.Render(NULL);
    rb->Unmap();
    tanto_TimerStop(&timer);
    tanto_PrintTime(&timer);
    //    //_renderThread->StopRender();
    //
    //        HdRenderPassAovBinding colorAov;
    //        colorAov.aovName = HdAovTokens->color;
    //        colorAov.renderBuffer = &_colorBuffer;
    //        colorAov.clearValue =
    //            VtValue(GfVec4f(0.0707f, 0.0707f, 0.0707f, 1.0f));
    //        aovBindings.push_back(colorAov);
    //        HdRenderPassAovBinding depthAov;
    //        depthAov.aovName = HdAovTokens->depth;
    //        depthAov.renderBuffer = &_depthBuffer;
    //        depthAov.clearValue = VtValue(1.0f);
    //        aovBindings.push_back(depthAov);
    //    }
    //    //_renderer->SetAovBindings(aovBindings);
    //    // In general, the render thread clears aov bindings, but make sure
    //    // they are cleared initially on this thread.
    //    //_renderer->Clear();
    //    //needStartRender = true;
    //}

    //TF_VERIFY(!_aovBindings.empty(), "No aov bindings to render into");
}

PXR_NAMESPACE_CLOSE_SCOPE
