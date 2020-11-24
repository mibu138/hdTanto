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
}

HdTantoRenderer::~HdTantoRenderer()
{
}

void HdTantoRenderer::Initialize()
{
    tanto_v_config.rayTraceEnabled = true;
#ifndef NDEBUG
    tanto_v_config.validationEnabled = true;
#else
    tanto_v_config.validationEnabled = false;
#endif
    tanto_v_Init();
    r_InitRenderer();
}

PXR_NAMESPACE_CLOSE_SCOPE
