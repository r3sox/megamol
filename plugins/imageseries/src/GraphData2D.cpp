#include "imageseries/graph/GraphData2D.h"

#include <type_traits>

using namespace megamol::ImageSeries::graph;

GraphData2D::NodeID GraphData2D::addNode(Node node) {
    const auto newNodeID = nodes.size();

    nodeMap[std::make_pair(node.frameIndex, node.label)] = newNodeID;
    nodes.push_back(std::move(node));

    return newNodeID;
}

void GraphData2D::addEdge(Edge edge) {
    if (std::find(edges.begin(), edges.end(), edge) == edges.end()) {
        getNode(edge.from).edgeCountOut++;
        getNode(edge.to).edgeCountIn++;

        getNode(edge.from).childNodes.insert(edge.to);
        getNode(edge.to).parentNodes.insert(edge.from);

        edges.push_back(std::move(edge));
    }
}

GraphData2D::Node GraphData2D::removeNode(NodeID id, bool lazy) {
    GraphData2D::Node toBeRemoved(nodes[id]);

    if (lazy) {
        // Set placeholder instead of removing the node completely
        nodes[id].valid = false;
    } else {
        // Remove edges connected to this node
        for (auto& parent : nodes[id].parentNodes) {
            removeEdge(parent, id);
        }
        for (auto& child : nodes[id].childNodes) {
            removeEdge(id, child);
        }

        // Remove node
        nodes.erase(nodes.begin() + id);

        // Remove node from map; update indices of all other nodes and edges
        std::TimeLabelPair toBeErased;
        for (auto& entry : nodeMap) {
            if (entry.second > id) {
                --entry.second;
            } else if (entry.second == id) {
                toBeErased = entry.first;
            }
        }
        nodeMap.erase(toBeErased);

        for (auto& edge : edges) {
            if (edge.from > id) {
                --edge.from;
            }
            if (edge.to > id) {
                --edge.to;
            }
        }

        for (auto& node : nodes) {
            std::unordered_set<NodeID> newParents, newChildren;
            for (const auto parentID : node.parentNodes) {
                if (parentID > id) {
                    newParents.insert(parentID - 1);
                } else {
                    newParents.insert(parentID);
                }
            }
            for (const auto childID : node.childNodes) {
                if (childID > id) {
                    newChildren.insert(childID - 1);
                } else {
                    newChildren.insert(childID);
                }
            }
            std::swap(node.parentNodes, newParents);
            std::swap(node.childNodes, newChildren);
        }
    }

    return toBeRemoved;
}

void GraphData2D::removeEdge(NodeID from, NodeID to) {
    --getNode(from).edgeCountOut;
    --getNode(to).edgeCountIn;

    getNode(from).childNodes.erase(to);
    getNode(to).parentNodes.erase(from);

    for (auto it = edges.begin(); it != edges.end(); ++it) {
        if (it->from == from && it->to == to) {
            edges.erase(it);
            break;
        }
    }
}

void GraphData2D::finalizeLazyRemoval() {
    // Remove invalidated edges and remove nodes from node map
    for (NodeID id = 0; id < nodes.size(); ++id) {
        const auto& node = nodes[id];

        if (!node.valid) {
            for (auto& parent : node.parentNodes) {
                removeEdge(parent, id);
            }
            for (auto& child : node.childNodes) {
                removeEdge(id, child);
            }

            for (auto& entry : nodeMap) {
                if (entry.second == id) {
                    nodeMap.erase(entry.first);
                    break;
                }
            }
        }
    }

    // Update node IDs in node map and edges
    std::vector<NodeID> offsets;

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (!nodes[i].valid) {
            offsets.push_back(i);
        }
    }

    auto getOffset = [&offsets](NodeID id) {
        NodeID offset = 0;

        for (std::size_t i = 0; i < offsets.size(); ++i) {
            if (id > offsets[i]) {
                offset = i + 1;
            }
        }

        return offset;
    };

    for (auto& entry : nodeMap) {
        entry.second -= getOffset(entry.second);
    }

    for (auto& edge : edges) {
        edge.from -= getOffset(edge.from);
        edge.to -= getOffset(edge.to);
    }

    for (auto& node : nodes) {
        std::unordered_set<NodeID> newParents, newChildren;
        for (const auto parentID : node.parentNodes) {
            newParents.insert(parentID - getOffset(parentID));
        }
        for (const auto childID : node.childNodes) {
            newChildren.insert(childID - getOffset(childID));
        }
        std::swap(node.parentNodes, newParents);
        std::swap(node.childNodes, newChildren);
    }

    for (auto it = nodes.begin(); it != nodes.end();) {
        if (!it->valid) {
            it = nodes.erase(it);
        } else {
            ++it;
        }
    }
}

