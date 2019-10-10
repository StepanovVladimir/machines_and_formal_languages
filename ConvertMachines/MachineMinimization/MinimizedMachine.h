#pragma once

#include <vector>

using EquivalenceClass = int;

struct MinimizedMachine
{
	std::vector<EquivalenceClass> equivalenceClasses;
	std::vector<std::vector<EquivalenceClass>> graph;
};

void SetSizeToMinimized(MinimizedMachine &minimizedMachine, size_t inputCharactersCount, size_t verticesCount);
size_t DoMinimizationStep(MinimizedMachine &minimizedMachine);