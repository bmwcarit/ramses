//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_READER_H
#define RAMSES_CITYMODEL_READER_H

#include "BoundingBox.h"
#include "SceneAPI/EDataType.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Effect.h"
#include "Math3d/Vector4.h"
#include "openctm.h"

#include "fstream"
#include "vector"
#include "mutex"
#include "AnimationPath.h"

class Tile;
class CitymodelScene;
class Material;
class TileResourceContainer;
class Citymodel;

namespace ramses
{
    class Node;
    class RamsesClient;
    class RenderGroup;
    class Scene;
    class Vector2fArray;
    class Vector3fArray;
    class Vector4fArray;
    class UInt32Array;
    class Effect;
}

class GeometryNode
{
public:
    const ramses::Vector3fArray* m_positions  = nullptr;
    const ramses::Vector3fArray* m_normals    = nullptr;
    const ramses::Vector2fArray* m_texCoords  = nullptr;
    const ramses::Vector4fArray* m_texCoords2 = nullptr;
    const ramses::UInt32Array*   m_indexArray = nullptr;
    ramses::Effect*              m_effect     = nullptr;

    std::vector<ramses_internal::Vector3> m_positionsData;
    std::vector<uint32_t>                 m_indexData;
};

/// Reader class for reading citymodel "rex" files.
class Reader
{
public:
    /// Constructor.
    Reader(Citymodel& citymodel);

    /// Adds an effect to the effects list.
    /** Effects in an ".rex" file are referenced by an index into this list.
     *  @param effect The effect to be added. */
    void addEffect(ramses::Effect* effect);

    /// Returns an effect by index.
    /** @param effectIndex Index of the effect.
     *  @return The effect. */
    ramses::Effect* getEffect(uint32_t effectIndex);

    /// Opens a file for reading.
    /** @param filename The name of the file to be read. */
    void open(const std::string& filename);

    /// Reads an object from the file.
    /** @param index Index of the object to be read.
     *  @param resourceContainer The container where the tile related resources are stored
     *  @param resetIds When set to "true", read objects are not referenced by further reads.
     *  @return The read object. */
    void* read(uint32_t index, TileResourceContainer& resourceContainer, bool resetIds = true);

    std::mutex& getSceneLock();

protected:
    /// Reference of object data in a ".rex" file.
    class FileReference
    {
    public:
        /// Constructor.
        /** @param position Byte position in the file
         *  @param compressedSize Stored size of the data in bytes.
         *  @param uncompressedSize Size of the data, when uncompressed. */
        FileReference(uint64_t position, uint32_t compressedSize, uint32_t uncompressedSize);

        /// Returns the position in the file.
        /** @return The position in bytes. */
        uint64_t position() const;

        /// Compressed size of the data in bytes.
        /** @return The size in bytes. */
        uint32_t compressedSize() const;

        /// Uncompressed size of the data in bytes.
        /** @return The size in bytes. */
        uint32_t uncompressedSize() const;

    protected:
        /// The position in bytes.
        uint64_t m_position;

        /// Compressed size of the data in bytes.
        uint32_t m_compressedSize;

        /// Uncompressed size of the data in bytes.
        uint32_t m_uncompressedSize;
    };

    /// Reads an object from the file.
    /** Can be either the object itself, or when already read just the pointer
     *  to the read object is returned.
     *  @param resourceContainer Loaded resources are stored here.
     *  @return The read object. */
    void* readObject(TileResourceContainer& resourceContainer);

    /// Reads a ramses node from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read node. */
    ramses::Node* readNode(TileResourceContainer& resourceContainer);

    /// Reads a ramses mesh node from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read mesh node. */
    ramses::Node* readMeshNode(TileResourceContainer& resourceContainer);

    ///// Reads a ramses geometry node from the file.
    /** @param resourceContainer Loaded resources are stored here.
        @return The geometry node. */
    GeometryNode* readGeometryNode(TileResourceContainer& resourceContainer);

    /// Reads a material from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read material. */
    Material* readMaterial(TileResourceContainer& resourceContainer);

    /// Reads a ramses vertex array resource from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @param elementType Type of the vertex array.
     *  @return The read vertex array resource. */
    void* readVertexArrayResource(TileResourceContainer& resourceContainer, ramses_internal::EDataType elementType);

