#ifndef VIEWER_R_COMMANDS_H
#define VIEWER_R_COMMANDS_H

#include <tanto/r_geo.h>
#include "common.h"

typedef struct {
    int foo;
    int bar;
} PushConstants;

typedef struct {
    Mat4 matModel;
    Mat4 matView;
    Mat4 matProj;
} UniformBuffer;

void  r_InitRenderer();
void  r_SetViewport(unsigned int width, unsigned int height);
void  r_UpdateRenderCommands(Tanto_V_BufferRegion* colorBuffer);
void  r_LoadMesh(Tanto_R_Mesh mesh);
void  r_ClearMesh(void);
void  r_CleanUp(void);
void  r_RecreateSwapchain(void);
const Tanto_R_Mesh* r_GetMesh(void);

#endif /* end of include guard: R_COMMANDS_H */
