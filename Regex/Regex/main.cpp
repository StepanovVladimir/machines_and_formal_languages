#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <queue>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

using NondetMachine = vector<vector<set<int>>>;

enum struct VertexType
{
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

void ReadFormula(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2);

void ReadFormulaOr(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2)
{
	int beginIndex = index2;
	set<int> endIndexes;

	char ch;
	while (inStrm >> ch)
	{
		if (ch == '(')
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes.size()][beginIndex].insert(machine[0].size() - 1);

			++index2;
			index1 = index2 - 1;
			
			int count = 1;
			string subFormula;
			while (count > 0)
			{
				inStrm >> ch;
				if (ch == '(')
				{
					++count;
				}
				else if (ch == ')')
				{
					--count;
				}

				if (count > 0)
				{
					subFormula += ch;
				}
			}
			istringstream strm(subFormula);
			ReadFormula(strm, machine, indexes, index1, index2);
			++index1;

			endIndexes.insert(index2);
		}
		else if (ch == '*')
		{
			machine[indexes.size()][index1].insert(index2);
			machine[indexes.size()][index2].insert(index1);
		}
		else if (ch == '+')
		{
			machine[indexes.size()][index2].insert(index1);
		}
		else
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes.size()][beginIndex].insert(machine[0].size() - 1);
			++index2;

			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes[ch]][index2].insert(machine[0].size() - 1);
			++index2;
			index1 = index2 - 1;

			endIndexes.insert(index2);
		}
	}

	SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
	for (int endIndex : endIndexes)
	{
		machine[indexes.size()][endIndex].insert(machine[0].size() - 1);
	}
	++index2;
}

void ReadFormula(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2)
{
	char ch;
	while (inStrm >> ch)
	{
		if (ch == '|')
		{
			ReadFormulaOr(inStrm, machine, indexes, index1, index2);
		}
		else if (ch == '(')
		{
			index1 = index2 - 1;
			int count = 1;
			string subFormula;
			while (count > 0)
			{
				inStrm >> ch;
				if (ch == '(')
				{
					++count;
				}
				else if (ch == ')')
				{
					--count;
				}

				if (count > 0)
				{
					subFormula += ch;
				}
			}
			istringstream strm(subFormula);
			ReadFormula(strm, machine, indexes, index1, index2);
			++index1;
		}
		else if (ch == '*')
		{
			machine[indexes.size()][index1].insert(index2);
			machine[indexes.size()][index2].insert(index1);
		}
		else if (ch == '+')
		{
			machine[indexes.size()][index2].insert(index1);
		}
		else
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes[ch]][index2].insert(machine[0].size() - 1);
			++index2;
			index1 = index2 - 1;
		}
	}
}

NondetMachine ReadRegex(const string& fileName, vector<char>& chars)
{
	ifstream fIn(fileName);
	map<char, size_t> indexes;

	string charsStr;
	getline(fIn, charsStr);
	istringstream inStrm(charsStr);
	size_t i = 0;
	char ch;
	while (inStrm >> ch)
	{
		indexes.emplace(ch, i);
		chars.push_back(ch);
		++i;
	}

	NondetMachine machine;
	SetSizeToNondet(machine, indexes.size() + 1, 1);
	int index2 = 0;

	ReadFormula(fIn, machine, indexes, -1, index2);

	return machine;
}

set<int> SetUnion(const set<int>& set1, const set<int>& set2)
{
	set<int> newSet;
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(newSet, newSet.begin()));
	return newSet;
}

DetMachine DetermineMachine(const NondetMachine& nondetMachine)
{
	DetMachine detMachine;
	SetSizeToDet(detMachine, nondetMachine.size() - 1, 1);

	map<int, set<int>> nondetTransitions;
	map<set<int>, int> detTransitions;

	queue<int> emptyTransitions;
	emptyTransitions.push(0);
	set<int> allEmptyTransitions{ 0 };
	while (!emptyTransitions.empty())
	{
		int vertex = emptyTransitions.front();
		emptyTransitions.pop();
		for (auto ver : nondetMachine[nondetMachine.size() - 1][vertex])
		{
			auto iter = allEmptyTransitions.find(ver);
			if (iter == allEmptyTransitions.end())
			{
				emptyTransitions.push(ver);
				allEmptyTransitions.insert(ver);
			}
		}
	}

	nondetTransitions.emplace(0, allEmptyTransitions);
	detTransitions.emplace(allEmptyTransitions, 0);
	int finishingVertex = nondetMachine[0].size() - 1;

	auto iter = allEmptyTransitions.find(finishingVertex);
	if (iter == allEmptyTransitions.end())
	{
		detMachine.types[0] = VertexType::Normal;
	}
	else
	{
		detMachine.types[0] = VertexType::Finishing;
	}

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

			queue<int> emptyTransitions;
			for (int ver : allNondetTransitions)
			{
				emptyTransitions.push(ver);
			}
			while (!emptyTransitions.empty())
			{
				int vertex = emptyTransitions.front();
				emptyTransitions.pop();
				for (auto ver : nondetMachine[nondetMachine.size() - 1][vertex])
				{
					auto iter = allNondetTransitions.find(ver);
					if (iter == allNondetTransitions.end())
					{
						emptyTransitions.push(ver);
						allNondetTransitions.insert(ver);
					}
				}
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

void PrintDetMachine(const DetMachine& machine, const vector<char>& chars, const string& fileName)
{
	ofstream fOut(fileName);

	fOut << "  ";
	for (size_t vertex = 0; vertex < machine.types.size(); ++vertex)
	{
		fOut << (char)machine.types[vertex] << ' ';
	}
	fOut << endl;
	for (size_t input = 0; input < machine.graph.size(); ++input)
	{
		fOut << chars[input] << ' ';
		for (size_t vertex = 0; vertex < machine.graph[0].size(); ++vertex)
		{
			if (machine.graph[input][vertex] == -1)
			{
				fOut << "- ";
			}
			else
			{
				fOut << machine.graph[input][vertex] << ' ';
			}
		}
		fOut << endl;
	}
}

void CreateGraph(const DetMachine& machine, const vector<char>& chars)
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
		vertexLabel += to_string(vertex);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t input = 0; input < machine.graph.size(); ++input)
	{
		for (size_t vertex = 0; vertex < machine.graph[0].size(); ++vertex)
		{
			if (machine.graph[input][vertex] != -1)
			{
				string edgeLabel;
				edgeLabel = chars[input];
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
	vector<char> chars;
	NondetMachine nondetMachine = ReadRegex(argv[1], chars);
	DetMachine detMachine = DetermineMachine(nondetMachine);
	PrintDetMachine(detMachine, chars, argv[2]);
	CreateGraph(detMachine, chars);
}