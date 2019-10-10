#include "pch.h"
#include "MinimizedMachine.h"

using namespace std;

void SetSizeToMinimized(MinimizedMachine &minimizedMachine, size_t inputCharactersCount, size_t verticesCount)
{
	minimizedMachine.equivalenceClasses.resize(verticesCount);
	minimizedMachine.graph.resize(inputCharactersCount);
	for (size_t input = 0; input < minimizedMachine.graph.size(); input++)
	{
		minimizedMachine.graph[input].resize(verticesCount);
	}
}

size_t DoMinimizationStep(MinimizedMachine &minimizedMachine)
{
	vector<pair<int, vector<int>>> equivalenceClasses;
	for (size_t vertex = 0; vertex < minimizedMachine.graph[0].size(); vertex++)
	{
		vector<int> transitions;
		for (size_t input = 0; input < minimizedMachine.graph.size(); input++)
		{
			transitions.push_back(minimizedMachine.graph[input][vertex]);
		}

		bool found = false;
		for (size_t equivalenceClass = 0; equivalenceClass < equivalenceClasses.size(); equivalenceClass++)
		{
			if (equivalenceClasses[equivalenceClass] == pair<int, vector<int>>{ minimizedMachine.equivalenceClasses[vertex], transitions })
			{
				minimizedMachine.equivalenceClasses[vertex] = equivalenceClass;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back({ minimizedMachine.equivalenceClasses[vertex], transitions });
			minimizedMachine.equivalenceClasses[vertex] = equivalenceClasses.size() - 1;
		}
	}
	return equivalenceClasses.size();
}