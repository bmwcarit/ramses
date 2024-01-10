//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/logic/PropertyLink.h"

#include <cstddef>
#include <optional>
#include <memory>
#include <string>

namespace ramses::internal
{
    class PropertyImpl;
}

namespace ramses
{
    class LogicNode;

    /**
    * Represents a generic property slot of the #ramses::LogicNode and its derived classes.
    * Properties can have primitive types (string, integer, etc.) or complex types (structs, arrays).
    * Complex types can have "children", i.e. nested properties: named fields (structs), or indexed
    * fields (arrays).
    * @ingroup LogicAPI
    */
    class RAMSES_API Property
    {
    public:
        /**
        * Returns the type of this Property
        *
        * @return the type of this Property
        */
        [[nodiscard]] EPropertyType getType() const;

        /**
        * Returns the name of this Property. Note that not all properties have a name -
        * for example an array element does not have a name. In that case the result
        * will be an empty string view. Struct fields always have a non-empty name.
        *
        * @return the name of this Property
        */
        [[nodiscard]] std::string_view getName() const;

        /**
        * Returns the amount of available child (nested) properties. If the Property
        * is of type #ramses::EPropertyType::Struct, the returned number will correspond
        * to the number of named properties of the struct. If the Property is of type
        * #ramses::EPropertyType::Array, the method will return the array size. For all
        * other property types #getChildCount returns zero.
        *
        * @return the number of nested properties.
        */
        [[nodiscard]] size_t getChildCount() const;

        /**
        * Return whether the #ramses::Property has a child with the given name
        *
        * @param name the name of the child that should be checked for existence
        * @return whether the child with the given name exists
        */
        [[nodiscard]] bool hasChild(std::string_view name) const;

        /**
        * Returns the child property with the given \p index. \p index must be < #getChildCount().
        *
        * This method can be used to get nested properties of structs and arrays. For primitive types this will
        * always return nullptr.
        *
        * Note that array indexing in #getChild follows C++ conventions and **not** Lua conventions! Inside the Lua scripts, you can
        * and must use Lua conventions when indexing arrays (start at 1, end at N) while in C++ you must use [0, N-1].
        *
        * Struct properties can also be retrieved by index. The ordering is **not** guaranteed to match the order of
        * declaration inside Lua scripts (Lua explicitly warns to not rely on ordering of named table entries!).
        * However, once a script is created, the index will not change, i.e. it is permitted that user code caches
        * the property index for faster future access.
        *
        * @param[in] index zero based index of child property to retrieve
        * @return the child with the given index, or nullptr if the property is primitive or the index is out of range
        */
        [[nodiscard]] const Property* getChild(size_t index) const;

        /**
        * @copydoc getChild(size_t index) const
        */
        [[nodiscard]] Property* getChild(size_t index);

        /**
        * Searches for a child with the given name. Only properties of type #ramses::EPropertyType::Struct can return a child by name.
        * In case of a primitive property or array this method will return nullptr.
        *
        * Note that this method may be slower than #getChild(size_t index) as it must do a string-based search.
        *
        * @param[in] name name of child property to retrieve
        * @return the child with the given name, or nullptr if property is not of type #ramses::EPropertyType::Struct
        */
        [[nodiscard]] Property* getChild(std::string_view name);

        /**
        * @copydoc getChild(std::string_view name)
        */
        [[nodiscard]] const Property* getChild(std::string_view name) const;

        /**
        * Returns the value of this property. The supported template types are defined by
        * #ramses::IsPrimitiveProperty where IsPrimitiveProperty<T>::value == true for a type T.
        *
        * Attention! We recommend always specifying the template argument T explicitly, and don't rely on the compiler's
        * type deduction! If T is not one of the supported types, a static_assert will be triggered!
        *
        * Returns nullopt if the template type T does not match the internal type of the property.
        *
        * @return the value of this Property as std::optional or std::nullopt if T does not match.
        */
        template <typename T> [[nodiscard]] std::optional<T> get() const;

        /**
        * Sets the value of this Property. Same rules apply to template parameter T as in #get()
        *
        * Attention! We recommend always specifying the template argument T explicitly, and don't rely on the compiler's
        * type deduction! If T is not one of the supported types, a static_assert will be triggered!
        *
        * #set() will report an error if the Property is linked. Use #hasIncomingLink() to check in advance.
        *
        * @param value the value to set for this Property
        * @return true if setting the \p value was successful, false otherwise.
        */
        template <typename T> bool set(T value);

