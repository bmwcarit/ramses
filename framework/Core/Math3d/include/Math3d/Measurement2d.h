//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEASUREMENT2D_H
#define RAMSES_MEASUREMENT2D_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include "Utils/Warnings.h"

namespace ramses_internal
{
    template<typename T>
    struct Measurement2d
    {
        /**
         *  The union is used to allow different ways of access
         */
        union
        {
            /**
             *  Access with width and height
             */
            IGNORE_UNNAMED_MEMBER_WARNING_START
            struct
            {
                T width;
                T height;
            };

            struct
            {
                T x;
                T y;
            };
            IGNORE_UNNAMED_MEMBER_WARNING_END

            /**
             *  Access to raw data. E.g. for usage as shader parameters
             */
            T data[2];
        };

        /**
         * Default constructor of Measurement2d
         */
        Measurement2d();

        /**
         *  Constructor of Rectangle
         *  @param width of the Rectangle
         *  @param height of the Rectangle
         */
        Measurement2d(const T _width, const T _height);

        /**
         * Sets the values of the measurement.
         * @param _x The x value.
         * @param _y The y value.
         */
        void set(const T _x, const T _y);

        /**
         * Copy constructor of Measurement2d
         */
        template<typename C>
        Measurement2d(const Measurement2d<C>& other)
            : width(static_cast<T>(other.width))
            , height(static_cast<T>(other.height))
        {
        }

        /**
         * Equal operator for Measurement2d
         * @param Measurement2d to compare with
         * @return true if Measurement2ds are equal, false otherwise
         */
        Bool operator==(const Measurement2d<T>& other) const;

        /**
         * Returns the ratio between the internal parameters
         * @return the ratio between the internal parameters
         */
        Float getAspect() const;

        /**
         * Returns the sum of this and an other measurement.
         * @param other The measurement to add.
         * @return The sum.
         */
        Measurement2d<T> operator+(const Measurement2d<T>& other) const;

        /**
         * Returns the difference between this and an other measurement.
         * @param other The measurement to subtract.
         * @return The difference.
         */
        Measurement2d<T> operator-(const Measurement2d<T>& other) const;
    };

    template<typename T>
    inline
    Measurement2d<T>::Measurement2d()
        : width(0)
        , height(0)
    {
    }

    template<typename T>
    inline
    Measurement2d<T>::Measurement2d(const T _width, const T _height)
        : width(_width)
        , height(_height)
    {
    }

    template<typename T>
    inline
    Float
    Measurement2d<T>::getAspect() const
    {
        return static_cast<Float>(width) / static_cast<Float>(height);
    }

    template<typename T>
    inline
    Bool Measurement2d<T>::operator==(const Measurement2d<T>& other) const
    {
        return PlatformMemory::Compare(data, other.data, sizeof(data)) == 0;
    }

    template<typename T>
        inline
    void Measurement2d<T>::set(const T _x, const T _y)
    {
        x = _x;
        y = _y;
    }

    template<typename T>
        inline
    Measurement2d<T> Measurement2d<T>::operator+(const Measurement2d<T>& other) const
    {
        return Measurement2d<T>(x + other.x, y + other.y);
    }

    template<typename T>
        inline
    Measurement2d<T> Measurement2d<T>::operator-(const Measurement2d<T>& other) const
    {
        return Measurement2d<T>(x - other.x, y - other.y);
    }
}

#endif
