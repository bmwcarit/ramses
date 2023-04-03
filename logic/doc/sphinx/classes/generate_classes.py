#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from string import Template
import textwrap

content = [
    {
    'directive': 'doxygenclass',
    'TOC_caption': 'API classes',
    'namespace_prefix': 'rlogic::',
    'options': '   :members:',
    'items': [
        'AnimationNode',
        'AnimationNodeConfig',
        'Collection',
        'DataArray',
        'Iterator',
        'LogicEngine',
        'LogicEngineReport',
        'LogicNode',
        'LogicObject',
        'LuaConfig',
        'LuaModule',
        'LuaInterface',
        'LuaScript',
        'Property',
        'RamsesAppearanceBinding',
        'RamsesBinding',
        'RamsesCameraBinding',
        'RamsesNodeBinding',
        'RamsesRenderPassBinding',
        'RamsesRenderGroupBinding',
        'RamsesRenderGroupBindingElements',
        'RamsesMeshNodeBinding',
        'SkinBinding',
        'SaveFileConfig',
        'TimerNode',
        'AnchorPoint',
    ],
    },
    {
    'directive': 'doxygenstruct',
    'TOC_caption': 'API structs',
    'namespace_prefix': 'rlogic::',
    'options': '   :members:',
    'items': [
        'AnimationChannel',
        'ErrorData',
        'IsPrimitiveProperty',
        'PropertyEnumToType',
        'PropertyTypeToEnum',
        'WarningData',
        'PropertyLink',
    ],
    },
    {
    'directive': 'doxygenfunction',
    'TOC_caption': 'Logging',
    'namespace_prefix': 'rlogic::Logger::',
    'options': '',
    'items': [
        'SetLogHandler',
        'SetDefaultLogging',
    ]
    },
    {
    'directive': 'doxygenfunction',
    'TOC_caption': 'Free functions',
    'namespace_prefix': 'rlogic::',
    'options': '',
    'items': [
        'GetRamsesLogicVersion',
    ]
    },
    {
    'directive': 'doxygenenum',
    'TOC_caption': 'Enums',
    'namespace_prefix': 'rlogic::',
    'options': '',
    'items': [
        'EInterpolationType',
        'ELogMessageType',
        'EPropertyType',
        'ERotationType',
        'EStandardModule',
        'EFeatureLevel',
        'ELuaSavingMode',
    ],
    },
    {
    'directive': 'doxygentypedef',
    'TOC_caption': 'Typedefs',
    'namespace_prefix': 'rlogic::',
    'options': '',
    'items': [
        'vec2f',
        'vec3f',
        'vec4f',
        'vec2i',
        'vec3i',
        'vec4i',
        'matrix44f',
    ],
    },
]


class_template = textwrap.dedent(
"""..
    -------------------------------------------------------------------------
    Copyright (C) 2022 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. default-domain:: cpp
.. highlight:: cpp

=========================
$title
=========================

.. $directive:: $symbol
$options
""")

index_template = textwrap.dedent(
"""..
    -------------------------------------------------------------------------
    Copyright (C) 2021 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

=========================
Class Index
=========================

$TOC


""")

toc_entry_template = textwrap.dedent(
"""
.. toctree::
    :maxdepth: 3
    :caption: $caption

$files

""")




def main():
    toc_contents = ""

    for section in content:
        toc_file_list = ""
        for item in section['items']:
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': section['directive'],
                'options': section['options']
            }
            with open(f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        toc_config = {
            'caption': section['TOC_caption'],
            'files': toc_file_list,
        }
        toc_contents = toc_contents + Template(toc_entry_template).substitute(toc_config)

    with open(f'index.rst', 'w') as f:
        fileContents = Template(index_template).substitute(TOC=toc_contents)
        f.write(fileContents)

if __name__ == '__main__':
    main()
