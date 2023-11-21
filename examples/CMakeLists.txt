#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(ramses-example-minimal)
add_subdirectory(ramses-example-basic-blending)
add_subdirectory(ramses-example-basic-effect-from-glsl)
add_subdirectory(ramses-example-basic-geometry)
add_subdirectory(ramses-example-basic-texturing)
add_subdirectory(ramses-example-basic-scenegraph)
add_subdirectory(ramses-example-basic-file-loading)
add_subdirectory(ramses-example-basic-rendergroups)
add_subdirectory(ramses-example-basic-renderpasses)
add_subdirectory(ramses-example-basic-rendertarget)
add_subdirectory(ramses-example-data-buffers-vertices)
add_subdirectory(ramses-example-data-buffers-texture)
add_subdirectory(ramses-example-geometry-instancing)
add_subdirectory(ramses-example-renderonce)
if(ramses-sdk_TEXT_SUPPORT)
    add_subdirectory(ramses-example-text-basic)
    add_subdirectory(ramses-example-text-languages)
endif()
add_subdirectory(ramses-example-interleaved-vertex-buffers)

if(ramses-sdk_BUILD_FULL_SHARED_LIB)
    add_subdirectory(ramses-example-local-client)
    add_subdirectory(ramses-example-local-displays)
    add_subdirectory(ramses-example-local-datalink)
    add_subdirectory(ramses-example-local-offscreenbuffer)
    add_subdirectory(ramses-example-local-dma-offscreenbuffer)
    add_subdirectory(ramses-example-local-viewport-link)
    add_subdirectory(ramses-example-local-pick-handling)
    add_subdirectory(ramses-example-local-scene-referencing)
    add_subdirectory(ramses-example-local-geometry-shaders)
    add_subdirectory(ramses-example-local-compositing)
endif()

if(ramses-sdk_ENABLE_LOGIC)
    add_subdirectory(logic)
endif()
