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

#include "framework/dispatch.gen.h"

#include "appinsights.h"
#include "utils.h"

namespace pimax_openxr {

    using namespace pimax_openxr::appinsights;
    using namespace pimax_openxr::utils;

    const std::string RuntimeName = "pimax-openxr";
    extern const std::string RuntimePrettyName;
    const std::string RegPrefix = "SOFTWARE\\PimaxXR";

    // This class implements all APIs that the runtime supports.
    class OpenXrRuntime : public OpenXrApi {
      public:
        OpenXrRuntime();
        ~OpenXrRuntime();

        XrResult xrGetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function);

        XrResult xrEnumerateInstanceExtensionProperties(const char* layerName,
                                                        uint32_t propertyCapacityInput,
                                                        uint32_t* propertyCountOutput,
                                                        XrExtensionProperties* properties) override;
        XrResult xrCreateInstance(const XrInstanceCreateInfo* createInfo, XrInstance* instance) override;
        XrResult xrDestroyInstance(XrInstance instance) override;
        XrResult xrGetInstanceProperties(XrInstance instance, XrInstanceProperties* instanceProperties) override;
        XrResult xrPollEvent(XrInstance instance, XrEventDataBuffer* eventData) override;
        XrResult xrResultToString(XrInstance instance, XrResult value, char buffer[XR_MAX_RESULT_STRING_SIZE]) override;
        XrResult xrStructureTypeToString(XrInstance instance,
                                         XrStructureType value,
                                         char buffer[XR_MAX_STRUCTURE_NAME_SIZE]) override;
        XrResult xrGetSystem(XrInstance instance, const XrSystemGetInfo* getInfo, XrSystemId* systemId) override;
        XrResult xrGetSystemProperties(XrInstance instance,
                                       XrSystemId systemId,
                                       XrSystemProperties* properties) override;
        XrResult xrEnumerateEnvironmentBlendModes(XrInstance instance,
                                                  XrSystemId systemId,
                                                  XrViewConfigurationType viewConfigurationType,
                                                  uint32_t environmentBlendModeCapacityInput,
                                                  uint32_t* environmentBlendModeCountOutput,
                                                  XrEnvironmentBlendMode* environmentBlendModes) override;
        XrResult xrCreateSession(XrInstance instance,
                                 const XrSessionCreateInfo* createInfo,
                                 XrSession* session) override;
        XrResult xrDestroySession(XrSession session) override;
        XrResult xrEnumerateReferenceSpaces(XrSession session,
                                            uint32_t spaceCapacityInput,
                                            uint32_t* spaceCountOutput,
                                            XrReferenceSpaceType* spaces) override;
        XrResult xrCreateReferenceSpace(XrSession session,
                                        const XrReferenceSpaceCreateInfo* createInfo,
                                        XrSpace* space) override;
        XrResult xrGetReferenceSpaceBoundsRect(XrSession session,
                                               XrReferenceSpaceType referenceSpaceType,
                                               XrExtent2Df* bounds) override;
        XrResult xrCreateActionSpace(XrSession session,
                                     const XrActionSpaceCreateInfo* createInfo,
                                     XrSpace* space) override;
        XrResult xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location) override;
        XrResult xrDestroySpace(XrSpace space) override;
        XrResult xrEnumerateViewConfigurations(XrInstance instance,
                                               XrSystemId systemId,
                                               uint32_t viewConfigurationTypeCapacityInput,
                                               uint32_t* viewConfigurationTypeCountOutput,
                                               XrViewConfigurationType* viewConfigurationTypes) override;
        XrResult xrGetViewConfigurationProperties(XrInstance instance,
                                                  XrSystemId systemId,
                                                  XrViewConfigurationType viewConfigurationType,
                                                  XrViewConfigurationProperties* configurationProperties) override;
        XrResult xrEnumerateViewConfigurationViews(XrInstance instance,
                                                   XrSystemId systemId,
                                                   XrViewConfigurationType viewConfigurationType,
                                                   uint32_t viewCapacityInput,
                                                   uint32_t* viewCountOutput,
                                                   XrViewConfigurationView* views) override;
        XrResult xrEnumerateSwapchainFormats(XrSession session,
                                             uint32_t formatCapacityInput,
                                             uint32_t* formatCountOutput,
                                             int64_t* formats) override;
        XrResult xrCreateSwapchain(XrSession session,
                                   const XrSwapchainCreateInfo* createInfo,
                                   XrSwapchain* swapchain) override;
        XrResult xrDestroySwapchain(XrSwapchain swapchain) override;
        XrResult xrEnumerateSwapchainImages(XrSwapchain swapchain,
                                            uint32_t imageCapacityInput,
                                            uint32_t* imageCountOutput,
                                            XrSwapchainImageBaseHeader* images) override;
        XrResult xrAcquireSwapchainImage(XrSwapchain swapchain,
                                         const XrSwapchainImageAcquireInfo* acquireInfo,
                                         uint32_t* index) override;
        XrResult xrWaitSwapchainImage(XrSwapchain swapchain, const XrSwapchainImageWaitInfo* waitInfo) override;
        XrResult xrReleaseSwapchainImage(XrSwapchain swapchain,
                                         const XrSwapchainImageReleaseInfo* releaseInfo) override;
        XrResult xrBeginSession(XrSession session, const XrSessionBeginInfo* beginInfo) override;
        XrResult xrEndSession(XrSession session) override;
        XrResult xrRequestExitSession(XrSession session) override;
        XrResult xrWaitFrame(XrSession session,
                             const XrFrameWaitInfo* frameWaitInfo,
                             XrFrameState* frameState) override;
        XrResult xrBeginFrame(XrSession session, const XrFrameBeginInfo* frameBeginInfo) override;
        XrResult xrEndFrame(XrSession session, const XrFrameEndInfo* frameEndInfo) override;
        XrResult xrLocateViews(XrSession session,
                               const XrViewLocateInfo* viewLocateInfo,
                               XrViewState* viewState,
                               uint32_t viewCapacityInput,
                               uint32_t* viewCountOutput,
                               XrView* views) override;
        XrResult xrStringToPath(XrInstance instance, const char* pathString, XrPath* path) override;
        XrResult xrPathToString(XrInstance instance,
                                XrPath path,
                                uint32_t bufferCapacityInput,
                                uint32_t* bufferCountOutput,
                                char* buffer) override;
        XrResult xrCreateActionSet(XrInstance instance,
                                   const XrActionSetCreateInfo* createInfo,
                                   XrActionSet* actionSet) override;
        XrResult xrDestroyActionSet(XrActionSet actionSet) override;
        XrResult xrCreateAction(XrActionSet actionSet, const XrActionCreateInfo* createInfo, XrAction* action) override;
        XrResult xrDestroyAction(XrAction action) override;
        XrResult xrSuggestInteractionProfileBindings(
            XrInstance instance, const XrInteractionProfileSuggestedBinding* suggestedBindings) override;
        XrResult xrAttachSessionActionSets(XrSession session, const XrSessionActionSetsAttachInfo* attachInfo) override;
        XrResult xrGetCurrentInteractionProfile(XrSession session,
                                                XrPath topLevelUserPath,
                                                XrInteractionProfileState* interactionProfile) override;
        XrResult xrGetActionStateBoolean(XrSession session,
                                         const XrActionStateGetInfo* getInfo,
                                         XrActionStateBoolean* state) override;
        XrResult xrGetActionStateFloat(XrSession session,
                                       const XrActionStateGetInfo* getInfo,
                                       XrActionStateFloat* state) override;
        XrResult xrGetActionStateVector2f(XrSession session,
                                          const XrActionStateGetInfo* getInfo,
                                          XrActionStateVector2f* state) override;
        XrResult xrGetActionStatePose(XrSession session,
                                      const XrActionStateGetInfo* getInfo,
                                      XrActionStatePose* state) override;
        XrResult xrSyncActions(XrSession session, const XrActionsSyncInfo* syncInfo) override;
        XrResult xrEnumerateBoundSourcesForAction(XrSession session,
                                                  const XrBoundSourcesForActionEnumerateInfo* enumerateInfo,
                                                  uint32_t sourceCapacityInput,
                                                  uint32_t* sourceCountOutput,
                                                  XrPath* sources) override;
        XrResult xrGetInputSourceLocalizedName(XrSession session,
                                               const XrInputSourceLocalizedNameGetInfo* getInfo,
                                               uint32_t bufferCapacityInput,
                                               uint32_t* bufferCountOutput,
                                               char* buffer) override;
        XrResult xrApplyHapticFeedback(XrSession session,
                                       const XrHapticActionInfo* hapticActionInfo,
                                       const XrHapticBaseHeader* hapticFeedback) override;
        XrResult xrStopHapticFeedback(XrSession session, const XrHapticActionInfo* hapticActionInfo) override;
        XrResult xrGetVulkanInstanceExtensionsKHR(XrInstance instance,
                                                  XrSystemId systemId,
                                                  uint32_t bufferCapacityInput,
                                                  uint32_t* bufferCountOutput,
                                                  char* buffer) override;
        XrResult xrGetVulkanDeviceExtensionsKHR(XrInstance instance,
                                                XrSystemId systemId,
                                                uint32_t bufferCapacityInput,
                                                uint32_t* bufferCountOutput,
                                                char* buffer) override;
        XrResult xrGetVulkanGraphicsDeviceKHR(XrInstance instance,
                                              XrSystemId systemId,
                                              VkInstance vkInstance,
                                              VkPhysicalDevice* vkPhysicalDevice) override;
        XrResult xrGetVulkanGraphicsRequirementsKHR(XrInstance instance,
                                                    XrSystemId systemId,
                                                    XrGraphicsRequirementsVulkanKHR* graphicsRequirements) override;
        XrResult xrGetD3D11GraphicsRequirementsKHR(XrInstance instance,
                                                   XrSystemId systemId,
                                                   XrGraphicsRequirementsD3D11KHR* graphicsRequirements) override;
        XrResult xrGetD3D12GraphicsRequirementsKHR(XrInstance instance,
                                                   XrSystemId systemId,
                                                   XrGraphicsRequirementsD3D12KHR* graphicsRequirements) override;
        XrResult xrGetVisibilityMaskKHR(XrSession session,
                                        XrViewConfigurationType viewConfigurationType,
                                        uint32_t viewIndex,
                                        XrVisibilityMaskTypeKHR visibilityMaskType,
                                        XrVisibilityMaskKHR* visibilityMask) override;
        XrResult xrConvertWin32PerformanceCounterToTimeKHR(XrInstance instance,
                                                           const LARGE_INTEGER* performanceCounter,
                                                           XrTime* time) override;
        XrResult xrConvertTimeToWin32PerformanceCounterKHR(XrInstance instance,
                                                           XrTime time,
                                                           LARGE_INTEGER* performanceCounter) override;
        XrResult xrCreateVulkanInstanceKHR(XrInstance instance,
                                           const XrVulkanInstanceCreateInfoKHR* createInfo,
                                           VkInstance* vulkanInstance,
                                           VkResult* vulkanResult) override;
        XrResult xrCreateVulkanDeviceKHR(XrInstance instance,
                                         const XrVulkanDeviceCreateInfoKHR* createInfo,
                                         VkDevice* vulkanDevice,
                                         VkResult* vulkanResult) override;
        XrResult xrGetVulkanGraphicsDevice2KHR(XrInstance instance,
                                               const XrVulkanGraphicsDeviceGetInfoKHR* getInfo,
                                               VkPhysicalDevice* vulkanPhysicalDevice) override;
        XrResult xrGetVulkanGraphicsRequirements2KHR(XrInstance instance,
                                                     XrSystemId systemId,
                                                     XrGraphicsRequirementsVulkanKHR* graphicsRequirements) override;
        XrResult xrGetOpenGLGraphicsRequirementsKHR(XrInstance instance,
                                                    XrSystemId systemId,
                                                    XrGraphicsRequirementsOpenGLKHR* graphicsRequirements) override;
        XrResult xrCreateHandTrackerEXT(XrSession session,
                                        const XrHandTrackerCreateInfoEXT* createInfo,
                                        XrHandTrackerEXT* handTracker) override;
        XrResult xrDestroyHandTrackerEXT(XrHandTrackerEXT handTracker) override;
        XrResult xrLocateHandJointsEXT(XrHandTrackerEXT handTracker,
                                       const XrHandJointsLocateInfoEXT* locateInfo,
                                       XrHandJointLocationsEXT* locations) override;
        XrResult xrEnumerateDisplayRefreshRatesFB(XrSession session,
                                                  uint32_t displayRefreshRateCapacityInput,
                                                  uint32_t* displayRefreshRateCountOutput,
                                                  float* displayRefreshRates) override;
        XrResult xrGetDisplayRefreshRateFB(XrSession session, float* displayRefreshRate) override;
        XrResult xrRequestDisplayRefreshRateFB(XrSession session, float displayRefreshRate) override;

      private:
        enum class ForcedInteractionProfile {
            OculusTouchController,
            MicrosoftMotionController,
        };

        struct Extension {
            const char* extensionName;
            uint32_t extensionVersion;
        };

        struct Swapchain {
            // The PVR swapchain objects. For texture arrays, we must have one swapchain per slice due to PVR
            // limitation.
            std::vector<pvrTextureSwapChain> pvrSwapchain;
            int pvrSwapchainLength{0};
            std::vector<ComPtr<ID3D11Texture2D>> images;

            // The cached textures used for copy between swapchains.
            std::vector<std::vector<ComPtr<ID3D11Texture2D>>> slices;

            // The last manipulated swapchain image index.
            std::deque<int> acquiredIndices;
            int lastWaitedIndex{-1};
            int lastReleasedIndex{-1};
            uint32_t nextIndex{0};

            // Whether a static image swapchain has been acquired at least once.
            bool frozen{false};

            // Resources needed to resolve MSAA and/or format conversion or alpha correction.
            std::vector<int> lastProcessedIndex;
            std::vector<std::vector<ComPtr<ID3D11ShaderResourceView>>> imagesResourceView;
            std::vector<std::vector<ComPtr<ID3D11RenderTargetView>>> renderTargetView;
            ComPtr<ID3D11Texture2D> resolved;
            ComPtr<ID3D11Buffer> convertConstants;
            ComPtr<ID3D11UnorderedAccessView> convertAccessView;
            ComPtr<ID3D11ShaderResourceView> convertResourceView;

            // Resources needed for interop.
            std::vector<ComPtr<ID3D11Texture2D>> d3d11Images;
            std::vector<ComPtr<ID3D12Resource>> d3d12Images;
            std::vector<VkDeviceMemory> vkDeviceMemory;
            std::vector<VkImage> vkImages;
            std::vector<GLuint> glMemory;
            std::vector<GLuint> glImages;

            // Information recorded at creation.
            XrSwapchainCreateInfo xrDesc;
            DXGI_FORMAT dxgiFormatForSubmission{DXGI_FORMAT_UNKNOWN};
            pvrTextureSwapChainDesc pvrDesc;
        };

        struct Space {
            // Information recorded at creation.
            XrReferenceSpaceType referenceType;
            XrAction action{XR_NULL_HANDLE};
            XrPath subActionPath{XR_NULL_PATH};
            XrPosef poseInSpace;
        };

        struct ActionSource {
            const float* floatValue{nullptr};

            const pvrVector2f* vector2fValue{nullptr};
            int vector2fIndex{-1};

            const uint32_t* buttonMap{nullptr};
            pvrButton buttonType;

            std::string realPath;
        };

        struct ActionSet {
            std::string name;
            std::string localizedName;

            std::set<XrPath> subactionPaths;

            // A copy of the input state. This is to handle when xrSyncActions() does not update all actionsets at once.
            pvrInputState cachedInputState;
        };

        struct Action {
            XrActionType type;
            std::string name;
            std::string localizedName;

            XrActionSet actionSet{XR_NULL_HANDLE};

            float lastFloatValue[2]{0.f, 0.f};
            XrTime lastFloatValueChangedTime[2]{0, 0};

            XrVector2f lastVector2fValue[2]{{0.f, 0.f}, {0.f, 0.f}};
            XrTime lastVector2fValueChangedTime[2]{0, 0};

            bool lastBoolValue[2]{false, false};
            XrTime lastBoolValueChangedTime[2]{0, 0};

            std::set<XrPath> subactionPaths;
            std::map<std::string, ActionSource> actionSources;
        };

        struct HandTracker {
            int side;
        };

        enum class EyeTracking {
            None = 0,
            PVR,
            aSeeVR,
            Simulated,
        };

        // instance.cpp
        void initializeExtensionsTable();
        std::optional<int> getSetting(const std::string& value) const;

        // system.cpp
        void fillDisplayDeviceInfo();
        bool ensurePvrSession();

        // session.cpp
        void updateSessionState(bool forceSendEvent = false);
        void refreshSettings();
        void initializeGuardianResources();

        // action.cpp
        void rebindControllerActions(int side);
        std::string getXrPath(XrPath path) const;
        XrPath stringToPath(const std::string& path, bool validate = false);
        int getActionSide(const std::string& fullPath, bool allowExtraPaths = false) const;
        bool isActionEyeTracker(const std::string& fullPath) const;
        XrVector2f handleJoystickDeadzone(pvrVector2f raw) const;
        void handleBuiltinActions(bool wasRecenteringPressed = false, bool wasSystemPressed = false);

        // mappings.cpp
        void initializeRemappingTables();
        bool mapPathToViveControllerInputState(const Action& xrAction,
                                               const std::string& path,
                                               ActionSource& source) const;
        bool mapPathToIndexControllerInputState(const Action& xrAction,
                                                const std::string& path,
                                                ActionSource& source) const;
        bool mapPathToCrystalControllerInputState(const Action& xrAction,
                                                  const std::string& path,
                                                  ActionSource& source) const;
        bool mapPathToSimpleControllerInputState(const Action& xrAction,
                                                 const std::string& path,
                                                 ActionSource& source) const;
        std::string getViveControllerLocalizedSourceName(const std::string& path) const;
        std::string getIndexControllerLocalizedSourceName(const std::string& path) const;
        std::string getCrystalControllerLocalizedSourceName(const std::string& path) const;
        std::string getSimpleControllerLocalizedSourceName(const std::string& path) const;
        std::optional<std::string> remapSimpleControllerToViveController(const std::string& path) const;
        std::optional<std::string> remapOculusTouchControllerToViveController(const std::string& path) const;
        std::optional<std::string> remapMicrosoftMotionControllerToViveController(const std::string& path) const;
        std::optional<std::string> remapIndexControllerToViveController(const std::string& path) const;
        std::optional<std::string> remapSimpleControllerToIndexController(const std::string& path) const;
        std::optional<std::string> remapOculusTouchControllerToIndexController(const std::string& path) const;
        std::optional<std::string> remapMicrosoftMotionControllerToIndexController(const std::string& path) const;
        std::optional<std::string> remapViveControllerToIndexController(const std::string& path) const;
        std::optional<std::string> remapSimpleControllerToCrystalController(const std::string& path) const;
        std::optional<std::string> remapMicrosoftMotionControllerToCrystalController(const std::string& path) const;
        std::optional<std::string> remapViveControllerToCrystalController(const std::string& path) const;
        std::optional<std::string> remapIndexControllerToCrystalController(const std::string& path) const;
        std::optional<std::string> remapOculusTouchControllerToSimpleController(const std::string& path) const;
        std::optional<std::string> remapMicrosoftMotionControllerToSimpleController(const std::string& path) const;
        std::optional<std::string> remapViveControllerToSimpleController(const std::string& path) const;
        std::optional<std::string> remapIndexControllerToSimpleController(const std::string& path) const;

        // space.cpp

        XrSpaceLocationFlags locateSpace(const Space& xrSpace,
                                         const Space& xrBaseSpace,
                                         XrTime time,
                                         XrPosef& pose,
                                         XrSpaceVelocity* velocity = nullptr,
                                         XrEyeGazeSampleTimeEXT* gazeSampleTime = nullptr) const;
        XrSpaceLocationFlags locateSpaceToOrigin(const Space& xrSpace,
                                                 XrTime time,
                                                 XrPosef& pose,
                                                 XrSpaceVelocity* velocity,
                                                 XrEyeGazeSampleTimeEXT* gazeSampleTime) const;
        XrSpaceLocationFlags getHmdPose(XrTime time, XrPosef& pose, XrSpaceVelocity* velocity) const;
        XrSpaceLocationFlags getControllerPose(int side, XrTime time, XrPosef& pose, XrSpaceVelocity* velocity) const;
        XrSpaceLocationFlags getEyeTrackerPose(XrTime time, XrPosef& pose, XrEyeGazeSampleTimeEXT* sampleTime) const;

        // eye_tracking.cpp
        bool getEyeGaze(XrTime time, bool getStateOnly, XrVector3f& unitVector, double& sampleTime) const;
