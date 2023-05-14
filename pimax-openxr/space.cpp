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

#include "pch.h"

#include "log.h"
#include "runtime.h"
#include "utils.h"

namespace pimax_openxr {

    using namespace pimax_openxr::log;
    using namespace pimax_openxr::utils;
    using namespace xr::math;

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrEnumerateReferenceSpaces
    XrResult OpenXrRuntime::xrEnumerateReferenceSpaces(XrSession session,
                                                       uint32_t spaceCapacityInput,
                                                       uint32_t* spaceCountOutput,
                                                       XrReferenceSpaceType* spaces) {
        std::vector<XrReferenceSpaceType> referenceSpaces;
        referenceSpaces.push_back(XR_REFERENCE_SPACE_TYPE_VIEW);
        referenceSpaces.push_back(XR_REFERENCE_SPACE_TYPE_LOCAL);
        referenceSpaces.push_back(XR_REFERENCE_SPACE_TYPE_STAGE);
        if (has_XR_VARJO_foveated_rendering) {
            referenceSpaces.push_back(XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO);
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrEnumerateReferenceSpaces",
                          TLXArg(session, "Session"),
                          TLArg(spaceCapacityInput, "SpaceCapacityInput"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (spaceCapacityInput && spaceCapacityInput < referenceSpaces.size()) {
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        *spaceCountOutput = (uint32_t)referenceSpaces.size();
        TraceLoggingWrite(g_traceProvider, "xrEnumerateReferenceSpaces", TLArg(*spaceCountOutput, "SpaceCountOutput"));

        if (spaceCapacityInput && spaces) {
            for (uint32_t i = 0; i < *spaceCountOutput; i++) {
                spaces[i] = referenceSpaces[i];
                TraceLoggingWrite(
                    g_traceProvider, "xrEnumerateReferenceSpaces", TLArg(xr::ToCString(spaces[i]), "Space"));
            }
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrCreateReferenceSpace
    XrResult OpenXrRuntime::xrCreateReferenceSpace(XrSession session,
                                                   const XrReferenceSpaceCreateInfo* createInfo,
                                                   XrSpace* space) {
        if (createInfo->type != XR_TYPE_REFERENCE_SPACE_CREATE_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrCreateReferenceSpace",
                          TLXArg(session, "Session"),
                          TLArg(xr::ToCString(createInfo->referenceSpaceType), "ReferenceSpaceType"),
                          TLArg(xr::ToString(createInfo->poseInReferenceSpace).c_str(), "PoseInReferenceSpace"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (createInfo->referenceSpaceType != XR_REFERENCE_SPACE_TYPE_VIEW &&
            createInfo->referenceSpaceType != XR_REFERENCE_SPACE_TYPE_LOCAL &&
            createInfo->referenceSpaceType != XR_REFERENCE_SPACE_TYPE_STAGE &&
            (!has_XR_VARJO_foveated_rendering ||
             createInfo->referenceSpaceType != XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO)) {
            return XR_ERROR_REFERENCE_SPACE_UNSUPPORTED;
        }

        if (!Quaternion::IsNormalized(createInfo->poseInReferenceSpace.orientation)) {
            return XR_ERROR_POSE_INVALID;
        }

        // Create the internal struct.
        Space& xrSpace = *new Space;
        xrSpace.referenceType = createInfo->referenceSpaceType;
        xrSpace.poseInSpace = createInfo->poseInReferenceSpace;

        *space = (XrSpace)&xrSpace;

        // Maintain a list of known spaces for validation and cleanup.
        m_spaces.insert(*space);

        TraceLoggingWrite(g_traceProvider, "xrCreateReferenceSpace", TLXArg(*space, "Space"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrCreateActionSpace
    XrResult OpenXrRuntime::xrCreateActionSpace(XrSession session,
                                                const XrActionSpaceCreateInfo* createInfo,
                                                XrSpace* space) {
        if (createInfo->type != XR_TYPE_ACTION_SPACE_CREATE_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrCreateActionSpace",
                          TLXArg(session, "Session"),
                          TLXArg(createInfo->action, "Action"),
                          TLArg(getXrPath(createInfo->subactionPath).c_str(), "SubactionPath"),
                          TLArg(xr::ToString(createInfo->poseInActionSpace).c_str(), "PoseInActionSpace"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (createInfo->action != XR_NULL_HANDLE) {
            if (!m_actions.count(createInfo->action)) {
                return XR_ERROR_HANDLE_INVALID;
            }

            Action& xrAction = *(Action*)createInfo->action;

            if (xrAction.type != XR_ACTION_TYPE_POSE_INPUT) {
                return XR_ERROR_ACTION_TYPE_MISMATCH;
            }
        }

        // Create the internal struct.
        Space& xrSpace = *new Space;
        xrSpace.referenceType = XR_REFERENCE_SPACE_TYPE_MAX_ENUM;
        xrSpace.action = createInfo->action;
        xrSpace.subActionPath = createInfo->subactionPath;
        xrSpace.poseInSpace = createInfo->poseInActionSpace;

        *space = (XrSpace)&xrSpace;

        // Maintain a list of known spaces for validation and cleanup.
        m_spaces.insert(*space);

        TraceLoggingWrite(g_traceProvider, "xrCreateActionSpace", TLXArg(*space, "Space"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetReferenceSpaceBoundsRect
    XrResult OpenXrRuntime::xrGetReferenceSpaceBoundsRect(XrSession session,
                                                          XrReferenceSpaceType referenceSpaceType,
                                                          XrExtent2Df* bounds) {
        TraceLoggingWrite(g_traceProvider,
                          "xrGetReferenceSpaceBoundsRect",
                          TLXArg(session, "Session"),
                          TLArg(xr::ToCString(referenceSpaceType), "ReferenceSpaceType"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (referenceSpaceType != XR_REFERENCE_SPACE_TYPE_VIEW && referenceSpaceType != XR_REFERENCE_SPACE_TYPE_LOCAL &&
            referenceSpaceType != XR_REFERENCE_SPACE_TYPE_STAGE) {
            return XR_ERROR_REFERENCE_SPACE_UNSUPPORTED;
        }

        bounds->width = bounds->height = 0.f;

        return XR_SPACE_BOUNDS_UNAVAILABLE;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrLocateSpace
    XrResult OpenXrRuntime::xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location) {
        if (location->type != XR_TYPE_SPACE_LOCATION) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrLocateSpace",
                          TLXArg(space, "Space"),
                          TLXArg(baseSpace, "BaseSpace"),
                          TLArg(time, "Time"));

        location->locationFlags = 0;

        if (!m_spaces.count(space) || !m_spaces.count(baseSpace)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (time <= 0) {
            return XR_ERROR_TIME_INVALID;
        }

        XrSpaceVelocity* velocity = reinterpret_cast<XrSpaceVelocity*>(location->next);
        while (velocity) {
            if (velocity->type == XR_TYPE_SPACE_VELOCITY) {
                break;
            }
            velocity = reinterpret_cast<XrSpaceVelocity*>(velocity->next);
        }

        XrEyeGazeSampleTimeEXT* gazeSampleTime = reinterpret_cast<XrEyeGazeSampleTimeEXT*>(location->next);
        while (gazeSampleTime) {
            if (gazeSampleTime->type == XR_TYPE_EYE_GAZE_SAMPLE_TIME_EXT) {
                break;
            }
            gazeSampleTime = reinterpret_cast<XrEyeGazeSampleTimeEXT*>(gazeSampleTime->next);
        }

        Space& xrSpace = *(Space*)space;
        Space& xrBaseSpace = *(Space*)baseSpace;

        XrPosef spaceToVirtual = Pose::Identity();
        XrSpaceVelocity spaceToVirtualVelocity{};
        XrPosef baseSpaceToVirtual = Pose::Identity();
        XrSpaceVelocity baseSpaceToVirtualVelocity{};
        XrSpaceLocationFlags flags1, flags2;
        if (xrSpace.referenceType != xrBaseSpace.referenceType ||
            (xrSpace.referenceType == XR_REFERENCE_SPACE_TYPE_MAX_ENUM && xrSpace.action != xrBaseSpace.action &&
             xrSpace.subActionPath != xrBaseSpace.subActionPath)) {
            flags1 = locateSpaceToOrigin(
                xrSpace, time, spaceToVirtual, velocity ? &spaceToVirtualVelocity : nullptr, gazeSampleTime);
            flags2 = locateSpaceToOrigin(xrBaseSpace,
                                         time,
                                         baseSpaceToVirtual,
                                         velocity ? &baseSpaceToVirtualVelocity : nullptr,
                                         gazeSampleTime);
        } else {
            // Optimize the case of locating against the same reference space or same action space.
            flags1 = flags2 = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT |
                              XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
            spaceToVirtual = xrSpace.poseInSpace;
            baseSpaceToVirtual = xrBaseSpace.poseInSpace;
            if (velocity) {
                spaceToVirtualVelocity.velocityFlags = baseSpaceToVirtualVelocity.velocityFlags =
                    XR_SPACE_VELOCITY_ANGULAR_VALID_BIT | XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
                spaceToVirtualVelocity.angularVelocity = spaceToVirtualVelocity.linearVelocity =
                    baseSpaceToVirtualVelocity.angularVelocity = baseSpaceToVirtualVelocity.linearVelocity = {};
            }
        }

        // If either pose is not valid, we cannot locate.
        if (!(Pose::IsPoseValid(flags1) && Pose::IsPoseValid(flags2))) {
            TraceLoggingWrite(g_traceProvider, "xrLocateSpace", TLArg(0, "LocationFlags"));
            return XR_SUCCESS;
        }

        location->locationFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;

        // Both poses need to be tracked for the location to be tracked.
        if (Pose::IsPoseTracked(flags1) && Pose::IsPoseTracked(flags2)) {
            location->locationFlags |=
                XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
        }

        // Combine the poses.
        location->pose = Pose::Multiply(spaceToVirtual, Pose::Invert(baseSpaceToVirtual));
        if (velocity) {
            velocity->velocityFlags = spaceToVirtualVelocity.velocityFlags & baseSpaceToVirtualVelocity.velocityFlags;
            if (velocity->velocityFlags & XR_SPACE_VELOCITY_ANGULAR_VALID_BIT) {
                velocity->angularVelocity =
                    spaceToVirtualVelocity.angularVelocity - baseSpaceToVirtualVelocity.angularVelocity;
            }
            if (velocity->velocityFlags & XR_SPACE_VELOCITY_LINEAR_VALID_BIT) {
                // TODO: Does not account for centripetral forces.
                velocity->linearVelocity =
                    spaceToVirtualVelocity.linearVelocity - baseSpaceToVirtualVelocity.linearVelocity;
            }
        }

        if (!velocity) {
            TraceLoggingWrite(g_traceProvider,
                              "xrLocateSpace",
                              TLArg(location->locationFlags, "LocationFlags"),
                              TLArg(xr::ToString(location->pose).c_str(), "Pose"));
        } else {
            TraceLoggingWrite(g_traceProvider,
                              "xrLocateSpace",
                              TLArg(location->locationFlags, "LocationFlags"),
                              TLArg(xr::ToString(location->pose).c_str(), "Pose"),
                              TLArg(velocity->velocityFlags, "VelocityFlags"),
                              TLArg(xr::ToString(velocity->angularVelocity).c_str(), "AngularVelocity"),
                              TLArg(xr::ToString(velocity->linearVelocity).c_str(), "LinearVelocity"));
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrLocateViews
    XrResult OpenXrRuntime::xrLocateViews(XrSession session,
                                          const XrViewLocateInfo* viewLocateInfo,
                                          XrViewState* viewState,
                                          uint32_t viewCapacityInput,
                                          uint32_t* viewCountOutput,
                                          XrView* views) {
        if (viewLocateInfo->type != XR_TYPE_VIEW_LOCATE_INFO || viewState->type != XR_TYPE_VIEW_STATE) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrLocateViews",
                          TLXArg(session, "Session"),
                          TLArg(xr::ToCString(viewLocateInfo->viewConfigurationType), "ViewConfigurationType"),
                          TLArg(viewLocateInfo->displayTime, "DisplayTime"),
                          TLXArg(viewLocateInfo->space, "Space"),
                          TLArg(viewCapacityInput, "ViewCapacityInput"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (viewLocateInfo->viewConfigurationType != XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO &&
            (m_primaryViewConfigurationType != XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO ||
             viewLocateInfo->viewConfigurationType != XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO)) {
            return XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED;
        }

        if (viewCapacityInput &&
            viewCapacityInput < (viewLocateInfo->viewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO
                                     ? xr::StereoView::Count
                                     : xr::QuadView::Count)) {
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        *viewCountOutput =
            (viewLocateInfo->viewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ? xr::StereoView::Count
                                                                                                : xr::QuadView::Count);
        TraceLoggingWrite(g_traceProvider, "xrLocateViews", TLArg(*viewCountOutput, "ViewCountOutput"));

        if (viewCapacityInput && views) {
            // Override default to specify whether foveated rendering is desired when the application does not specify.
            bool foveatedRenderingActive =
                viewLocateInfo->viewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO &&
                m_preferFoveatedRendering;

            if (has_XR_VARJO_foveated_rendering) {
                const XrViewLocateFoveatedRenderingVARJO* foveatedLocate =
                    reinterpret_cast<const XrViewLocateFoveatedRenderingVARJO*>(viewLocateInfo->next);
                while (foveatedLocate) {
                    if (foveatedLocate->type == XR_TYPE_VIEW_LOCATE_FOVEATED_RENDERING_VARJO) {
                        foveatedRenderingActive = foveatedLocate->foveatedRenderingActive;
                        break;
                    }
                    foveatedLocate = reinterpret_cast<const XrViewLocateFoveatedRenderingVARJO*>(foveatedLocate->next);
                }

                TraceLoggingWrite(
                    g_traceProvider, "xrLocateViews", TLArg(foveatedRenderingActive, "FoveatedRenderingActive"));
            }

            // Get the HMD pose in the base space.
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            const auto result =
                xrLocateSpace(m_viewSpace, viewLocateInfo->space, viewLocateInfo->displayTime, &location);
            if (XR_FAILED(result)) {
                return result;
            }
            viewState->viewStateFlags = location.locationFlags;

            // Query the eye tracker if needed.
            bool isGazeValid = false;
            XrVector3f gazeUnitVector{};
            if (foveatedRenderingActive) {
                double dummyTime;
                isGazeValid =
                    getEyeGaze(viewLocateInfo->displayTime, false /* getStateOnly */, gazeUnitVector, dummyTime);
            }

            if (viewState->viewStateFlags & (XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_VALID_BIT)) {
                // Calculate poses for each eye.
                pvrPosef hmdToEyePose[xr::StereoView::Count];
                hmdToEyePose[xr::StereoView::Left] = m_cachedEyeInfo[xr::StereoView::Left].HmdToEyePose;
                hmdToEyePose[xr::StereoView::Right] = m_cachedEyeInfo[xr::StereoView::Right].HmdToEyePose;

                pvrPosef eyePoses[xr::StereoView::Count]{{}, {}};
                pvr_calcEyePoses(m_pvr, xrPoseToPvrPose(location.pose), hmdToEyePose, eyePoses);

                TraceLoggingWrite(g_traceProvider, "xrLocateViews", TLArg(viewState->viewStateFlags, "ViewStateFlags"));

                for (uint32_t i = 0; i < *viewCountOutput; i++) {
                    if (views[i].type != XR_TYPE_VIEW) {
                        return XR_ERROR_VALIDATION_FAILURE;
                    }

                    XrView viewForGazeProjection{};
                    if (i >= xr::StereoView::Count && isGazeValid) {
                        viewForGazeProjection.pose = pvrPoseToXrPose(hmdToEyePose[i - 2]);
                        viewForGazeProjection.fov = views[i - 2].fov;
                    }
                    views[i].pose = pvrPoseToXrPose(eyePoses[i % xr::StereoView::Count]);
                    XrVector2f projectedGaze;
                    if (i < xr::StereoView::Count || !isGazeValid ||
                        !ProjectPoint(viewForGazeProjection, gazeUnitVector, projectedGaze)) {
                        views[i].fov = m_cachedEyeFov[i];

                    } else {
                        // Shift FOV according to the eye gaze.
                        // We also widen the FOV when near the edges of the headset to make sure there's enough overlap
                        // between the two eyes.
                        const float MaxWidenAngle = PVR::DegreeToRad(7.f);
                        constexpr float Deadzone = 0.15f;
                        const XrVector2f centerOfFov{(projectedGaze.x + 1.f) / 2.f, (1.f - projectedGaze.y) / 2.f};
                        const XrVector2f v = centerOfFov - m_centerOfFov[i - 2];
                        const float distanceFromCenter = std::sqrt(v.x * v.x + v.y * v.y);
                        const float widenHalfAngle =
                            std::clamp(distanceFromCenter - Deadzone, 0.f, 0.5f) * MaxWidenAngle;
                        XrFovf globalFov = m_cachedEyeFov[i % xr::StereoView::Count];
                        std::tie(views[i].fov.angleLeft, views[i].fov.angleRight) =
                            Fov::Lerp(std::make_pair(globalFov.angleLeft, globalFov.angleRight),
                                      std::make_pair(m_cachedEyeFov[i + 2].angleLeft - widenHalfAngle,
                                                     m_cachedEyeFov[i + 2].angleRight + widenHalfAngle),
                                      centerOfFov.x);
                        std::tie(views[i].fov.angleDown, views[i].fov.angleUp) =
                            Fov::Lerp(std::make_pair(globalFov.angleDown, globalFov.angleUp),
                                      std::make_pair(m_cachedEyeFov[i + 2].angleDown - widenHalfAngle,
                                                     m_cachedEyeFov[i + 2].angleUp + widenHalfAngle),
                                      centerOfFov.y);
                    }

                    TraceLoggingWrite(g_traceProvider,
                                      "xrLocateViews",
                                      TLArg(i, "ViewIndex"),
                                      TLArg(xr::ToString(views[i].pose).c_str(), "Pose"),
                                      TLArg(xr::ToString(views[i].fov).c_str(), "Fov"));
                }
            } else {
                // All or nothing.
                viewState->viewStateFlags = 0;
                TraceLoggingWrite(g_traceProvider, "xrLocateViews", TLArg(viewState->viewStateFlags, "ViewStateFlags"));
            }
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrDestroySpace
    XrResult OpenXrRuntime::xrDestroySpace(XrSpace space) {
        TraceLoggingWrite(g_traceProvider, "xrDestroySpace", TLXArg(space, "Space"));

        if (!m_spaces.count(space)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Space* xrSpace = (Space*)space;

        delete xrSpace;
        m_spaces.erase(space);

        return XR_SUCCESS;
    }

    XrSpaceLocationFlags OpenXrRuntime::locateSpaceToOrigin(const Space& xrSpace,
                                                            XrTime time,
                                                            XrPosef& pose,
                                                            XrSpaceVelocity* velocity,
                                                            XrEyeGazeSampleTimeEXT* gazeSampleTime) const {
        XrSpaceLocationFlags result = 0;

        if (velocity) {
            velocity->angularVelocity = velocity->linearVelocity = {0, 0, 0};
            velocity->velocityFlags = 0;
        }

        if (xrSpace.referenceType == XR_REFERENCE_SPACE_TYPE_VIEW) {
            // VIEW space if the headset pose.
            result = getHmdPose(time, pose, velocity);
        } else if (xrSpace.referenceType == XR_REFERENCE_SPACE_TYPE_LOCAL) {
            // LOCAL space is the origin reference.
            pose = Pose::Identity();
            result = (XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT |
                      XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
            if (velocity) {
                velocity->velocityFlags = XR_SPACE_VELOCITY_ANGULAR_VALID_BIT | XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
            }
        } else if (xrSpace.referenceType == XR_REFERENCE_SPACE_TYPE_STAGE) {
            // STAGE space is the origin reference at eye level.
            pose = Pose::Translation({0, -m_floorHeight, 0});
            result = (XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT |
                      XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
            if (velocity) {
                velocity->velocityFlags = XR_SPACE_VELOCITY_ANGULAR_VALID_BIT | XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
            }
        } else if (xrSpace.referenceType == XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO) {
            pose = Pose::Identity();

            XrVector3f dummyVector{};
            double dummyTime;
            if (getEyeGaze(time, true /* getStateOnly */, dummyVector, dummyTime)) {
                result = XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
            }
        } else if (xrSpace.action != XR_NULL_HANDLE) {
            // Action spaces for motion controllers.
            Action& xrAction = *(Action*)xrSpace.action;

            const std::string& subActionPath = getXrPath(xrSpace.subActionPath);
            for (const auto& source : xrAction.actionSources) {
                if (!startsWith(source.first, subActionPath)) {
                    continue;
                }

                const std::string& fullPath = source.first;
                TraceLoggingWrite(g_traceProvider, "xrLocateSpace", TLArg(fullPath.c_str(), "ActionSourcePath"));

                const bool isEyeTracker = isActionEyeTracker(fullPath);
                const int trackerIndex = getTrackerIndex(fullPath);

                if (isEyeTracker) {
                    result = getEyeTrackerPose(time, pose, gazeSampleTime);

                    // Per spec we must consistently pick one source. We pick the first one.
                    break;
                } else if (trackerIndex >= 0) {
                    result = getDevicePose(xr::Side::Count + trackerIndex, time, pose, velocity);

                    // Per spec we must consistently pick one source. We pick the first one.
                    break;
                } else {
                    const bool isGripPose = endsWith(fullPath, "/input/grip/pose");
                    const bool isAimPose = endsWith(fullPath, "/input/aim/pose");
                    const int side = getActionSide(fullPath);
                    if ((isGripPose || isAimPose) && side >= 0) {
                        result = getDevicePose(side, time, pose, velocity);

                        // Apply the pose offsets.
                        const bool useAimPose = m_swapGripAimPoses ? isGripPose : isAimPose;
                        if (useAimPose) {
                            pose = Pose::Multiply(m_controllerAimPose[side], pose);
                        } else {
                            pose = Pose::Multiply(m_controllerGripPose[side], pose);
                        }

                        // Per spec we must consistently pick one source. We pick the first one.
                        break;
                    }
                }
            }
        }

        // Apply the offset transform.
        pose = Pose::Multiply(xrSpace.poseInSpace, pose);

        return result;
    }

    XrSpaceLocationFlags OpenXrRuntime::getHmdPose(XrTime time, XrPosef& pose, XrSpaceVelocity* velocity) const {
        XrSpaceLocationFlags locationFlags = 0;
        pvrPoseStatef state{};
        CHECK_PVRCMD(pvr_getTrackedDevicePoseState(m_pvrSession, pvrTrackedDevice_HMD, xrTimeToPvrTime(time), &state));
        TraceLoggingWrite(g_traceProvider,
                          "PVR_HmdPoseState",
                          TLArg(state.StatusFlags, "StatusFlags"),
                          TLArg(xr::ToString(state.ThePose).c_str(), "Pose"),
                          TLArg(xr::ToString(state.AngularVelocity).c_str(), "AngularVelocity"),
                          TLArg(xr::ToString(state.LinearVelocity).c_str(), "LinearVelocity"));

        pose = pvrPoseToXrPose(state.ThePose);
        if (state.StatusFlags & pvrStatus_OrientationTracked) {
            locationFlags |= (XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT);
        } else {
            pose.orientation = Quaternion::Identity();
        }
        // For 9-axis setups, we propagate the Orientation bit to Position.
        if (state.StatusFlags & pvrStatus_PositionTracked || state.StatusFlags & pvrStatus_OrientationTracked) {
            locationFlags |= (XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
        } else {
            pose.position = {};
        }

        if (velocity) {
            velocity->velocityFlags = 0;

            if (state.StatusFlags & pvrStatus_OrientationTracked) {
                velocity->angularVelocity = pvrVector3dToXrVector3f(state.AngularVelocity);
                velocity->velocityFlags |= XR_SPACE_VELOCITY_ANGULAR_VALID_BIT;
            }
            if (state.StatusFlags & pvrStatus_PositionTracked) {
                velocity->linearVelocity = pvrVector3dToXrVector3f(state.LinearVelocity);
                velocity->velocityFlags |= XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
            }
        }

        return locationFlags;
    }

    XrSpaceLocationFlags
    OpenXrRuntime::getDevicePose(int deviceIndex, XrTime time, XrPosef& pose, XrSpaceVelocity* velocity) const {
        constexpr pvrTrackedDeviceType device[] = {pvrTrackedDevice_LeftController,
                                                   pvrTrackedDevice_RightController,
                                                   pvrTrackedDevice_Tracker0,
                                                   pvrTrackedDevice_Tracker1,
                                                   pvrTrackedDevice_Tracker2,
                                                   pvrTrackedDevice_Tracker3,
                                                   pvrTrackedDevice_Tracker4,
                                                   pvrTrackedDevice_Tracker5,
                                                   pvrTrackedDevice_Tracker6,
                                                   pvrTrackedDevice_Tracker7,
                                                   pvrTrackedDevice_Tracker8,
                                                   pvrTrackedDevice_Tracker9,
                                                   pvrTrackedDevice_Tracker10,
                                                   pvrTrackedDevice_Tracker11,
                                                   pvrTrackedDevice_Tracker12};

        XrSpaceLocationFlags locationFlags = 0;
        pvrPoseStatef state{};
        CHECK_PVRCMD(pvr_getTrackedDevicePoseState(m_pvrSession, device[deviceIndex], xrTimeToPvrTime(time), &state));
        TraceLoggingWrite(g_traceProvider,
                          "PVR_DevicePoseState",
                          TLArg(deviceIndex == xr::Side::Left
                                    ? "Left"
                                    : (deviceIndex == xr::Side::Right
                                           ? "Right"
                                           : fmt::format("Tracker{}", deviceIndex - xr::Side::Count).c_str()),
                                "Side"),
                          TLArg(state.StatusFlags, "StatusFlags"),
                          TLArg(xr::ToString(state.ThePose).c_str(), "Pose"),
                          TLArg(xr::ToString(state.AngularVelocity).c_str(), "AngularVelocity"),
                          TLArg(xr::ToString(state.LinearVelocity).c_str(), "LinearVelocity"));

        pose = pvrPoseToXrPose(state.ThePose);
        if (state.StatusFlags & pvrStatus_OrientationTracked) {
            locationFlags |= XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
        } else {
            pose.orientation = Quaternion::Identity();
        }
        if (state.StatusFlags & pvrStatus_PositionTracked) {
            locationFlags |= XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
        } else {
            pose.position = {};
        }

        if (velocity) {
            velocity->velocityFlags = 0;

            if (state.StatusFlags & pvrStatus_OrientationTracked) {
                velocity->angularVelocity = pvrVector3dToXrVector3f(state.AngularVelocity);
                velocity->velocityFlags |= XR_SPACE_VELOCITY_ANGULAR_VALID_BIT;
            }
            if (state.StatusFlags & pvrStatus_PositionTracked) {
                velocity->linearVelocity = pvrVector3dToXrVector3f(state.LinearVelocity);
                velocity->velocityFlags |= XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
            }
        }

        return locationFlags;
    }

    XrSpaceLocationFlags OpenXrRuntime::getEyeTrackerPose(XrTime time,
                                                          XrPosef& pose,
                                                          XrEyeGazeSampleTimeEXT* sampleTime) const {
        if (!m_isEyeTrackingAvailable) {
            return 0;
        }

        XrVector3f eyeGazeVector{0, 0, -1};
        double pvrSampleTime;
        if (!getEyeGaze(time, false /* getStateOnly */, eyeGazeVector, pvrSampleTime)) {
            return 0;
        }

        const XrPosef eyeGaze = Pose::MakePose(
            Quaternion::RotationRollPitchYaw({-tan(eyeGazeVector.y), -tan(eyeGazeVector.x), 0.f}), XrVector3f{0, 0, 0});

        // TODO: Need optimization here, in all likelyhood, the caller is looking for eye gaze relative to VIEW
        // space,
        // in which case we are doing 2 back-to-back getHmdPose() that are cancelling each other.
        XrPosef headPose;
        if (!Pose::IsPoseValid(getHmdPose(time, headPose, nullptr))) {
            return 0;
        }

        // Combine poses.
        pose = Pose::Multiply(eyeGaze, headPose);

        if (sampleTime) {
            sampleTime->time = pvrTimeToXrTime(pvrSampleTime);
        }

        return XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT |
               XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
    }

} // namespace pimax_openxr
