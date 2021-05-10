//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_BaseMock.h"

using namespace testing;

namespace ramses_internal
{
    Platform_BaseMock::Platform_BaseMock(const RendererConfig& config)
        : Platform_Base(config)
    {
        ON_CALL(*this, createWindow(_, _)).WillByDefault(Invoke([this](auto&, auto&) {
            assert(!m_window);
            m_window = std::move(windowOwningPtr);
            return true;
        }));

        ON_CALL(*this, createContext(_)).WillByDefault(Invoke([this](auto&) {
            assert(!m_context);
            m_context = std::move(contextOwningPtr);
            return true;
        }));
        ON_CALL(*this, createContextUploading()).WillByDefault(Invoke([this]() {
            assert(!m_contextUploading);
            m_contextUploading = std::move(contextUploadingOwningPtr);
            return true;
        }));

        ON_CALL(*this, createDevice()).WillByDefault(Invoke([this]() {
            assert(!m_device);
            m_device = std::move(deviceOwningPtr);
            return true;
        }));
        ON_CALL(*this, createDeviceUploading()).WillByDefault(Invoke([this]() {
            assert(!m_deviceUploading);
            m_deviceUploading = std::move(deviceUploadingOwningPtr);
            return true;
        }));

        ON_CALL(*this, createEmbeddedCompositor(_)).WillByDefault(Invoke([this](auto&) {
            assert(!m_embeddedCompositor);
            m_embeddedCompositor = std::move(embeddedCompositorOwningPtr);
            return true;
        }));

        ON_CALL(*this, createSystemCompositorController()).WillByDefault(Invoke([this]() {
            assert(!m_systemCompositorController);
            m_systemCompositorController = std::move(systemCompositorControllerOwningPtr);
            return m_systemCompositorController.get();
        }));

        ON_CALL(*this, createTextureUploadingAdapter()).WillByDefault(Invoke([this]() {
            assert(!m_textureUploadingAdapter);
            m_textureUploadingAdapter = std::move(textureUploadingAdapterOwningPtr);
        }));
    }

    Platform_BaseMock::~Platform_BaseMock() = default;
}
