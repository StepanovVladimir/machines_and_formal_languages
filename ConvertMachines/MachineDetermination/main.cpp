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

enum struct VertexType
{
	Starting = 'S',
	Normal = 'N',
	Finishing = 'F'
};

struct DetMachine
{
	vector<VertexType> types;
	vector<vector<int>> graph;
};

struct VertexProps
{
	string label;
};

struct EdgeProps
{
	string label;
};

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;

void SetSizeToNondet(NondetMachine& machine, size_t inputCharactersCount, size_t verticesCount)
{
	machine.resize(inputCharactersCount);
	for (size_t input = 0; input < machine.size(); ++input)
	{
		machine[input].resize(verticesCount);
	}
}

void SetSizeToDet(DetMachine& machine, size_t inputCharactersCount, size_t verticesCount)
{
	machine.types.resize(verticesCount);
	machine.graph.resize(inputCharactersCount);
	for (size_t input = 0; input < machine.graph.size(); ++input)
	{
		machine.graph[input].resize(verticesCount);
	}
}

NondetMachine ReadNondetMachine(istream& strm, size_t inputCharactersCount, size_t verticesCount)
{
	NondetMachine machine;
	SetSizeToNondet(machine, inputCharactersCount, verticesCount);
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
	SetSizeToDet(detMachine, nondetMachine.size(), 1);
	detMachine.types[0] = VertexType::Starting;

	map<int, set<int>> nondetTransitions;
	map<set<int>, int> detTransitions;
	nondetTransitions.emplace(0, set<int>{ 0 });
	detTransitions.emplace(set<int>{ 0 }, 0);

	int finishingVertex = nondetMachine[0].size() - 1;
	for (size_t vertex = 0; vertex < detMachine.graph[0].size(); ++vertex)
	{
		for (size_t input = 0; input < detMachine.graph.size(); ++input)
		{
			set<int> allNondetTransitions;
			set<int> nondetTransition = nondetTransitions[vertex];
			for (int ver : nondetTransition)
			{
				allNondetTransitions = SetUnion(allNondetTransitions, nondetMachine[input][ver]);
			}

			if (allNondetTransitions.empty())
			{
				detMachine.graph[input][vertex] = -1;
				continue;
			}

			auto iter = detTransitions.find(allNondetTransitions);
			if (iter != detTransitions.end())
			{
				detMachine.graph[input][vertex] = iter->second;
			}
			else
			{
				SetSizeToDet(detMachine, detMachine.graph.size(), detMachine.graph[0].size() + 1);

				detMachine.graph[input][vertex] = detMachine.graph[0].size() - 1;

				nondetTransitions.emplace(detMachine.graph[0].size() - 1, allNondetTransitions);
				detTransitions.emplace(allNondetTransitions, detMachine.graph[0].size() - 1);
				
				auto iter = allNondetTransitions.find(finishingVertex);
				if (iter == allNondetTransitions.end())
				{
					detMachine.types[detMachine.graph[0].size() - 1] = VertexType::Normal;
				}
				else
				{
					detMachine.types[detMachine.graph[0].size() - 1] = VertexType::Finishing;
				}
			}
		}
	}

	return detMachine;
}

void PrintDetMachine(const DetMachine& machine, const string& fileName)
{
	ofstream fOut(fileName);
	
	for (size_t vertex = 0; vertex < machine.types.size(); ++vertex)
	{
		fOut << (char)machine.types[vertex] << ' ';
	}
	fOut << endl;
	for (size_t input = 0; input < machine.graph.size(); ++input)
	{
		for (size_t vertex = 0; vertex < machine.graph[0].size(); ++vertex)
		{
			if (machine.graph[input][vertex] == -1)
			{
				fOut << "- ";
			}
			else
			{
				fOut << 's' << machine.graph[input][vertex] << ' ';
			}
		}
		fOut << endl;
	}
}

void CreateGraph(const DetMachine& machine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t vertex = 0; vertex < machine.graph[0].size(); ++vertex)
	{
		string vertexLabel;
		if (machine.types[vertex] != VertexType::Normal)
		{
			vertexLabel = (char)machine.types[vertex];
		}
		vertexLabel += "s" + to_string(vertex);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t input = 0; input < machine.graph.size(); ++input)
	{
		for (size_t vertex = 0; vertex < machine.graph[0].size(); ++vertex)
		{
			if (machine.graph[input][vertex] != -1)
			{
				string edgeLabel = 'x' + to_string(input + 1);
				boost::add_edge(vertices[vertex], vertices[machine.graph[input][vertex]], { edgeLabel }, graph);
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