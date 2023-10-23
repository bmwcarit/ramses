//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/logic/LogicNodeDependencies.h"

#include "ramses/client/logic/Property.h"

#include "impl/logic/LogicNodeImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/logic/RamsesBindingImpl.h"

#include "impl/ErrorReporting.h"
#include "internal/logic/TypeUtils.h"

#include <cassert>
#include "fmt/format.h"

namespace ramses::internal
{

    void LogicNodeDependencies::addNode(LogicNodeImpl& node)
    {
        assert(!m_logicNodeDAG.containsNode(node));
        m_logicNodeDAG.addNode(node);
        m_nodeTopologyChanged = true;
    }

    void LogicNodeDependencies::removeNode(LogicNodeImpl& node)
    {
        assert(m_logicNodeDAG.containsNode(node));
        m_logicNodeDAG.removeNode(node);

        // Remove the node from the cache without reordering the rest (unless there is no cache yet)
        // Removing nodes does not require topology update (we don't guarantee specific ordering when
        // nodes are not related, we only guarantee relative ordering when nodes are linked)
        if (m_cachedTopologicallySortedNodes)
        {
            NodeVector& cachedNodes = *m_cachedTopologicallySortedNodes;
            cachedNodes.erase(std::remove(cachedNodes.begin(), cachedNodes.end(), &node), cachedNodes.end());
        }
    }

    bool LogicNodeDependencies::isLinked(const LogicNodeImpl& logicNode) const
    {
        auto inputs = logicNode.getInputs();
        if (isLinked(inputs->impl()))
        {
            return true;
        }

        const auto outputs = logicNode.getOutputs();
        if (nullptr != outputs)
        {
            return isLinked(outputs->impl());
        }
        return false;
    }

    bool LogicNodeDependencies::isLinked(const PropertyImpl& input) const
    {
        const auto inputCount = input.getChildCount();
        // check if an input of this node is a target of another node
        for (size_t i = 0; i < inputCount; ++i)
        {
            const auto child = input.getChild(i);
            if (TypeUtils::CanHaveChildren(child->getType()))
            {
                if (isLinked(child->impl()))
                {
                    return true;
                }
            }
            else
            {
                assert(TypeUtils::IsPrimitiveType(child->getType()));
                if (child->impl().isLinked())
                {
                    return true;
                }
            }
        }
        return false;
    }

    const std::optional<NodeVector>& LogicNodeDependencies::getTopologicallySortedNodes()
    {
        if (m_nodeTopologyChanged)
        {
            m_cachedTopologicallySortedNodes = m_logicNodeDAG.getTopologicallySortedNodes();
            m_nodeTopologyChanged = false;
        }

        return m_cachedTopologicallySortedNodes;
    }

    bool LogicNodeDependencies::link(PropertyImpl& output, PropertyImpl& input, bool isWeakLink, ErrorReporting& errorReporting)
    {
        if (!m_logicNodeDAG.containsNode(output.getLogicNode()))
        {
            errorReporting.set(fmt::format("LogicNode '{}' is not an instance of this LogicEngine", output.getLogicNode().getName()), nullptr);
            return false;
        }

        if (!m_logicNodeDAG.containsNode(input.getLogicNode()))
        {
            errorReporting.set(fmt::format("LogicNode '{}' is not an instance of this LogicEngine", input.getLogicNode().getName()), nullptr);
            return false;
        }

        if (&output.getLogicNode() == &input.getLogicNode())
        {
            errorReporting.set(fmt::format("Link source and target can't belong to the same node! ('{}')", input.getLogicNode().getName()), nullptr);
            return false;
        }

        if (!(output.isOutput() && input.isInput()))
        {
            std::string_view lhsType = output.isOutput() ? "output" : "input";
            std::string_view rhsType = input.isOutput() ? "output" : "input";
            errorReporting.set(fmt::format("Failed to link {} property '{}' to {} property '{}'. Only outputs can be linked to inputs", lhsType, output.getName(), rhsType, input.getName()), nullptr);
            return false;
        }

        if (output.getType() != input.getType())
        {
            errorReporting.set(fmt::format("Types of source property '{}:{}' does not match target property '{}:{}'",
                output.getName(),
                GetLuaPrimitiveTypeName(output.getType()),
                input.getName(),
                GetLuaPrimitiveTypeName(input.getType())), nullptr);
            return false;
        }

        // No need to also test input type, above check already makes sure output and input are of the same type
        if (!TypeUtils::IsPrimitiveType(output.getType()))
        {
            errorReporting.set(fmt::format("Can't link properties of complex types directly, currently only primitive properties can be linked"), nullptr);
            return false;
        }

        const PropertyImpl* linkedIncomingProperty = input.getIncomingLink().property;
        if (linkedIncomingProperty != nullptr)
        {
            errorReporting.set(fmt::format("The property '{}' of LogicNode '{}' is already linked (to property '{}' of LogicNode '{}')",
                input.getName(),
                input.getLogicNode().getName(),
                linkedIncomingProperty->getName(),
                linkedIncomingProperty->getLogicNode().getName()
            ), nullptr);
            return false;
        }

        input.setIncomingLink(output, isWeakLink);

        if (!isWeakLink)
        {
            const bool isNewEdge = m_logicNodeDAG.addEdge(output.getLogicNode(), input.getLogicNode());
            if (isNewEdge)
            {
                m_nodeTopologyChanged = true;
            }
        }

        // TODO Violin don't set anything dirty here, handle dirtiness purely in update()
        input.getLogicNode().setDirty(true);
        output.getLogicNode().setDirty(true);

        return true;
    }

    bool LogicNodeDependencies::unlink(PropertyImpl& output, PropertyImpl& input, ErrorReporting& errorReporting)
    {
        if (TypeUtils::CanHaveChildren(input.getType()))
        {
            errorReporting.set(fmt::format("Can't unlink properties of complex types directly!"), nullptr);
            return false;
        }

        const PropertyImpl* linkedIncomingProperty = input.getIncomingLink().property;
        if (linkedIncomingProperty == nullptr)
        {
            errorReporting.set(fmt::format("Input property '{}' is not currently linked!", input.getName()), nullptr);
            return false;
        }

        if (linkedIncomingProperty != &output)
        {
            errorReporting.set(fmt::format("Input property '{}' is currently linked to another property '{}'", linkedIncomingProperty->getName(), input.getName()), nullptr);
            return false;
        }

        if (!input.getIncomingLink().isWeakLink)
        {
            auto& node = output.getLogicNode();
            auto& targetNode = input.getLogicNode();
            m_logicNodeDAG.removeEdge(node, targetNode);
        }

        input.resetIncomingLink();

        return true;
    }

    void LogicNodeDependencies::addBindingDependency(RamsesBindingImpl& binding, LogicNodeImpl& node)
    {
        assert(m_logicNodeDAG.containsNode(node));
        assert(m_logicNodeDAG.containsNode(binding));
        assert(&node != &binding);

        if (m_logicNodeDAG.addEdge(binding, node))
            m_nodeTopologyChanged = true;
    }

    void LogicNodeDependencies::removeBindingDependency(RamsesBindingImpl& binding, LogicNodeImpl& node)
    {
        assert(m_logicNodeDAG.containsNode(node));
        assert(m_logicNodeDAG.containsNode(binding));
        assert(&node != &binding);

        m_logicNodeDAG.removeEdge(binding, node);
    }
}
