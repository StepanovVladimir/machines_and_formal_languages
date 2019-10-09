#pragma once

#include <vector>

struct MinimizedMachine
{
	std::vector<std::vector<int>> graph;
	std::vector<int> equivalenceClasses;
};

void SetSizeToMinimized(MinimizedMachine &minimizedMachine, size_t inputCharactersCount, size_t verticesCount);
size_t DoMinimizationStep(MinimizedMachine &minimizedMachine);