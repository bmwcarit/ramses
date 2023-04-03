//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/DirectedAcyclicGraph.h"

#include <cassert>
#include <algorithm>
#include <unordered_set>
#include <numeric>
#include <iterator>

namespace rlogic::internal
{
    void DirectedAcyclicGraph::addNode(Node& node)
    {
        assert(m_nodeOutgoingEdges.count(&node) == 0);
        assert(m_nodeIncomingEdges.count(&node) == 0);
        m_nodeOutgoingEdges.insert({ &node, {} });
        m_nodeIncomingEdges.insert({ &node, {} });
    }

    void DirectedAcyclicGraph::removeNode(Node& nodeToRemove)
    {
        assert(m_nodeOutgoingEdges.count(&nodeToRemove) != 0);
        assert(m_nodeIncomingEdges.count(&nodeToRemove) != 0);

        // remove node from all edge lists pointing to it from source nodes
        const auto srcNodesIt = m_nodeIncomingEdges.find(&nodeToRemove);
        for (const auto srcNode : srcNodesIt->second)
        {
            EdgeList& srcNodeOutgoingEdges = m_nodeOutgoingEdges.find(srcNode)->second;
            const auto it = std::remove_if(srcNodeOutgoingEdges.begin(), srcNodeOutgoingEdges.end(),
                [&nodeToRemove](const auto& e) { return e.target == &nodeToRemove; });
            srcNodeOutgoingEdges.erase(it, srcNodeOutgoingEdges.end());
        }

        // remove node from all edge lists pointing to it from its target nodes
        const auto tgtNodesIt = m_nodeOutgoingEdges.find(&nodeToRemove);
        for (const auto& tgtNode : tgtNodesIt->second)
        {
            auto& tgtNodeEdges = m_nodeIncomingEdges.find(tgtNode.target)->second;
            tgtNodeEdges.erase(std::find(tgtNodeEdges.begin(), tgtNodeEdges.end(), &nodeToRemove));
        }

        // remove node from both maps
        m_nodeIncomingEdges.erase(srcNodesIt);
        m_nodeOutgoingEdges.erase(&nodeToRemove);
    }

    // This is a slightly exotic sorting algorithm for DAGs
    // It works based on these general principles:
    // - Traverse the DAG starting from the root nodes
    // - Keep the nodes in a sparsely sorted queue (with more slots than actual nodes, and some empty slots)
    // - Any time a new 'edge' is traversed, moves the 'target' node of the edge to the last position of the queue
    // - If number of iterations exceeds N^2, there was a loop in the graph -> abort
    // This is supposed to work fast, because the queue is never re-allocated or re-sorted, only grows incrementally, and
    // we only need to run a second time and remove the 'empty slots' to get the final order.
    std::optional<NodeVector> DirectedAcyclicGraph::getTopologicallySortedNodes() const
    {
        const size_t totalNodeCount = m_nodeOutgoingEdges.size();

        // This remembers temporarily the position of node N in 'nodeQueue' (see below)
        // This index can change in different loops of the code below
        std::unordered_map<const Node*, size_t> nodeIndexIntoQueue;
        nodeIndexIntoQueue.reserve(totalNodeCount);

        // This is a queue of nodes which is:
        // - partially sorted (at any given time during the loops, the first X entries are sorted, while the rest is not sorted yet)
        // - sparse (some entries can be nullptr) - of nodes which were moved during the algorithm, see below
        // - sorted at the end of the loop (by their topological rank)
        // - starts with the root nodes (they are always at the beginning)
        NodeVector sparseNodeQueue = collectRootNodes();

        // Cycle condition - can't find root nodes among a non-empty set of nodes
        if (sparseNodeQueue.empty() && !m_nodeOutgoingEdges.empty())
        {
            return std::nullopt;
        }

        // sparseNodeQueue grows here all the time
        // TODO Violin rework algorithm to not grow and loop over the same container
        for (size_t i = 0; i < sparseNodeQueue.size(); ++i)
        {
            if (i > totalNodeCount * totalNodeCount)
            {
                // TODO Violin this is a primitive loop detection. Replace with a proper graph-based solution!
                // The inner and the outer loop are bound by N(number of nodes), so exceeding that is a
                // sufficient condition that there was a loop
                return std::nullopt;
            }

            // Get the next node in the queue and process based on its outgoing edges
            Node* nextNode = sparseNodeQueue[i];
            // sparseNodeQueue has nullptr holes - skip those
            if (nextNode != nullptr)
            {
                const EdgeList& nextNodeEdges = m_nodeOutgoingEdges.find(nextNode)->second;

                // For each edge, put the 'target' node to the end of the queue (this order may be temporarily wrong,
                // because we don't know if those nodes have also edges between them which would affect this order)
                // What happens if it's wrong? see the if() inside the loop
                for (const auto& outgoingEdge : nextNodeEdges)
                {
                    // Put the node at the end of the 'sparseNodeQueue' and remember the index
                    Node* outgoingEdgeTarget = outgoingEdge.target;
                    sparseNodeQueue.emplace_back(outgoingEdgeTarget);
                    const size_t targetNodeIndex = sparseNodeQueue.size() - 1;

                    const auto potentiallyAlreadyProcessedNode = nodeIndexIntoQueue.find(outgoingEdgeTarget);
                    // target node not processed yet?
                    if (potentiallyAlreadyProcessedNode == nodeIndexIntoQueue.end())
                    {
                        // => insert to processed queue, with current index from 'queue'
                        nodeIndexIntoQueue.insert({ outgoingEdgeTarget, targetNodeIndex });
                    }
                    // target node processed already?
                    else
                    {
                        // => move the node from its last computed index to the current one
                        // (and set to nullptr on its last position so that it does not occur twice in the queue)
                        // Why do we do this? Because it makes sure that any time there is a 'new edge' to a node,
                        // it is moved to the last position in the queue, unless it has no 'incoming edges' (root node)
                        // or it has exactly one incoming edge (and never needs to be re-sorted)
                        sparseNodeQueue[potentiallyAlreadyProcessedNode->second] = nullptr;
                        potentiallyAlreadyProcessedNode->second = targetNodeIndex;
                    }
                }
            }
        }

        // Some nodes are nullptr because of the special 'bubble sort' sorting method
        sparseNodeQueue.erase(std::remove(sparseNodeQueue.begin(), sparseNodeQueue.end(), nullptr), sparseNodeQueue.end());

        return sparseNodeQueue;
    }