    /// Reads a ramses index array resource from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read index array resource. */
    const ramses::UInt32Array* readIndexArrayResource(TileResourceContainer& resourceContainer);

    /// Reads a ramses texture 2d resource from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read texture 2d resource. */
    ramses::Texture2D* readTexture2DResource(TileResourceContainer& resourceContainer);

    /// Reads a scene from the file.
    /** @param resourceContainer Loaded resources are stored here.
     *  @return The read scene. */
    CitymodelScene* readScene(TileResourceContainer& resourceContainer);

    /// Reads the tile meta data from the file.
    /** @return The read tile. */
    Tile* readTile();

    /// Reads a uint8 value from the file.
    /** @param value The read value is returned here. */
    void read_uint8(uint8_t& value);

    /// Reads a uint32 value from the file.
    /** @param value The read value is returned here. */
    void read_uint32(uint32_t& value);

    /// Reads a int32 value from the file.
    /** @param value The read value is returned here. */
    void read_int32(int32_t& value);

    /// Reads a uint64 value from the file.
    /** @param value The read value is returned here. */
    void read_uint64(uint64_t& value);

    /// Reads a float value from the file.
    /** @param value The read value is returned here. */
    void read_float(float& value);

    /// Reads a string from the file.
    /** @param value The string is returned here. */
    void read_string(std::string& value);

    /// Reads a vector3 from the file.
    /** @param value The read value is returned here. */
    void read(ramses_internal::Vector3& value);

    /// Reads a color from the file.
    /** @param color The read value is returned here. */
    void read(ramses_internal::Vector4& color);

    /// Reads a bounding box from the file.
    /** @param color The read value is returned here. */
    void read(BoundingBox& bbox);

    /// Reads a number of bytes from the file.
    /** @param dest Destination where to write the read data.
     *  @param size Size of the data to be read. */
    void read(uint8_t* dest, uint32_t size);

    /// Reads a number of bytes from the file.
    /** @param size Size of the data to be read.
     *  @return The read data. */
    uint8_t* read(uint32_t size);

    /// Creates and returns a new id number for an read object.
    /** The id is used, when the same object is referenced later by an other read object. */
    uint32_t createId();

    /// Reads an animation path from the file.
    /** @param animationPath The read animation path is returned here. */
    void readAnimationPath(AnimationPath& animationPath);

    /// Reads a names list from the file.
    /** @param names The read name list is returned here. */
    void readNames(std::vector<std::string>& names);

    /// Reads a name points list from the file.
    /** @param names The read name point list is returned here. */
    void readNamePoints(std::vector<ramses_internal::Vector3>& namePoints);

    /// Reads a route point list from the file.
    /** @param names The read route point list is returned here. */
    void readRoutePoints(std::vector<ramses_internal::Vector3>& routePoints);

    /// Callback function for reading CTM compressed data.
    /** @param buffer Data that was read.
     *  @param count Number of bytes that are stored in buffer. */
    CTMuint ctmRead(void* buffer, CTMuint count);

    /// Converts euler XYZ rotation angles into euler ZYX rotation angles.
    /** @param rotationXYZ The XYZ euler rotation angles to be converted into ZYX angles.
     *  @return The converted angles. */
    static ramses_internal::Vector3 ConvertXYZRotationToZYX(const ramses_internal::Vector3& rotationXYZ);

    /// Callback function for reading CTM compressed data.
    /** @param buffer Data that was read.
     *  @param count Number of bytes that are stored in buffer.
     *  @param userData The reader instance. */
    static CTMuint CTMRead(void* buffer, CTMuint count, void* userData);

    /// The input stream.
    std::ifstream m_f;

    /// Data read from the file.
    uint8_t* m_dataBuffer = nullptr;

    /// Pointer to the current data in the buffer mDataBuffer.
    uint8_t* m_data = nullptr;

    /// Stores all read objects.
    std::vector<void*> m_object;

    /// The citymodel main class.
    Citymodel& m_citymodel;

    /// Mutex to synchronize scene changes between the reader and the main thread.
    std::mutex m_sceneLock;

    /// The read scene.
    CitymodelScene* m_scene = nullptr;

    /// List of effects.
    ramses_internal::Vector<ramses::Effect*> m_effects;

    /// List of objects referencing the "rex" archive file.
    ramses_internal::Vector<FileReference> m_objectReferences;

    /// Index for read tiles.
    uint32_t m_tileIndex = 0;
};

#endif
