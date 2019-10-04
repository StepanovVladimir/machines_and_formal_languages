#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

using namespace std;

struct TransitionMealy
{
	int position;
	string output;
};

using MealyMachine = vector<vector<TransitionMealy>>;

struct MooreMachine
{
	vector<vector<int>> graph;
	vector<string> outputs;
};

bool operator<(const TransitionMealy &transition1, const TransitionMealy &transition2)
{
	return transition1.position < transition2.position ||
		transition1.position == transition2.position && transition1.output < transition2.output;
}

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

void PrintMealyMachine(const MealyMachine &mealyMachine, const string &fileName = "output.txt")
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
			auto iter = transitions.find(mealyMachine[i][transition.position]);
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
			int nextPosition = mooreMachine.graph[i][j];
			mealyMachine[i][j] = { nextPosition, mooreMachine.outputs[nextPosition] };
		}
	}

	return mealyMachine;
}

int main(int argc, char *argv[])
{
	ifstream fIn(argv[1]);
	size_t inputCharactersCount;
	size_t outputCharactersCount;
	size_t positionsCount;
	string typeOfMachine;
	fIn >> inputCharactersCount >> outputCharactersCount >> positionsCount >> typeOfMachine;

	if (typeOfMachine == "mealy")
	{
		MealyMachine mealyMachine = ReadMealyMachine(fIn, inputCharactersCount, positionsCount);
		MooreMachine mooreMachine = MealyToMoore(mealyMachine);
		PrintMooreMachine(mooreMachine, argv[2]);
	}
	else
	{
		MooreMachine mooreMachine = ReadMooreMachine(fIn, inputCharactersCount, positionsCount);
		MealyMachine mealyMachine = MooreToMealy(mooreMachine);
		PrintMealyMachine(mealyMachine, argv[2]);
	}

	return 0;
}