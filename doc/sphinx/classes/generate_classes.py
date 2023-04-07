#!/usr/bin/env python3

#  -------------------------------------------------------------------------
#  Copyright (C) 2022 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from string import Template
from pathlib import Path
import textwrap
import shutil

content = [
    {
        'TOC_caption': 'Ramses Client API',
        'namespace_prefix': 'ramses::',
        'classes': [
            'Appearance',
            'ArrayBuffer',
            'ArrayResource',
            'AttributeInput',
            'BlitPass',
            'Camera',
            'ClientObject',
            'DataFloat',
            'DataInt32',
            'DataMatrix22f',
            'DataMatrix33f',
            'DataMatrix44f',
            'DataObject',
            'DataVector2f',
            'DataVector2i',
            'DataVector3f',
            'DataVector3i',
            'DataVector4f',
            'DataVector4i',
            'EffectDescription',
            'Effect',
            'EffectInput',
            'GeometryBinding',
            'IClientEventHandler',
            'MeshNode',
            'Node',
            'OrthographicCamera',
            'PerspectiveCamera',
            'PickableObject',
            'RamsesClient',
            'RamsesObject',
            'RenderBuffer',
            'RenderGroup',
            'RenderGroupMeshIterator',
            'RenderPassGroupIterator',
            'RenderPass',
            'RenderTargetDescription',
            'RenderTarget',
            'Resource',
            'SceneConfig',
            'SceneGraphIterator',
            'Scene',
            'SceneIterator',
            'SceneObject',
            'SceneObjectIterator',
            'SceneReference',
            'Texture2DBuffer',
            'Texture2D',
            'Texture3D',
            'TextureCube',
            'TextureSamplerExternal',
            'TextureSampler',
            'TextureSamplerMS',
            'UniformInput',
        ],
        'structs': [
            'MipLevelData',
            'TextureSwizzle',
        ],
        'enums': [
            'EBlendOperation',
            'EBlendFactor',
            'ECullMode',
            'EDepthWrite',
            'EEffectAttributeSemantic',
            'EEffectInputDataType',
            'EEffectUniformSemantic',
            'EScissorTest',
            'EDepthFunc',
            'EStencilFunc',
            'EStencilOperation',
            'EDrawMode',
            'ERotationConvention',
            'EScenePublicationMode',
            'EVisibilityMode',
            'ETextureSamplingMethod',
            'ETextureAddressMode',
            'ERenderBufferType',
            'ERenderBufferAccessMode',
            'ETextureFormat',
            'ETextureCubeFace',
            'ETextureChannelColor',
        ],
    },
    {
        'TOC_caption': 'Ramses Text API',
        'namespace_prefix': 'ramses::',
        'classes': [
            'FontRegistry',
            'IFontAccessor',
            'IFontInstance',
            'TextCache',
        ],
        'structs': [
            'FontCascade',
            'FontInstanceOffset',
            'GlyphKey',
            'GlyphMetrics',
            # 'StringBoundingBox',
            'TextLine',
            'ExtractedUnicodePoint',
        ],
        'typedefs': [
            'FontInstanceId',
            'FontInstanceOffsets',
            'GlyphKeyVector',
            'GlyphMetricsVector',
            'TextLineId',
        ],
        'functions': [
            # 'UtfUtils::ConvertUtf8ToUtf32',
            # 'UtfUtils::ConvertCharUtf32ToUtf8',
            # 'UtfUtils::ConvertStrUtf32ToUtf8',
            # 'UtfUtils::ExtractUnicodePointFromUTF8',
            # 'UtfUtils::ConvertUtf32ToUtf8String',
            # 'UtfUtils::ExtractUnicodePointFromUTF16',
            # 'UtfUtils::GetBoundingBoxForString',
        ],
    },
    {
        'TOC_caption': 'Ramses Logic API',
        'namespace_prefix': 'rlogic::',
        'classes': [
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
        'structs': [
            'AnimationChannel',
            'ErrorData',
            'IsPrimitiveProperty',
            'PropertyEnumToType',
            'PropertyTypeToEnum',
            'WarningData',
            'PropertyLink',
        ],
        'enums': [
            'EInterpolationType',
            'ELogMessageType',
            'EPropertyType',
            'ERotationType',
            'EStandardModule',
            'EFeatureLevel',
            'ELuaSavingMode',
        ],
        'typedefs': [
            'vec2f',
            'vec3f',
            'vec4f',
            'vec2i',
            'vec3i',
            'vec4i',
            'matrix44f',
        ],
    },
    {
        'TOC_caption': 'Logic Logging',
        'namespace_prefix': 'rlogic::Logger::',
        'functions': [
            'SetLogHandler',
            'SetDefaultLogging',
        ]
    },
    {
        'TOC_caption': 'Free functions',
        'namespace_prefix': 'rlogic::',
        'functions': [
            'GetRamsesLogicVersion',
        ]
    },
]


class_template = textwrap.dedent("""..
    -------------------------------------------------------------------------
    Copyright (C) 2022 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. default-domain:: cpp
.. highlight:: cpp

==========================================
$title
==========================================

.. $directive:: $symbol
$options
""")


index_template = textwrap.dedent("""..
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

toc_entry_template = textwrap.dedent("""
.. toctree::
    :maxdepth: 3
    :caption: $caption

$files

""")


def main():
    toc_contents = ""

    generated = Path('generated')
    shutil.rmtree(generated)
    generated.mkdir()

    for section in content:
        toc_file_list = ""
        for item in section.get('classes', []):
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': 'doxygenclass',
                'options': '   :members:'
            }
            with open(generated / f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        for item in section.get('structs', []):
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': 'doxygenstruct',
                'options': '   :members:'
            }
            with open(generated / f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        for item in section.get('enums', []):
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': 'doxygenenum',
                'options': ''
            }
            with open(generated / f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        for item in section.get('functions', []):
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': 'doxygenfunction',
                'options': ''
            }
            with open(generated / f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        for item in section.get('typedefs', []):
            toc_file_list = toc_file_list + "\n    " + item
            config = {
                'title': item,
                'symbol': section['namespace_prefix'] + item,
                'directive': 'doxygentypedef',
                'options': ''
            }
            with open(generated / f'{item}.rst', 'w') as f:
                fileContents = Template(class_template).substitute(config)
                f.write(fileContents)

        toc_config = {
            'caption': section['TOC_caption'],
            'files': toc_file_list,
        }
        toc_contents = toc_contents + Template(toc_entry_template).substitute(toc_config)

    with open(generated / 'index.rst', 'w') as f:
        fileContents = Template(index_template).substitute(TOC=toc_contents)
        f.write(fileContents)


if __name__ == '__main__':
    main()
