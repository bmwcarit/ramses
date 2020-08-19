#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------


# allow limiting link parallelism
set(ramses-sdk_PARALLEL_LINK_JOBS "" CACHE STRING "Maximum number of parallel link jobs")

if(ramses-sdk_PARALLEL_LINK_JOBS)
  set_property(GLOBAL APPEND PROPERTY JOB_POOLS link_job_pool=${ramses-sdk_PARALLEL_LINK_JOBS})
  set(CMAKE_JOB_POOL_LINK link_job_pool)
endif()
