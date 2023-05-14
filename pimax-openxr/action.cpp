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

    namespace {

        // https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#well-formed-path-strings
        bool validateString(const std::string_view& str) {
            for (const auto c : str) {
                if (!islower(c) && !isdigit(c) && c != '-' && c != '_' && c != '.') {
                    return false;
                }
            }
            return true;
        }
        bool validatePath(std::string path) {
            if (path.size() < 2 || path[0] != '/' || path[path.size() - 1] == '/') {
                return false;
            }

            path.erase(0, 1);
            size_t pos = 0;
            while (!path.empty()) {
                pos = path.find('/');
                const auto token = pos != std::string::npos ? path.substr(0, pos) : path;
                if (token.empty() || !validateString(token)) {
                    return false;
                }
                bool notADot = false;
                for (const auto c : token) {
                    if (c != '.') {
                        notADot = true;
                    }
                }
                if (!notADot) {
                    return false;
                }
                path.erase(0, std::min(token.size() + 1, path.size()));
            }
            return true;
        }

    } // namespace

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrStringToPath
    XrResult OpenXrRuntime::xrStringToPath(XrInstance instance, const char* pathString, XrPath* path) {
        TraceLoggingWrite(g_traceProvider, "xrStringToPath", TLXArg(instance, "Instance"), TLArg(pathString, "String"));

        if (instance != XR_NULL_PATH && (!m_instanceCreated || instance != (XrInstance)1)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        std::string_view str(pathString);

        bool found = false;
        for (auto entry : m_strings) {
            if (entry.second == str) {
                *path = entry.first;
                found = true;
                break;
            }
        }

        if (!found) {
            if (str.length() >= XR_MAX_PATH_LENGTH || !validatePath(pathString)) {
                return XR_ERROR_PATH_FORMAT_INVALID;
            }

            *path = (XrPath)++m_stringIndex;
            m_strings.insert_or_assign(*path, str);
        }

        TraceLoggingWrite(g_traceProvider, "xrStringToPath", TLArg(*path, "Path"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrPathToString
    XrResult OpenXrRuntime::xrPathToString(
        XrInstance instance, XrPath path, uint32_t bufferCapacityInput, uint32_t* bufferCountOutput, char* buffer) {
        TraceLoggingWrite(g_traceProvider,
                          "xrPathToString",
                          TLXArg(instance, "Instance"),
                          TLArg(path, "Path"),
                          TLArg(bufferCapacityInput, "BufferCapacityInput"));

        if (instance != XR_NULL_PATH && (!m_instanceCreated || instance != (XrInstance)1)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        const auto it = m_strings.find(path);
        if (it == m_strings.cend()) {
            return XR_ERROR_PATH_INVALID;
        }

        const auto& str = it->second;
        if (bufferCapacityInput && bufferCapacityInput < str.length()) {
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        *bufferCountOutput = (uint32_t)str.length() + 1;
        TraceLoggingWrite(g_traceProvider, "xrPathToString", TLArg(*bufferCountOutput, "BufferCountOutput"));

        if (bufferCapacityInput && buffer) {
            sprintf_s(buffer, bufferCapacityInput, "%s", str.c_str());
            TraceLoggingWrite(g_traceProvider, "xrPathToString", TLArg(buffer, "String"));
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrCreateActionSet
    XrResult OpenXrRuntime::xrCreateActionSet(XrInstance instance,
                                              const XrActionSetCreateInfo* createInfo,
                                              XrActionSet* actionSet) {
        if (createInfo->type != XR_TYPE_ACTION_SET_CREATE_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrCreateActionSet",
                          TLXArg(instance, "Instance"),
                          TLArg(createInfo->actionSetName, "Name"),
                          TLArg(createInfo->localizedActionSetName, "LocalizedName"),
                          TLArg(createInfo->priority, "Priority"));

        if (!m_instanceCreated || instance != (XrInstance)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        const std::string_view name(createInfo->actionSetName);
        if (name.empty()) {
            return XR_ERROR_NAME_INVALID;
        }

        if (!validateString(name)) {
            return XR_ERROR_PATH_FORMAT_INVALID;
        }

        const std::string_view localizedName(createInfo->localizedActionSetName);
        if (localizedName.empty()) {
            return XR_ERROR_LOCALIZED_NAME_INVALID;
        }

        for (const auto& entry : m_actionSets) {
            const ActionSet& xrActionSet = *(ActionSet*)entry;

            if (xrActionSet.name == name) {
                return XR_ERROR_NAME_DUPLICATED;
            }
            if (xrActionSet.localizedName == localizedName) {
                return XR_ERROR_LOCALIZED_NAME_DUPLICATED;
            }
        }

        // CONFORMANCE: We do not support the notion of priority. TODO: Sort actionSources by priority.

        // Create the internal struct.
        ActionSet& xrActionSet = *new ActionSet;
        xrActionSet.name = name;
        xrActionSet.localizedName = localizedName;

        *actionSet = (XrActionSet)&xrActionSet;

        // Maintain a list of known actionsets for validation.
        m_actionSets.insert(*actionSet);

        TraceLoggingWrite(g_traceProvider, "xrCreateActionSet", TLXArg(*actionSet, "ActionSet"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrDestroyActionSet
    XrResult OpenXrRuntime::xrDestroyActionSet(XrActionSet actionSet) {
        TraceLoggingWrite(g_traceProvider, "xrDestroyActionSet", TLXArg(actionSet, "ActionSet"));

        if (!m_actionSets.count(actionSet)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        ActionSet* xrActionSet = (ActionSet*)actionSet;

        delete xrActionSet;
        m_actionSets.erase(actionSet);
        m_activeActionSets.erase(actionSet);

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrCreateAction
    XrResult OpenXrRuntime::xrCreateAction(XrActionSet actionSet,
                                           const XrActionCreateInfo* createInfo,
                                           XrAction* action) {
        if (createInfo->type != XR_TYPE_ACTION_CREATE_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrCreateAction",
                          TLXArg(actionSet, "ActionSet"),
                          TLArg(createInfo->actionName, "Name"),
                          TLArg(createInfo->localizedActionName, "LocalizedName"),
                          TLArg(xr::ToCString(createInfo->actionType), "Type"));
        for (uint32_t i = 0; i < createInfo->countSubactionPaths; i++) {
            TraceLoggingWrite(g_traceProvider,
                              "xrCreateAction",
                              TLArg(getXrPath(createInfo->subactionPaths[i]).c_str(), "SubactionPath"));
        }

        if (createInfo->actionType != XR_ACTION_TYPE_BOOLEAN_INPUT &&
            createInfo->actionType != XR_ACTION_TYPE_FLOAT_INPUT &&
            createInfo->actionType != XR_ACTION_TYPE_POSE_INPUT &&
            createInfo->actionType != XR_ACTION_TYPE_VECTOR2F_INPUT &&
            createInfo->actionType != XR_ACTION_TYPE_VIBRATION_OUTPUT) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        if (!m_actionSets.count(actionSet)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (m_activeActionSets.count(actionSet)) {
            return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED;
        }

        const std::string_view name(createInfo->actionName);
        if (name.empty()) {
            return XR_ERROR_NAME_INVALID;
        }

        if (!validateString(name)) {
            return XR_ERROR_PATH_FORMAT_INVALID;
        }

        const std::string_view localizedName(createInfo->localizedActionName);
        if (localizedName.empty()) {
            return XR_ERROR_LOCALIZED_NAME_INVALID;
        }

        for (const auto& entry : m_actions) {
            const Action& xrAction = *(Action*)entry;

            if (xrAction.actionSet != actionSet) {
                continue;
            }

            if (xrAction.name == name) {
                return XR_ERROR_NAME_DUPLICATED;
            }
            if (xrAction.localizedName == localizedName) {
                return XR_ERROR_LOCALIZED_NAME_DUPLICATED;
            }
        }

        for (uint32_t i = 0; i < createInfo->countSubactionPaths; i++) {
            const std::string& subactionPath = getXrPath(createInfo->subactionPaths[i]);
            if (subactionPath != "/user/hand/left" && subactionPath != "/user/hand/right") {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        // Create the internal struct.
        Action& xrAction = *new Action;
        xrAction.type = createInfo->actionType;
        xrAction.name = name;
        xrAction.localizedName = localizedName;
        xrAction.actionSet = actionSet;
        for (uint32_t i = 0; i < createInfo->countSubactionPaths; i++) {
            xrAction.subactionPaths.insert(createInfo->subactionPaths[i]);
        }

        *action = (XrAction)&xrAction;

        // Maintain a list of known actions for validation.
        m_actions.insert(*action);
        m_actionsForCleanup.insert(*action);

        TraceLoggingWrite(g_traceProvider, "xrCreateAction", TLXArg(*action, "Action"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrDestroyAction
    XrResult OpenXrRuntime::xrDestroyAction(XrAction action) {
        TraceLoggingWrite(g_traceProvider, "xrDestroyAction", TLXArg(action, "Action"));

        if (!m_actions.count(action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        // We do not delete the action as it might still be used internally (eg: referenced by action spaces).

        m_actions.erase(action);

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrSuggestInteractionProfileBindings
    XrResult OpenXrRuntime::xrSuggestInteractionProfileBindings(
        XrInstance instance, const XrInteractionProfileSuggestedBinding* suggestedBindings) {
        if (suggestedBindings->type != XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrSuggestInteractionProfileBindings",
                          TLXArg(instance, "Instance"),
                          TLArg(getXrPath(suggestedBindings->interactionProfile).c_str(), "InteractionProfile"));

        if (!m_instanceCreated || instance != (XrInstance)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!suggestedBindings->countSuggestedBindings) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        for (uint32_t i = 0; i < suggestedBindings->countSuggestedBindings; i++) {
            TraceLoggingWrite(g_traceProvider,
                              "xrSuggestInteractionProfileBindings",
                              TLXArg(suggestedBindings->suggestedBindings[i].action, "Action"),
                              TLArg(getXrPath(suggestedBindings->suggestedBindings[i].binding).c_str(), "Path"));
        }

        if (m_activeActionSets.size()) {
            return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED;
        }

        const std::string& interactionProfile = getXrPath(suggestedBindings->interactionProfile);
        const bool isEyeTracker = interactionProfile == "/interaction_profiles/ext/eye_gaze_interaction";
        const bool isViveTracker = interactionProfile == "/interaction_profiles/htc/vive_tracker_htcx";
        if (isEyeTracker) {
            // Only allow this if the extension is enabled.
            if (!has_XR_EXT_eye_gaze_interaction) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }

            LOG_TELEMETRY_ONCE(logFeature("EyeGazeInteraction"));

            // Eye tracker does not go through the controller mappings. Instead, we directly bind the action source.
            for (uint32_t i = 0; i < suggestedBindings->countSuggestedBindings; i++) {
                const std::string& path = getXrPath(suggestedBindings->suggestedBindings[i].binding);
                if (!isActionEyeTracker(path)) {
                    return XR_ERROR_PATH_UNSUPPORTED;
                }

                Action& xrAction = *(Action*)suggestedBindings->suggestedBindings[i].action;

                ActionSource source{};
                source.realPath = path;
                xrAction.actionSources.insert_or_assign(path, source);
            }

            m_hasEyeTrackerBindings = true;
            m_currentInteractionProfileDirty = true;
        } else if (isViveTracker) {
            // Only allow this if the extension is enabled.
            if (!has_XR_HTCX_vive_tracker_interaction) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }

            LOG_TELEMETRY_ONCE(logFeature("ViveTrackerInteraction"));
        }

        if (!isEyeTracker) {
            // Set up to use the controller mappings when a controller/tracker is rebinding.
            const auto checkValidPathIt = m_controllerValidPathsTable.find(interactionProfile);
            if (checkValidPathIt == m_controllerValidPathsTable.cend()) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }

            std::vector<XrActionSuggestedBinding> bindings;
            for (uint32_t i = 0; i < suggestedBindings->countSuggestedBindings; i++) {
                const std::string& path = getXrPath(suggestedBindings->suggestedBindings[i].binding);
                if (getActionSide(path, true) < 0 || !checkValidPathIt->second(path)) {
                    return XR_ERROR_PATH_UNSUPPORTED;
                }

                bindings.push_back(suggestedBindings->suggestedBindings[i]);
            }

            m_suggestedBindings.insert_or_assign(getXrPath(suggestedBindings->interactionProfile), bindings);
        }

        if (isViveTracker) {
            m_hasViveTrackerBindings = true;
            m_currentInteractionProfileDirty = true;
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrAttachSessionActionSets
    XrResult OpenXrRuntime::xrAttachSessionActionSets(XrSession session,
                                                      const XrSessionActionSetsAttachInfo* attachInfo) {
        if (attachInfo->type != XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        if (!attachInfo->countActionSets) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider, "xrAttachSessionActionSets", TLXArg(session, "Session"));
        for (uint32_t i = 0; i < attachInfo->countActionSets; i++) {
            TraceLoggingWrite(
                g_traceProvider, "xrAttachSessionActionSets", TLXArg(attachInfo->actionSets[i], "ActionSet"));
        }

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (m_activeActionSets.size()) {
            return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED;
        }

        for (uint32_t i = 0; i < attachInfo->countActionSets; i++) {
            if (!m_actionSets.count(attachInfo->actionSets[i])) {
                return XR_ERROR_HANDLE_INVALID;
            }
        }

        for (uint32_t i = 0; i < attachInfo->countActionSets; i++) {
            m_activeActionSets.insert(attachInfo->actionSets[i]);

            ActionSet& xrActionSet = *(ActionSet*)attachInfo->actionSets[i];

            // Identify all valid subaction paths for the actionset.
            for (const auto& entry : m_actions) {
                const Action& xrAction = *(Action*)entry;

                xrActionSet.subactionPaths.insert(xrAction.subactionPaths.begin(), xrAction.subactionPaths.end());
            }
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetCurrentInteractionProfile
    XrResult OpenXrRuntime::xrGetCurrentInteractionProfile(XrSession session,
                                                           XrPath topLevelUserPath,
                                                           XrInteractionProfileState* interactionProfile) {
        if (interactionProfile->type != XR_TYPE_INTERACTION_PROFILE_STATE) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetCurrentInteractionProfile",
                          TLXArg(session, "Session"),
                          TLArg(getXrPath(topLevelUserPath).c_str(), "TopLevelUserPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (m_activeActionSets.empty()) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        const auto topLevelPath = getXrPath(topLevelUserPath);
        if (topLevelPath.empty() || topLevelPath == "<unknown>") {
            return XR_ERROR_PATH_INVALID;
        }

        interactionProfile->interactionProfile = XR_NULL_PATH;
        if (topLevelPath == "/user/hand/left" || topLevelPath == "/user/hand/right") {
            interactionProfile->interactionProfile = m_currentInteractionProfile[getActionSide(topLevelPath)];
        } else if (topLevelPath == "/user/eyes_ext") {
            if (m_hasEyeTrackerBindings) {
                CHECK_XRCMD(xrStringToPath(XR_NULL_HANDLE,
                                           "/interaction_profiles/ext/eye_gaze_interaction",
                                           &interactionProfile->interactionProfile));
            }
        } else if (topLevelPath == "/user/vive_tracker_htcx") {
            if (m_hasViveTrackerBindings) {
                CHECK_XRCMD(xrStringToPath(XR_NULL_HANDLE,
                                           "/interaction_profiles/htc/vive_tracker_htcx",
                                           &interactionProfile->interactionProfile));
            }
        } else if (topLevelPath == "/user/head" || topLevelPath == "/user/gamepad") {
        } else {
            return XR_ERROR_PATH_UNSUPPORTED;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetCurrentInteractionProfile",
                          TLArg(getXrPath(interactionProfile->interactionProfile).c_str(), "InteractionProfile"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetActionStateBoolean
    XrResult OpenXrRuntime::xrGetActionStateBoolean(XrSession session,
                                                    const XrActionStateGetInfo* getInfo,
                                                    XrActionStateBoolean* state) {
        if (getInfo->type != XR_TYPE_ACTION_STATE_GET_INFO || state->type != XR_TYPE_ACTION_STATE_BOOLEAN) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStateBoolean",
                          TLXArg(session, "Session"),
                          TLXArg(getInfo->action, "Action"),
                          TLArg(getXrPath(getInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(getInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)getInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_BOOLEAN_INPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (getInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(getInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(getInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        std::optional<bool> combinedState;
        const std::string& subActionPath = getXrPath(getInfo->subactionPath);
        const int subActionSide = std::max(0, getActionSide(subActionPath));
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            const auto& value = source.second;
            const bool isBound = value.buttonMap != nullptr || value.floatValue != nullptr;
            TraceLoggingWrite(g_traceProvider,
                              "xrGetActionStateBoolean",
                              TLArg(fullPath.c_str(), "ActionSourcePath"),
                              TLArg(isBound, "Bound"));

            // We only support hands paths, not gamepad etc.
            const int side = getActionSide(fullPath);
            if (isBound && side >= 0) {
                if (m_isControllerActive[side]) {
                    // Per spec, the combined state is the OR of all values.
                    if (value.buttonMap) {
                        combinedState = combinedState.value_or(false) || value.buttonMap[side] & value.buttonType;
                    } else if (value.floatValue) {
                        combinedState = combinedState.value_or(false) || value.floatValue[side] > 0.99f;
                    }
                }
            }
        }

        state->isActive = combinedState ? XR_TRUE : XR_FALSE;
        if (combinedState) {
            state->currentState = combinedState.value();
            state->changedSinceLastSync = !!state->currentState != xrAction.lastBoolValue[subActionSide];

            const ActionSet& xrActionSet = *(ActionSet*)xrAction.actionSet;
            state->lastChangeTime = state->changedSinceLastSync
                                        ? pvrTimeToXrTime(xrActionSet.cachedInputState.TimeInSeconds)
                                        : xrAction.lastBoolValueChangedTime[subActionSide];
        } else {
            state->currentState = state->changedSinceLastSync = XR_FALSE;
            state->lastChangeTime = 0;
        }

        xrAction.lastBoolValue[subActionSide] = state->currentState;
        xrAction.lastBoolValueChangedTime[subActionSide] = state->lastChangeTime;

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStateBoolean",
                          TLArg(!!state->isActive, "Active"),
                          TLArg(!!state->currentState, "CurrentState"),
                          TLArg(!!state->changedSinceLastSync, "ChangedSinceLastSync"),
                          TLArg(state->lastChangeTime, "LastChangeTime"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetActionStateFloat
    XrResult OpenXrRuntime::xrGetActionStateFloat(XrSession session,
                                                  const XrActionStateGetInfo* getInfo,
                                                  XrActionStateFloat* state) {
        if (getInfo->type != XR_TYPE_ACTION_STATE_GET_INFO || state->type != XR_TYPE_ACTION_STATE_FLOAT) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStateFloat",
                          TLXArg(session, "Session"),
                          TLXArg(getInfo->action, "Action"),
                          TLArg(getXrPath(getInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(getInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)getInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_FLOAT_INPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (getInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(getInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(getInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        std::optional<float> combinedState;
        const std::string& subActionPath = getXrPath(getInfo->subactionPath);
        const int subActionSide = std::max(0, getActionSide(subActionPath));
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            const auto& value = source.second;
            const bool isBound = value.floatValue != nullptr ||
                                 (value.vector2fValue != nullptr && value.vector2fIndex >= 0) ||
                                 value.buttonMap != nullptr;
            TraceLoggingWrite(g_traceProvider,
                              "xrGetActionStateFloat",
                              TLArg(fullPath.c_str(), "ActionSourcePath"),
                              TLArg(isBound, "Bound"));

            // We only support hands paths, not gamepad etc.
            const int side = getActionSide(fullPath);
            if (isBound && side >= 0) {
                if (m_isControllerActive[side]) {
                    // Per spec, the combined state is the absolute maximum of all values.
                    if (value.floatValue) {
                        combinedState = std::max(combinedState.value_or(-std::numeric_limits<float>::infinity()),
                                                 value.floatValue[side]);
                    } else if (value.buttonMap) {
                        combinedState = std::max(combinedState.value_or(-std::numeric_limits<float>::infinity()),
                                                 value.buttonMap[side] & value.buttonType ? 1.f : 0.f);
                    } else if (value.vector2fValue) {
                        const XrVector2f vector2fValue = handleJoystickDeadzone(value.vector2fValue[side]);

                        combinedState = std::max(combinedState.value_or(-std::numeric_limits<float>::infinity()),
                                                 value.vector2fIndex == 0 ? vector2fValue.x : vector2fValue.y);
                    }
                }
            }
        }

        state->isActive = combinedState ? XR_TRUE : XR_FALSE;
        if (combinedState) {
            state->currentState = combinedState.value();
            state->changedSinceLastSync = state->currentState != xrAction.lastFloatValue[subActionSide];

            const ActionSet& xrActionSet = *(ActionSet*)xrAction.actionSet;
            state->lastChangeTime = state->changedSinceLastSync
                                        ? pvrTimeToXrTime(xrActionSet.cachedInputState.TimeInSeconds)
                                        : xrAction.lastFloatValueChangedTime[subActionSide];
        } else {
            state->currentState = 0.0f;
            state->changedSinceLastSync = XR_FALSE;
            state->lastChangeTime = 0;
        }

        xrAction.lastFloatValue[subActionSide] = state->currentState;
        xrAction.lastFloatValueChangedTime[subActionSide] = state->lastChangeTime;

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStateFloat",
                          TLArg(!!state->isActive, "Active"),
                          TLArg(state->currentState, "CurrentState"),
                          TLArg(!!state->changedSinceLastSync, "ChangedSinceLastSync"),
                          TLArg(state->lastChangeTime, "LastChangeTime"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetActionStateVector2f
    XrResult OpenXrRuntime::xrGetActionStateVector2f(XrSession session,
                                                     const XrActionStateGetInfo* getInfo,
                                                     XrActionStateVector2f* state) {
        if (getInfo->type != XR_TYPE_ACTION_STATE_GET_INFO || state->type != XR_TYPE_ACTION_STATE_VECTOR2F) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStateVector2f",
                          TLXArg(session, "Session"),
                          TLXArg(getInfo->action, "Action"),
                          TLArg(getXrPath(getInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(getInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)getInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_VECTOR2F_INPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (getInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(getInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(getInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        std::optional<XrVector2f> combinedState;
        const std::string& subActionPath = getXrPath(getInfo->subactionPath);
        const int subActionSide = std::max(0, getActionSide(subActionPath));
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            const auto& value = source.second;
            const bool isBound = value.vector2fValue != nullptr;
            TraceLoggingWrite(g_traceProvider,
                              "xrGetActionStateVector2f",
                              TLArg(fullPath.c_str(), "ActionSourcePath"),
                              TLArg(isBound, "Bound"));

            // We only support hands paths, not gamepad etc.
            const int side = getActionSide(fullPath);
            if (isBound && side >= 0) {
                if (m_isControllerActive[side] && value.vector2fValue) {
                    const XrVector2f vector2fValue = handleJoystickDeadzone(value.vector2fValue[side]);

                    // Per spec, the combined state if the one of the vector with the longest length.
                    const float l1 = combinedState ? sqrt(combinedState.value().x * combinedState.value().x +
                                                          combinedState.value().y * combinedState.value().y)
                                                   : 0.f;
                    const float l2 = sqrt(vector2fValue.x * vector2fValue.x + vector2fValue.y * vector2fValue.y);
                    if (l2 >= l1) {
                        combinedState = vector2fValue;
                    }
                }
            }
        }

        state->isActive = combinedState ? XR_TRUE : XR_FALSE;
        if (combinedState) {
            state->currentState = combinedState.value();

            state->changedSinceLastSync = state->currentState.x != xrAction.lastVector2fValue[subActionSide].x ||
                                          state->currentState.y != xrAction.lastVector2fValue[subActionSide].y;

            const ActionSet& xrActionSet = *(ActionSet*)xrAction.actionSet;
            state->lastChangeTime = state->changedSinceLastSync
                                        ? pvrTimeToXrTime(xrActionSet.cachedInputState.TimeInSeconds)
                                        : xrAction.lastVector2fValueChangedTime[subActionSide];
        } else {
            state->currentState = {0.0f, 0.0f};
            state->changedSinceLastSync = XR_FALSE;
            state->lastChangeTime = 0;
        }

        xrAction.lastVector2fValue[subActionSide] = state->currentState;
        xrAction.lastVector2fValueChangedTime[subActionSide] = state->lastChangeTime;

        TraceLoggingWrite(
            g_traceProvider,
            "xrGetActionStateVector2f",
            TLArg(!!state->isActive, "Active"),
            TLArg(fmt::format("{}, {}", state->currentState.x, state->currentState.y).c_str(), "CurrentState"),
            TLArg(!!state->changedSinceLastSync, "ChangedSinceLastSync"),
            TLArg(state->lastChangeTime, "LastChangeTime"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetActionStatePose
    XrResult OpenXrRuntime::xrGetActionStatePose(XrSession session,
                                                 const XrActionStateGetInfo* getInfo,
                                                 XrActionStatePose* state) {
        if (getInfo->type != XR_TYPE_ACTION_STATE_GET_INFO || state->type != XR_TYPE_ACTION_STATE_POSE) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetActionStatePose",
                          TLXArg(session, "Session"),
                          TLXArg(getInfo->action, "Action"),
                          TLArg(getXrPath(getInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(getInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)getInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_POSE_INPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (getInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(getInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(getInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        const std::string& subActionPath = getXrPath(getInfo->subactionPath);
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            TraceLoggingWrite(g_traceProvider, "xrGetActionStatePose", TLArg(fullPath.c_str(), "ActionSourcePath"));

            // We only support hands paths and eye tracker, not gamepad etc.
            if (!isActionEyeTracker(fullPath)) {
                const int side = getActionSide(fullPath);
                if (side >= 0) {
                    state->isActive = m_isControllerActive[side] ? XR_TRUE : XR_FALSE;

                    // Per spec we must consistently pick one source. We pick the first one.
                    break;
                } else if (getTrackerIndex(fullPath) >= 0) {
                    state->isActive = XR_TRUE;

                    // Per spec we must consistently pick one source. We pick the first one.
                    break;
                }
            } else {
                state->isActive = m_isEyeTrackingAvailable ? XR_TRUE : XR_FALSE;

                // Per spec we must consistently pick one source. We pick the first one.
                break;
            }
        }

        TraceLoggingWrite(g_traceProvider, "xrGetActionStatePose", TLArg(!!state->isActive, "Active"));

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrSyncActions
    XrResult OpenXrRuntime::xrSyncActions(XrSession session, const XrActionsSyncInfo* syncInfo) {
        if (syncInfo->type != XR_TYPE_ACTIONS_SYNC_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider, "xrSyncActions", TLXArg(session, "Session"));
        for (uint32_t i = 0; i < syncInfo->countActiveActionSets; i++) {
            TraceLoggingWrite(g_traceProvider,
                              "xrSyncActions",
                              TLXArg(syncInfo->activeActionSets[i].actionSet, "ActionSet"),
                              TLArg(getXrPath(syncInfo->activeActionSets[i].subactionPath).c_str(), "SubactionPath"));
        }

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        bool doSide[xr::Side::Count] = {false, false};
        for (uint32_t i = 0; i < syncInfo->countActiveActionSets; i++) {
            if (!m_activeActionSets.count(syncInfo->activeActionSets[i].actionSet)) {
                return XR_ERROR_ACTIONSET_NOT_ATTACHED;
            }

            if (syncInfo->activeActionSets[i].subactionPath == XR_NULL_PATH) {
                doSide[xr::Side::Left] = doSide[xr::Side::Right] = true;
            } else {
                const ActionSet& xrActionSet = *(ActionSet*)syncInfo->activeActionSets[i].actionSet;

                if (!xrActionSet.subactionPaths.count(syncInfo->activeActionSets[i].subactionPath)) {
                    return XR_ERROR_PATH_UNSUPPORTED;
                }

                const int side = getActionSide(getXrPath(syncInfo->activeActionSets[i].subactionPath));
                if (side == xr::Side::Left || side == xr::Side::Right) {
                    doSide[side] = true;
                }
            }
        }

        if (m_sessionState != XR_SESSION_STATE_FOCUSED) {
            return XR_SESSION_NOT_FOCUSED;
        }

        // Latch the state of all inputs, and we will let the further calls to xrGetActionState*() do the triage.
        CHECK_PVRCMD(pvr_getInputState(m_pvrSession, &m_cachedInputState));
        bool wasRecenteringPressed = false;
        for (uint32_t side = 0; side < xr::Side::Count; side++) {
            if (!doSide[side]) {
                continue;
            }

            TraceLoggingWrite(
                g_traceProvider,
                "PVR_InputState",
                TLArg(side == xr::Side::Left ? "Left" : "Right", "Side"),
                TLArg(m_cachedInputState.TimeInSeconds, "TimeInSeconds"),
                TLArg(m_cachedInputState.HandButtons[side], "ButtonPress"),
                TLArg(m_cachedInputState.HandTouches[side], "ButtonTouches"),
                TLArg(m_cachedInputState.Trigger[side], "Trigger"),
                TLArg(m_cachedInputState.Grip[side], "Grip"),
                TLArg(m_cachedInputState.GripForce[side], "GripForce"),
                TLArg(fmt::format("{}, {}", m_cachedInputState.JoyStick[side].x, m_cachedInputState.JoyStick[side].y)
                          .c_str(),
                      "Joystick"),
                TLArg(fmt::format("{}, {}", m_cachedInputState.TouchPad[side].x, m_cachedInputState.TouchPad[side].y)
                          .c_str(),
                      "Touchpad"),
                TLArg(m_cachedInputState.TouchPadForce[side], "TouchpadForce"),
                TLArg(m_cachedInputState.fingerIndex[side], "IndexFinger"),
                TLArg(m_cachedInputState.fingerMiddle[side], "MiddleFinger"),
                TLArg(m_cachedInputState.fingerRing[side], "RingFinger"),
                TLArg(m_cachedInputState.fingerPinky[side], "PinkyFinger"));

            // Propagate the input state to the entire action state.
            for (uint32_t i = 0; i < syncInfo->countActiveActionSets; i++) {
                ActionSet& xrActionSet = *(ActionSet*)syncInfo->activeActionSets[i].actionSet;

                xrActionSet.cachedInputState = m_cachedInputState;
            }

            // Look for changes in controller/interaction profiles.
            const auto lastControllerType = m_cachedControllerType[side];
            const int size = pvr_getTrackedDeviceStringProperty(
                m_pvrSession,
                side == xr::Side::Left ? pvrTrackedDevice_LeftController : pvrTrackedDevice_RightController,
                pvrTrackedDeviceProp_ControllerType_String,
                nullptr,
                0);
            m_isControllerActive[side] = size > 0;
            if (m_isControllerActive[side]) {
                if (m_debugControllerType.empty()) {
                    m_cachedControllerType[side].resize(size, 0);
                    pvr_getTrackedDeviceStringProperty(m_pvrSession,
                                                       side == xr::Side::Left ? pvrTrackedDevice_LeftController
                                                                              : pvrTrackedDevice_RightController,
                                                       pvrTrackedDeviceProp_ControllerType_String,
                                                       m_cachedControllerType[side].data(),
                                                       (int)m_cachedControllerType[side].size() + 1);
                    // Remove trailing 0.
                    m_cachedControllerType[side].resize(size - 1, 0);
                } else {
                    m_cachedControllerType[side] = m_debugControllerType;
                }
            } else {
                m_cachedControllerType[side].clear();
            }

            if (lastControllerType != m_cachedControllerType[side] ||
                m_forcedInteractionProfile != m_lastForcedInteractionProfile) {
                if (!m_cachedControllerType[side].empty()) {
                    Log("Detected controller: %s (%s)\n",
                        m_cachedControllerType[side].c_str(),
                        side == xr::Side::Left ? "Left" : "Right");
                }
                TraceLoggingWrite(g_traceProvider,
                                  "PVR_ControllerType",
                                  TLArg(side == xr::Side::Left ? "Left" : "Right", "Side"),
                                  TLArg(m_cachedControllerType[side].c_str(), "Type"));
                rebindControllerActions(side);
            }

            // Check for built-in actions.
            wasRecenteringPressed =
                wasRecenteringPressed || (((m_cachedInputState.HandButtons[side] & pvrButton_System) ||
                                           (m_cachedInputState.HandButtons[side] & pvrButton_ApplicationMenu)) &&
                                          (m_cachedInputState.HandButtons[side] & pvrButton_Trigger));
        }
        m_lastForcedInteractionProfile = m_forcedInteractionProfile;

        // Handle Vive Trackers attach/detach.
        {
            constexpr pvrTrackedDeviceType tracker[] = {pvrTrackedDevice_Tracker0,
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

            const uint32_t numTrackers = pvr_getTrackerCount(m_pvrSession);
            std::map<std::string, uint32_t> trackers;
            for (uint32_t i = 0; i < numTrackers; i++) {
                const int size = pvr_getTrackedDeviceStringProperty(
                    m_pvrSession, tracker[i], pvrTrackedDeviceProp_Serial_String, nullptr, 0);
                if (size > 0) {
                    std::string serial;
                    serial.resize(size, 0);
                    pvr_getTrackedDeviceStringProperty(m_pvrSession,
                                                       tracker[i],
                                                       pvrTrackedDeviceProp_Serial_String,
                                                       serial.data(),
                                                       (int)serial.size() + 1);
                    // Remove trailing 0.
                    serial.resize(size - 1, 0);
                    std::transform(
                        serial.begin(), serial.end(), serial.begin(), [](unsigned char c) { return std::tolower(c); });

                    // Generate a connected event.
                    if (m_trackers.find(serial) == m_trackers.end()) {
                        Log("Detected tracker: %s\n", serial.c_str());
                        TraceLoggingWrite(g_traceProvider,
                                          "PVR_Tracker",
                                          TLArg(i, "Index"),
                                          TLArg(serial.c_str(), "Serial"),
                                          TLArg(getTrackerRolePath(serial).c_str(), "RolePath"));
                        m_trackersNotifications.push_back(serial);

                        rebindTrackerActions(serial, true);
                    }
                    trackers.insert_or_assign(serial, i);
                }
            }

            std::unique_lock lock(m_trackersLock);

            if (m_trackers.size() != trackers.size()) {
                // Unbind trackers that were disconnected.
                for (const auto& tracker : m_trackers) {
                    const std::string& serial = tracker.first;

                    if (trackers.find(serial) == trackers.cend()) {
                        rebindTrackerActions(serial, false);
                    }
                }
            }

            m_trackers = trackers;
        }

        // Execute built-in actions.
        handleBuiltinActions(wasRecenteringPressed);
        m_actionsSyncedThisFrame = true;

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrEnumerateBoundSourcesForAction
    XrResult OpenXrRuntime::xrEnumerateBoundSourcesForAction(XrSession session,
                                                             const XrBoundSourcesForActionEnumerateInfo* enumerateInfo,
                                                             uint32_t sourceCapacityInput,
                                                             uint32_t* sourceCountOutput,
                                                             XrPath* sources) {
        if (enumerateInfo->type != XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrEnumerateBoundSourcesForAction",
                          TLXArg(session, "Session"),
                          TLXArg(enumerateInfo->action, "Action"),
                          TLArg(sourceCapacityInput, "SourceCapacityInput"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(enumerateInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)enumerateInfo->action;

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (sourceCapacityInput && sourceCapacityInput < xrAction.actionSources.size()) {
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        *sourceCountOutput = (uint32_t)xrAction.actionSources.size();
        TraceLoggingWrite(
            g_traceProvider, "xrEnumerateBoundSourcesForAction", TLArg(*sourceCountOutput, "SourceCountOutput"));

        if (sourceCapacityInput && sources) {
            uint32_t i = 0;
            for (const auto& source : xrAction.actionSources) {
                CHECK_XRCMD(xrStringToPath(XR_NULL_HANDLE, source.second.realPath.c_str(), &sources[i]));
                TraceLoggingWrite(g_traceProvider,
                                  "xrEnumerateBoundSourcesForAction",
                                  TLArg(source.first.c_str(), "Source"),
                                  TLArg(sources[i], "Path"));
                i++;
            }
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrGetInputSourceLocalizedName
    XrResult OpenXrRuntime::xrGetInputSourceLocalizedName(XrSession session,
                                                          const XrInputSourceLocalizedNameGetInfo* getInfo,
                                                          uint32_t bufferCapacityInput,
                                                          uint32_t* bufferCountOutput,
                                                          char* buffer) {
        if (getInfo->type != XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrGetInputSourceLocalizedName",
                          TLXArg(session, "Session"),
                          TLArg(getXrPath(getInfo->sourcePath).c_str(), "SourcePath"),
                          TLArg(getInfo->whichComponents, "WhichComponents"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (m_activeActionSets.empty()) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (!getInfo->whichComponents) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        const std::string path = getXrPath(getInfo->sourcePath);
        if (path.empty() || path == "<unknown>") {
            return XR_ERROR_PATH_INVALID;
        }

        // Build the string.
        std::string localizedName;
        if (!isActionEyeTracker(path)) {
            const int side = getActionSide(path);
            std::string trackerRole;
            if (startsWith(path, "/user/vive_tracker_htcx/role/")) {
                trackerRole = path.substr(29);
            }
            if (side >= 0 || !trackerRole.empty()) {
                bool needSpace = false;

                if ((getInfo->whichComponents & XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT)) {
                    if (side >= 0) {
                        localizedName += side == xr::Side::Left ? "Left Hand" : "Right Hand";
                    } else {
                        if (trackerRole == "handheld_object") {
                            localizedName += "Object held in hand";
                        } else if (trackerRole == "left_foot") {
                            localizedName += "Left Foot";
                        } else if (trackerRole == "right_foot") {
                            localizedName += "Right Foot";
                        } else if (trackerRole == "left_shoulder") {
                            localizedName += "Left Shoulder";
                        } else if (trackerRole == "right_shoulder") {
                            localizedName += "Right Shoulder";
                        } else if (trackerRole == "left_elbow") {
                            localizedName += "Left Elbow";
                        } else if (trackerRole == "right_elbow") {
                            localizedName += "Right Elbow";
                        } else if (trackerRole == "left_knee") {
                            localizedName += "Left Knee";
                        } else if (trackerRole == "right_knee") {
                            localizedName += "Right Knee";
                        } else if (trackerRole == "waist") {
                            localizedName += "Waist";
                        } else if (trackerRole == "chest") {
                            localizedName += "Chest";
                        } else if (trackerRole == "camera") {
                            localizedName += "Camera";
                        } else if (trackerRole == "keyboard") {
                            localizedName += "Keyboard";
                        }
                    }
                    needSpace = true;
                }

                if ((getInfo->whichComponents & XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT)) {
                    if (needSpace) {
                        localizedName += " ";
                    }
                    if (side >= 0) {
                        localizedName += m_localizedControllerType[side];
                    } else {
                        localizedName += "Vive Tracker";
                    }
                    needSpace = true;
                }

                if ((getInfo->whichComponents & XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT)) {
                    if (needSpace) {
                        localizedName += " ";
                    }
                    if (side >= 0) {
                        if (m_cachedControllerType[side] == "vive_controller") {
                            localizedName += getViveControllerLocalizedSourceName(path);
                        } else if (m_cachedControllerType[side] == "knuckles") {
                            localizedName += getIndexControllerLocalizedSourceName(path);
                        } else if (m_cachedControllerType[side] == "pimax_crystal") {
                            localizedName += getCrystalControllerLocalizedSourceName(path);
                        } else {
                            localizedName += getSimpleControllerLocalizedSourceName(path);
                        }
                    } else {
                        localizedName += getViveTrackerLocalizedSourceName(path);
                    }
                    needSpace = true;
                }
            }
        } else {
            bool needSpace = false;

            if ((getInfo->whichComponents & XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT)) {
                localizedName += "Eye Gaze Interaction";
                needSpace = true;
            }

            if ((getInfo->whichComponents & XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT)) {
                if (needSpace) {
                    localizedName += " ";
                }
                localizedName += "Eye Tracker";
                needSpace = true;
            }
        }

        if (bufferCapacityInput && bufferCapacityInput < localizedName.length()) {
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        *bufferCountOutput = (uint32_t)localizedName.length() + 1;
        TraceLoggingWrite(
            g_traceProvider, "xrGetInputSourceLocalizedName", TLArg(*bufferCountOutput, "BufferCountOutput"));

        if (bufferCapacityInput && buffer) {
            sprintf_s(buffer, bufferCapacityInput, "%s", localizedName.c_str());
            TraceLoggingWrite(g_traceProvider, "xrGetInputSourceLocalizedName", TLArg(buffer, "String"));
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrApplyHapticFeedback
    XrResult OpenXrRuntime::xrApplyHapticFeedback(XrSession session,
                                                  const XrHapticActionInfo* hapticActionInfo,
                                                  const XrHapticBaseHeader* hapticFeedback) {
        if (hapticActionInfo->type != XR_TYPE_HAPTIC_ACTION_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrApplyHapticFeedback",
                          TLXArg(session, "Session"),
                          TLXArg(hapticActionInfo->action, "Action"),
                          TLArg(getXrPath(hapticActionInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(hapticActionInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)hapticActionInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_VIBRATION_OUTPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (m_sessionState != XR_SESSION_STATE_FOCUSED) {
            return XR_SESSION_NOT_FOCUSED;
        }

        if (hapticActionInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(hapticActionInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(hapticActionInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        const std::string& subActionPath = getXrPath(hapticActionInfo->subactionPath);
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            const bool isOutput = endsWith(fullPath, "/output/haptic");
            TraceLoggingWrite(g_traceProvider, "xrApplyHapticFeedback", TLArg(fullPath.c_str(), "ActionSourcePath"));

            // We only support hands paths, not gamepad etc.
            const int side = getActionSide(fullPath);
            const int trackerIndex = getTrackerIndex(fullPath);
            if (isOutput && (side >= 0 || trackerIndex >= 0)) {
                const XrHapticBaseHeader* entry = reinterpret_cast<const XrHapticBaseHeader*>(hapticFeedback);
                while (entry) {
                    if (entry->type == XR_TYPE_HAPTIC_VIBRATION) {
                        const XrHapticVibration* vibration = reinterpret_cast<const XrHapticVibration*>(entry);

                        TraceLoggingWrite(g_traceProvider,
                                          "xrApplyHapticFeedback",
                                          TLArg(vibration->amplitude, "Amplitude"),
                                          TLArg(vibration->frequency, "Frequency"),
                                          TLArg(vibration->duration, "Duration"));

                        constexpr pvrTrackedDeviceType tracker[] = {pvrTrackedDevice_Tracker0,
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

                        // NOTE: PVR only supports pulses, so there is nothing we can do with the
                        // frequency/duration? OpenComposite seems to pass an amplitude of 0 sometimes, which is not
                        // supported.
                        if (vibration->amplitude > 0) {
                            CHECK_PVRCMD(pvr_triggerHapticPulse(
                                m_pvrSession,
                                side == xr::Side::Left ? pvrTrackedDevice_LeftController
                                                       : (side == xr::Side::Right ? pvrTrackedDevice_RightController
                                                                                  : tracker[trackerIndex]),
                                vibration->amplitude));
                        }
                        break;
                    }

                    entry = reinterpret_cast<const XrHapticBaseHeader*>(entry->next);
                }
            }
        }

        return XR_SUCCESS;
    }

    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#xrStopHapticFeedback
    XrResult OpenXrRuntime::xrStopHapticFeedback(XrSession session, const XrHapticActionInfo* hapticActionInfo) {
        if (hapticActionInfo->type != XR_TYPE_HAPTIC_ACTION_INFO) {
            return XR_ERROR_VALIDATION_FAILURE;
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrStopHapticFeedback",
                          TLXArg(session, "Session"),
                          TLXArg(hapticActionInfo->action, "Action"),
                          TLArg(getXrPath(hapticActionInfo->subactionPath).c_str(), "SubactionPath"));

        if (!m_sessionCreated || session != (XrSession)1) {
            return XR_ERROR_HANDLE_INVALID;
        }

        if (!m_actions.count(hapticActionInfo->action)) {
            return XR_ERROR_HANDLE_INVALID;
        }

        Action& xrAction = *(Action*)hapticActionInfo->action;

        if (xrAction.type != XR_ACTION_TYPE_VIBRATION_OUTPUT) {
            return XR_ERROR_ACTION_TYPE_MISMATCH;
        }

        if (!m_activeActionSets.count(xrAction.actionSet)) {
            return XR_ERROR_ACTIONSET_NOT_ATTACHED;
        }

        if (hapticActionInfo->subactionPath != XR_NULL_PATH) {
            if (m_strings.find(hapticActionInfo->subactionPath) == m_strings.cend()) {
                return XR_ERROR_PATH_INVALID;
            }
            if (!xrAction.subactionPaths.count(hapticActionInfo->subactionPath)) {
                return XR_ERROR_PATH_UNSUPPORTED;
            }
        }

        const std::string& subActionPath = getXrPath(hapticActionInfo->subactionPath);
        for (const auto& source : xrAction.actionSources) {
            if (!startsWith(source.first, subActionPath)) {
                continue;
            }

            const std::string& fullPath = source.first;
            const bool isOutput = endsWith(fullPath, "/output/haptic");
            TraceLoggingWrite(g_traceProvider, "xrStopHapticFeedback", TLArg(fullPath.c_str(), "ActionSourcePath"));

            // We only support hands paths, not gamepad etc.
            const int side = getActionSide(fullPath);
            if (isOutput && side >= 0) {
                // Nothing to do here.
            }
        }

        // We do this at the very end to avoid any haptics to continue infinitely.
        if (m_sessionState != XR_SESSION_STATE_FOCUSED) {
            return XR_SESSION_NOT_FOCUSED;
        }

        return XR_SUCCESS;
    }

    // Update all actions with the appropriate bindings for the controller.
    void OpenXrRuntime::rebindControllerActions(int side) {
        std::string preferredInteractionProfile;
        std::string actualInteractionProfile;
        XrPosef gripPose = Pose::Identity();
        XrPosef aimPose = Pose::Identity();
        XrPosef handPose = Pose::Identity();

        // Remove all old bindings for this controller.
        for (const auto& action : m_actions) {
            Action& xrAction = *(Action*)action;

            for (auto it = xrAction.actionSources.begin(); it != xrAction.actionSources.end();) {
                if (getActionSide(it->first) == side) {
                    it = xrAction.actionSources.erase(it);
                } else {
                    it++;
                }
            }
        }

        if (!m_cachedControllerType[side].empty()) {
            // Identify the physical controller type.
            if (m_cachedControllerType[side] == "vive_controller") {
                preferredInteractionProfile = "/interaction_profiles/htc/vive_controller";
                m_localizedControllerType[side] = "Vive Controller";
                aimPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-45.f), 0, 0}),
                                         XrVector3f{0, 0, -0.05f});
                handPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-32.f), 0, 0}),
                                          XrVector3f{0.03f, -0.062f, -0.1f});
            } else if (m_cachedControllerType[side] == "knuckles") {
                preferredInteractionProfile = "/interaction_profiles/valve/index_controller";
                m_localizedControllerType[side] = "Index Controller";
                aimPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-40.f), 0, 0}),
                                         XrVector3f{0, 0, -0.05f});
                handPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-32.f), 0, 0}),
                                          XrVector3f{0.03f, -0.062f, -0.1f});
            } else if (m_cachedControllerType[side] == "pimax_crystal") {
                preferredInteractionProfile = "/interaction_profiles/oculus/touch_controller";
                m_localizedControllerType[side] = "Crystal Controller";
                aimPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-40.f), 0, 0}),
                                         XrVector3f{0, 0, -0.05f});
                handPose = Pose::MakePose(Quaternion::RotationRollPitchYaw({PVR::DegreeToRad(-32.f), 0, 0}),
                                          XrVector3f{0.03f, -0.062f, -0.1f});
            } else {
                // Fallback to simple controller.
                preferredInteractionProfile = "/interaction_profiles/khr/simple_controller";
                m_localizedControllerType[side] = "Controller";
            }

            // Try to map with the preferred bindings.
            auto bindings = m_suggestedBindings.find(preferredInteractionProfile);
            if (bindings != m_suggestedBindings.cend()) {
                actualInteractionProfile = preferredInteractionProfile;
            }
            if (bindings == m_suggestedBindings.cend() || m_forcedInteractionProfile) {
                const bool hasOculusTouchControllerProfile =
                    m_suggestedBindings.find("/interaction_profiles/oculus/touch_controller") !=
                    m_suggestedBindings.cend();
                const bool hasMicrosoftMotionControllerProfile =
                    m_suggestedBindings.find("/interaction_profiles/microsoft/motion_controller") !=
                    m_suggestedBindings.cend();

                // In order of preference.
                if (m_forcedInteractionProfile &&
                    m_forcedInteractionProfile.value() == ForcedInteractionProfile::OculusTouchController &&
                    hasOculusTouchControllerProfile) {
                    actualInteractionProfile = "/interaction_profiles/oculus/touch_controller";
                } else if (m_forcedInteractionProfile &&
                           m_forcedInteractionProfile.value() == ForcedInteractionProfile::MicrosoftMotionController &&
                           hasMicrosoftMotionControllerProfile) {
                    actualInteractionProfile = "/interaction_profiles/microsoft/motion_controller";
                } else if (hasOculusTouchControllerProfile) {
                    actualInteractionProfile = "/interaction_profiles/oculus/touch_controller";
                } else if (hasMicrosoftMotionControllerProfile) {
                    actualInteractionProfile = "/interaction_profiles/microsoft/motion_controller";
                } else if (m_suggestedBindings.find("/interaction_profiles/valve/index_controller") !=
                           m_suggestedBindings.cend()) {
                    actualInteractionProfile = "/interaction_profiles/valve/index_controller";
                } else if (m_suggestedBindings.find("/interaction_profiles/htc/vive_controller") !=
                           m_suggestedBindings.cend()) {
                    actualInteractionProfile = "/interaction_profiles/htc/vive_controller";
                } else if (m_suggestedBindings.find("/interaction_profiles/khr/simple_controller") !=
                           m_suggestedBindings.cend()) {
                    actualInteractionProfile = "/interaction_profiles/khr/simple_controller";
                }
                if (!actualInteractionProfile.empty()) {
                    bindings = m_suggestedBindings.find(actualInteractionProfile);
                }
            }

            // Map all possible actions sources for this controller.
            if (bindings != m_suggestedBindings.cend()) {
                const auto& mapping = m_controllerMappingTable
                                          .find(std::make_pair(actualInteractionProfile, preferredInteractionProfile))
                                          ->second;
                for (const auto& binding : bindings->second) {
                    if (!m_actions.count(binding.action)) {
                        continue;
                    }

                    const auto& sourcePath = getXrPath(binding.binding);
                    if (getActionSide(sourcePath) != side) {
                        continue;
                    }

                    Action& xrAction = *(Action*)binding.action;

                    // Map to the PVR input state.
                    ActionSource newSource{};
                    if (mapping(xrAction, binding.binding, newSource)) {
                        // Avoid duplicates.
                        bool duplicated = false;
                        for (const auto& source : xrAction.actionSources) {
                            if (source.second.realPath == newSource.realPath) {
                                duplicated = true;
                                break;
                            }
                        }

                        if (!duplicated) {
                            TraceLoggingWrite(g_traceProvider,
                                              "xrSyncActions_MapActionSource",
                                              TLXArg(binding.action, "Action"),
                                              TLXArg(xrAction.actionSet, "ActionSet"),
                                              TLArg(sourcePath.c_str(), "ActionPath"),
                                              TLArg(newSource.realPath.c_str(), "SourcePath"),
                                              TLArg(!!newSource.buttonMap, "IsButton"),
                                              TLArg(!!newSource.floatValue, "IsFloat"),
                                              TLArg(!!newSource.vector2fValue, "IsVector2"));

                            // Relocate the pointers to the copy of the input state within the actionset.
                            const ActionSet& xrActionSet = *(ActionSet*)xrAction.actionSet;
                            const auto relocatePointer = [&](void* pointer) {
                                if (!pointer) {
                                    return (uint8_t*)nullptr;
                                }

                                uint8_t* p = (uint8_t*)pointer;
                                uint8_t* oldBase = (uint8_t*)&m_cachedInputState;
                                uint8_t* newBase = (uint8_t*)&xrActionSet.cachedInputState;

                                return newBase + (p - oldBase);
                            };
                            newSource.buttonMap = (uint32_t*)relocatePointer((void*)newSource.buttonMap);
                            newSource.floatValue = (float*)relocatePointer((void*)newSource.floatValue);
                            newSource.vector2fValue = (pvrVector2f*)relocatePointer((void*)newSource.vector2fValue);

                            xrAction.actionSources.insert_or_assign(sourcePath, newSource);
                        }
                    }
                }
            }
        }

        TraceLoggingWrite(g_traceProvider,
                          "xrSyncActions",
                          TLArg(side == xr::Side::Left ? "Left" : "Right", "Side"),
                          TLArg(actualInteractionProfile.c_str(), "InteractionProfile"));

        const auto prevInterationProfile = m_currentInteractionProfile[side];
        if (!actualInteractionProfile.empty()) {
            Log("Using interaction profile: %s (%s)\n",
                actualInteractionProfile.c_str(),
                side == xr::Side::Left ? "Left" : "Right");

            CHECK_XRCMD(
                xrStringToPath(XR_NULL_HANDLE, actualInteractionProfile.c_str(), &m_currentInteractionProfile[side]));

            auto adjustedGripPose = Pose::Multiply(m_controllerGripOffset, gripPose);
            auto adjustedAimPose = Pose::Multiply(m_controllerAimOffset, aimPose);
            auto adjustedHandPose = Pose::Multiply(m_controllerHandOffset, handPose);
            if (side == xr::Side::Right) {
                const auto flipHandedness = [&](XrPosef& pose) {
                    // Mirror pose along the X axis.
                    // https://stackoverflow.com/a/33999726/15056285
                    pose.position.x = -pose.position.x;
                    pose.orientation.y = -pose.orientation.y;
                    pose.orientation.z = -pose.orientation.z;
                };
                flipHandedness(adjustedGripPose);
                flipHandedness(adjustedAimPose);
                flipHandedness(adjustedHandPose);
            }

            m_controllerGripPose[side] = adjustedGripPose;
            m_controllerAimPose[side] = adjustedAimPose;
            m_controllerHandPose[side] = adjustedHandPose;
        } else {
            m_currentInteractionProfile[side] = XR_NULL_PATH;
            m_controllerGripPose[side] = m_controllerAimPose[side] = Pose::Identity();
        }

        m_currentInteractionProfileDirty =
            m_currentInteractionProfileDirty ||
            (m_currentInteractionProfile[side] != prevInterationProfile && !m_activeActionSets.empty());
    }

    void OpenXrRuntime::rebindTrackerActions(const std::string& serial, bool connected) {
        const std::string rolePath = getTrackerRolePath(serial);
        if (rolePath.empty()) {
            return;
        }

        // Remove all old bindings for this controller.
        for (const auto& action : m_actions) {
            Action& xrAction = *(Action*)action;

            for (auto it = xrAction.actionSources.begin(); it != xrAction.actionSources.end();) {
                if (startsWith(it->first, rolePath)) {
                    it = xrAction.actionSources.erase(it);
                } else {
                    it++;
                }
            }
        }

        if (!connected) {
            return;
        }

        // Map all possible actions sources for this controller.
        auto bindings = m_suggestedBindings.find("/interaction_profiles/htc/vive_tracker_htcx");
        if (bindings != m_suggestedBindings.cend()) {
            const auto& mapping = m_controllerMappingTable
                                      .find(std::make_pair("/interaction_profiles/htc/vive_tracker_htcx",
                                                           "/interaction_profiles/htc/vive_tracker_htcx"))
                                      ->second;
            for (const auto& binding : bindings->second) {
                if (!m_actions.count(binding.action)) {
                    continue;
                }

                const auto& sourcePath = getXrPath(binding.binding);
                if (!startsWith(sourcePath, rolePath)) {
                    continue;
                }

                Action& xrAction = *(Action*)binding.action;

                // Map to the PVR input state.
                ActionSource newSource{};
                if (mapping(xrAction, binding.binding, newSource)) {
                    // Avoid duplicates.
                    bool duplicated = false;
                    for (const auto& source : xrAction.actionSources) {
                        if (source.second.realPath == newSource.realPath) {
                            duplicated = true;
                            break;
                        }
                    }

                    if (!duplicated) {
                        TraceLoggingWrite(g_traceProvider,
                                          "xrSyncActions_MapActionSource",
                                          TLXArg(binding.action, "Action"),
                                          TLXArg(xrAction.actionSet, "ActionSet"),
                                          TLArg(sourcePath.c_str(), "ActionPath"),
                                          TLArg(newSource.realPath.c_str(), "SourcePath"));

                        // TODO: PVR does not seem to handle GPIOs on the trackers, so there are no inputs to bind here.

                        xrAction.actionSources.insert_or_assign(sourcePath, newSource);
                    }
                }
            }
        }
    }

    std::string OpenXrRuntime::getXrPath(XrPath path) const {
        if (path == XR_NULL_PATH) {
            return "";
        }

        const auto it = m_strings.find(path);
        if (it == m_strings.cend()) {
            return "<unknown>";
        }

        return it->second;
    }

    int OpenXrRuntime::getActionSide(const std::string& fullPath, bool allowExtraPaths) const {
        if (startsWith(fullPath, "/user/hand/left")) {
            return xr::Side::Left;
        } else if (startsWith(fullPath, "/user/hand/right")) {
            return xr::Side::Right;
        } else if (allowExtraPaths &&
                   (startsWith(fullPath, "/user/head") || startsWith(fullPath, "/user/gamepad") ||
                    startsWith(fullPath, "/user/eyes_ext") || startsWith(fullPath, "/user/vive_tracker_htcx"))) {
            return xr::Side::Count; // Valid, but not a side.
        }

        return -1;
    }

    XrVector2f OpenXrRuntime::handleJoystickDeadzone(pvrVector2f raw) const {
        const float length = std::sqrt(raw.x * raw.x + raw.y * raw.y);
        if (length < m_joystickDeadzone) {
            return {0, 0};
        }
        XrVector2f normalizedInput{raw.x / length, raw.y / length};
        const float scaling = (length - m_joystickDeadzone) / (1 - m_joystickDeadzone);
        return {normalizedInput.x * scaling, normalizedInput.y * scaling};
    }

    void OpenXrRuntime::handleBuiltinActions(bool wasRecenteringPressed) {
        wasRecenteringPressed =
            wasRecenteringPressed ||
            (GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_MENU) < 0 && GetAsyncKeyState(VK_SPACE) < 0);
        if (wasRecenteringPressed) {
            const auto now = pvr_getTimeSeconds(m_pvr);
            if (m_isRecenteringPressed) {
                // Requires a 3 seconds press.
                if (now - m_isRecenteringPressed.value() > 2.f) {
                    // Recenter view.
                    TraceLoggingWrite(g_traceProvider, "PVR_RecenterTrackingOrigin");
                    CHECK_PVRCMD(pvr_recenterTrackingOrigin(m_pvrSession));
                }
            } else {
                m_isRecenteringPressed = now;
            }
            wasRecenteringPressed = true;
        } else {
            m_isRecenteringPressed.reset();
        }
    }

} // namespace pimax_openxr
