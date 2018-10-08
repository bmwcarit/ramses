//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BITMAP_H
#define RAMSES_BITMAP_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <vector>

namespace ramses_internal
{
    class String;

    class Bitmap
    {
    public:
        explicit Bitmap(UInt32 width = 0u, UInt32 height = 0u, const UInt8* data = nullptr, bool flipVertically = false);
        Bitmap(const Bitmap& other) = default;
        Bitmap(Bitmap&& other) = default;

        void loadFromFileBMP(const String& filename);
        void saveToFileBMP(const String& filename) const;
        void loadFromFilePNG(const String& filename);
        void saveToFilePNG(const String& filename) const;

        UInt32 getWidth() const;
        UInt32 getHeight() const;
        UInt32 getNumberOfPixels() const;
        UInt64 getSumOfPixelValues() const;
        UInt32 getNumberOfNonBlackPixels(UInt8 maxDiffPerColorChannel = 1) const;

        Bitmap createDiffTo(const Bitmap& other, bool enableDifferentSize = false) const;

        std::vector<UInt8>& getData();
        const std::vector<UInt8>& getData() const;

        Bitmap& operator=(const Bitmap& other) = delete;
        Bitmap& operator=(Bitmap&& other) = delete;
        Bool operator==(const Bitmap& other) const;
        Bool operator!=(const Bitmap& other) const;

    private:
        UInt32 m_width;
        UInt32 m_height;
        std::vector<UInt8> m_data;
    };
}

#endif
