//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_CITYMODEL_ETYPE_H
#define RAMSES_CITYMODEL_ETYPE_H

/// Type of objects in ".rex" files.
enum EObjectType
{
    EType_Null = 0,
    EType_Index,
    EType_Node,
    EType_MeshNode,
    EType_Material,
    EType_GeometryNode,
    EType_VertexArrayResource2f,
    EType_VertexArrayResource3f,
    EType_VertexArrayResource4f,
    EType_IndexArrayResource,
    EType_Texture2DResource,
    EType_TextureCubeResource,
    EType_Scene,
    EType_Tile
};

#endif
