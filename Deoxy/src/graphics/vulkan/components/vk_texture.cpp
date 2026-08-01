#include "vk_texture.hpp"

#include "vk_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_command_pool.hpp"
#include "../components/vk_commands.hpp"
#include "../components/vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanTexture::VulkanTexture(
        VkDevice device,
        const VulkanAllocator& allocator,
        const VulkanCommandPool& commandPool,
        VkQueue queue,
        const ImageData& imageData
    ) : m_image(
            device, allocator.GetHandle(),
            VkExtent2D{
                .width = imageData.Width,
                .height = imageData.Height
            },
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        ), m_sampler(device){
        CheckBool(queue != VK_NULL_HANDLE, "Texture received a null queue");
        CheckBool(!imageData.Pixels.empty(), "Texture received empty pixel data");

        const size_t expectedSize = static_cast<size_t>(imageData.Width) * static_cast<size_t>(imageData.Height) * 4;
        CheckBool(imageData.Pixels.size() == expectedSize, "Texture received an invalid RGBA pixel count");

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(imageData.Pixels.size());
        VulkanBuffer stagingBuffer {
            allocator, imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        };

        stagingBuffer.Upload(imageData.Pixels.data(), imageData.Pixels.size());

        CopyBufferToImageImmediate(
            queue, commandPool, stagingBuffer.GetHandle(), m_image.GetImage(),
            imageData.Width, imageData.Height
        );
    }
}
