#include "pch.h"
#include "MealyMachine.h"
#include "MooreMachine.h"

using namespace std;

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
		mealyMachine = MinimizeMealyMachine(mealyMachine);
		PrintMealyMachine(mealyMachine, argv[2]);
		CreateMealyGraph(mealyMachine);
	}
	else
	{
		MooreMachine mooreMachine = ReadMooreMachine(fIn, inputCharactersCount, verticesCount);
		mooreMachine = MinimizeMooreMachine(mooreMachine);
		PrintMooreMachine(mooreMachine, argv[2]);
		CreateMooreGraph(mooreMachine);
	}
}