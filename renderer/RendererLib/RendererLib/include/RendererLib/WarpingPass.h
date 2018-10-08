//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WARPINGPASS_H
#define RAMSES_WARPINGPASS_H

#include "SceneAPI/RenderBuffer.h"
#include "RendererAPI/IDevice.h"

namespace ramses_internal
{
    class WarpingMeshData;

    class WarpingPass
    {
    public:
        explicit WarpingPass(IDevice& device, const WarpingMeshData& warpingMeshData);
        virtual ~WarpingPass();

        void execute(DeviceResourceHandle sourceColorBuffer);

    private:
        void initEffect();
        void initGeometry(const WarpingMeshData& warpingMeshData);

        template<typename T>
        void setConstant(const DataFieldHandle& constantNodeHandle, UInt32 count, const T* value) const;

        UInt32 m_indexCount;
        DeviceResourceHandle m_texcoordBufferResource;
        DataFieldHandle m_texcoordField;

        DataFieldHandle m_inputRenderBufferField;

        // From old ScreenspacePass
        IDevice& m_device;

        DeviceResourceHandle m_shaderResource;
        DeviceResourceHandle m_vertexBufferResource;
        DeviceResourceHandle m_indexBufferResource;
        DataFieldHandle m_vertexPositionField;
    };


    template<typename T>
    void WarpingPass::setConstant(const DataFieldHandle& constantNodeHandle, UInt32 count, const T* value) const
    {
        m_device.setConstant(constantNodeHandle, count, value);
    }
}

#endif