        /**
        * Checks if this property is linked to a property of another node.
        *
        * Property can be either on inputs side or outputs side of a node (or both in case of #ramses::LuaInterface),
        * which defines what type of link is applicable - an incoming or outgoing.
        * This query checks for any links, regardless of data flow direction.
        * See #hasIncomingLink() #hasOutgoingLink() for dedicated checks
        *
        * @return true if the property is linked, false otherwise.
        */
        [[nodiscard]] bool isLinked() const;

        /**
        * Checks if there is any incoming link to this property, i.e. data is flowing to it from another node's property.
        * There can be only one incoming link per input property.
        *
        * @return true if the property is an input and is linked, false otherwise.
        */
        [[nodiscard]] bool hasIncomingLink() const;

        /**
        * Checks if there is any outgoing link from this property, i.e. data is flowing from it to another node's property.
        * There can be multiple outgoing links per output property.
        *
        * @return true if the property is an output and is linked, false otherwise.
        */
        [[nodiscard]] bool hasOutgoingLink() const;

        /**
        * Returns information about the incoming link to this property, namely which property is the source of the link.
        * Only certain properties can have an incoming link, see #ramses::LogicEngine::link for details.
        *
        * @return incoming link data or std::nullopt if there is no link.
        */
        [[nodiscard]] std::optional<PropertyLinkConst> getIncomingLink() const;

        /// @copydoc getIncomingLink() const
        [[nodiscard]] std::optional<PropertyLink> getIncomingLink();

        /**
        * Returns the number of outgoing links from this property.
        * Only certain properties can have outgoing links, see #ramses::LogicEngine::link for details.
        *
        * @return number of outgoing links from this property.
        */
        [[nodiscard]] size_t getOutgoingLinksCount() const;

        /**
        * Returns information about the outgoing link from this property, namely which property is the target of the link with given index.
        * Use #getOutgoingLinksCount to query how many outgoing links there are.
        * Only certain properties can have outgoing links, see #ramses::LogicEngine::link for details.
        *
        * @param[in] index zero based index of outgoing link to retrieve
        * @return outgoing link data or std::nullopt if there is no link with given index.
        */
        [[nodiscard]] std::optional<PropertyLinkConst> getOutgoingLink(size_t index) const;

        /// @copydoc getOutgoingLink(size_t index) const
        [[nodiscard]] std::optional<PropertyLink> getOutgoingLink(size_t index);

        /**
        * Get #ramses::LogicNode that owns this property.
        * Every property is owned by a #ramses::LogicNode and represents either its input or output.
        *
        * @return #ramses::LogicNode that owns this property.
        */
        [[nodiscard]] const LogicNode& getOwningLogicNode() const;

        /// @copydoc getOwningLogicNode() const
        [[nodiscard]] LogicNode& getOwningLogicNode();

        /**
         * Get the internal data for implementation specifics of #Property.
         */
        [[nodiscard]] internal::PropertyImpl& impl();

        /**
         * Get the internal data for implementation specifics of #Property.
         */
        [[nodiscard]] const internal::PropertyImpl& impl() const;

        /**
        * Copy Constructor of Property is deleted because properties are not supposed to be copied
        * @param other property to copy from
        */
        Property(const Property& other) = delete;

        /**
        * Move Constructor of Property is deleted because properties are not supposed to be moved
        * @param other property to move from
        */
        Property(Property&& other) = delete;

        /**
        * Assignment operator of Property is deleted because properties are not supposed to be copied
        * @param other property to assign from
        */
        Property& operator=(const Property& other) = delete;

        /**
        * Move assignment operator of Property is deleted because properties are not supposed to be moved
        * @param other property to move from
        */
        Property& operator=(Property&& other) = delete;

    protected:
        /**
         * Constructor of Property. User is not supposed to call this - properties are created by other factory classes
         *
         * @param impl implementation details of the property
         */
        explicit Property(std::unique_ptr<internal::PropertyImpl> impl) noexcept;

        /**
         * Destructor of Property. User is not supposed to call this - properties are destroyed by other factory classes
         */
        ~Property() noexcept;

        /**
         * Implementation details of the Property class
         */
        std::unique_ptr<internal::PropertyImpl> m_impl;

    private:
        /**
         * Internal implementation of #get
         */
        template <typename T> [[nodiscard]] std::optional<T> getInternal() const;
        /**
         * Internal implementation of #set
         */
        template <typename T> bool setInternal(T value);

        friend class internal::PropertyImpl;
    };

    template <typename T> std::optional<T> Property::get() const
    {
        static_assert(IsPrimitiveProperty<T>::value, "Call get<T> only with types which have a value! Read the docs of the method!");
        return getInternal<T>();
    }

    template <typename T> bool Property::set(T value)
    {
        static_assert(IsPrimitiveProperty<T>::value, "Call set<T> only with types which have a value! Read the docs of the method!");
        return setInternal<T>(value);
    }
}
