// MIT License
//
// Copyright(c) 2022-2023 Matthieu Bucchianeri
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "pch.h"

// clang-format off
#define LOG_TELEMETRY_ONCE(call)        \
    {                                   \
        static bool logged = false;     \
        if (!logged) {                  \
            m_telemetry.call;           \
            logged = true;              \
        }                               \
    }
// clang-format on

#define CHECK_PVRCMD(cmd) xr::detail::_CheckPVRResult(cmd, #cmd, FILE_AND_LINE)
#define CHECK_VKCMD(cmd) xr::detail::_CheckVKResult(cmd, #cmd, FILE_AND_LINE)

namespace xr {
    static inline std::string ToString(XrVersion version) {
        return fmt::format("{}.{}.{}", XR_VERSION_MAJOR(version), XR_VERSION_MINOR(version), XR_VERSION_PATCH(version));
    }

    static inline std::string ToString(const pvrPosef& pose) {
        return fmt::format("p: ({:.3f}, {:.3f}, {:.3f}), o:({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                           pose.Position.x,
                           pose.Position.y,
                           pose.Position.z,
                           pose.Orientation.x,
                           pose.Orientation.y,
                           pose.Orientation.z,
                           pose.Orientation.w);
    }

    static inline std::string ToString(const XrPosef& pose) {
        return fmt::format("p: ({:.3f}, {:.3f}, {:.3f}), o:({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                           pose.position.x,
                           pose.position.y,
                           pose.position.z,
                           pose.orientation.x,
                           pose.orientation.y,
                           pose.orientation.z,
                           pose.orientation.w);
    }