bool GraphData2D::hasNode(NodeID id) const {
    return id < nodes.size();
}

GraphData2D::NodeID GraphData2D::getNodeCount() const {
    return nodes.size();
}

GraphData2D::Node& GraphData2D::getNode(NodeID id) {
    return id < nodes.size() ? nodes[id] : placeholderNode;
}

const GraphData2D::Node& GraphData2D::getNode(NodeID id) const {
    return id < nodes.size() ? nodes[id] : placeholderNode;
}

bool GraphData2D::hasEdge(NodeID from, NodeID to) const {
    for (auto& edge : edges) {
        if (edge.from == from && edge.to == to) {
            return true;
        }
    }

    return false;
}

GraphData2D::Edge& GraphData2D::getEdge(NodeID from, NodeID to) {
    for (auto& edge : edges) {
        if (edge.from == from && edge.to == to) {
            return edge;
        }
    }

    throw std::runtime_error("Edge not found for given nodes.");
}

const GraphData2D::Edge& GraphData2D::getEdge(NodeID from, NodeID to) const {
    for (const auto& edge : edges) {
        if (edge.from == from && edge.to == to) {
            return edge;
        }
    }

    throw std::runtime_error("Edge not found for given nodes.");
}

std::pair<GraphData2D::NodeID, std::reference_wrapper<GraphData2D::Node>> GraphData2D::findNode(
    Timestamp time, Label label) {
    auto it = nodeMap.find(std::make_pair(time, label));

    if (it == nodeMap.end()) {
        throw std::runtime_error("Node not found for given time and label.");
    }

    return std::make_pair(static_cast<NodeID>(it->second), std::reference_wrapper<Node>(getNode(it->second)));
}

std::pair<GraphData2D::NodeID, std::reference_wrapper<const GraphData2D::Node>> GraphData2D::findNode(
    Timestamp time, Label label) const {
    auto it = nodeMap.find(std::make_pair(time, label));

    if (it == nodeMap.end()) {
        throw std::runtime_error("Node not found for given time and label.");
    }

    return std::make_pair(static_cast<NodeID>(it->second), std::reference_wrapper<const Node>(getNode(it->second)));
}

std::vector<GraphData2D::Node>& GraphData2D::getNodes() {
    return nodes;
}

const std::vector<GraphData2D::Node>& GraphData2D::getNodes() const {
    return nodes;
}

std::vector<GraphData2D::Edge>& GraphData2D::getEdges() {
    return edges;
}

const std::vector<GraphData2D::Edge>& GraphData2D::getEdges() const {
    return edges;
}

std::vector<std::vector<GraphData2D::NodeID>> GraphData2D::getOutboundEdges() const {
    std::vector<std::vector<GraphData2D::NodeID>> result(nodes.size());
    for (auto& edge : edges) {
        if (edge.from < result.size()) {
            result[edge.from].push_back(edge.to);
        }
    }
    return result;
}

std::vector<std::vector<GraphData2D::NodeID>> GraphData2D::getInboundEdges() const {
    std::vector<std::vector<GraphData2D::NodeID>> result(nodes.size());
    for (auto& edge : edges) {
        if (edge.to < result.size()) {
            result[edge.to].push_back(edge.from);
        }
    }
    return result;
}
