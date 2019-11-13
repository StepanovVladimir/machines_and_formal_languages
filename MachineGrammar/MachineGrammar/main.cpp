#include "pch.h"
#include <fstream>
#include <string>
#include <vector>
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

NondetMachine ReadRightGrammarRules(ifstream& fIn, map<char, size_t>& termIndexes, map<char, size_t>& nontermIndexes, size_t rulesCount)
{
	NondetMachine machine;
	SetSizeToNondet(machine, termIndexes.size(), nontermIndexes.size() + 1);
	for (size_t i = 0; i < rulesCount; ++i)
	{
		char nontermChar1, nontermChar2, termChar;
		fIn >> nontermChar1 >> termChar;
		fIn.get(nontermChar2);
		if (nontermChar2 == '\n' || !fIn)
		{
			machine[termIndexes[termChar]][nontermIndexes[nontermChar1]].insert(nontermIndexes.size());
		}
		else
		{
			machine[termIndexes[termChar]][nontermIndexes[nontermChar1]].insert(nontermIndexes[nontermChar2]);
		}
	}
	return machine;
}

NondetMachine ReadLeftGrammarRules(ifstream& fIn, map<char, size_t>& termIndexes, map<char, size_t>& nontermIndexes, size_t rulesCount)
{
	NondetMachine machine;
	SetSizeToNondet(machine, termIndexes.size(), nontermIndexes.size() + 1);
	for (size_t i = 0; i < rulesCount; ++i)
	{
		char nontermChar1, nontermChar2, termChar, ch;
		fIn >> nontermChar1 >> ch;
		fIn.get(termChar);
		if (termChar == '\n' || !fIn)
		{
			termChar = ch;
			machine[termIndexes[termChar]][0].insert(nontermIndexes[nontermChar1]);
		}
		else
		{
			nontermChar2 = ch;
			machine[termIndexes[termChar]][nontermIndexes[nontermChar2]].insert(nontermIndexes[nontermChar1]);
		}
	}
	return machine;
}

NondetMachine ReadGrammar(const string& fileName, vector<char>& termChars)
{
	ifstream fIn(fileName);

	string grammarType;
	size_t termCharsCount, nontermCharsCount, rulesCount;
	fIn >> grammarType >> termCharsCount >> nontermCharsCount >> rulesCount;

	map<char, size_t> termIndexes;
	for (size_t i = 0; i < termCharsCount; ++i)
	{
		char ch;
		fIn >> ch;
		termIndexes.emplace(ch, i);
		termChars.push_back(ch);
	}

	if (grammarType == "right")
	{
		map<char, size_t> nontermIndexes;
		for (size_t i = 0; i < nontermCharsCount; ++i)
		{
			char ch;
			fIn >> ch;
			nontermIndexes.emplace(ch, i);
		}
		return ReadRightGrammarRules(fIn, termIndexes, nontermIndexes, rulesCount);
	}
	else
	{
		map<char, size_t> nontermIndexes;
		for (size_t i = nontermCharsCount; i > 0; --i)
		{
			char ch;
			fIn >> ch;
			nontermIndexes.emplace(ch, i);
		}
		return ReadLeftGrammarRules(fIn, termIndexes, nontermIndexes, rulesCount);
	}
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

void PrintDetMachine(const DetMachine& machine, const vector<char>& termChars, const string& fileName)
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
		fOut << termChars[input] << ' ';
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

void CreateGraph(const DetMachine& machine, const vector<char>& termChars)
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
				edgeLabel = termChars[input];
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
	vector<char> termChars;
	NondetMachine nondetMachine = ReadGrammar(argv[1], termChars);
	DetMachine detMachine = DetermineMachine(nondetMachine);
	PrintDetMachine(detMachine, termChars, argv[2]);
	CreateGraph(detMachine, termChars);
}