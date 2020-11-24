#include "render.h"
#include "tanto/m_math.h"
#include "tanto/v_image.h"
#include "tanto/v_memory.h"
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <tanto/v_video.h>
#include <tanto/t_def.h>
#include <tanto/t_utils.h>
#include <tanto/r_pipeline.h>
#include <tanto/r_raytrace.h>
#include <tanto/r_renderpass.h>
#include <tanto/v_command.h>
#include <vulkan/vulkan_core.h>

#define SPVDIR "./shaders/spv"

static uint32_t windowWidth;
static uint32_t windowHeight;

static Tanto_V_Image attachmentColor;
static Tanto_V_Image attachmentDepth;

static VkRenderPass  renderpass;
static VkFramebuffer framebuffer;
static VkPipeline    pipelineMain;

static VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
static VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

static Tanto_V_BufferRegion uniformBufferRegion;

static Tanto_V_CommandPool cmdPoolRender;

static Tanto_R_Primitive triangle;

typedef enum {
    R_PIPE_LAYOUT_MAIN,
} R_PipelineLayoutId;

typedef enum {
    R_DESC_SET_MAIN,
} R_DescriptorSetId;

// TODO: we should implement a way to specify the offscreen renderpass format at initialization
static void initAttachments(void)
{
    attachmentColor = tanto_v_CreateImage(
        windowWidth, windowHeight,
        colorFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_SAMPLE_COUNT_1_BIT);

    attachmentDepth = tanto_v_CreateImage(
        windowWidth, windowHeight,
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_SAMPLE_COUNT_1_BIT);
}

static void initRenderPass(void)
{
    const VkAttachmentDescription attachmentColor = {
        .flags = 0,
        .format = colorFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };

    const VkAttachmentDescription attachmentDepth = {
        .flags = 0,
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentDescription attachments[] = {
        attachmentColor, attachmentDepth
    };

    VkAttachmentReference colorReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthReference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .flags                   = 0,
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = NULL,
        .colorAttachmentCount    = 1,
        .pColorAttachments       = &colorReference,
        .pResolveAttachments     = NULL,
        .pDepthStencilAttachment = &depthReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = NULL,
    };

    Tanto_R_RenderPassInfo rpi = {
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    tanto_r_CreateRenderPass(&rpi, &renderpass);
}

static void initFramebuffer(void)
{
    const VkImageView attachments[] = {
        attachmentColor.view, attachmentDepth.view
    };


    const VkFramebufferCreateInfo fbi = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .renderPass = renderpass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = windowWidth,
        .height = windowHeight,
        .layers = 1,
    };

    V_ASSERT( vkCreateFramebuffer(device, &fbi, NULL, &framebuffer) );
}

static void initDescriptorSetsAndPipelineLayouts(void)
{
    const Tanto_R_DescriptorSet descriptorSets[] = {{
        .id = R_DESC_SET_MAIN,
        .bindingCount = 1,
        .bindings = {{
            .descriptorCount = 1,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        }}
    }};

    const Tanto_R_PipelineLayout pipelayouts[] = {{
        .id = R_PIPE_LAYOUT_MAIN, 
        .descriptorSetCount = 1, 
        .descriptorSetIds = {R_DESC_SET_MAIN},
        .pushConstantCount = 0,
        .pushConstantsRanges = {}
    }};

    tanto_r_InitDescriptorSets(descriptorSets, TANTO_ARRAY_SIZE(descriptorSets));
    tanto_r_InitPipelineLayouts(pipelayouts, TANTO_ARRAY_SIZE(pipelayouts));
}

static void initPipelines(void)
{
    const Tanto_R_PipelineInfo pipeInfo = {
        .type     = TANTO_R_PIPELINE_RASTER_TYPE,
        .layoutId = R_PIPE_LAYOUT_MAIN,
        .payload.rasterInfo = {
            .renderPass = renderpass, 
            .sampleCount = VK_SAMPLE_COUNT_1_BIT,
            .vertexDescription = tanto_r_GetVertexDescription3D_Simple(),
            .vertShader = SPVDIR"/template-vert.spv",
            .fragShader = SPVDIR"/template-frag.spv"
        }
    };

    tanto_r_CreatePipeline(&pipeInfo, &pipelineMain);
}

// descriptors that do only need to have update called once and can be updated on initialization
static void updateStaticDescriptors(void)
{
    uniformBufferRegion = tanto_v_RequestBufferRegion(sizeof(UniformBuffer), 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, TANTO_V_MEMORY_HOST_GRAPHICS_TYPE);
    memset(uniformBufferRegion.hostData, 0, sizeof(Parms));
    UniformBuffer* uboData = (UniformBuffer*)(uniformBufferRegion.hostData);

    Mat4 view = m_Ident_Mat4();
    view = m_Translate_Mat4((Vec3){0, 0, -1}, &view);

    uboData->matModel = m_Ident_Mat4();
    uboData->matView  = view;
    uboData->matProj  = m_BuildPerspective(0.001, 100);

    VkDescriptorBufferInfo uboInfo = {
        .buffer = uniformBufferRegion.buffer,
        .offset = uniformBufferRegion.offset,
        .range  = uniformBufferRegion.size
    };

    VkWriteDescriptorSet writes[] = {{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstArrayElement = 0,
        .dstSet = descriptorSets[R_DESC_SET_MAIN],
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &uboInfo
    }};

    vkUpdateDescriptorSets(device, TANTO_ARRAY_SIZE(writes), writes, 0, NULL);
}

