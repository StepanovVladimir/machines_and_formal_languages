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
	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		mooreMachine.graph[input].resize(verticesCount);
	}
}

MooreMachine ReadMooreMachine(istream &strm, size_t inputCharactersCount, size_t verticesCount)
{
	MooreMachine mooreMachine;
	SetSizeToMoore(mooreMachine, inputCharactersCount, verticesCount);

	for (size_t vertex = 0; vertex < mooreMachine.outputs.size(); vertex++)
	{
		strm >> mooreMachine.outputs[vertex];
	}

	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			string transition;
			strm >> transition;
			string vertexNumber;
			vertexNumber.append(transition, 1);
			mooreMachine.graph[input][vertex] = stoi(vertexNumber);
		}
	}

	return mooreMachine;
}

void PrintMooreMachine(const MooreMachine &mooreMachine, const string &fileName)
{
	ofstream fOut(fileName);

	for (size_t vertex = 0; vertex < mooreMachine.outputs.size(); vertex++)
	{
		fOut << mooreMachine.outputs[vertex] << ' ';
	}
	fOut << endl;
	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			fOut << 'q' << mooreMachine.graph[input][vertex] << ' ';
		}
		fOut << endl;
	}
}

void CreateMooreGraph(const MooreMachine &mooreMachine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
	{
		string vertexLabel = 'q' + to_string(vertex) + mooreMachine.outputs[vertex];
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			string edgeLabel = 'x' + to_string(input + 1);
			boost::add_edge(vertices[vertex], vertices[mooreMachine.graph[input][vertex]], { edgeLabel }, graph);
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
	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		int nextVertex = mooreMachine.graph[input][thisVertex];
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
		for (size_t vertex = 1; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			auto iter = reachableVertices.find(vertex);
			if (iter == reachableVertices.end())
			{
				reduction++;
			}
			reductions.push_back(reduction);
		}

		MooreMachine optimizedMoore;
		SetSizeToMoore(optimizedMoore, mooreMachine.graph.size(), reachableVertices.size());

		size_t newVertex = 0;
		for (int reachableVertex : reachableVertices)
		{
			optimizedMoore.outputs[newVertex] = mooreMachine.outputs[reachableVertex];
			newVertex++;
		}

		for (size_t input = 0; input < mooreMachine.graph.size(); input++)
		{
			size_t newVertex = 0;
			for (int reachableVertex : reachableVertices)
			{
				reduction = reductions[mooreMachine.graph[input][reachableVertex]];
				optimizedMoore.graph[input][newVertex] = mooreMachine.graph[input][reachableVertex] - reduction;
				newVertex++;
			}
		}

		return optimizedMoore;
	}
}

size_t FindOutputEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MooreMachine &mooreMachine)
{
	vector<string> equivalenceClasses;
	for (size_t vertex = 0; vertex < mooreMachine.outputs.size(); vertex++)
	{
		string output = mooreMachine.outputs[vertex];
		bool found = false;
		for (size_t equivalenceClass = 0; equivalenceClass < equivalenceClasses.size(); equivalenceClass++)
		{
			if (equivalenceClasses[equivalenceClass] == output)
			{
				minimizedMachine.equivalenceClasses[vertex] = equivalenceClass;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back(output);
			minimizedMachine.equivalenceClasses[vertex] = equivalenceClasses.size() - 1;
		}
	}

	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			minimizedMachine.graph[input][vertex] = minimizedMachine.equivalenceClasses[mooreMachine.graph[input][vertex]];
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
			for (size_t input = 0; input < mooreMachine.graph.size(); input++)
			{
				for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
				{
					int equivalenceClass = minimizedMachine.equivalenceClasses[mooreMachine.graph[input][vertex]];
					minimizedMachine.graph[input][vertex] = equivalenceClass;
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

	for (size_t vertex = 0; vertex < mooreMachine.outputs.size(); vertex++)
	{
		minimizedMooreMachine.outputs[minimizedMachine.equivalenceClasses[vertex]] = mooreMachine.outputs[vertex];
	}

	for (size_t input = 0; input < mooreMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < mooreMachine.graph[0].size(); vertex++)
		{
			minimizedMooreMachine.graph[input][minimizedMachine.equivalenceClasses[vertex]] = minimizedMachine.graph[input][vertex];
		}
	}

	return minimizedMooreMachine;
}

MooreMachine MinimizeMooreMachine(const MooreMachine &mooreMachine)
{
	MooreMachine optimizedMoore = RemoveUnreachableVertices(mooreMachine);
	return FindEquivalentVertices(optimizedMoore);
}