    bool DirectedAcyclicGraph::addEdge(Node& source, Node& target)
    {
        assert(m_nodeOutgoingEdges.count(&source) != 0);
        assert(m_nodeOutgoingEdges.count(&target) != 0);
        assert(m_nodeIncomingEdges.count(&source) != 0);
        assert(m_nodeIncomingEdges.count(&target) != 0);

        auto& nodeEdges = m_nodeOutgoingEdges.find(&source)->second;
        auto edgeBetweenNodes = FindEdgeToNode(nodeEdges, target);
        const bool isNewConnection = (edgeBetweenNodes == nodeEdges.end());
        if (isNewConnection)
        {
            nodeEdges.push_back({ &target, 1u });
            auto& tgtToSourcesList = m_nodeIncomingEdges.find(&target)->second;
            assert(std::find(tgtToSourcesList.cbegin(), tgtToSourcesList.cend(), &source) == tgtToSourcesList.cend());
            tgtToSourcesList.push_back(&source);
        }
        else
        {
            edgeBetweenNodes->multiplicity++;
        }

        return isNewConnection;
    }

    void DirectedAcyclicGraph::removeEdge(Node& source, Node& target)
    {
        assert(m_nodeOutgoingEdges.count(&source) != 0);
        assert(m_nodeOutgoingEdges.count(&target) != 0);
        assert(m_nodeIncomingEdges.count(&source) != 0);
        assert(m_nodeIncomingEdges.count(&target) != 0);

        auto& srcNodeEdges = m_nodeOutgoingEdges.find(&source)->second;
        auto outgoingEdge = FindEdgeToNode(srcNodeEdges, target);
        assert(outgoingEdge != srcNodeEdges.end());
        assert(outgoingEdge->multiplicity > 0u);
        --outgoingEdge->multiplicity;
        if (outgoingEdge->multiplicity == 0)
        {
            srcNodeEdges.erase(outgoingEdge);
            auto& tgtToSourcesList = m_nodeIncomingEdges.find(&target)->second;
            assert(std::find(tgtToSourcesList.cbegin(), tgtToSourcesList.cend(), &source) != tgtToSourcesList.cend());
            tgtToSourcesList.erase(std::find(tgtToSourcesList.begin(), tgtToSourcesList.end(), &source));
        }
    }

    size_t DirectedAcyclicGraph::getInDegree(Node& node) const
    {
        assert(m_nodeOutgoingEdges.count(&node) != 0);
        assert(m_nodeIncomingEdges.count(&node) != 0);

        size_t edgeCount = 0u;
        const auto srcNodesList = m_nodeIncomingEdges.find(&node)->second;
        for (const auto srcNode : srcNodesList)
        {
            const EdgeList& srcNodeOutgoingEdges = m_nodeOutgoingEdges.find(srcNode)->second;
            const auto edgeIt = FindEdgeToNode(srcNodeOutgoingEdges, node);
            assert(edgeIt != srcNodeOutgoingEdges.cend());
            edgeCount += edgeIt->multiplicity;
        }

        return edgeCount;
    }

    size_t DirectedAcyclicGraph::getOutDegree(Node& node) const
    {
        assert(containsNode(node));
        const auto it = m_nodeOutgoingEdges.find(&node);
        return std::accumulate(it->second.cbegin(), it->second.cend(), size_t(0u), [](size_t sum, const Edge& e) {
            return sum + e.multiplicity;
        });
    }

    NodeVector DirectedAcyclicGraph::collectRootNodes() const
    {
        NodeVector rootNodes;
        // reserve to all nodes count because it will be used to store all sorted nodes later
        rootNodes.reserve(m_nodeIncomingEdges.size());

        for (const auto& nodeIngoingEdges : m_nodeIncomingEdges)
        {
            if (nodeIngoingEdges.second.empty())
                rootNodes.push_back(nodeIngoingEdges.first);
        }

        return rootNodes;
    }

    bool DirectedAcyclicGraph::containsNode(Node& node) const
    {
        return m_nodeOutgoingEdges.find(&node) != m_nodeOutgoingEdges.end();
    }

    DirectedAcyclicGraph::EdgeList::const_iterator DirectedAcyclicGraph::FindEdgeToNode(const EdgeList& vec, const Node& node)
    {
        return std::find_if(vec.begin(), vec.end(), [&node](const auto& e) { return e.target == &node; });
    }

    DirectedAcyclicGraph::EdgeList::iterator DirectedAcyclicGraph::FindEdgeToNode(EdgeList& vec, const Node& node)
    {
        return std::find_if(vec.begin(), vec.end(), [&node](const auto& e) { return e.target == &node; });
    }
}
