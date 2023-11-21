//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/SceneObject.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicObject.h"

#include "impl/SceneObjectImpl.h"
#include "impl/RamsesObjectVector.h"
#include "impl/RamsesObjectTypeTraits.h"
#include "impl/RamsesObjectTypeUtils.h"

#include <string_view>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

namespace ramses::internal
{
    using SceneObjectVector = std::vector<ramses::SceneObject*>;

    class SceneObjectRegistry final
    {
    public:
        template <typename T, typename ImplT>
        T& createAndRegisterObject(std::unique_ptr<ImplT> impl);
        void destroyAndUnregisterObject(SceneObject& object);

        void reserveAdditionalGeneralCapacity(size_t additionalCount);
        void reserveAdditionalObjectCapacity(ERamsesObjectType type, size_t additionalCount);
        [[nodiscard]] size_t getNumberOfObjects(ERamsesObjectType type) const;

        void getObjectsOfType(SceneObjectVector& objects, ERamsesObjectType ofType) const;

        template <typename T> [[nodiscard]] const T* findObjectByName(std::string_view name) const;
        template <typename T> [[nodiscard]] T* findObjectByName(std::string_view name);

        [[nodiscard]] const SceneObject* findObjectById(sceneObjectId_t id) const;
        SceneObject* findObjectById(sceneObjectId_t id);

        void setNodeDirty(NodeImpl& node, bool dirty);
        [[nodiscard]] bool isNodeDirty(const NodeImpl& node) const;

        [[nodiscard]] const NodeImplSet& getDirtyNodes() const;
        void clearDirtyNodes();

    private:
        void registerObjectInternal(SceneObject& object);
        [[nodiscard]] bool containsObject(const SceneObject& object) const;
        void trackSceneObjectById(SceneObject& object);

        std::unordered_map<sceneObjectId_t, SceneObject*> m_objectsById;

        std::array<std::vector<SceneObject*>, RamsesObjectTypeCount> m_objects;

        using SceneObjectUniquePtr = std::unique_ptr<SceneObject, std::function<void(SceneObject*)>>;
        std::vector<SceneObjectUniquePtr> m_objectsOwningContainer;

        NodeImplSet m_dirtyNodes;

        friend class SceneObjectRegistryIterator;
    };

    template <typename T, typename ImplT>
    T& SceneObjectRegistry::createAndRegisterObject(std::unique_ptr<ImplT> impl)
    {
        static_assert(std::is_base_of_v<SceneObject, T>, "Meant for SceneObject instances only");

        std::unique_ptr<T, std::function<void(SceneObject*)>> object{ new T{ std::move(impl) }, [](SceneObject* o) { delete o; } };
        T* objectRawPtr = object.get();
        this->m_objectsOwningContainer.push_back(std::move(object));

        this->registerObjectInternal(*objectRawPtr);

        return *objectRawPtr;
    }

    template <typename T> T* SceneObjectRegistry::findObjectByName(std::string_view name)
    {
        if constexpr (!std::is_base_of_v<LogicObject, T>) // if searching for logic object don't bother going thru scene registry
        {
            constexpr ERamsesObjectType typeToReturn = TYPE_ID_OF_RAMSES_OBJECT<T>::ID;
            for (size_t typeIdx = 0u; typeIdx < RamsesObjectTypeCount; ++typeIdx)
            {
                const auto type = static_cast<ERamsesObjectType>(typeIdx);
                if (RamsesObjectTypeUtils::IsConcreteType(type) && RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, typeToReturn))
                {
                    const auto& objs = m_objects[typeIdx];
                    const auto it = std::find_if(objs.begin(), objs.end(), [name](const auto o) { return o->getName() == name; });
                    if (it != objs.end())
                        return (*it)->template as<T>();
                }
            }
        }

        // NOLINTNEXTLINE(readability-misleading-indentation) for some reason clang is confused about constexpr branch above
        if constexpr (std::is_base_of_v<T, LogicObject>) // additionally search logic engines if type is base of LogicObject
        {
            auto& logicEngines = m_objects[static_cast<uint32_t>(ERamsesObjectType::LogicEngine)];
            for (auto& le : logicEngines)
            {
                auto obj = le->as<LogicEngine>()->findObject(name);
                if (obj)
                    return obj;
            }
        }

        // NOLINTNEXTLINE(readability-misleading-indentation) for some reason clang is confused about constexpr branch above
        return nullptr;
    }

    template <typename T> const T* SceneObjectRegistry::findObjectByName(std::string_view name) const
    {
        // const version of findObjectByName cast to its non-const version to avoid duplicating code
        return const_cast<SceneObject*>((const_cast<SceneObjectRegistry&>(*this)).findObjectByName<T>(name));
    }
}
