#include "renderer.h"
#include <cstring>
#include <iostream>
#include <pxr/base/gf/matrix4f.h>

extern "C" 
{
#include "tantoren/render.h"
#include "tantoren/common.h"
#include <tanto/v_video.h>
#include <tanto/d_display.h>
#include <tanto/r_render.h>
#include <tanto/t_utils.h>
#include <tanto/i_input.h>
}

PXR_NAMESPACE_OPEN_SCOPE

HdTantoRenderer::HdTantoRenderer()
{
    tanto_v_config.rayTraceEnabled = true;
#ifndef NDEBUG
    tanto_v_config.validationEnabled = true;
#else
    tanto_v_config.validationEnabled = false;
#endif
    tanto_v_Init();
    r_InitScene();
}

HdTantoRenderer::~HdTantoRenderer()
{
}

void HdTantoRenderer::Initialize(unsigned int width, unsigned int height)
{
    r_SetViewport(width, height);
    r_InitRenderer();
}

void HdTantoRenderer::UpdateViewport(unsigned int width, unsigned int height,
        HdTantoRenderBuffer* colorBuffer)
{
    r_UpdateViewport(width, height, colorBuffer->GetBufferRegion());
}

void HdTantoRenderer::UpdateRender(HdTantoRenderBuffer* colorBuffer)
{
    r_UpdateRenderCommands(colorBuffer->GetBufferRegion());
}

void HdTantoRenderer::SetCamera(const GfMatrix4f& viewMatrix, const GfMatrix4f& projMatrix)
{
    //GfMatrix4f viewT = viewMatrix.GetTranspose();
    //GfMatrix4f projT = viewMatrix.GetTranspose();
    Mat4* view = (Mat4*)(viewMatrix.data());
    Mat4* proj = (Mat4*)(projMatrix.data());

    //std::cout << "View0: \n" << viewMatrix << '\n';
    //std::cout << "Proj0: \n" << projMatrix << '\n';

    //proj->x[1][1] *= -1;

    //printf("View1:\n");
    //printMat4(view);
    //printf("Proj1:\n");
    //printMat4(proj);

    Tanto_Camera camera = {
        .view = *view,
        .proj = *proj,
    };

    r_UpdateCamera(camera);
}

//void HdTantoRenderer::SetPrimTransform(const GfMatrix4f& xform)
//{
//    Mat4* m = (Mat4*)(xform.data());
//    r_UpdatePrimTransform(*m);
//}
//
//void HdTantoRenderer::UpdatePrim(Tanto_R_Primitive prim)
//{
//    printf("6\n");
//    r_UpdatePrimitive(prim);
//}

Tanto_PrimId HdTantoRenderer::AddPrim(PrimData data)
{
    const std::lock_guard<std::mutex> lock(mutexAddPrim);

    Tanto_R_Primitive prim = tanto_r_CreatePrimitive(data.points.size(), data.indices.size() * 3, 2);
    //printf("3\n");
    memcpy(prim.vertexRegion.hostData, data.points.data(), prim.vertexCount * sizeof(Tanto_R_Attribute));
    //printf("4\n");
    memcpy(prim.indexRegion.hostData,  data.indices.data(), prim.indexCount * sizeof(Tanto_R_Index));
    Vec3* nIter = (Vec3*)(prim.vertexRegion.hostData + prim.attrOffsets[1]);
    for (int i = 0; i < prim.vertexCount; i++) 
    {
        *nIter++ = *(Vec3*)data.color->data();
    }
    
    Mat4* transform = (Mat4*)data.xform.data();

    Tanto_R_Material mat;
    std::cout << "Data.Color: " << *data.color << '\n';
    if (data.color)
    {
        mat.color.x[0] = (*data.color)[0];
        mat.color.x[1] = (*data.color)[1];
        mat.color.x[2] = (*data.color)[2];
    }
    else
    {
        mat.color.x[0] = 0.5;
        mat.color.x[1] = 0.5;
        mat.color.x[2] = 0.5;
    }
    mat.color.x[3] = 1;

    return r_AddNewPrim(prim, mat, *transform);
}

void HdTantoRenderer::Render(HdRenderThread *renderThread)
{
    r_Render();
}

PXR_NAMESPACE_CLOSE_SCOPE
