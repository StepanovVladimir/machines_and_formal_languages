#include "pch.h"
#include "MooreMachine.h"
#include "MinimizedMachine.h"
#include <set>

using namespace std;

void SetSizeToMoore(MooreMachine &mooreMachine, size_t inputCharactersCount, size_t positionsCount)
{
	mooreMachine.outputs.resize(positionsCount);
	mooreMachine.graph.resize(inputCharactersCount);
	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		mooreMachine.graph[i].resize(positionsCount);
	}
}

MooreMachine ReadMooreMachine(istream &strm, size_t inputCharactersCount, size_t positionsCount)
{
	MooreMachine mooreMachine;
	SetSizeToMoore(mooreMachine, inputCharactersCount, positionsCount);

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
			string positionNumber;
			positionNumber.append(transition, 1);
			mooreMachine.graph[i][j] = stoi(positionNumber);
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

/*MooreMachine RemoveUnreachablePositions(const MooreMachine &mooreMachine)
{
	set<int> unreachableVertices;
	for (size_t i = 1; i < mooreMachine.graph[0].size(); i++)
	{
		unreachableVertices.insert(i);
	}

	for (size_t i = 0; i < mooreMachine.graph.size(); i++)
	{
		for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
		{
			unreachableVertices.erase(mooreMachine.graph[i][j]);
		}
	}

	if (unreachableVertices.empty())
	{
		return mooreMachine;
	}

	MooreMachine optimizedMoore = mooreMachine;

	for (int unreachableVertice : unreachableVertices)
	{
		optimizedMoore.outputs.erase(optimizedMoore.outputs.begin() + unreachableVertice);
	}

	for (size_t i = 0; i < optimizedMoore.graph.size(); i++)
	{
		for (int unreachableVertice : unreachableVertices)
		{
			optimizedMoore.graph[i].erase(optimizedMoore.graph[i].begin() + unreachableVertice);
		}
	}

	return optimizedMoore;
}*/

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
		vector<int> transitions;
		vector<pair<int, vector<int>>> equivalenceClasses;
		for (size_t i = 0; i < minimizedMachine.graph[0].size(); i++)
		{
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

			transitions = {};
		}

		if (equivalenceClassesCount < equivalenceClasses.size())
		{
			equivalenceClassesCount = equivalenceClasses.size();
			for (size_t i = 0; i < mooreMachine.graph.size(); i++)
			{
				for (size_t j = 0; j < mooreMachine.graph[0].size(); j++)
				{
					int nextPosition = minimizedMachine.equivalenceClasses[mooreMachine.graph[i][j]];
					minimizedMachine.graph[i][j] = nextPosition;
				}
			}
		}
		else
		{
			return equivalenceClassesCount;
		}
	}
}

MooreMachine FindEquivalentPositions(const MooreMachine &mooreMachine)
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
	//MooreMachine optimizedMoore = RemoveUnreachablePositions(mooreMachine);
	return FindEquivalentPositions(mooreMachine);
}