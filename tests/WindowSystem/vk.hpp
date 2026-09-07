#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include <ranges>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;

template<auto func, typename Type, typename... Args>
Type get_vk_struct(const std::string& name, Args&&... args)
{
	using namespace Nyaanwork;

	Type data = {};

	union {
		VkResult capi;
		vk::Result hpp;
	} result;

    result.capi = func(std::forward<Args>(args)...,
					   reinterpret_cast<typename Type::NativeType*>(&data));
	vk::detail::resultCheck(result.hpp, name.c_str());

    return data;
}

template<auto func, typename Type, typename... Args>
std::vector<Type> get_vk_array(const std::string& name, Args&&... args)
{
	using namespace Nyaanwork;

	std::vector<Type> data;
	u32 count = 0;

	union {
		VkResult capi;
		vk::Result hpp;
	} result;

	result.capi = func(std::forward<Args>(args)..., &count, nullptr);
	vk::detail::resultCheck(result.hpp, name.c_str());

	data.resize(count, {});
	result.capi = func(std::forward<Args>(args)..., &count,
					   reinterpret_cast<typename Type::NativeType*>(data.data()));
	vk::detail::resultCheck(result.hpp, name.c_str());

	return data;
}


#ifdef _WIN32
static constexpr bool enable_validation = false;
#else
static constexpr bool enable_validation = true;
#endif

static vk::UniqueInstance make_instance(const WindowSystem::Instance& instance,
										bool validation = enable_validation)
{
	auto ex = instance->vulkan_instance_extensions();

	auto wcn = instance->window_class_name();
	vk::ApplicationInfo app = {
		.pApplicationName = wcn.c_str(),
		.applicationVersion = vk::makeVersion(0, 0, 1),
		.pEngineName = "Nyaanwork",
		.engineVersion = vk::makeVersion(0, 0, 1),
		.apiVersion = vk::ApiVersion14,
	};

	vk::InstanceCreateInfo info = {};
	info.setPApplicationInfo(&app);
	info.setPEnabledExtensionNames(ex);

	if (validation)
	{
		static constexpr auto l = {"VK_LAYER_KHRONOS_validation"};
		info.setPEnabledLayerNames(l);
	}

	return vk::createInstanceUnique(info);
}

struct Context
{
	Context(vk::Instance instance, Ptr<WindowSystem::Window> window)
		: surface(window->create_vulkan_surface(instance), {instance}),
		  pdevice(instance.enumeratePhysicalDevices()[0])

