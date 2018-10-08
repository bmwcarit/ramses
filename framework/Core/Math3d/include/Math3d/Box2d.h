//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BOX2D_H
#define RAMSES_BOX2D_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformMemory.h>
#include <PlatformAbstraction/PlatformMath.h>
#include "Math3d/Measurement2d.h"

namespace ramses_internal
{
    /// Class for storing a two-dimensional box.
    template<typename T, typename S=T>
        class Box2d
        {
        public:

            /// Constructor.
            /** @param position Position of the box.
             *  @param size Size of the box. */
            Box2d(const Measurement2d<T>& position, const Measurement2d<S>& size);

            /// Constructor.
            /** Creates an empty box. */
            Box2d();

            /// Returns the position.
            /** @return The position. */
            const Measurement2d<T>& position() const;

            /// Returns the size.
            /** @return The size. */
            const Measurement2d<S>& size() const;

            /// Returns the maximum position (position+size).
            /** @return The maximum position. */
            const Measurement2d<T> maxPosition() const;

            /// Sets the position of the box.
            /** @param position Position of the box. */
            void setPosition(const Measurement2d<T>& position);

            /// Sets the size of the box.
            /** @param size Size of the box. */
            void setSize(const Measurement2d<S>& size);

            /// Returns whether this box is empty or not.
            /** @return "true", when the size is 0 in at least one direction. */
            bool isEmpty() const;

            /// Merge this box together with another box.
            /** The resulting box contains both boxes.
             *  @param other The box to merge with. */
            void mergeWith(const Box2d<T, S>& other);

            /// Checks wheter this box has a common edge together with another box.
            /** @param other The box to check with.
             *  @return "true", when the boxes have a common edge. */
            bool hasCommonEdge(const Box2d<T, S>& other) const;

            bool operator==(const Box2d<T, S>& other) const;

        protected:

            /// Position of the box.
            Measurement2d<T> m_position;

            /// Size of the box.
            Measurement2d<S> m_size;
        };

    template<typename T, typename S>
        Box2d<T, S>::Box2d(const Measurement2d<T>& position, const Measurement2d<S>& size) :
            m_position(position),
            m_size(size)
        {
        }

    template<typename T, typename S>
        Box2d<T, S>::Box2d()
        {
        }

    template<typename T, typename S>
        const Measurement2d<T>& Box2d<T, S>::position() const
        {
            return m_position;
        }

    template<typename T, typename S>
        const Measurement2d<S>& Box2d<T, S>::size() const
        {
            return m_size;
        }

    template<typename T, typename S>
        const Measurement2d<T> Box2d<T, S>::maxPosition() const
        {
            return m_position + m_size;
        }

    template<typename T, typename S>
        void Box2d<T, S>::setPosition(const Measurement2d<T>& position)
        {
            m_position = position;
        }

    template<typename T, typename S>
        void Box2d<T, S>::setSize(const Measurement2d<S>& size)
        {
            m_size = size;
        }

    template<typename T, typename S>
        bool Box2d<T, S>::isEmpty() const
        {
            return m_size.x == 0 || m_size.y == 0;
        }

    template<typename T, typename S>
        void Box2d<T, S>::mergeWith(const Box2d<T, S>& other)
        {
            if (isEmpty())
            {
                m_position = other.position();
                m_size = other.size();
            }
            else
            {
                if (!other.isEmpty())
                {
                    Measurement2d<T> max1 = maxPosition();
                    Measurement2d<T> max2 = other.maxPosition();

                    m_position.x = min(m_position.x, other.position().x);
                    m_position.y = min(m_position.y, other.position().y);

                    m_size.x = max(max1.x, max2.x) - m_position.x;
                    m_size.y = max(max1.y, max2.y) - m_position.y;
                }
            }

        }

    template<typename T, typename S>
        bool Box2d<T, S>::hasCommonEdge(const Box2d<T, S>& other) const
        {
            const Measurement2d<T>& min1 = m_position;
            const Measurement2d<T>& min2 = other.position();
            const Measurement2d<T> max1 = maxPosition();
            const Measurement2d<T> max2 = other.maxPosition();

            if ((min1.y == min2.y) && (max1.y == max2.y))
            {
                return (max1.x == min2.x) || (max2.x == min1.x);
            }

            if ((min1.x == min2.x) && (max1.x == max2.x))
            {
                return (max1.y == min2.y) || (max2.y == min1.y);
            }
            return false;
        }

        template <typename T, typename S>
        bool Box2d<T, S>::operator==(const Box2d<T, S>& other) const
        {
            return (m_position == other.m_position) && (m_size == other.m_size);
        }
}

#endif
