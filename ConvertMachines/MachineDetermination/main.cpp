#include "pch.h"
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

using NondetMachine = vector<vector<set<int>>>;
using DetMachine = vector<vector<int>>;

struct VertexProps
{
	string label;
};

struct EdgeProps
{
	string label;
};

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;

template<typename Matrix>
void SetSizeToMatrix(Matrix& matrix, size_t size1, size_t size2)
{
	matrix.resize(size1);
	for (size_t i = 0; i < matrix.size(); ++i)
	{
		matrix[i].resize(size2);
	}
}

NondetMachine ReadNondetMachine(istream& strm, size_t inputCharactersCount, size_t verticesCount)
{
	NondetMachine machine;
	SetSizeToMatrix(machine, inputCharactersCount, verticesCount);
	for (size_t input = 0; input < machine.size(); ++input)
	{
		for (size_t vertex = 0; vertex < machine[0].size(); ++vertex)
		{
			string str;
			strm >> str;

			if (str == "-")
			{
				continue;
			}

			size_t index = 1;
			while (index < str.size())
			{
				size_t foundIndex = str.find('q', index);
				if (foundIndex != string::npos)
				{
					size_t nextVertex = stoi(string(str, index, foundIndex - index));
					machine[input][vertex].insert(nextVertex);
					index = foundIndex + 1;
				}
				else
				{
					size_t nextVertex = stoi(string(str, index));
					machine[input][vertex].insert(nextVertex);
					break;
				}
			}
		}
	}
	return machine;
}

set<int> SetUnion(set<int> set1, set<int> set2)
{
	set<int> newSet;
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(newSet, newSet.begin()));
	return newSet;
}

DetMachine DetermineMachine(const NondetMachine& nondetMachine)
{
	DetMachine detMachine;
	SetSizeToMatrix(detMachine, nondetMachine.size(), 1);

	map<int, set<int>> nondetTransitions;
	map<set<int>, int> detTransitions;
	nondetTransitions.emplace(0, set<int>{ 0 });
	detTransitions.emplace(set<int>{ 0 }, 0);

	for (size_t vertex = 0; vertex < detMachine[0].size(); ++vertex)
	{
		for (size_t input = 0; input < detMachine.size(); ++input)
		{
			set<int> allNondetTransitions;
			set<int> nondetTransition = nondetTransitions[vertex];
			for (int ver : nondetTransition)
			{
				allNondetTransitions = SetUnion(allNondetTransitions, nondetMachine[input][ver]);
			}

			if (allNondetTransitions.empty())
			{
				detMachine[input][vertex] = -1;
				continue;
			}

			auto iter = detTransitions.find(allNondetTransitions);
			if (iter != detTransitions.end())
			{
				detMachine[input][vertex] = iter->second;
			}
			else
			{
				detMachine[input][vertex] = detMachine[0].size();

				nondetTransitions.emplace(detMachine[0].size(), allNondetTransitions);
				detTransitions.emplace(allNondetTransitions, detMachine[0].size());

				SetSizeToMatrix(detMachine, detMachine.size(), detMachine[0].size() + 1);
			}
		}
	}

	return detMachine;
}

void PrintDetMachine(const DetMachine& machine, const string& fileName)
{
	ofstream fOut(fileName);
	for (size_t input = 0; input < machine.size(); input++)
	{
		for (size_t vertex = 0; vertex < machine[0].size(); vertex++)
		{
			if (machine[input][vertex] == -1)
			{
				fOut << "- ";
			}
			else
			{
				fOut << 's' << machine[input][vertex] << ' ';
			}
		}
		fOut << endl;
	}
}

void CreateGraph(const DetMachine& machine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t vertex = 0; vertex < machine[0].size(); ++vertex)
	{
		string vertexLabel = 's' + to_string(vertex);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t input = 0; input < machine.size(); ++input)
	{
		for (size_t vertex = 0; vertex < machine[0].size(); ++vertex)
		{
			if (machine[input][vertex] != -1)
			{
				string edgeLabel = 'x' + to_string(input + 1);
				boost::add_edge(vertices[vertex], vertices[machine[input][vertex]], { edgeLabel }, graph);
			}
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));
	ofstream ofs("graph.dot");
	boost::write_graphviz_dp(ofs, graph, dp);
}

int main(int argc, char* argv[])
{
	ifstream fIn(argv[1]);
	size_t inputCharactersCount;
	size_t verticesCount;
	fIn >> inputCharactersCount >> verticesCount;
	NondetMachine nondetMachine = ReadNondetMachine(fIn, inputCharactersCount, verticesCount);
	DetMachine detMachine = DetermineMachine(nondetMachine);
	PrintDetMachine(detMachine, argv[2]);
	CreateGraph(detMachine);
}