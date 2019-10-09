#pragma once

#include <vector>
#include <string>
#include <fstream>

struct MooreMachine
{
	std::vector<std::vector<int>> graph;
	std::vector<std::string> outputs;
};

MooreMachine ReadMooreMachine(std::istream &strm, size_t inputCharactersCount, size_t verticesCount);
void PrintMooreMachine(const MooreMachine &mooreMachine, const std::string &fileName = "output.txt");
MooreMachine MinimizeMooreMachine(const MooreMachine &mooreMachine);
void CreateMooreGraph(const MooreMachine &mooreMachine);