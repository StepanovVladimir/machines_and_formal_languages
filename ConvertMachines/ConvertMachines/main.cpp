#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

struct TransitionMealy
{
	int vertex;
	string output;
};

using MealyMachine = vector<vector<TransitionMealy>>;

struct MooreMachine
{
	vector<vector<int>> graph;
	vector<string> outputs;
};

/*using Edge = pair<int, int>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
	boost::property<boost::vertex_color_t, boost::default_color_type>,
	boost::property<boost::edge_weight_t, string>>;*/

struct VertexProps
{
	string label;
};

struct EdgeProps
{
	string label;
};

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;

bool operator<(const TransitionMealy &transition1, const TransitionMealy &transition2)
{
	return transition1.vertex < transition2.vertex ||
		transition1.vertex == transition2.vertex && transition1.output < transition2.output;
}

void SetSizeToMealy(MealyMachine &mealyMachine, size_t inputCharactersCount, size_t verticesCount)
{
	mealyMachine.resize(inputCharactersCount);
	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		mealyMachine[i].resize(verticesCount);
	}
}

MealyMachine ReadMealyMachine(istream &strm, size_t inputCharactersCount, size_t verticesCount)
{
	MealyMachine mealyMachine;
	SetSizeToMealy(mealyMachine, inputCharactersCount, verticesCount);

	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			string transition;
			strm >> transition;

			size_t indexOfY = transition.find("y");
			string vertexNumber;
			vertexNumber.append(transition, 1, indexOfY - 1);

			mealyMachine[i][j].vertex = stoi(vertexNumber);
			mealyMachine[i][j].output.append(transition, indexOfY);
		}
	}

	return mealyMachine;
}

void PrintMealyMachine(const MealyMachine &mealyMachine, const string &fileName = "output.txt")
{
	ofstream fOut(fileName);
	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			fOut << 'q' << mealyMachine[i][j].vertex << mealyMachine[i][j].output << ' ';
		}
		fOut << endl;
	}
}

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

void PrintMooreMachine(const MooreMachine &mooreMachine, const string &fileName = "output.txt")
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

MooreMachine MealyToMoore(const MealyMachine &mealyMachine)
{
	set<TransitionMealy> transitions;
	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			transitions.insert(mealyMachine[i][j]);
		}
	}

	MooreMachine mooreMachine;
	SetSizeToMoore(mooreMachine, mealyMachine.size(), transitions.size());

	size_t i = 0;
	for (auto transition : transitions)
	{
		mooreMachine.outputs[i] = transition.output;
		i++;
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		size_t j = 0;
		for (auto transition : transitions)
		{
			auto iter = transitions.find(mealyMachine[i][transition.vertex]);
			mooreMachine.graph[i][j] = distance(transitions.begin(), iter);
			j++;
		}
	}

	return mooreMachine;
}

MealyMachine MooreToMealy(const MooreMachine &mooreMachine)
{
	MealyMachine mealyMachine;
	SetSizeToMealy(mealyMachine, mooreMachine.graph.size(), mooreMachine.graph[0].size());

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			int nextVertex = mooreMachine.graph[i][j];
			mealyMachine[i][j] = { nextVertex, mooreMachine.outputs[nextVertex] };
		}
	}

	return mealyMachine;
}

void CreateMealyGraph(const MealyMachine &mealyMachine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t i = 0; i < mealyMachine[0].size(); i++)
	{
		string vertexLabel = 'q' + to_string(i);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			string edgeLabel = 'x' + std::to_string(i + 1) + mealyMachine[i][j].output;
			boost::add_edge(vertices[j], vertices[mealyMachine[i][j].vertex], { edgeLabel }, graph);
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));
	ofstream ofs("graph.dot");
	boost::write_graphviz_dp(ofs, graph, dp);
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

int main(int argc, char *argv[])
{
	ifstream fIn(argv[1]);
	size_t inputCharactersCount;
	size_t outputCharactersCount;
	size_t verticesCount;
	string typeOfMachine;
	fIn >> inputCharactersCount >> outputCharactersCount >> verticesCount >> typeOfMachine;

	if (typeOfMachine == "mealy")
	{
		MealyMachine mealyMachine = ReadMealyMachine(fIn, inputCharactersCount, verticesCount);
		MooreMachine mooreMachine = MealyToMoore(mealyMachine);
		PrintMooreMachine(mooreMachine, argv[2]);
		CreateMooreGraph(mooreMachine);
	}
	else
	{
		MooreMachine mooreMachine = ReadMooreMachine(fIn, inputCharactersCount, verticesCount);
		MealyMachine mealyMachine = MooreToMealy(mooreMachine);
		PrintMealyMachine(mealyMachine, argv[2]);
		CreateMealyGraph(mealyMachine);
	}

	return 0;
}