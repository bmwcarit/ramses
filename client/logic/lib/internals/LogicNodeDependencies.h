//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/DirectedAcyclicGraph.h"

#include <unordered_set>

namespace rlogic::internal
{
    class LogicNodeImpl;
    class ErrorReporting;
    class PropertyImpl;
    class RamsesBindingImpl;

    using NodeSet = std::unordered_set<LogicNodeImpl*>;

    // Tracks the links between logic nodes and orders them based on the topological structure derived
    // from those links.
    class LogicNodeDependencies
    {
    public:
        // The primary purpose of this class
        [[nodiscard]] const std::optional<NodeVector>& getTopologicallySortedNodes();

        // Nodes management
        void addNode(LogicNodeImpl& node);
        void removeNode(LogicNodeImpl& node);

        // Link management
        bool link(PropertyImpl& output, PropertyImpl& input, bool isWeakLink, ErrorReporting& errorReporting);
        bool unlink(PropertyImpl& output, PropertyImpl& input, ErrorReporting& errorReporting);
        [[nodiscard]] bool isLinked(const LogicNodeImpl& node) const;

        // Dependency between binding and node, i.e. node depends on binding
        void addBindingDependency(RamsesBindingImpl& binding, LogicNodeImpl& node);
        void removeBindingDependency(RamsesBindingImpl& binding, LogicNodeImpl& node);

    private:
        DirectedAcyclicGraph m_logicNodeDAG;

        [[nodiscard]] bool isLinked(PropertyImpl& input) const;

        // Initial state: no nodes and no need to re-compute node topology
        std::optional<NodeVector> m_cachedTopologicallySortedNodes = NodeVector{};
        bool m_nodeTopologyChanged = false;
    };
}
