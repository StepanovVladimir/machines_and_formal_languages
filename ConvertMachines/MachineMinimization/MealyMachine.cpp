#include "pch.h"
#include "MealyMachine.h"
#include "MinimizedMachine.h"
#include "Graph.h"
#include <set>

using namespace std;

void SetSizeToMealy(MealyMachine &mealyMachine, size_t inputCharactersCount, size_t verticesCount)
{
	mealyMachine.resize(inputCharactersCount);
	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		mealyMachine[input].resize(verticesCount);
	}
}

MealyMachine ReadMealyMachine(istream &strm, size_t inputCharactersCount, size_t verticesCount)
{
	MealyMachine mealyMachine;
	SetSizeToMealy(mealyMachine, inputCharactersCount, verticesCount);

	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
		{
			string transition;
			strm >> transition;

			size_t indexOfY = transition.find("y");
			string vertexNumber;
			vertexNumber.append(transition, 1, indexOfY - 1);

			mealyMachine[input][vertex].vertex = stoi(vertexNumber);
			mealyMachine[input][vertex].output.append(transition, indexOfY);
		}
	}

	return mealyMachine;
}

void PrintMealyMachine(const MealyMachine &mealyMachine, const string &fileName)
{
	ofstream fOut(fileName);
	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
		{
			fOut << 'q' << mealyMachine[input][vertex].vertex << mealyMachine[input][vertex].output << ' ';
		}
		fOut << endl;
	}
}

void CreateMealyGraph(const MealyMachine &mealyMachine)
{
	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
	{
		string vertexLabel = 'q' + to_string(vertex);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
		{
			string edgeLabel = 'x' + to_string(input + 1) + mealyMachine[input][vertex].output;
			boost::add_edge(vertices[vertex], vertices[mealyMachine[input][vertex].vertex], { edgeLabel }, graph);
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));
	ofstream ofs("graph.dot");
	boost::write_graphviz_dp(ofs, graph, dp);
}

void RecursiveTraversal(const MealyMachine &mealyMachine, set<int> &reachableVertices, int thisVertex)
{
	reachableVertices.insert(thisVertex);
	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		int nextVertex = mealyMachine[input][thisVertex].vertex;
		auto iter = reachableVertices.find(nextVertex);
		if (iter == reachableVertices.end())
		{
			RecursiveTraversal(mealyMachine, reachableVertices, nextVertex);
		}
	}
}

MealyMachine RemoveUnreachableVertices(const MealyMachine &mealyMachine)
{
	set<int> reachableVertices;

	RecursiveTraversal(mealyMachine, reachableVertices, 0);

	if (reachableVertices.size() == mealyMachine[0].size())
	{
		return mealyMachine;
	}
	else
	{
		vector<int> reductions{ 0 };
		int reduction = 0;
		for (size_t vertex = 1; vertex < mealyMachine[0].size(); vertex++)
		{
			auto iter = reachableVertices.find(vertex);
			if (iter == reachableVertices.end())
			{
				reduction++;
			}
			reductions.push_back(reduction);
		}

		MealyMachine optimizedMealy;
		SetSizeToMealy(optimizedMealy, mealyMachine.size(), reachableVertices.size());

		for (size_t input = 0; input < mealyMachine.size(); input++)
		{
			size_t newVertex = 0;
			for (int reachableVertex : reachableVertices)
			{
				reduction = reductions[mealyMachine[input][reachableVertex].vertex];
				optimizedMealy[input][newVertex].vertex = mealyMachine[input][reachableVertex].vertex - reduction;
				optimizedMealy[input][newVertex].output = mealyMachine[input][reachableVertex].output;
				newVertex++;
			}
		}

		return optimizedMealy;
	}
}

size_t FindOutputEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MealyMachine &mealyMachine)
{
	vector<string> equivalenceClasses;
	for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
	{
		string outputs;
		for (size_t input = 0; input < mealyMachine.size(); input++)
		{
			outputs += mealyMachine[input][vertex].output;
		}

		bool found = false;
		for (size_t equivalenceClass = 0; equivalenceClass < equivalenceClasses.size(); equivalenceClass++)
		{
			if (equivalenceClasses[equivalenceClass] == outputs)
			{
				minimizedMachine.equivalenceClasses[vertex] = equivalenceClass;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back(outputs);
			minimizedMachine.equivalenceClasses[vertex] = equivalenceClasses.size() - 1;
		}
	}

	for (size_t input = 0; input < mealyMachine.size(); input++)
	{
		for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
		{
			minimizedMachine.graph[input][vertex] = minimizedMachine.equivalenceClasses[mealyMachine[input][vertex].vertex];
		}
	}

	return equivalenceClasses.size();
}

size_t FindEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MealyMachine &mealyMachine, size_t equivalenceClassesCount)
{
	while (true)
	{
		size_t newEquivalenceClassesCount = DoMinimizationStep(minimizedMachine);

		if (equivalenceClassesCount < newEquivalenceClassesCount)
		{
			equivalenceClassesCount = newEquivalenceClassesCount;
			for (size_t input = 0; input < mealyMachine.size(); input++)
			{
				for (size_t vertex = 0; vertex < mealyMachine[0].size(); vertex++)
				{
					int equivalenceClass = minimizedMachine.equivalenceClasses[mealyMachine[input][vertex].vertex];
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

MealyMachine FindEquivalentVertices(const MealyMachine &mealyMachine)
{
	MinimizedMachine minimizedMachine;
	SetSizeToMinimized(minimizedMachine, mealyMachine.size(), mealyMachine[0].size());

	size_t equivalenceClassesCount = FindOutputEquivalenceClasses(minimizedMachine, mealyMachine);
	equivalenceClassesCount = FindEquivalenceClasses(minimizedMachine, mealyMachine, equivalenceClassesCount);

	MealyMachine minimizedMealyMachine;
	SetSizeToMealy(minimizedMealyMachine, mealyMachine.size(), equivalenceClassesCount);
	for (size_t input = 0; input < minimizedMachine.graph.size(); input++)
	{
		for (size_t vertex = 0; vertex < minimizedMachine.graph[0].size(); vertex++)
		{
			minimizedMealyMachine[input][minimizedMachine.equivalenceClasses[vertex]] = { minimizedMachine.graph[input][vertex], mealyMachine[input][vertex].output };
		}
	}

	return minimizedMealyMachine;
}

MealyMachine MinimizeMealyMachine(const MealyMachine &mealyMachine)
{
	MealyMachine optimizedMealy = RemoveUnreachableVertices(mealyMachine);
	return FindEquivalentVertices(optimizedMealy);
}