	{
		{
			vk::DeviceCreateInfo info = {};

			static constexpr auto ex = {vk::KHRSwapchainExtensionName};
			info.setPEnabledExtensionNames(ex);

			auto qf = pdevice.getQueueFamilyProperties2();

			auto q_graphics = std::ranges::find_if(qf, [](const auto& qfp)
			{
				return !!(qfp.queueFamilyProperties.queueFlags
							& vk::QueueFlagBits::eGraphics);
			});

			auto qi_present = [&qf, this]
			{
				for (usize i = 0; i < qf.size(); ++i)
				{
					const auto& qfp = qf[i];

					if (pdevice.getSurfaceSupportKHR(i, *surface))
						return i;
				}

				throw std::runtime_error("No present support");
			}();

			qi_graphics = std::distance(qf.begin(), q_graphics);

			static constexpr auto qprior = {1.f};

			std::vector<vk::DeviceQueueCreateInfo> qis;
			qis.reserve(2);

			if (qi_graphics == qi_present)
			{
				auto& q = qis.emplace_back();
				q.setQueueFamilyIndex(qi_graphics);
				q.setQueuePriorities(qprior);
			}
			else
			{
				auto& qg = qis.emplace_back();
				qg.setQueueFamilyIndex(qi_graphics);
				qg.setQueuePriorities(qprior);

				auto& qp = qis.emplace_back();
				qp.setQueueFamilyIndex(qi_present);
				qp.setQueuePriorities(qprior);
			}

			info.setQueueCreateInfos(qis);

			using Features = vk::StructureChain<vk::PhysicalDeviceFeatures2,
							   vk::PhysicalDeviceVulkan11Features,
							   vk::PhysicalDeviceVulkan13Features,
							   vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>;
			Features features = {
				{}, {.shaderDrawParameters = true},
				{.dynamicRendering = true}, {.extendedDynamicState = true},
			};

			info.setPNext(features.get<vk::PhysicalDeviceFeatures2>());

			device = pdevice.createDeviceUnique(info);
			queue_graphics = device->getQueue(qi_graphics, 0);
			queue_present = device->getQueue(qi_present, 0);
		}

		auto get_physical_device_surface_capabilitie = [this](vk::SurfaceKHR surface)
		{
			std::string msg = "get_physical_device_surface_capabilitie";

			return get_vk_struct<vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
								 vk::SurfaceCapabilitiesKHR>(msg, pdevice, surface);
		};

		auto get_physical_device_surface_formats = [this](vk::SurfaceKHR surface)
		{
			std::string msg = "get_physical_device_surface_formats";

			return get_vk_array<vkGetPhysicalDeviceSurfaceFormatsKHR,
								vk::SurfaceFormatKHR>(msg, pdevice, surface);
		};

		{
			vk::RenderPassCreateInfo info;

			vk::AttachmentDescription pass(
				{}, vk::Format::eB8G8R8A8Srgb, vk::SampleCountFlagBits::e1
			);

			pass.loadOp = vk::AttachmentLoadOp::eClear;
			pass.storeOp = vk::AttachmentStoreOp::eStore;
			pass.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			pass.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			pass.initialLayout = vk::ImageLayout::eUndefined;
			pass.finalLayout = vk::ImageLayout::ePresentSrcKHR;

			vk::AttachmentReference ref(0, vk::ImageLayout::eColorAttachmentOptimal);
			vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, ref);
			vk::RenderPassCreateInfo inf({}, pass, subpass);

			render_pass = device->createRenderPassUnique(inf);
		}

		{
			auto cap = get_physical_device_surface_capabilitie(*surface);
			auto formats = get_physical_device_surface_formats(*surface);
			auto present_modes = pdevice.getSurfacePresentModesKHR(*surface);

			using SF = vk::SurfaceFormatKHR;
			auto sf_it = std::ranges::find_if(formats, [](const SF& sformat)
			{
				bool format = sformat.format == vk::Format::eB8G8R8A8Srgb;
				bool color_space = sformat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
				return format && color_space;
			});

			if (sf_it == formats.end())
				throw std::runtime_error("Format not supported");

			using PM = vk::PresentModeKHR;
			auto pm_mode = PM::eFifoRelaxed;
			if (std::ranges::find(present_modes, PM::eFifoRelaxed) == present_modes.end())
				pm_mode = PM::eFifo;

			swap_extend = [&cap, &window]{
				if (cap.currentExtent.width != limits<u32>::max())
				{
					return cap.currentExtent;
				}
				else
				{
					vec2<u32> min = {cap.minImageExtent.width, cap.minImageExtent.height};
					vec2<u32> max = {cap.maxImageExtent.width, cap.maxImageExtent.height};

					auto [x, y] = clamp(vec2<u32>(window->resolution()), min, max);
					return vk::Extent2D{x, y};
				}
			}();

			vk::SwapchainCreateInfoKHR info = {
				.surface= *surface,
				.minImageCount = cap.minImageCount,
				.imageFormat = vk::Format::eB8G8R8A8Srgb,
				.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
				.imageExtent = swap_extend,
				.imageArrayLayers = 1,
				.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
				.imageSharingMode = vk::SharingMode::eExclusive,
				.preTransform = cap.currentTransform,
				.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
				.presentMode = pm_mode,
				.clipped = true,
			};

			swapchain = device->createSwapchainKHRUnique(info);

			vk::ImageViewCreateInfo imgv_info = {
				.viewType = vk::ImageViewType::e2D,
				.format = vk::Format::eB8G8R8A8Srgb,
				.components = {
					vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
				},
				.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
			};

			vk::FramebufferCreateInfo fb_info = {
				.renderPass = *render_pass,
				.width = swap_extend.width,
				.height = swap_extend.height,
				.layers = 1
			};

			auto imgs = device->getSwapchainImagesKHR(*swapchain);
			for (const auto& img : imgs)
			{
				auto& [rimg, rimgv, rfb] = images.emplace_back(img,
									vk::UniqueImageView{}, vk::UniqueFramebuffer{});

				imgv_info.image = rimg;
				rimgv = device->createImageViewUnique(imgv_info);

				fb_info.setAttachments(*rimgv);
				rfb = device->createFramebufferUnique(fb_info);
			}
		}

