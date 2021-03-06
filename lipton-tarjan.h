#pragma once
#include "Partition.h"
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <vector>
#define STLALL(x) (x).begin(), (x).end()

typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, boost::property<boost::vertex_index_t, int>, boost::property<boost::edge_index_t, int>> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor                                                                                                                 vertex_t;
typedef boost::graph_traits<Graph>::vertex_iterator                                                                                                                   VertIter;
typedef std::map<vertex_t, size_t>                                                                                                                                    vertex_map;
typedef boost::graph_traits<Graph>::edge_iterator                                                                                                                     EdgeIter;
typedef boost::graph_traits<Graph>::edge_descriptor                                                                                                                   edge_t;
typedef boost::graph_traits<Graph>::out_edge_iterator                                                                                                                 OutEdgeIter;
typedef boost::graph_traits<Graph>::edges_size_type                                                                                                                   EdgesSizeType;
typedef std::vector<std::vector<edge_t>>                                                                                                                              EmbeddingStorage;
typedef boost::iterator_property_map<EmbeddingStorage::iterator, boost::property_map<Graph, boost::vertex_index_t>::type>                                             Embedding; 
typedef boost::property_map<Graph, boost::edge_index_t>::type                                                                                                         EdgeIndex; 

Partition lipton_tarjan(Graph const& g);

extern std::map<vertex_t, uint> vert2uint;
extern std::map<uint, vertex_t> uint2vert;
