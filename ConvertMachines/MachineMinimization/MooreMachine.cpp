#include "pch.h"
#include "MooreMachine.h"
#include "MinimizedMachine.h"
#include "Graph.h"
#include <set>

using namespace std;

void SetSizeToMoore(MooreMachine &mooreMachine, size_t inputCharactersCount, size_t verticesCount)
{
	mooreMachine.outputs.resize(verticesCount);
	mooreMachine.graph.resize(inputCharactersCount);
	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		mooreMachine.graph[i].resize(verticesCount);
	}
}

MooreMachine ReadMooreMachine(istream &strm, size_t inputCharactersCount, size_t verticesCount)
{
	MooreMachine mooreMachine;
	SetSizeToMoore(mooreMachine, inputCharactersCount, verticesCount);

	for (size_t i = 0; i < mooreMachine.outputs.size(); i++)
	{
		strm >> mooreMachine.outputs[i];
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			string transition;
			strm >> transition;
			string vertexNumber;
			vertexNumber.append(transition, 1);
			mooreMachine.graph[i][j] = stoi(vertexNumber);
		}
	}

	return mooreMachine;
}

void PrintMooreMachine(const MooreMachine &mooreMachine, const string &fileName)
{
	ofstream fOut(fileName);

	for (size_t i = 0; i < mooreMachine.outputs.size(); i++)
	{
		fOut << mooreMachine.outputs[i] << ' ';
	}
	fOut << endl;
	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			fOut << 'q' << mooreMachine.graph[i][j] << ' ';
		}
		fOut << endl;
	}
}

void CreateMooreGraph(const MooreMachine &mooreMachine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t i = 0; i < mooreMachine.graph[0].size(); ++i)
	{
		string vertexLabel = 'q' + to_string(i) + mooreMachine.outputs[i];
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); ++i)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); ++j)
		{
			string edgeLabel = 'x' + to_string(i + 1);
			boost::add_edge(vertices[j], vertices[mooreMachine.graph[i][j]], { edgeLabel }, graph);
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));
	ofstream ofs("graph.dot");
	boost::write_graphviz_dp(ofs, graph, dp);
}

void RecursiveTraversal(const MooreMachine &mooreMachine, set<int> &reachableVertices, int thisVertex)
{
	reachableVertices.insert(thisVertex);
	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		int nextVertex = mooreMachine.graph[i][thisVertex];
		auto iter = reachableVertices.find(nextVertex);
		if (iter == reachableVertices.end())
		{
			RecursiveTraversal(mooreMachine, reachableVertices, nextVertex);
		}
	}
}

MooreMachine RemoveUnreachableVertices(const MooreMachine &mooreMachine)
{
	set<int> reachableVertices;

	RecursiveTraversal(mooreMachine, reachableVertices, 0);

	if (reachableVertices.size() == mooreMachine.graph[0].size())
	{
		return mooreMachine;
	}
	else
	{
		vector<int> reductions{ 0 };
		int reduction = 0;
		for (size_t i = 1; i < mooreMachine.graph[0].size(); i++)
		{
			auto iter = reachableVertices.find(i);
			if (iter == reachableVertices.end())
			{
				reduction++;
			}
			reductions.push_back(reduction);
		}

		MooreMachine optimizedMoore;
		SetSizeToMoore(optimizedMoore, mooreMachine.graph.size(), reachableVertices.size());

		size_t i = 0;
		for (int vertex : reachableVertices)
		{
			optimizedMoore.outputs[i] = mooreMachine.outputs[vertex];
			i++;
		}

		for (size_t i = 0; i < mooreMachine.graph.size(); i++)
		{
			size_t j = 0;
			for (int vertex : reachableVertices)
			{
				reduction = reductions[mooreMachine.graph[i][vertex]];
				optimizedMoore.graph[i][j] = mooreMachine.graph[i][vertex] - reduction;
				j++;
			}
		}

		return optimizedMoore;
	}
}

size_t FindOutputEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MooreMachine &mooreMachine)
{
	vector<string> equivalenceClasses;
	for (size_t i = 0; i < mooreMachine.outputs.size(); i++)
	{
		string output = mooreMachine.outputs[i];
		bool found = false;
		for (size_t j = 0; j < equivalenceClasses.size(); j++)
		{
			if (equivalenceClasses[j] == output)
			{
				minimizedMachine.equivalenceClasses[i] = j;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back(output);
			minimizedMachine.equivalenceClasses[i] = equivalenceClasses.size() - 1;
		}
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			minimizedMachine.graph[i][j] = minimizedMachine.equivalenceClasses[mooreMachine.graph[i][j]];
		}
	}

	return equivalenceClasses.size();
}

size_t FindEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MooreMachine &mooreMachine, size_t equivalenceClassesCount)
{
	while (true)
	{
		size_t newEquivalenceClassesCount = DoMinimizationStep(minimizedMachine);

		if (equivalenceClassesCount < newEquivalenceClassesCount)
		{
			equivalenceClassesCount = newEquivalenceClassesCount;
			for (size_t i = 0; i < mooreMachine.graph.size(); i++)
			{
				for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
				{
					int nextVertex = minimizedMachine.equivalenceClasses[mooreMachine.graph[i][j]];
					minimizedMachine.graph[i][j] = nextVertex;
				}
			}
		}
		else
		{
			return equivalenceClassesCount;
		}
	}
}

MooreMachine FindEquivalentVertices(const MooreMachine &mooreMachine)
{
	MinimizedMachine minimizedMachine;
	SetSizeToMinimized(minimizedMachine, mooreMachine.graph.size(), mooreMachine.graph[0].size());

	size_t equivalenceClassesCount = FindOutputEquivalenceClasses(minimizedMachine, mooreMachine);
	equivalenceClassesCount = FindEquivalenceClasses(minimizedMachine, mooreMachine, equivalenceClassesCount);

	MooreMachine minimizedMooreMachine;
	SetSizeToMoore(minimizedMooreMachine, mooreMachine.graph.size(), equivalenceClassesCount);

	for (size_t i = 0; i < mooreMachine.outputs.size(); i++)
	{
		minimizedMooreMachine.outputs[minimizedMachine.equivalenceClasses[i]] = mooreMachine.outputs[i];
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			minimizedMooreMachine.graph[i][minimizedMachine.equivalenceClasses[j]] = minimizedMachine.graph[i][j];
		}
	}

	return minimizedMooreMachine;
}

MooreMachine MinimizeMooreMachine(const MooreMachine &mooreMachine)
{
	MooreMachine optimizedMoore = RemoveUnreachableVertices(mooreMachine);
	return FindEquivalentVertices(optimizedMoore);
}