#ifndef NOASEEVRCLIENT
        bool initializeDroolon();
        void startDroolonTracking();
        void stopDroolonTracking();
        void setDroolonCoefficients(const aSeeVRCoefficient& coefficients);
        void setDroolonReady(bool ready);
        void setDroolonData(int64_t timestamp, const XrVector2f& gaze);
        static void _7INVENSUN_CALL aSeeVRgetCoefficientCallback(const aSeeVRCoefficient* data, void* context);
        static void _7INVENSUN_CALL aSeeVRstateCallback(const aSeeVRState* state, void* context);
        static void _7INVENSUN_CALL aSeeVReyeDataCallback(const aSeeVREyeData* eyeData, void* context);
#endif

        // frame.cpp
        void asyncSubmissionThread();
        void waitForAsyncSubmissionIdle(bool doRunningStart = false);

        // d3d11_native.cpp
        XrResult initializeD3D11(const XrGraphicsBindingD3D11KHR& d3dBindings);
        void cleanupD3D11();
        void initializeSubmissionDevice(const std::string& appGraphicsApi);
        void initializeSubmissionResources();
        void cleanupSubmissionDevice();
        std::vector<HANDLE> getSwapchainImages(Swapchain& xrSwapchain);
        XrResult getSwapchainImagesD3D11(Swapchain& xrSwapchain, XrSwapchainImageD3D11KHR* d3d11Images, uint32_t count);
        void prepareAndCommitSwapchainImage(Swapchain& xrSwapchain,
                                            uint32_t layerIndex,
                                            uint32_t slice,
                                            XrCompositionLayerFlags compositionFlags,
                                            bool isFocusView,
                                            std::set<std::pair<pvrTextureSwapChain, uint32_t>>& committed);
        void ensureSwapchainSliceResources(Swapchain& xrSwapchain, uint32_t slice) const;
        void ensureSwapchainIntermediateResources(Swapchain& xrSwapchain) const;
        void flushD3D11Context();
        void flushSubmissionContext();
        void serializeD3D11Frame();
        void waitOnSubmissionDevice();

        // d3d12_interop.cpp
        XrResult initializeD3D12(const XrGraphicsBindingD3D12KHR& d3dBindings);
        void cleanupD3D12();
        bool isD3D12Session() const;
        XrResult getSwapchainImagesD3D12(Swapchain& xrSwapchain, XrSwapchainImageD3D12KHR* d3d12Images, uint32_t count);
        void flushD3D12CommandQueue();
        void serializeD3D12Frame();

        // vulkan_interop.cpp
        XrResult initializeVulkan(const XrGraphicsBindingVulkanKHR& vkBindings);
        void initializeVulkanDispatch(VkInstance instance);
        void cleanupVulkan();
        bool isVulkanSession() const;
        XrResult getSwapchainImagesVulkan(Swapchain& xrSwapchain, XrSwapchainImageVulkanKHR* vkImages, uint32_t count);
        void flushVulkanCommandQueue();
        void serializeVulkanFrame();

        // opengl_interop.cpp
        XrResult initializeOpenGL(const XrGraphicsBindingOpenGLWin32KHR& glBindings);
        void initializeOpenGLDispatch();
        void cleanupOpenGL();
        bool isOpenGLSession() const;
        XrResult getSwapchainImagesOpenGL(Swapchain& xrSwapchain, XrSwapchainImageOpenGLKHR* glImages, uint32_t count);
        void flushOpenGLContext();
        void serializeOpenGLFrame();

        // visibility_mask.cpp
        void convertSteamVRToOpenXRHiddenMesh(const pvrFovPort& fov,
                                              XrVector2f* vertices,
                                              uint32_t* indices,
                                              uint32_t count) const;

        // mirror_window.cpp
        void createMirrorWindow();
        void updateMirrorWindow(bool preferSRGB = false);
        LRESULT CALLBACK mirrorWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        friend LRESULT CALLBACK wndProcWrapper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        // overlay.cpp
        void initializeOverlayResources();
        void refreshOverlay();

        // Instance & PVR state.
        pvrEnvHandle m_pvr{nullptr};
        pvrSessionHandle m_pvrSession{nullptr};
        bool m_instanceCreated{false};
        bool m_systemCreated{false};
        bool m_useFrameTimingOverride{false};
        std::vector<Extension> m_extensionsTable;
        bool m_graphicsRequirementQueried{false};
        LUID m_adapterLuid{};
        float m_displayRefreshRate{0};
        double m_idealFrameDuration{0};
        double m_predictedFrameDuration{0};
        pvrHmdInfo m_cachedHmdInfo{};
        pvrEyeRenderInfo m_cachedEyeInfo[xr::StereoView::Count]{};
        float m_floorHeight{0.f};
        LARGE_INTEGER m_qpcFrequency{};
        double m_pvrTimeFromQpcTimeOffset{0};
        XrPath m_stringIndex{0};
        using MappingFunction = std::function<bool(const Action&, XrPath, ActionSource&)>;
        using CheckValidPathFunction = std::function<bool(const std::string&)>;
        std::map<std::pair<std::string, std::string>, MappingFunction> m_controllerMappingTable;
        std::map<std::string, CheckValidPathFunction> m_controllerValidPathsTable;
        wil::unique_registry_watcher m_registryWatcher;
        bool m_loggedResolution{false};
        std::string m_applicationName;
        bool m_needWorldLockedQuadLayerQuirk{false};
        bool m_disableFramePipeliningQuirk{false};
        bool m_completeDiscardedFramesQuirk{false};
        bool m_alwaysUseFrameIdZero{false};
        bool m_useApplicationDeviceForSubmission{true};
        EyeTracking m_eyeTrackingType{EyeTracking::None};