    static inline std::string ToString(const pvrVector3f& vec) {
        return fmt::format("({:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z);
    }

    static inline std::string ToString(const XrVector3f& vec) {
        return fmt::format("({:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z);
    }

    static inline std::string ToString(const pvrVector2f& vec) {
        return fmt::format("({:.3f}, {:.3f})", vec.x, vec.y);
    }

    static inline std::string ToString(const XrVector2f& vec) {
        return fmt::format("({:.3f}, {:.3f})", vec.x, vec.y);
    }

    static inline std::string ToString(const XrFovf& fov) {
        return fmt::format(
            "(l:{:.3f}, r:{:.3f}, u:{:.3f}, d:{:.3f})", fov.angleLeft, fov.angleRight, fov.angleUp, fov.angleDown);
    }

    static inline std::string ToString(const XrRect2Di& rect) {
        return fmt::format("x:{}, y:{} w:{} h:{}", rect.offset.x, rect.offset.y, rect.extent.width, rect.extent.height);
    }

    static inline std::string ToString(pvrResult result) {
        switch (result) {
        case pvr_success:
            return "Success";
        case pvr_failed:
            return "Failed";
        case pvr_dll_failed:
            return "DLL Failed";
        case pvr_dll_wrong:
            return "DLL Wrong";
        case pvr_interface_not_found:
            return "Interface not found";
        case pvr_invalid_param:
            return "Invalid Parameter";
        case pvr_rpc_failed:
            return "RPC Failed";
        case pvr_share_mem_failed:
            return "Share Memory Failed";
        case pvr_unsupport_render_name:
            return "Unsupported Render Name";
        case pvr_no_display:
            return "No Display";
        case pvr_no_render_device:
            return "No Render Device";
        case pvr_app_not_visible:
            return "App Not Visible";
        case pvr_srv_not_ready:
            return "Service Not Ready";
        case pvr_dll_srv_mismatch:
            return "DLL Mismatch";
        case pvr_app_adapter_mismatch:
            return "App Adapter Mismatch";
        case pvr_not_support:
            return "Not Supported";

        default:
            return fmt::format("pvrResult_{}", result);
        }
    }

#ifndef NOASEEVRCLIENT
    static inline std::string ToString(aSeeVRReturnCode result) {
        switch (result) {
        case aSeeVRReturnCode::success:
            return "Success";
        case aSeeVRReturnCode::bind_local_port_failed:
            return "Bind Port Failed";
        case aSeeVRReturnCode::permission_denied:
            return "Permission Denied";
        case aSeeVRReturnCode::invalid_value:
            return "Invalid Value";
        case aSeeVRReturnCode::invalid_parameter:
            return "Invalid Parameter";
        case aSeeVRReturnCode::failed:
            return "Failed";

        default:
            return fmt::format("aSeeVRReturnCode_{}", result);
        }
    }
#endif

    namespace math {

        namespace Pose {

            static inline bool Equals(const XrPosef& a, const XrPosef& b) {
                return std::abs(b.position.x - a.position.x) < 0.00001f &&
                       std::abs(b.position.y - a.position.y) < 0.00001f &&
                       std::abs(b.position.z - a.position.z) < 0.00001f &&
                       std::abs(b.orientation.x - a.orientation.x) < 0.00001f &&
                       std::abs(b.orientation.y - a.orientation.y) < 0.00001f &&
                       std::abs(b.orientation.z - a.orientation.z) < 0.00001f &&
                       std::abs(b.orientation.w - a.orientation.w) < 0.00001f;
            }

        } // namespace Pose

        namespace Fov {

            static inline std::pair<float, float> Scale(std::pair<float, float> angles, float scale) {
                assert(angles.second > angles.first);
                const float angleCenter = (angles.first + angles.second) / 2;
                const float angleSpread = angles.second - angles.first;
                const float angleSpreadScaled = angleSpread * scale;
                const float angleLowerScaled = angleCenter - (angleSpreadScaled / 2);
                const float angleUpperScaled = angleCenter + (angleSpreadScaled / 2);

                return std::make_pair(angleLowerScaled, angleUpperScaled);
            };

            static inline std::pair<float, float> Lerp(std::pair<float, float> range,
                                                       std::pair<float, float> angles,
                                                       float lerp) {
                assert(angles.second > angles.first);
                assert(range.second > range.first);
                const float rangeSpread = range.second - range.first;
                const float angleSpread = angles.second - angles.first;
                const float lerpedCenter = range.first + lerp * rangeSpread;
                float angleLower = lerpedCenter - angleSpread / 2.f;
                float angleUpper = lerpedCenter + angleSpread / 2.f;

                // Clamp to the FOV boundaries.
                if (angleUpper > range.second) {
                    angleUpper = range.second;
                    angleLower = angleUpper - angleSpread;
                } else if (angleLower < range.first) {
                    angleLower = range.first;
                    angleUpper = angleLower + angleSpread;
                }

                return std::make_pair(angleLower, angleUpper);
            };

        } // namespace Fov

        static bool ProjectPoint(const XrView& eyeInViewSpace,
                                 const XrVector3f& forward,
                                 XrVector2f& projectedPosition) {
            // 1) Compute the view space to camera transform for this eye.
            const auto cameraProjection = xr::math::ComposeProjectionMatrix(eyeInViewSpace.fov, {0.001f, 100.f});
            const auto cameraView = xr::math::LoadXrPose(eyeInViewSpace.pose);
            const auto viewToCamera = DirectX::XMMatrixMultiply(cameraProjection, cameraView);

            // 2) Transform the 3D point to camera space.
            const auto projectedInCameraSpace =
                DirectX::XMVector3Transform(DirectX::XMVectorSet(forward.x, forward.y, forward.z, 1.f), viewToCamera);

            // 3) Project the 3D point in camera space to a 2D point in normalized device coordinates.
            XrVector4f point;
            xr::math::StoreXrVector4(&point, projectedInCameraSpace);
            if (std::abs(point.w) < FLT_EPSILON) {
                return false;
            }

            // 4) Output NDC (-1,+1)
            projectedPosition.x = point.x / point.w;
            projectedPosition.y = point.y / point.w;

            return true;
        }

    } // namespace math

    namespace QuadView {
        constexpr uint32_t Left = 0;
        constexpr uint32_t Right = 1;
        constexpr uint32_t FocusLeft = 2;
        constexpr uint32_t FocusRight = 3;
        constexpr uint32_t Count = 4;
    } // namespace QuadView

    namespace detail {

        [[noreturn]] static inline void _ThrowPVRResult(pvrResult pvr,
                                                        const char* originator = nullptr,
                                                        const char* sourceLocation = nullptr) {
            xr::detail::_Throw(xr::detail::_Fmt("pvrResult failure [%d]", pvr), originator, sourceLocation);
        }

        static inline HRESULT _CheckPVRResult(pvrResult pvr,
                                              const char* originator = nullptr,
                                              const char* sourceLocation = nullptr) {
            if (pvr != pvr_success) {
                xr::detail::_ThrowPVRResult(pvr, originator, sourceLocation);
            }

            return pvr;
        }

        [[noreturn]] static inline void _ThrowVKResult(VkResult vks,
                                                       const char* originator = nullptr,
                                                       const char* sourceLocation = nullptr) {
            xr::detail::_Throw(xr::detail::_Fmt("VkStatus failure [%d]", vks), originator, sourceLocation);
        }

        static inline HRESULT _CheckVKResult(VkResult vks,
                                             const char* originator = nullptr,
                                             const char* sourceLocation = nullptr) {
            if ((vks) != VK_SUCCESS) {
                xr::detail::_ThrowVKResult(vks, originator, sourceLocation);
            }

            return vks;
        }
    } // namespace detail

} // namespace xr

namespace pimax_openxr::utils {

    // A generic timer.
    struct ITimer {
        virtual ~ITimer() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual uint64_t query(bool reset = true) const = 0;
    };

    // A synchronous CPU timer.
    class CpuTimer : public ITimer {
        using clock = std::chrono::high_resolution_clock;

      public:
        void start() override {
            m_timeStart = clock::now();
        }

        void stop() override {
            m_duration += clock::now() - m_timeStart;
        }

        uint64_t query(bool reset = true) const override {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_duration);
            if (reset)
                m_duration = clock::duration::zero();
            return duration.count();
        }

      private:
        clock::time_point m_timeStart;
        mutable clock::duration m_duration{0};
    };

    // API dispatch table for Vulkan.
    struct VulkanDispatch {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{nullptr};

        PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2{nullptr};
        PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties{nullptr};
        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR{nullptr};
        PFN_vkGetDeviceQueue vkGetDeviceQueue{nullptr};
        PFN_vkQueueSubmit vkQueueSubmit{nullptr};
        PFN_vkCreateImage vkCreateImage{nullptr};
        PFN_vkDestroyImage vkDestroyImage{nullptr};
        PFN_vkAllocateMemory vkAllocateMemory{nullptr};
        PFN_vkFreeMemory vkFreeMemory{nullptr};
        PFN_vkCreateCommandPool vkCreateCommandPool{nullptr};
        PFN_vkDestroyCommandPool vkDestroyCommandPool{nullptr};
        PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers{nullptr};
        PFN_vkFreeCommandBuffers vkFreeCommandBuffers{nullptr};
        PFN_vkResetCommandBuffer vkResetCommandBuffer{nullptr};
        PFN_vkBeginCommandBuffer vkBeginCommandBuffer{nullptr};
        PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier{nullptr};
        PFN_vkCmdResetQueryPool vkCmdResetQueryPool{nullptr};
        PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp{nullptr};
        PFN_vkEndCommandBuffer vkEndCommandBuffer{nullptr};
        PFN_vkGetMemoryWin32HandlePropertiesKHR vkGetMemoryWin32HandlePropertiesKHR{nullptr};
        PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR{nullptr};
        PFN_vkCreateSemaphore vkCreateSemaphore{nullptr};
        PFN_vkDestroySemaphore vkDestroySemaphore{nullptr};
        PFN_vkImportSemaphoreWin32HandleKHR vkImportSemaphoreWin32HandleKHR{nullptr};
        PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR{nullptr};
        PFN_vkDeviceWaitIdle vkDeviceWaitIdle{nullptr};
        PFN_vkCreateQueryPool vkCreateQueryPool{nullptr};
        PFN_vkDestroyQueryPool vkDestroyQueryPool{nullptr};
        PFN_vkGetQueryPoolResults vkGetQueryPoolResults{nullptr};
    };

    // API dispatch table for OpenGL.
    struct GlDispatch {
        PFNGLGETUNSIGNEDBYTEVEXTPROC glGetUnsignedBytevEXT{nullptr};
        PFNGLCREATETEXTURESPROC glCreateTextures{nullptr};
        PFNGLCREATEMEMORYOBJECTSEXTPROC glCreateMemoryObjectsEXT{nullptr};
        PFNGLDELETEMEMORYOBJECTSEXTPROC glDeleteMemoryObjectsEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM2DEXTPROC glTextureStorageMem2DEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC glTextureStorageMem2DMultisampleEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM3DEXTPROC glTextureStorageMem3DEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC glTextureStorageMem3DMultisampleEXT{nullptr};
        PFNGLGENSEMAPHORESEXTPROC glGenSemaphoresEXT{nullptr};
        PFNGLDELETESEMAPHORESEXTPROC glDeleteSemaphoresEXT{nullptr};
        PFNGLSEMAPHOREPARAMETERUI64VEXTPROC glSemaphoreParameterui64vEXT{nullptr};
        PFNGLSIGNALSEMAPHOREEXTPROC glSignalSemaphoreEXT{nullptr};
        PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC glImportMemoryWin32HandleEXT{nullptr};
        PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC glImportSemaphoreWin32HandleEXT{nullptr};
        PFNGLGENQUERIESPROC glGenQueries{nullptr};
        PFNGLDELETEQUERIESPROC glDeleteQueries{nullptr};
        PFNGLQUERYCOUNTERPROC glQueryCounter{nullptr};
        PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv{nullptr};
        PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v{nullptr};
    };

    struct GlContext {
        HDC glDC;
        HGLRC glRC;

        bool valid{false};
    };

    class GlContextSwitch {
      public:
        GlContextSwitch(const GlContext& context) : m_valid(context.valid) {
            if (m_valid) {
                m_glDC = wglGetCurrentDC();
                m_glRC = wglGetCurrentContext();

                wglMakeCurrent(context.glDC, context.glRC);

                // Reset error codes.
                while (glGetError() != GL_NO_ERROR)
                    ;
            }
        }

        ~GlContextSwitch() noexcept(false) {
            if (m_valid) {
                const auto error = glGetError();

                wglMakeCurrent(m_glDC, m_glRC);

                CHECK_MSG(error == GL_NO_ERROR, fmt::format("OpenGL error: 0x{:x}", error));
            }
        }

      private:
        const bool m_valid;
        HDC m_glDC;
        HGLRC m_glRC;
    };

    // https://docs.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry
    static std::optional<int> RegGetDword(HKEY hKey, const std::string& subKey, const std::string& value) {
        DWORD data{};
        DWORD dataSize = sizeof(data);
        LONG retCode = ::RegGetValue(hKey,
                                     xr::utf8_to_wide(subKey).c_str(),
                                     xr::utf8_to_wide(value).c_str(),
                                     RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_DWORD,
                                     nullptr,
                                     &data,
                                     &dataSize);
        if (retCode != ERROR_SUCCESS) {
            return {};
        }
        return data;
    }

    static std::optional<std::string> RegGetString(HKEY hKey, const std::string& subKey, const std::string& value) {
        DWORD dataSize = 0;
        LONG retCode = ::RegGetValue(hKey,
                                     xr::utf8_to_wide(subKey).c_str(),
                                     xr::utf8_to_wide(value).c_str(),
                                     RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_SZ,
                                     nullptr,
                                     nullptr,
                                     &dataSize);
        if (retCode != ERROR_SUCCESS || !dataSize) {
            return {};
        }

        std::wstring data(dataSize / sizeof(wchar_t), 0);
        retCode = ::RegGetValue(hKey,
                                xr::utf8_to_wide(subKey).c_str(),
                                xr::utf8_to_wide(value).c_str(),
                                RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_SZ,
                                nullptr,
                                data.data(),
                                &dataSize);
        if (retCode != ERROR_SUCCESS) {
            return {};
        }

        return xr::wide_to_utf8(data).substr(0, dataSize / sizeof(wchar_t) - 1);
    }

    static std::vector<const char*> ParseExtensionString(char* names) {
        std::vector<const char*> list;
        while (*names != 0) {
            list.push_back(names);
            while (*(++names) != 0) {
                if (*names == ' ') {
                    *names++ = '\0';
                    break;
                }
            }
        }
        return list;
    }

    static inline XrTime pvrTimeToXrTime(double pvrTime) {
        return (XrTime)(pvrTime * 1e9);
    }

    static inline double xrTimeToPvrTime(XrTime xrTime) {
        return xrTime / 1e9;
    }

    static inline XrPosef pvrPoseToXrPose(const pvrPosef& pvrPose) {
        XrPosef xrPose;
        xrPose.position.x = pvrPose.Position.x;
        xrPose.position.y = pvrPose.Position.y;
        xrPose.position.z = pvrPose.Position.z;
        xrPose.orientation.x = pvrPose.Orientation.x;
        xrPose.orientation.y = pvrPose.Orientation.y;
        xrPose.orientation.z = pvrPose.Orientation.z;
        xrPose.orientation.w = pvrPose.Orientation.w;

        return xrPose;
    }

    static inline pvrPosef xrPoseToPvrPose(const XrPosef& xrPose) {
        pvrPosef pvrPose;
        pvrPose.Position.x = xrPose.position.x;
        pvrPose.Position.y = xrPose.position.y;
        pvrPose.Position.z = xrPose.position.z;
        pvrPose.Orientation.x = xrPose.orientation.x;
        pvrPose.Orientation.y = xrPose.orientation.y;
        pvrPose.Orientation.z = xrPose.orientation.z;
        pvrPose.Orientation.w = xrPose.orientation.w;

        return pvrPose;
    }

    static inline XrVector3f pvrVector3dToXrVector3f(const pvrVector3f& pvrVector3f) {
        XrVector3f xrVector3f;
        xrVector3f.x = pvrVector3f.x;
        xrVector3f.y = pvrVector3f.y;
        xrVector3f.z = pvrVector3f.z;

        return xrVector3f;
    }

    static DXGI_FORMAT getTypelessFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_R32_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_R32G8X24_TYPELESS;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case DXGI_FORMAT_D16_UNORM:
            return DXGI_FORMAT_R16_TYPELESS;
        }

