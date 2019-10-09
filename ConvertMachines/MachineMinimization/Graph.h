#pragma once

#include <string>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

struct VertexProps
{
	std::string label;
};

struct EdgeProps
{
	std::string label;
};

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;