		render_pass_info = {
			.renderPass = *render_pass,
			.renderArea = {{0, 0}, swap_extend},
			.clearValueCount = 1,
			.pClearValues = &clear_value,
		};

		{
			vk::CommandPoolCreateInfo pool_info = {
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = qi_graphics
			};

			cmd_pool = device->createCommandPoolUnique(pool_info);

			vk::CommandBufferAllocateInfo buf_info = {
				.commandPool = *cmd_pool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = 1,
			};

			cmd_buf = std::move(device->allocateCommandBuffersUnique(buf_info).front());
		}

		{
			present = device->createSemaphoreUnique({});
			render = device->createSemaphoreUnique({});
			fence = device->createFenceUnique({
				.flags = vk::FenceCreateFlagBits::eSignaled
			});
		}
	}

	~Context()
	{
		device->waitIdle();
	}

	void render_frame()
	{
		std::ignore = device->waitForFences(*fence, true, limits<u64>::max());
		device->resetFences(*fence);

		std::ignore = device->acquireNextImageKHR(*swapchain, limits<u64>::max(),
												  *present, nullptr, &frame_index);

		render_pass_info.setFramebuffer(*std::get<2>(images[frame_index]));

		cmd_buf->begin(vk::CommandBufferBeginInfo{});
		cmd_buf->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
		cmd_buf->endRenderPass();
		cmd_buf->end();

		queue_graphics.waitIdle();

		vk::SubmitInfo submit = {};
		submit.setWaitSemaphores(*present);
		submit.setSignalSemaphores(*render);
		submit.setCommandBuffers(*cmd_buf);

		vk::PipelineStageFlags pipeline_flags =
			vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submit.setPWaitDstStageMask(&pipeline_flags);

		queue_graphics.submit(submit, *fence);

		vk::PresentInfoKHR present = {};
		present.setWaitSemaphores(*render);
		present.setSwapchains(*swapchain);
		present.setImageIndices(frame_index);

		std::ignore = queue_graphics.presentKHR(present);
	}

	vk::UniqueSurfaceKHR surface;
	vk::PhysicalDevice pdevice;
	vk::UniqueDevice device;
	vk::Queue queue_graphics;
	vk::Queue queue_present;
	u32 qi_graphics = 0;
	vk::UniqueRenderPass render_pass;
	vk::UniqueSwapchainKHR swapchain;
	vk::Extent2D swap_extend = {};
	std::vector<std::tuple<vk::Image, vk::UniqueImageView, vk::UniqueFramebuffer>> images;
	vk::ClearValue clear_value = vk::ClearColorValue{0.2f, 0.2f, 0.2f, 1.f};
	vk::UniqueCommandPool cmd_pool;
	vk::UniqueCommandBuffer cmd_buf;
	vk::UniqueSemaphore present, render;
	vk::UniqueFence fence;
	vk::RenderPassBeginInfo render_pass_info;
	u32 frame_index = 0;
};
