#include "pch.h"
#include "MealyMachine.h"
#include "MooreMachine.h"

using namespace std;

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
		mealyMachine = MinimizeMealyMachine(mealyMachine);
		PrintMealyMachine(mealyMachine, argv[2]);
	}
	else
	{
		MooreMachine mooreMachine = ReadMooreMachine(fIn, inputCharactersCount, positionsCount);
		mooreMachine = MinimizeMooreMachine(mooreMachine);
		PrintMooreMachine(mooreMachine, argv[2]);
	}
}