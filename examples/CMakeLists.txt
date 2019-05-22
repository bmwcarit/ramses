#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(ramses-sdk_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(ramses-example-minimal)
    ADD_SUBDIRECTORY(ramses-example-basic-animation)
    ADD_SUBDIRECTORY(ramses-example-basic-animation-realtime)
    ADD_SUBDIRECTORY(ramses-example-basic-blending)
    ADD_SUBDIRECTORY(ramses-example-basic-effect-from-glsl)
    ADD_SUBDIRECTORY(ramses-example-basic-geometry)
    ADD_SUBDIRECTORY(ramses-example-basic-texturing)
    ADD_SUBDIRECTORY(ramses-example-basic-scenegraph)
    ADD_SUBDIRECTORY(ramses-example-basic-file-loading)
    ADD_SUBDIRECTORY(ramses-example-basic-rendergroups)
    ADD_SUBDIRECTORY(ramses-example-basic-renderpasses)
    ADD_SUBDIRECTORY(ramses-example-basic-rendertarget)
    ADD_SUBDIRECTORY(ramses-example-basic-compositing)
    ADD_SUBDIRECTORY(ramses-example-data-buffers-vertices)
    ADD_SUBDIRECTORY(ramses-example-data-buffers-texture)
    ADD_SUBDIRECTORY(ramses-example-geometry-instancing)
    ADD_SUBDIRECTORY(ramses-example-renderonce)
    ADD_SUBDIRECTORY(ramses-example-local-client)
    ADD_SUBDIRECTORY(ramses-example-local-displays)
    ADD_SUBDIRECTORY(ramses-example-local-datalink)
    ADD_SUBDIRECTORY(ramses-example-local-offscreenbuffer)
    ADD_SUBDIRECTORY(ramses-example-local-viewport-link)
    ADD_SUBDIRECTORY(ramses-example-text-basic)
    ADD_SUBDIRECTORY(ramses-example-text-languages)

ENDIF()

