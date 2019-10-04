#pragma once

#include <vector>
#include <string>
#include <fstream>

struct TransitionMealy
{
	int position;
	std::string output;
};

using MealyMachine = std::vector<std::vector<TransitionMealy>>;

MealyMachine ReadMealyMachine(std::istream &strm, size_t inputCharactersCount, size_t positionsCount);
void PrintMealyMachine(const MealyMachine &mealyMachine, const std::string &fileName = "output.txt");
MealyMachine MinimizeMealyMachine(const MealyMachine &mealyMachine);