#ifndef NOASEEVRCLIENT
        aSeeVRCoefficient m_droolonCoefficients{};
        mutable std::mutex m_droolonMutex;
        bool m_isDroolonReady{false};
        double m_droolonTimestamp{0};
        XrVector2f m_droolonGaze{};
#endif
        float m_droolonProjectionDistance{0.35f};
        bool m_isEyeTrackingAvailable{false};
        float m_focusPixelDensity{1.f};
        float m_peripheralPixelDensity{0.5f};
        // [0] = non-foveated, [1] = foveated
        float m_horizontalFovSection[2]{0.75f, 0.5f};
        float m_verticalFovSection[2]{0.7f, 0.5f};
        bool m_preferFoveatedRendering{true};

        // Session state.
        ComPtr<ID3D11Device5> m_pvrSubmissionDevice;
        ComPtr<ID3D11DeviceContext4> m_pvrSubmissionContext;
        ComPtr<ID3DDeviceContextState> m_pvrSubmissionContextState;
        ComPtr<ID3D11Fence> m_pvrSubmissionFence;
        wil::unique_handle m_eventForSubmissionFence;
        bool m_syncGpuWorkInEndFrame{false};
        ComPtr<ID3D11ComputeShader> m_alphaCorrectShader[2];
        ComPtr<IDXGISwapChain1> m_dxgiSwapchain;
        bool m_sessionCreated{false};
        XrViewConfigurationType m_primaryViewConfigurationType{XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM};
        XrSessionState m_sessionState{XR_SESSION_STATE_UNKNOWN};
        std::deque<std::pair<XrSessionState, double>> m_sessionEventQueue;
        pvrHmdStatus m_hmdStatus{};
        bool m_sessionBegun{false};
        bool m_sessionLossPending{false};
        bool m_sessionStopping{false};
        bool m_sessionExiting{false};
        bool m_useParallelProjection{false};
        int m_fovLevel{0};
        // [0] = left, [1] = right
        // [2] = left focus non-foveated, [3] = right focus non-foveated,
        // [4] = left focus foveated, [5] = right focus foveated
        XrFovf m_cachedEyeFov[xr::QuadView::Count + 2];
        XrVector2f m_centerOfFov[xr::StereoView::Count];
        std::mutex m_actionsAndSpacesMutex;
        std::map<XrPath, std::string> m_strings; // protected by actionsAndSpacesMutex
        std::set<XrActionSet> m_actionSets;
        std::set<XrActionSet> m_activeActionSets;
        std::set<XrAction> m_actions;
        std::set<XrAction> m_actionsForCleanup;
        std::mutex m_handTrackersMutex;
        std::set<XrHandTrackerEXT> m_handTrackers;
        std::set<XrSpace> m_spaces;
        Space* m_originSpace{nullptr};
        Space* m_viewSpace{nullptr};
        std::map<std::string, std::vector<XrActionSuggestedBinding>> m_suggestedBindings;
        bool m_isControllerActive[2]{false, false};
        std::string m_cachedControllerType[2];
        XrPosef m_controllerAimOffset;
        XrPosef m_controllerGripOffset;
        XrPosef m_controllerAimPose[2];
        XrPosef m_controllerGripPose[2];
        std::string m_localizedControllerType[2];
        XrPath m_currentInteractionProfile[2]{XR_NULL_PATH, XR_NULL_PATH};
        bool m_currentInteractionProfileDirty{false};
        std::optional<ForcedInteractionProfile> m_forcedInteractionProfile;
        std::optional<ForcedInteractionProfile> m_lastForcedInteractionProfile;
        bool m_useAnalogGrip{true};
        std::string m_debugControllerType;
        std::optional<double> m_isRecenteringPressed;
        int64_t m_frameTimeOverrideOffsetUs{0};
        uint64_t m_frameTimeOverrideUs{0};
        size_t m_frameTimeFilterLength{3};
        float m_joystickDeadzone{0.f};
        bool m_swapGripAimPoses{false};
        bool m_useDeferredFrameWait{true};
        bool m_lockFramerate{false};
        bool m_useDeferredFrameWaitThisFrame{false};
        bool m_honorPremultiplyFlagOnProj0{false};
        bool m_useRunningStart{true};
        bool m_debugFocusViews{false};
        bool m_postProcessFocusView{true};

        // Swapchains and other graphics stuff.
        std::mutex m_swapchainsMutex;
        std::set<XrSwapchain> m_swapchains;

        // Mirror window.
        bool m_useMirrorWindow{false};
        std::mutex m_mirrorWindowMutex;
        HWND m_mirrorWindowHwnd{nullptr};
        bool m_mirrorWindowReady{false};
        std::thread m_mirrorWindowThread;
        ComPtr<IDXGISwapChain1> m_mirrorWindowSwapchain;
        pvrMirrorTexture m_pvrMirrorSwapChain{nullptr};
        ComPtr<ID3D11Texture2D> m_mirrorTexture;

        // Async submittion thread.
        bool m_useAsyncSubmission{false};
        bool m_needStartAsyncSubmissionThread{false};
        bool m_terminateAsyncThread{false};
        std::thread m_asyncSubmissionThread;
        std::mutex m_asyncSubmissionMutex;
        std::condition_variable m_asyncSubmissionCondVar;
        std::vector<pvrLayer_Union> m_layersForAsyncSubmission;
        std::chrono::high_resolution_clock::time_point m_lastWaitToBeginFrameTime{};

        // Guardian state.
        pvrTextureSwapChain m_guardianSwapchain{nullptr};
        Space* m_guardianSpace{nullptr};
        XrExtent2Di m_guardianExtent{};
        float m_guardianThreshold{1.1f};
        float m_guardianRadius{1.6f};

        // Overlay resources.
        ComPtr<IFW1Factory> m_fontWrapperFactory;
        ComPtr<IFW1FontWrapper> m_fontNormal;
        pvrTextureSwapChain m_overlaySwapchain{nullptr};
        DXGI_FORMAT m_overlaySwapchainFormat{DXGI_FORMAT_UNKNOWN};
        ComPtr<ID3D11Resource> m_overlayBackground;
        XrExtent2Di m_overlayExtent{};
        XrPosef m_overlayPose{};
        std::optional<double> m_isSystemPressed;
        bool m_isOverlayVisible{false};
        XrExtent2Di m_proj0Extent{};
        std::time_t m_lastOverlayRefresh{0};

        // Graphics API interop.
        ComPtr<ID3D11Device5> m_d3d11Device;
        ComPtr<ID3D11DeviceContext4> m_d3d11Context;
        ComPtr<ID3DDeviceContextState> m_d3d11ContextState;
        ComPtr<ID3D12Device> m_d3d12Device;
        ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
        ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;
        ComPtr<ID3D12GraphicsCommandList> m_d3d12CommandList;
        VkInstance m_vkBootstrapInstance{VK_NULL_HANDLE};
        VkPhysicalDevice m_vkBootstrapPhysicalDevice{VK_NULL_HANDLE};
        VkInstance m_vkInstance{VK_NULL_HANDLE};
        VkDevice m_vkDevice{VK_NULL_HANDLE};
        VkCommandPool m_vkCmdPool{VK_NULL_HANDLE};
        VkCommandBuffer m_vkCmdBuffer{VK_NULL_HANDLE};
        // Pointers in the dispatcher must be initialized in initializeVulkanDispatch().
        VulkanDispatch m_vkDispatch;
        std::optional<VkAllocationCallbacks> m_vkAllocator;
        VkPhysicalDevice m_vkPhysicalDevice{VK_NULL_HANDLE};
        VkPhysicalDeviceMemoryProperties m_vkMemoryProperties;
        VkQueue m_vkQueue{VK_NULL_HANDLE};
        GlContext m_glContext{};
        // Pointers in the dispatcher must be initialized in initializeOpenGLDispatch().
        GlDispatch m_glDispatch;

        ComPtr<ID3D11Fence> m_d3d11Fence;
        ComPtr<ID3D12Fence> m_d3d12Fence;
        VkSemaphore m_vkTimelineSemaphore{VK_NULL_HANDLE};
        GLuint m_glSemaphore{0};
        UINT64 m_fenceValue{0};

        // Due to Vulkan semaphore transference rules(?) it looks like we may not be able to both signal and wait on an
        // imported semaphore. Use a separate one for host-side flushes.
        VkSemaphore m_vkTimelineSemaphoreForFlush{VK_NULL_HANDLE};

        // Workaround: the AMD driver does not seem to like closing the handle for the shared fence when using
        // OpenGL. We keep it alive for the whole session.
        wil::shared_handle m_fenceHandleForAMDWorkaround;

        // Common resources needed for sRGB color conversion.
        ComPtr<ID3D11SamplerState> m_linearClampSampler;
        ComPtr<ID3D11RasterizerState> m_noDepthRasterizer;
        ComPtr<ID3D11VertexShader> m_fullQuadVS;
        ComPtr<ID3D11PixelShader> m_colorConversionPS;

        // Frame state.
        std::mutex m_frameMutex;
        std::condition_variable m_frameCondVar;
        uint64_t m_frameWaited{0};
        uint64_t m_frameBegun{0};
        uint64_t m_frameCompleted{0};
        uint64_t m_lastCpuFrameTimeUs{0};
        uint64_t m_lastGpuFrameTimeUs{0};
        pvrInputState m_cachedInputState;
        bool m_actionsSyncedThisFrame{false};
        XrTime m_lastPredictedDisplayTime{0};
        mutable std::optional<XrPosef> m_lastValidHmdPose;
        std::deque<uint64_t> m_frameTimeFilter;
        bool m_isSmartSmoothingEnabled{false};
        bool m_isSmartSmoothingActive{false};

        // FOV submission correction.
        bool m_needFocusFovCorrectionQuirk{false};
        std::map<XrTime, std::pair<XrFovf, XrFovf>> m_focusFovForDisplayTime; // protected by actionsAndSpacesMutex

        // Statistics.
        AppInsights m_telemetry;
        double m_sessionStartTime{0.0};
        uint64_t m_sessionTotalFrameCount{0};
        std::deque<double> m_frameTimes;
        CpuTimer m_frameTimerApp;
        CpuTimer m_renderTimerApp;
        static constexpr uint32_t k_numGpuTimers = 3;
        std::unique_ptr<ITimer> m_gpuTimerApp[k_numGpuTimers];
        std::unique_ptr<ITimer> m_gpuTimerPrecomposition[k_numGpuTimers];
        uint32_t m_currentTimerIndex{0};

        friend AppInsights* GetTelemetry();
    };

    // Singleton accessor.
    OpenXrApi* GetInstance();

    // A function to reset (delete) the singleton.
    void ResetInstance();

    // Get telemetry object if available.
    AppInsights* GetTelemetry();

    extern std::filesystem::path dllHome;
    extern std::filesystem::path localAppData;

} // namespace pimax_openxr
