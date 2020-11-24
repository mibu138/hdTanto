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
#include "renderDelegate.h"
#include "mesh.h"
#include "renderPass.h"
#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include "renderBuffer.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

const TfTokenVector HdTantoDelegate::SUPPORTED_RPRIM_TYPES =
{
    HdPrimTypeTokens->mesh,
};

const TfTokenVector HdTantoDelegate::SUPPORTED_SPRIM_TYPES =
{
    HdPrimTypeTokens->camera
};

const TfTokenVector HdTantoDelegate::SUPPORTED_BPRIM_TYPES =
{
    HdPrimTypeTokens->renderBuffer
};

HdTantoDelegate::HdTantoDelegate()
    : HdRenderDelegate()
{
    _Initialize();
}

HdTantoDelegate::HdTantoDelegate(
    HdRenderSettingsMap const& settingsMap)
    : HdRenderDelegate(settingsMap)
{
    _Initialize();
}

void
HdTantoDelegate::_Initialize()
{
    std::cout << "Creating Tanto RenderDelegate" << std::endl;
    _resourceRegistry = std::make_shared<HdResourceRegistry>();
}

HdTantoDelegate::~HdTantoDelegate()
{
    _resourceRegistry.reset();
    std::cout << "Destroying Tanto RenderDelegate" << std::endl;
}

TfTokenVector const&
HdTantoDelegate::GetSupportedRprimTypes() const
{
    return SUPPORTED_RPRIM_TYPES;
}

TfTokenVector const&
HdTantoDelegate::GetSupportedSprimTypes() const
{
    return SUPPORTED_SPRIM_TYPES;
}

TfTokenVector const&
HdTantoDelegate::GetSupportedBprimTypes() const
{
    return SUPPORTED_BPRIM_TYPES;
}

HdResourceRegistrySharedPtr
HdTantoDelegate::GetResourceRegistry() const
{
    return _resourceRegistry;
}

void 
HdTantoDelegate::CommitResources(HdChangeTracker *tracker)
{
    std::cout << "=> CommitResources RenderDelegate" << std::endl;
}

HdRenderPassSharedPtr 
HdTantoDelegate::CreateRenderPass(
    HdRenderIndex *index,
    HdRprimCollection const& collection)
{
    std::cout << "Create RenderPass with Collection=" 
        << collection.GetName() << std::endl; 

    return HdRenderPassSharedPtr(new HdTantoPass(index, collection, _renderer));  
}

HdRprim *
HdTantoDelegate::CreateRprim(TfToken const& typeId,
                                    SdfPath const& rprimId,
                                    SdfPath const& instancerId)
{
    std::cout << "Create Tanto Rprim type=" << typeId.GetText() 
        << " id=" << rprimId 
        << " instancerId=" << instancerId 
        << std::endl;

    if (typeId == HdPrimTypeTokens->mesh) {
        return new HdTantoMesh(rprimId, instancerId);
    } else {
        TF_CODING_ERROR("Unknown Rprim type=%s id=%s", 
            typeId.GetText(), 
            rprimId.GetText());
    }
    return nullptr;
}

void
HdTantoDelegate::DestroyRprim(HdRprim *rPrim)
{
    std::cout << "Destroy Tanto Rprim id=" << rPrim->GetId() << std::endl;
    delete rPrim;
}

HdSprim *
HdTantoDelegate::CreateSprim(TfToken const& typeId,
                                    SdfPath const& sprimId)
{
    if (typeId == HdPrimTypeTokens->camera)
        return new HdCamera(sprimId);
    TF_CODING_ERROR("Unknown Sprim type=%s id=%s", 
        typeId.GetText(), 
        sprimId.GetText());
    return nullptr;
}

HdSprim *
HdTantoDelegate::CreateFallbackSprim(TfToken const& typeId)
{
    if (typeId == HdPrimTypeTokens->camera)
        return new HdCamera(SdfPath::EmptyPath());
    TF_CODING_ERROR("Creating unknown fallback sprim type=%s", 
        typeId.GetText()); 
    return nullptr;
}

void
HdTantoDelegate::DestroySprim(HdSprim *sPrim)
{
    delete sPrim;
}

HdBprim *
HdTantoDelegate::CreateBprim(TfToken const& typeId, SdfPath const& bprimId)
{
    if (typeId == HdPrimTypeTokens->renderBuffer) 
        return new HdTantoRenderBuffer(bprimId);
    TF_CODING_ERROR("Unknown Bprim type=%s id=%s", 
        typeId.GetText(), 
        bprimId.GetText());
    return nullptr;
}

HdBprim *
HdTantoDelegate::CreateFallbackBprim(TfToken const& typeId)
{
    if (typeId == HdPrimTypeTokens->renderBuffer) 
        return new HdTantoRenderBuffer(SdfPath::EmptyPath());
    TF_CODING_ERROR("Creating unknown fallback bprim type=%s", 
        typeId.GetText()); 
    return nullptr;
}

void
HdTantoDelegate::DestroyBprim(HdBprim *bPrim)
{
    delete bPrim;
}

HdInstancer *
HdTantoDelegate::CreateInstancer(
    HdSceneDelegate *delegate,
    SdfPath const& id,
    SdfPath const& instancerId)
{
    TF_CODING_ERROR("Creating Instancer not supported id=%s instancerId=%s", 
        id.GetText(), instancerId.GetText());
    return nullptr;
}

void 
HdTantoDelegate::DestroyInstancer(HdInstancer *instancer)
{
    TF_CODING_ERROR("Destroy instancer not supported");
}

HdRenderParam *
HdTantoDelegate::GetRenderParam() const
{
    return nullptr;
}

HdAovDescriptor
HdTantoDelegate::GetDefaultAovDescriptor(TfToken const& name) const
{
    if (name == HdAovTokens->color) {
        return HdAovDescriptor(HdFormatUNorm8Vec4, true,
                               VtValue(GfVec4f(0.0f)));
    } else if (name == HdAovTokens->normal || name == HdAovTokens->Neye) {
        return HdAovDescriptor(HdFormatFloat32Vec3, false,
                               VtValue(GfVec3f(-1.0f)));
    } else if (name == HdAovTokens->depth) {
        return HdAovDescriptor(HdFormatFloat32, false, VtValue(1.0f));
    } else if (name == HdAovTokens->cameraDepth) {
        return HdAovDescriptor(HdFormatFloat32, false, VtValue(0.0f));
    } else if (name == HdAovTokens->primId ||
               name == HdAovTokens->instanceId ||
               name == HdAovTokens->elementId) {
        return HdAovDescriptor(HdFormatInt32, false, VtValue(-1));
    } else {
        HdParsedAovToken aovId(name);
        if (aovId.isPrimvar) {
            return HdAovDescriptor(HdFormatFloat32Vec3, false,
                                   VtValue(GfVec3f(0.0f)));
        }
    }

    return HdAovDescriptor();
}

PXR_NAMESPACE_CLOSE_SCOPE
