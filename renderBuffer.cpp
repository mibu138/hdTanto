#include "renderBuffer.h"
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

HdTantoRenderBuffer::HdTantoRenderBuffer(SdfPath const& id)
    : HdRenderBuffer(id)
    , _width(0)
    , _height(0)
    , _format(HdFormatInvalid)
    , _buffer()
    , _isMapped(false)
{
}

HdTantoRenderBuffer::~HdTantoRenderBuffer()
{
}

/*virtual*/
void
HdTantoRenderBuffer::Sync(HdSceneDelegate *sceneDelegate,
                           HdRenderParam *renderParam,
                           HdDirtyBits *dirtyBits)
{
    //if (*dirtyBits & DirtyDescription) {
    //    // Tanto has the background thread write directly into render buffers,
    //    // so we need to stop the render thread before reallocating them.
    //    static_cast<HdTantoRenderParam*>(renderParam)->AcquireSceneForEdit();
    //}

    HdRenderBuffer::Sync(sceneDelegate, renderParam, dirtyBits);
}

/*virtual*/
void
HdTantoRenderBuffer::Finalize(HdRenderParam *renderParam)
{
    // Tanto has the background thread write directly into render buffers,
    // so we need to stop the render thread before removing them.
    //static_cast<HdTantoRenderParam*>(renderParam)->AcquireSceneForEdit();

    HdRenderBuffer::Finalize(renderParam);
}

/*virtual*/
void
HdTantoRenderBuffer::_Deallocate()
{
    // If the buffer is mapped while we're doing this, there's not a great
    // recovery path...
    TF_VERIFY(!IsMapped());

    _width = 0;
    _height = 0;
    _format = HdFormatInvalid;
    _buffer.resize(0);
    _isMapped = false;
}

/*static*/
static size_t _GetBufferSize(GfVec2i const &dims, HdFormat format)
{
    return dims[0] * dims[1] * HdDataSizeOfFormat(format);
}
/*virtual*/
bool
HdTantoRenderBuffer::Allocate(GfVec3i const& dimensions,
                               HdFormat format,
                               bool multiSampled)
{
    _Deallocate();

    std::cout << "ALLOCATE CALLED!@!! " << '\n';

    if (dimensions[2] != 1) {
        TF_WARN("Render buffer allocated with dims <%d, %d, %d> and"
                " format %s; depth must be 1!",
                dimensions[0], dimensions[1], dimensions[2],
                TfEnum::GetName(format).c_str());
        return false;
    }

    _width = dimensions[0];
    _height = dimensions[1];
    _format = format;
    const size_t bufferSize = _GetBufferSize(GfVec2i(_width, _height), format);
    _buffer.resize(bufferSize);
    std::cout << "Setting buffer size to: " << bufferSize << '\n';

    return true;
}


/*virtual*/
void
HdTantoRenderBuffer::Resolve()
{
    // Resolve the image buffer: find the average value per pixel by
    // dividing the summed value by the number of samples.
    //
    // not multiSampled yet

    return;
}

PXR_NAMESPACE_CLOSE_SCOPE
