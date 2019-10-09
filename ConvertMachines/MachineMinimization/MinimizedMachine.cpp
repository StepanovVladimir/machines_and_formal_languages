#include "pch.h"
#include "MinimizedMachine.h"

using namespace std;

void SetSizeToMinimized(MinimizedMachine &minimizedMachine, size_t inputCharactersCount, size_t verticesCount)
{
	minimizedMachine.equivalenceClasses.resize(verticesCount);
	minimizedMachine.graph.resize(inputCharactersCount);
	for (size_t i = 0; i < minimizedMachine.graph.size(); i++)
	{
		minimizedMachine.graph[i].resize(verticesCount);
	}
}

size_t DoMinimizationStep(MinimizedMachine &minimizedMachine)
{
	vector<pair<int, vector<int>>> equivalenceClasses;
	for (size_t i = 0; i < minimizedMachine.graph[0].size(); i++)
	{
		vector<int> transitions;
		for (size_t j = 0; j < minimizedMachine.graph.size(); j++)
		{
			transitions.push_back(minimizedMachine.graph[j][i]);
		}

		bool found = false;
		for (size_t j = 0; j < equivalenceClasses.size(); j++)
		{
			if (equivalenceClasses[j] == pair<int, vector<int>>{ minimizedMachine.equivalenceClasses[i], transitions })
			{
				minimizedMachine.equivalenceClasses[i] = j;
				found = true;
				break;
			}
		}

		if (!found)
		{
			equivalenceClasses.push_back({ minimizedMachine.equivalenceClasses[i], transitions });
			minimizedMachine.equivalenceClasses[i] = equivalenceClasses.size() - 1;
		}
	}
	return equivalenceClasses.size();
}