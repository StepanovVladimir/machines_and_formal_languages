#include "pch.h"
#include "MinimizedMachine.h"

void SetSizeToMinimized(MinimizedMachine &minimizedMachine, size_t inputCharactersCount, size_t positionsCount)
{
	minimizedMachine.equivalenceClasses.resize(positionsCount);
	minimizedMachine.graph.resize(inputCharactersCount);
	for (size_t i = 0; i < minimizedMachine.graph.size(); i++)
	{
		minimizedMachine.graph[i].resize(positionsCount);
	}
}