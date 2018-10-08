//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/Bitmap.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "lodepng.h"
#include "Utils/LogMacros.h"
#include "Collections/String.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    Bitmap::Bitmap(UInt32 width, UInt32 height, const UInt8* data, bool flipVertically /*= false*/)
        : m_width(width)
        , m_height(height)
        , m_data(width * height * 4u)
    {
        if (data != nullptr)
        {
            if(flipVertically)
            {
                const size_t rowSize = m_width * 4u;
                for (size_t row = 0u; row < m_height; ++row)
                {
                    const UInt8* src = &data[row * rowSize];
                    UInt8* dst = &m_data[(m_height - 1 - row) * rowSize];
                    PlatformMemory::Copy(dst, src, rowSize);
                }
            }
            else
            {
                PlatformMemory::Copy(&m_data[0], data, width * height * 4);
            }
        }
    }

    void Bitmap::loadFromFileBMP(const String& filename)
    {
        File sourceFile(filename);
        if (!sourceFile.exists())
        {
            assert(false);
            return;
        }

        BinaryFileInputStream stream(sourceFile);

        if (EStatus_RAMSES_OK != stream.getState())
        {
            assert(false);
            return;
        }

        UInt32 dummy32 = 0;   // used for unused data :)
        UInt16 dummy16 = 0;   // used for unused data :)
        UInt32 headerSize = 0;
        UInt32 totalSize = 0;
        UInt16 colorDepth = 24;
        UInt32 imageDataSize = 0;

        //string "BM", or 19778 as integer value
        stream >> dummy16;

        stream >> totalSize;

        //reserved (standard: 0)
        stream >> dummy32;

        //image data offset
        stream >> headerSize;

        //image data offset (standard: 40)
        stream >> dummy32;

        //width and height
        stream >> m_width;
        stream >> m_height;

        //ununsed (standard: 1)
        stream >> dummy16;

        //color depth of the bitmap (standard: 24)
        stream >> colorDepth;
        assert(24 == colorDepth);

        //no compression
        stream >> dummy32;

        //size of image data in bytes
        stream >> imageDataSize;

        // imageDataSize includes padding! - not equivalent to real data size
        assert(m_width * m_height * 3 <= imageDataSize);

        //pixel per meter (standard: 0)
        stream >> dummy32;
        stream >> dummy32;

        //number of entires in the color table
        stream >> dummy32;
        stream >> dummy32;

        const UInt32 bytesPerRow = (((m_width * 3) + 3) & ~3); // rounded up to a multiple of 4
        assert(bytesPerRow * m_height == imageDataSize);

        const UInt32 paddingBytes = bytesPerRow - m_width * 3;
        assert(paddingBytes < 4);

        //internal representation of bitmap is in 32bpp format!
        m_data.resize(m_width * m_height * 4u);
        Char dummy[3];
        //read image data
        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                UInt8* pixel = &m_data[4 * (x + m_width*y)];

                pixel[3] = 255;                                     //a
                stream.read(reinterpret_cast<Char*>(&pixel[2]), 1); //b
                stream.read(reinterpret_cast<Char*>(&pixel[1]), 1); //g
                stream.read(reinterpret_cast<Char*>(&pixel[0]), 1); //r
            }
            stream.read(dummy,paddingBytes);
        }

        if (EStatus_RAMSES_OK != stream.getState())
        {
            assert(false);
            m_width = 0;
            m_height = 0;
            m_data.clear();
            return;
        }

        sourceFile.close();
    }

    void Bitmap::saveToFileBMP(const String& filename) const
    {
        File targetFile(filename);

        BinaryFileOutputStream stream(targetFile);

        const UInt32 bytesPerRow  = (((m_width * 3) + 3) & ~3); // rounded up to a multiple of 4
        const UInt32 paddingBytes = bytesPerRow - m_width * 3;
        assert(paddingBytes < 4);
        UInt32 imageDataSize = bytesPerRow * m_height;  //saved in 24 bpp format

        //bitmap
        stream.write("BM", 2);

        //total size
        UInt32 headerSize = 54;
        UInt32 totalSize = headerSize + imageDataSize;
        stream << totalSize;

        //reserved (standard: 0)
        stream << static_cast<UInt32>(0);

        //image data offset
        stream << headerSize;

        //image data offset (standard: 40)
        UInt32 sizeOfBitmapInfoHeader = 40;
        stream << sizeOfBitmapInfoHeader;

        //width and height
        stream << m_width;
        stream << m_height;

        //ununsed (standard: 1)
        stream << static_cast<UInt16>(1);

        //color depth of the bitmap (standard: 24)
        stream << static_cast<UInt16>(24);

        //no compression
        stream << static_cast<UInt32>(0);

        //size of image data in bytes
        stream << imageDataSize;

        //pixel per meter (standard: 0)
        stream << static_cast<UInt32>(0);
        stream << static_cast<UInt32>(0);

        //number of entires in the color table
        stream << static_cast<UInt32>(0);
        stream << static_cast<UInt32>(0);

        //write image data
        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                const UInt8* pixel = &m_data[4 * (x + m_width*y)];

                stream.write(&pixel[2], 1); //b
                stream.write(&pixel[1], 1); //g
                stream.write(&pixel[0], 1); //r
            }
            stream.write("\0\0\0",paddingBytes);
        }

        stream.flush();
        targetFile.close();
    }

    void Bitmap::loadFromFilePNG(const String& filename)
    {
        if (lodepng::decode(m_data, m_width, m_height, filename.c_str()) != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while loading PNG file: " << filename);
    }

    void Bitmap::saveToFilePNG(const String& filename) const
    {
        if (lodepng::encode(filename.c_str(), m_data, m_width, m_height) != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while saving PNG file: " << filename);
    }

    Bool Bitmap::operator==(const Bitmap& other) const
    {
        if(m_width != other.m_width || m_height != other.m_height)
        {
            return false;
        }

        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                const UInt8* myPixel = &m_data[4 * (x + m_width*y)];
                const UInt8* otherPixel = &other.m_data[4 * (x + m_width*y)];

                if (myPixel[2] != otherPixel[2]) return false; //b
                if (myPixel[1] != otherPixel[1]) return false; //g
                if (myPixel[0] != otherPixel[0]) return false; //r
            }
        }

        return true;
    }

    Bool Bitmap::operator!=(const Bitmap& other) const
    {
        return !operator==(other);
    }

    Bitmap Bitmap::createDiffTo(const Bitmap& other, bool enableDifferentSize) const
    {
        const UInt32 resultWidth = std::max<UInt32>(m_width, other.m_width);
        const UInt32 resultHeight = std::max<UInt32>(m_height, other.m_height);

        if (!enableDifferentSize && (m_width != other.m_width || m_height != other.m_height))
        {
            return Bitmap();
        }

        Bitmap result(resultWidth, resultHeight, nullptr);

        const UInt8 blackPixel[4] = { 0, 0, 0, 255 };

        for (UInt32 col = 0; col < resultWidth; ++col)
        {
            for (UInt32 row = 0; row < resultHeight; ++row)
            {
                UInt8* resultPixel = &result.m_data[(resultWidth * row + col) * 4];
                const UInt8* thisPixel = blackPixel;
                const UInt8* otherPixel = blackPixel;

                if(row < m_height && col < m_width)
                    thisPixel = &m_data[(m_width * row + col) * 4];

                if (row < other.m_height && col < other.m_width)
                    otherPixel = &other.m_data[(other.m_width * row + col) * 4];

                for (size_t i = 0; i < 3; ++i)
                {
                    resultPixel[i] = (thisPixel[i] > otherPixel[i]) ? thisPixel[i] - otherPixel[i] : otherPixel[i] - thisPixel[i];
                }
                resultPixel[3] = std::max<UInt8>(thisPixel[3], otherPixel[3]); // keep max alpha
            }
        }

        return result;
    }

    UInt32 Bitmap::getWidth() const
    {
        return m_width;
    }

    UInt32 Bitmap::getHeight() const
    {
        return m_height;
    }

    UInt32 Bitmap::getNumberOfPixels() const
    {
        return m_width * m_height;
    }

    UInt64 Bitmap::getSumOfPixelValues() const
    {
        UInt64 result = 0;

        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                const UInt8* myPixel = &m_data[4 * (x + m_width*y)];

                result += myPixel[2]; //b
                result += myPixel[1]; //g
                result += myPixel[0]; //r
            }
        }

        return result;
    }

    UInt32 Bitmap::getNumberOfNonBlackPixels(UInt8 maxDiffPerColorChannel) const
    {
        UInt32 result = 0;
        for (UInt32 px = 0; px < m_data.size() / 4; ++px)
        {
            const UInt8* pxData = &m_data[4 * px];
            if (pxData[0] > maxDiffPerColorChannel ||
                pxData[1] > maxDiffPerColorChannel ||
                pxData[2] > maxDiffPerColorChannel)
            {
                result++;
            }
        }

        return result;
    }

    std::vector<ramses_internal::UInt8>& Bitmap::getData()
    {
        return m_data;
    }

    const std::vector<ramses_internal::UInt8>& Bitmap::getData() const
    {
        return m_data;
    }
}
