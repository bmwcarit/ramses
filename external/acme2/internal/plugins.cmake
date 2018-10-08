############################################################################
#
# Copyright (C) 2014 BMW Car IT GmbH
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

MACRO(ACME_CALL_PLUGIN_HOOK name)
    STRING(TOLOWER "${name}" name_lower)
    FOREACH(pluginDir ${ACME2_PLUGIN_DIR})
        FILE(GLOB_RECURSE plugins ${pluginDir}/${name_lower}*.cmake)

        FOREACH(plugin ${plugins})

            FILE(RELATIVE_PATH plugin_file ${pluginDir} ${plugin})

            # Let's make sure we have the right separator
            FILE(TO_CMAKE_PATH ${plugin_file} plugin_file_cmake)
            # Put the path into a list to be able to manipulate it
            STRING(REPLACE "/" ";" plist "${plugin_file_cmake}")
            # Get the leftmost element and set it as plugin name
            LIST(GET plist 0 plugin_name)

            get_filename_component(file_name ${plugin_file} NAME_WE)

            STRING(TOUPPER "${plugin_name}" plugin_name_upper)
            OPTION(ACME_ENABLE_PLUGIN_${plugin_name_upper} "Enable ACME 2 plugin '${plugin_name}'" OFF)

            IF(ACME_ENABLE_PLUGIN_${plugin_name_upper})
                ACME_DEBUG("plugin-hook: ${plugin_name}/${file_name}")
                INCLUDE(${plugin} OPTIONAL)
            ENDIF()
        ENDFOREACH()
    ENDFOREACH()
ENDMACRO()
