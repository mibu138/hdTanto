#ifndef VIEWER_R_COMMANDS_H
#define VIEWER_R_COMMANDS_H

#include <tanto/r_geo.h>
#include "common.h"

typedef struct {
    Mat4 view;
    Mat4 proj;
} Tanto_Camera;

typedef struct {
    Vec4 color;
} Tanto_R_Material;

typedef uint16_t Tanto_PrimId;

void r_InitScene(void);
void r_InitRenderer(void);
void r_SetViewport(unsigned int width, unsigned int height);
void r_UpdateRenderCommands(Tanto_V_BufferRegion* colorBuffer);
void r_LoadMesh(Tanto_R_Mesh mesh);
void r_Render(void);
void r_ClearMesh(void);
void r_CleanUp(void);
void r_UpdateCamera(Tanto_Camera camera);
Tanto_PrimId r_AddNewPrim(Tanto_R_Primitive newPrim, Tanto_R_Material newMat, Mat4 xform);
void r_UpdateViewport(unsigned int width, unsigned int height,
        Tanto_V_BufferRegion* colorBuffer);
const Tanto_R_Mesh* r_GetMesh(void);

#endif /* end of include guard: R_COMMANDS_H */
