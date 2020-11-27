#ifndef RENDERER_H
#define RENDERER_H

#include <pxr/pxr.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/imaging/hd/renderThread.h>

#include "renderBuffer.h"

extern "C" 
{
#include <tanto/r_geo.h>
#include "tantoren/render.h"
}

PXR_NAMESPACE_OPEN_SCOPE

/// \class HdEmbreeRenderer
///
/// HdEmbreeRenderer implements a renderer on top of Embree's raycasting
/// abilities.  This is currently a very simple renderer.  It breaks the
/// framebuffer into tiles for multithreading; sends out jittered camera
/// rays; and implements the following shading:
///  - Colors via the "color" primvar.
///  - Lighting via N dot Camera-ray, simulating a point light at the camera
///    origin.
///  - Ambient occlusion.
///
class HdTantoRenderer final {
public:
    /// Renderer constructor.
    HdTantoRenderer();

    /// Renderer destructor.
    ~HdTantoRenderer();

    /// Specify a new viewport size for the sample/color buffer.
    ///   \param width The new viewport width.
    ///   \param height The new viewport height.
    void UpdateViewport(unsigned int width, unsigned int height,
        HdTantoRenderBuffer* colorBuffer);

    /// Set the camera to use for rendering.
    ///   \param viewMatrix The camera's world-to-view matrix.
    ///   \param projMatrix The camera's view-to-NDC projection matrix.
    void SetCamera(const GfMatrix4f& viewMatrix, const GfMatrix4f& projMatrix);
    
    void UpdateRender(HdTantoRenderBuffer* colorBuffer);

    Tanto_PrimId AddPrim(Tanto_R_Primitive prim, const GfMatrix4f& xform, const GfVec3f& color);

    /// Set the aov bindings to use for rendering.
    ///   \param aovBindings A list of aov bindings.
    void SetAovBindings(HdRenderPassAovBindingVector const &aovBindings);

    /// Get the aov bindings being used for rendering.
    ///   \return the current aov bindings.
    HdRenderPassAovBindingVector const& GetAovBindings() const {
        return _aovBindings;
    }

    void Render(HdRenderThread *renderThread);

    /// Clear the bound aov buffers (typically before rendering).
    void Clear();

    void Initialize(unsigned int width, unsigned int height);

private:
    HdRenderPassAovBindingVector _aovBindings;

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif /* end of include guard: RENDERER_H */
