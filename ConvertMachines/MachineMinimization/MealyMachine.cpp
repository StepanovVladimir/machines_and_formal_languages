#include "pch.h"
#include "MealyMachine.h"
#include "MinimizedMachine.h"
#include <set>

using namespace std;

void SetSizeToMealy(MealyMachine &mealyMachine, size_t inputCharactersCount, size_t positionsCount)
{
	mealyMachine.resize(inputCharactersCount);
	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		mealyMachine[i].resize(positionsCount);
	}
}

MealyMachine ReadMealyMachine(istream &strm, size_t inputCharactersCount, size_t positionsCount)
{
	MealyMachine mealyMachine;
	SetSizeToMealy(mealyMachine, inputCharactersCount, positionsCount);

	for (size_t i = 0; i < mealyMachine.size(); i++)
	{
		for (size_t j = 0; j < mealyMachine[0].size(); j++)
		{
			string transition;
			strm >> transition;

			size_t indexOfY = transition.find("y");
			string positionNumber;
			positionNumber.append(transition, 1, indexOfY - 1);

			mealyMachine[i][j].position = stoi(positionNumber);
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
			fOut << 'q' << mealyMachine[i][j].position << mealyMachine[i][j].output << ' ';
		}
		fOut << endl;
	}
}

/*MealyMachine RemoveUnreachablePositions(const MealyMachine &mealyMachine)
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
			unreachableVertices.erase(mealyMachine[i][j].position);
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
			minimizedMachine.graph[i][j] = minimizedMachine.equivalenceClasses[mealyMachine[i][j].position];
		}
	}

	return equivalenceClasses.size();
}

size_t FindEquivalenceClasses(MinimizedMachine &minimizedMachine,
	const MealyMachine &mealyMachine, size_t equivalenceClassesCount)
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
			for (size_t i = 0; i < mealyMachine.size(); i++)
			{
				for (size_t j = 0; j < mealyMachine[0].size(); j++)
				{
					int nextPosition = minimizedMachine.equivalenceClasses[mealyMachine[i][j].position];
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

MealyMachine FindEquivalentPositions(const MealyMachine &mealyMachine)
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
	//MealyMachine optimizedMealy = RemoveUnreachablePositions(mealyMachine);
	return FindEquivalentPositions(mealyMachine);
}