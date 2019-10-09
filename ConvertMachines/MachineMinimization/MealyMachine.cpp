#include "pch.h"
#include "MealyMachine.h"
#include "MinimizedMachine.h"
#include "Graph.h"
#include <set>

using namespace std;

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

void PrintMealyMachine(const MealyMachine &mealyMachine, const string &fileName)
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

/*MealyMachine RemoveUnreachableVertices(const MealyMachine &mealyMachine)
{
	set<int> unreachableVertices;
	for (size_t i = 1; i < mealyMachine[0].size(); i++)
	{
		unreachableVertices.insert(i);
	}

	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			unreachableVertices.erase(mealyMachine[i][j].vertex);
		}
	}

	if (unreachableVertices.empty())
	{
		return mealyMachine;
	}

	MealyMachine optimizedMealy = mealyMachine;
	for (size_t i = 0; i < optimizedMealy.size(); i++)
	{
		for (int unreachableVertice : unreachableVertices)
		{
			optimizedMealy[i].erase(optimizedMealy[i].begin() + unreachableVertice);
		}
	}

	return optimizedMealy;
}*/

size_t FindOutputEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MealyMachine &mealyMachine)
{
	string outputs;
	vector<string> equivalenceClasses;
	for (size_t i = 0; i < mealyMachine[0].size(); i++)
	{
		for (size_t j = 0; j < mealyMachine.size(); j++)
		{
			outputs += mealyMachine[j][i].output;
		}

		bool found = false;
		for (size_t j = 0; j < equivalenceClasses.size(); j++)
		{
			if (equivalenceClasses[j] == outputs)
			{
				minimizedMachine.equivalenceClasses[i] = j;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back(outputs);
			minimizedMachine.equivalenceClasses[i] = equivalenceClasses.size() - 1;
		}

		outputs = "";
	}

	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			minimizedMachine.graph[i][j] = minimizedMachine.equivalenceClasses[mealyMachine[i][j].vertex];
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
			for (size_t i = 0; i < mealyMachine.size(); i++)
			{
				for (size_t j = 0; j < mealyMachine[0].size(); j++)
				{
					int nextVertex = minimizedMachine.equivalenceClasses[mealyMachine[i][j].vertex];
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

MealyMachine FindEquivalentVertices(const MealyMachine &mealyMachine)
{
	MinimizedMachine minimizedMachine;
	SetSizeToMinimized(minimizedMachine, mealyMachine.size(), mealyMachine[0].size());

	size_t equivalenceClassesCount = FindOutputEquivalenceClasses(minimizedMachine, mealyMachine);
	equivalenceClassesCount = FindEquivalenceClasses(minimizedMachine, mealyMachine, equivalenceClassesCount);

	MealyMachine minimizedMealyMachine;
	SetSizeToMealy(minimizedMealyMachine, mealyMachine.size(), equivalenceClassesCount);
	for (size_t i = 0; i < minimizedMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < minimizedMachine.graph[0].size(); j++)
		{
			minimizedMealyMachine[i][minimizedMachine.equivalenceClasses[j]] = { minimizedMachine.graph[i][j], mealyMachine[i][j].output };
		}
	}

	return minimizedMealyMachine;
}

MealyMachine MinimizeMealyMachine(const MealyMachine &mealyMachine)
{
	//MealyMachine optimizedMealy = RemoveUnreachableVertices(mealyMachine);
	return FindEquivalentVertices(mealyMachine);
}