#include "renderer.h"

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

void HdTantoRenderer::SetCamera(const GfMatrix4f* viewMatrix, const GfMatrix4f* projMatrix)
{
    Mat4* view = (Mat4*)viewMatrix;
    Mat4* proj = (Mat4*)projMatrix;

    Tanto_Camera camera = {
        .view = *view,
        .proj = *proj,
    };

    r_UpdateCamera(camera);
}

void HdTantoRenderer::Render(HdRenderThread *renderThread)
{
    r_Render();
}

PXR_NAMESPACE_CLOSE_SCOPE
