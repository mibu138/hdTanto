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
#include "mesh.h"
#include <cstring>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/flatNormals.h>

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

HdTantoMesh::HdTantoMesh(HdTantoRenderer& renderer, SdfPath const& id, SdfPath const& instancerId)
    : HdMesh(id, instancerId),
    _renderer(renderer)
{
}

HdDirtyBits
HdTantoMesh::GetInitialDirtyBitsMask() const
{
    return HdChangeTracker::Clean
        | HdChangeTracker::DirtyTransform
        | HdChangeTracker::InitRepr
        | HdChangeTracker::DirtyPoints
        | HdChangeTracker::DirtyTopology
        | HdChangeTracker::DirtyTransform
        | HdChangeTracker::DirtyVisibility
        | HdChangeTracker::DirtyCullStyle;
}

HdDirtyBits
HdTantoMesh::_PropagateDirtyBits(HdDirtyBits bits) const
{
    return bits;
}

void 
HdTantoMesh::_InitRepr(TfToken const &reprToken, HdDirtyBits *dirtyBits)
{
    TF_UNUSED(dirtyBits);

    std::cout << "_InitRepr. ReprToken: " << reprToken << '\n';

    // Create an empty repr.
    _ReprVector::iterator it = std::find_if(_reprs.begin(), _reprs.end(),
                                            _ReprComparator(reprToken));
    if (it == _reprs.end()) {
        _reprs.emplace_back(reprToken, HdReprSharedPtr());
    }
}

void
HdTantoMesh::Sync(HdSceneDelegate *sceneDelegate,
                   HdRenderParam   *renderParam,
                   HdDirtyBits     *dirtyBits,
                   TfToken const   &reprToken)
{
    std::cout << "* (multithreaded) Sync Tanto Mesh id=" << GetId() << std::endl;
    //
    // XXX: A mesh repr can have multiple repr decs; this is done, for example, 
    // when the drawstyle specifies different rasterizing modes between front
    // faces and back faces.
    // With raytracing, this concept makes less sense, but
    // combining semantics of two HdMeshReprDesc is tricky in the general case.
    // For now, HdEmbreeMesh only respects the first desc; this should be fixed.
    _MeshReprConfig::DescArray descs = _GetReprDesc(reprToken);
    const HdMeshReprDesc &desc = descs[0];

    // Pull top-level embree state out of the render param.
    // Create embree geometry objects.
    _PopulateTantoMesh(sceneDelegate, dirtyBits, desc);
}

void HdTantoMesh::_PopulateTantoMesh(HdSceneDelegate *sceneDelegate,
                         HdDirtyBits *dirtyBits,
                         HdMeshReprDesc const &desc)
{
    SdfPath const& id = GetId();

    ////////////////////////////////////////////////////////////////////////
    // 1. Pull scene data.

    bool topologyDirty  = false;
    bool pointsDirty    = false;
    bool transformDirty = false;

    if (HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, HdTokens->points)) 
    {
        VtValue value = GetPoints(sceneDelegate);
        _points = value.Get<VtVec3fArray>();
        std::cout << "Points dirty!!" << '\n';
        pointsDirty = true;
    }

    if (HdChangeTracker::IsTopologyDirty(*dirtyBits, id)) 
    {
        // When pulling a new topology, we don't want to overwrite the
        // refine level or subdiv tags, which are provided separately by the
        // scene delegate, so we save and restore them.
        PxOsdSubdivTags subdivTags = _topology.GetSubdivTags();
        int refineLevel = _topology.GetRefineLevel();
        _topology = HdMeshTopology(GetMeshTopology(sceneDelegate), refineLevel);
        _topology.SetSubdivTags(subdivTags);
        std::cout << "Topology dirty!!" << '\n';
        topologyDirty = true;
    }

    if (HdChangeTracker::IsTransformDirty(*dirtyBits, id)) 
    {
        _transform = GfMatrix4f(sceneDelegate->GetTransform(id));
        std::cout << "Transform dirty!!" << '\n';
        transformDirty = true;
        _renderer.SetPrimTransform(_transform);
    }

    if (pointsDirty && topologyDirty)
    {
        // must create a new prim
        const uint32_t pointCount = _points.size();
        std::cout << "Points size: " << pointCount << '\n';
        std::cout << "Points\n" << _points << '\n';
        HdMeshUtil meshUtil(&_topology, GetId());
        meshUtil.ComputeTriangleIndices(&_triangulatedIndices, &_trianglePrimitiveParams);
        VtValue triangulatedPosition;
        VtArray<GfVec3f> faceNormals = Hd_FlatNormals::ComputeFlatNormals(&_topology, _points.data());
        bool success = meshUtil.ComputeTriangulatedFaceVaryingPrimvar(faceNormals.data(), _points.size(), HdTypeFloatVec3, &triangulatedPosition);
        assert(success);
        std::cout << "Triangulated Normals : \n" << triangulatedPosition << '\n';
        std::cout << "Trangulated Indices, size: " << _triangulatedIndices.size() << "\n" << _triangulatedIndices << "\n";
        std::cout << "Trangulated Primitive Params, size: " << _trianglePrimitiveParams.size() << "\n" << _trianglePrimitiveParams << "\n";
        Tanto_R_Primitive prim = tanto_r_CreatePrimitive(pointCount, _triangulatedIndices.size() * 3, 2);
        printf("3\n");
        memcpy(prim.vertexRegion.hostData, _points.data(), prim.vertexCount * sizeof(Tanto_R_Attribute));
        printf("4\n");
        memcpy(prim.indexRegion.hostData,  _triangulatedIndices.data(), prim.indexCount * sizeof(Tanto_R_Index));
        Vec3* nIter = (Vec3*)(prim.vertexRegion.hostData + prim.attrOffsets[1]);
        for (int i = 0; i < prim.vertexCount; i++) 
        {
            *nIter++ = (Vec3){{0.5, 0.5, 0.5}};
        }
        printf("5\n");
        VtValue normals = GetNormals(sceneDelegate);
        std::cout << "Normals size: " << normals.GetArraySize() << '\n';
        printf("Normals!\n");
        std::cout << GetNormals(sceneDelegate) << '\n';
        _renderer.UpdatePrim(prim);
    }
}


PXR_NAMESPACE_CLOSE_SCOPE