        return format;
    }

    static bool isSRGBFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return true;
        }

        return false;
    }

    static pvrTextureFormat dxgiToPvrTextureFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return PVR_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return PVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return PVR_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return PVR_FORMAT_B8G8R8A8_UNORM_SRGB;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return PVR_FORMAT_B8G8R8X8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return PVR_FORMAT_B8G8R8X8_UNORM_SRGB;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return PVR_FORMAT_R16G16B16A16_FLOAT;
        case DXGI_FORMAT_D16_UNORM:
            return PVR_FORMAT_D16_UNORM;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return PVR_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_D32_FLOAT:
            return PVR_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return PVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return PVR_FORMAT_UNKNOWN;
        }
    }

    static DXGI_FORMAT pvrToDxgiTextureFormat(pvrTextureFormat format) {
        switch (format) {
        case PVR_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PVR_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case PVR_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PVR_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case PVR_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case PVR_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        case PVR_FORMAT_R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PVR_FORMAT_D16_UNORM:
            return DXGI_FORMAT_D16_UNORM;
        case PVR_FORMAT_D24_UNORM_S8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case PVR_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        case PVR_FORMAT_D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    static pvrTextureFormat vkToPvrTextureFormat(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return PVR_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return PVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return PVR_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return PVR_FORMAT_B8G8R8A8_UNORM_SRGB;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return PVR_FORMAT_R16G16B16A16_FLOAT;
        case VK_FORMAT_D16_UNORM:
            return PVR_FORMAT_D16_UNORM;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return PVR_FORMAT_D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT:
            return PVR_FORMAT_D32_FLOAT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return PVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return PVR_FORMAT_UNKNOWN;
        }
    }

    static pvrTextureFormat glToPvrTextureFormat(GLenum format) {
        switch (format) {
        case GL_RGBA8:
            return PVR_FORMAT_R8G8B8A8_UNORM;
        case GL_SRGB8_ALPHA8:
            return PVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case GL_RGBA16F:
            return PVR_FORMAT_R16G16B16A16_FLOAT;
        case GL_DEPTH_COMPONENT16:
            return PVR_FORMAT_D16_UNORM;
        case GL_DEPTH24_STENCIL8:
            return PVR_FORMAT_D24_UNORM_S8_UINT;
        case GL_DEPTH_COMPONENT32F:
            return PVR_FORMAT_D32_FLOAT;
        case GL_DEPTH32F_STENCIL8:
            return PVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return PVR_FORMAT_UNKNOWN;
        }
    }

    static size_t glGetBytePerPixels(GLenum format) {
        switch (format) {
        case GL_DEPTH_COMPONENT16:
            return 2;
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH_COMPONENT32F:
        case GL_R11F_G11F_B10F:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return 4;
        case GL_RGBA16F:
        case GL_DEPTH32F_STENCIL8:
            return 8;
        default:
            return 0;
        }
    }

    static inline bool isValidSwapchainRect(pvrTextureSwapChainDesc desc, XrRect2Di rect) {
        if (rect.offset.x < 0 || rect.offset.y < 0 || rect.extent.width <= 0 || rect.extent.height <= 0) {
            return false;
        }

        if (rect.offset.x + rect.extent.width > desc.Width || rect.offset.y + rect.extent.height > desc.Height) {
            return false;
        }

        return true;
    }

    static inline void setDebugName(ID3D11DeviceChild* resource, std::string_view name) {
        if (resource && !name.empty()) {
            resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());
        }
    }

    static inline void setDebugName(ID3D12Object* resource, std::string_view name) {
        if (resource && !name.empty()) {
            resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());
        }
    }

    static inline bool startsWith(const std::string& str, const std::string& substr) {
        return str.find(substr) == 0;
    }

    static inline bool endsWith(const std::string& str, const std::string& substr) {
        const auto pos = str.find(substr);
        return pos != std::string::npos && pos == str.size() - substr.size();
    }

    template <typename TMethod>
    void DetourDllAttach(const char* dll, const char* target, TMethod hooked, TMethod& original) {
        if (original) {
            // Already hooked.
            return;
        }

        HMODULE handle;
        CHECK_MSG(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN, dll, &handle), "Failed to get DLL handle");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        original = (TMethod)GetProcAddress(handle, target);
        CHECK_MSG(original, "Failed to resolve symbol");
        DetourAttach((PVOID*)&original, hooked);

        CHECK_MSG(DetourTransactionCommit() == NO_ERROR, "Detour failed");
    }

    template <typename TMethod>
    void DetourDllDetach(const char* dll, const char* target, TMethod hooked, TMethod& original) {
        if (!original) {
            // Not hooked.
            return;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach((PVOID*)&original, hooked);

        CHECK_MSG(DetourTransactionCommit() == NO_ERROR, "Detour failed");

        original = nullptr;
    }

    // https://stackoverflow.com/questions/60700302/win32-api-to-get-machine-uuid
    static std::string getMachineUuid() {
        typedef struct _dmi_header {
            BYTE type;
            BYTE length;
            WORD handle;
        } dmi_header;

        typedef struct _RawSMBIOSData {
            BYTE Used20CallingMethod;
            BYTE SMBIOSMajorVersion;
            BYTE SMBIOSMinorVersion;
            BYTE DmiRevision;
            DWORD Length;
#pragma warning(push)
#pragma warning(disable : 4200)
            BYTE SMBIOSTableData[];
#pragma warning(pop)
        } RawSMBIOSData;

        DWORD bufsize = 0;
        static BYTE buf[65536] = {0};
        int ret = 0;
        RawSMBIOSData* Smbios;
        dmi_header* h = NULL;
        int flag = 1;

        ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
        if (!ret) {
            return "";
        }

        bufsize = ret;

        ret = GetSystemFirmwareTable('RSMB', 0, buf, bufsize);
        if (!ret) {
            return "";
        }

        Smbios = (RawSMBIOSData*)buf;
        BYTE* p = Smbios->SMBIOSTableData;

        if (Smbios->Length != bufsize - 8) {
            return "";
        }

        for (DWORD i = 0; i < Smbios->Length; i++) {
            h = (dmi_header*)p;
            if (h->type == 1) {
                const BYTE* p2 = p + 0x8;
                short ver = Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion;

                int only0xFF = 1, only0x00 = 1;
                int i;

                for (i = 0; i < 16 && (only0x00 || only0xFF); i++) {
                    if (p2[i] != 0x00)
                        only0x00 = 0;
                    if (p2[i] != 0xFF)
                        only0xFF = 0;
                }

                if (only0xFF) {
                    return "";
                }

                if (only0x00) {
                    return "";
                }

                char buf[128];
                if (ver >= 0x0206) {
                    sprintf_s(buf,
                              sizeof(buf),
                              "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              p2[3],
                              p2[2],
                              p2[1],
                              p2[0],
                              p2[5],
                              p2[4],
                              p2[7],
                              p2[6],
                              p2[8],
                              p2[9],
                              p2[10],
                              p2[11],
                              p2[12],
                              p2[13],
                              p2[14],
                              p2[15]);
                } else {
                    sprintf_s(buf,
                              sizeof(buf),
                              "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              p2[0],
                              p2[1],
                              p2[2],
                              p2[3],
                              p2[4],
                              p2[5],
                              p2[6],
                              p2[7],
                              p2[8],
                              p2[9],
                              p2[10],
                              p2[11],
                              p2[12],
                              p2[13],
                              p2[14],
                              p2[15]);
                }
                return buf;
            }

            p += h->length;
            while ((*(WORD*)p) != 0) {
                p++;
            }
            p += 2;
        }

        return "";
    }

} // namespace pimax_openxr::utils

#include "gpu_timers.h"