static void updateDynamicDescriptors(void)
{
}

static void mainRender(const VkCommandBuffer* cmdBuf, const VkRenderPassBeginInfo* rpassInfo)
{
    vkCmdBindPipeline(*cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineMain);

    vkCmdBindDescriptorSets(
        *cmdBuf, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipelineLayouts[R_PIPE_LAYOUT_MAIN], 
        0, 1, &descriptorSets[R_DESC_SET_MAIN],
        0, NULL);

    vkCmdBeginRenderPass(*cmdBuf, rpassInfo, VK_SUBPASS_CONTENTS_INLINE);

    const VkBuffer vertBuffers[2] = {
        triangle.vertexRegion.buffer,
        triangle.vertexRegion.buffer
    };

    const VkDeviceSize attrOffsets[2] = {
        triangle.attrOffsets[0] + triangle.vertexRegion.offset,
        triangle.attrOffsets[1] + triangle.vertexRegion.offset,
    };

    vkCmdBindVertexBuffers(*cmdBuf, 0, 2, vertBuffers, attrOffsets);

    vkCmdBindIndexBuffer(*cmdBuf, triangle.indexRegion.buffer, 
            triangle.indexRegion.offset, TANTO_VERT_INDEX_TYPE);

    vkCmdDrawIndexed(*cmdBuf, triangle.indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(*cmdBuf);
}

void r_InitRenderer()
{
    initAttachments();
    initRenderPass();
    initFramebuffer();
    initDescriptorSetsAndPipelineLayouts();
    initPipelines();
    updateStaticDescriptors();

    cmdPoolRender = tanto_v_RequestCommandPool(TANTO_V_QUEUE_GRAPHICS_TYPE);

    triangle = tanto_r_CreateTriangle();
}

void r_UpdateRenderCommands(Tanto_V_BufferRegion* colorBuffer)
{
    vkResetCommandPool(device, cmdPoolRender.handle, 0);

    VkCommandBufferBeginInfo cbbi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    V_ASSERT( vkBeginCommandBuffer(cmdPoolRender.buffer, &cbbi) );

    VkClearValue clearValueColor = {0.002f, 0.023f, 0.009f, 1.0f};
    VkClearValue clearValueDepth = {1.0, 0};

    VkClearValue clears[] = {clearValueColor, clearValueDepth};

    const VkRenderPassBeginInfo rpassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .clearValueCount = 2,
        .pClearValues = clears,
        .renderArea = {{0, 0}, {TANTO_WINDOW_WIDTH, TANTO_WINDOW_HEIGHT}},
        .renderPass =  renderpass,
        .framebuffer = framebuffer
    };

    mainRender(&cmdPoolRender.buffer, &rpassInfo);

    const VkImageSubresourceLayers subRes = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseArrayLayer = 0,
        .layerCount = 1, 
        .mipLevel = 0,
    };

    const VkOffset3D imgOffset = {
        .x = 0,
        .y = 0,
        .z = 0
    };

    const VkBufferImageCopy imgCopy = {
        .imageOffset = imgOffset,
        .imageExtent = attachmentColor.extent,
        .imageSubresource = subRes,
        .bufferOffset = colorBuffer->offset,
        .bufferImageHeight = 0,
        .bufferRowLength = 0
    };

    vkCmdCopyImageToBuffer(cmdPoolRender.buffer, attachmentColor.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, colorBuffer->buffer, 1, &imgCopy);

    V_ASSERT( vkEndCommandBuffer(cmdPoolRender.buffer) );
}

void r_Render(void)
{
    tanto_v_SubmitAndWait(&cmdPoolRender, 0);
}

void r_RecreateSwapchain(void)
{
    vkDeviceWaitIdle(device);

    r_CleanUp();

    tanto_r_RecreateSwapchain();
    initAttachments();
    initPipelines();
    initFramebuffer();

    for (int i = 0; i < TANTO_FRAME_COUNT; i++) 
    {
        r_UpdateRenderCommands(i);
    }
}

void r_CleanUp(void)
{
    vkDestroyFramebuffer(device, framebuffer, NULL);
    tanto_v_DestroyImage(attachmentDepth);
    tanto_v_DestroyImage(attachmentColor);
    vkDestroyPipeline(device, pipelineMain, NULL);
}

void  r_SetViewport(unsigned int width, unsigned int height)
{
    windowWidth = width;
    windowHeight = height;
}
