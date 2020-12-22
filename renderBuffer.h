//
// Copyright 2018 Pixar
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
#ifndef TANTO_RENDER_BUFFER_H
#define TANTO_RENDER_BUFFER_H

#include "pxr/pxr.h"
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/imaging/hgiVulkan/hgi.h"

extern "C" 
{
#include <tanto/v_memory.h>
}

PXR_NAMESPACE_OPEN_SCOPE

class HdTantoRenderBuffer : public HdRenderBuffer
{
public:
    HdTantoRenderBuffer(SdfPath const& id);
    ~HdTantoRenderBuffer();

    /// Allocate a new buffer with the given dimensions and format.
    ///   \param dimensions   Width, height, and depth of the desired buffer.
    ///                       (Only depth==1 is supported).
    ///   \param format       The format of the desired buffer, taken from the
    ///                       HdFormat enum.
    ///   \param multisampled Whether the buffer is multisampled.
    ///   \return             True if the buffer was successfully allocated,
    ///                       false with a warning otherwise.
    virtual bool Allocate(GfVec3i const& dimensions,
                          HdFormat format,
                          bool multiSampled) override;

    /// Accessor for buffer width.
    ///   \return The width of the currently allocated buffer.
    virtual unsigned int GetWidth() const override { return _width; }

    /// Accessor for buffer height.
    ///   \return The height of the currently allocated buffer.
    virtual unsigned int GetHeight() const override { return _height; }

    /// Accessor for buffer depth.
    ///   \return The depth of the currently allocated buffer.
    virtual unsigned int GetDepth() const override { return 1; }

    /// Accessor for buffer format.
    ///   \return The format of the currently allocated buffer.
    virtual HdFormat GetFormat() const override { return _format; }

    /// Accessor for the buffer multisample state.
    ///   \return Whether the buffer is multisampled or not.
    virtual bool IsMultiSampled() const override { return false; }

    /// Map the buffer for reading/writing. The control flow should be Map(),
    /// before any I/O, followed by memory access, followed by Unmap() when
    /// done.
    ///   \return The address of the buffer.
    virtual void* Map() override {
        _isMapped = true;
        return _buffer.hostData;
    }

    /// Unmap the buffer.
    virtual void Unmap() override {
        _isMapped = false;
    }

    /// Return whether any clients have this buffer mapped currently.
    ///   \return True if the buffer is currently mapped by someone.
    virtual bool IsMapped() const override {
        return _isMapped;
    }

    /// Is the buffer converged?
    ///   \return True if the buffer is converged (not currently being
    ///           rendered to).
    virtual bool IsConverged() const override {
        return true;
    }

    /// Resolve the sample buffer into final values.
    virtual void Resolve() override;

    Tanto_V_BufferRegion* GetBufferRegion(void);

    virtual VtValue GetResource(bool multiSampled) const override;

private:
    // Release any allocated resources.
    virtual void _Deallocate() override;

    // Buffer width.
    unsigned int _width;
    // Buffer height.
    unsigned int _height;
    // Buffer format.
    HdFormat _format;

    // The resolved output buffer.
    Tanto_V_BufferRegion _buffer;

    bool _isMapped;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // TANTO_RENDER_BUFFER_H
