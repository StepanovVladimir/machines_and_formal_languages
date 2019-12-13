#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>

using namespace std;

using NondetMachine = vector<vector<set<int>>>;

enum struct VertexType
{
	Normal = 'N',
	Finishing = 'F'
};

struct DetMachine
{
	vector<VertexType> types;
	vector<vector<int>> graph;
};

void SetSizeToNondet(NondetMachine& machine, size_t inputCharactersCount, size_t verticesCount)
{
	machine.resize(inputCharactersCount);
	for (size_t input = 0; input < machine.size(); ++input)
	{
		machine[input].resize(verticesCount);
	}
}

void ReadFormula(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2);

void ReadFormulaOr(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2)
{
	int beginIndex = index2;
	set<int> endIndexes;

	char ch;
	while (inStrm >> ch)
	{
		if (ch == '(')
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes.size()][beginIndex].insert(machine[0].size() - 1);

			++index2;
			index1 = index2 - 1;
			
			int count = 1;
			string subFormula;
			while (count > 0)
			{
				inStrm >> ch;
				if (ch == '(')
				{
					++count;
				}
				else if (ch == ')')
				{
					--count;
				}

				if (count > 0)
				{
					subFormula += ch;
				}
			}
			istringstream strm(subFormula);
			ReadFormula(strm, machine, indexes, index1, index2);
			++index1;

			endIndexes.insert(index2);
		}
		else if (ch == '*')
		{
			machine[indexes.size()][index1].insert(index2);
			machine[indexes.size()][index2].insert(index1);
		}
		else if (ch == '+')
		{
			machine[indexes.size()][index2].insert(index1);
		}
		else
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes.size()][beginIndex].insert(machine[0].size() - 1);
			++index2;

			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes[ch]][index2].insert(machine[0].size() - 1);
			++index2;
			index1 = index2 - 1;

			endIndexes.insert(index2);
		}
	}

	SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
	for (int endIndex : endIndexes)
	{
		machine[indexes.size()][endIndex].insert(machine[0].size() - 1);
	}
	++index2;
}

void ReadFormula(istream& inStrm, NondetMachine& machine, map<char, size_t>& indexes, int index1, int& index2)
{
	char ch;
	while (inStrm >> ch)
	{
		if (ch == '|')
		{
			ReadFormulaOr(inStrm, machine, indexes, index1, index2);
		}
		else if (ch == '(')
		{
			index1 = index2 - 1;
			int count = 1;
			string subFormula;
			while (count > 0)
			{
				inStrm >> ch;
				if (ch == '(')
				{
					++count;
				}
				else if (ch == ')')
				{
					--count;
				}

				if (count > 0)
				{
					subFormula += ch;
				}
			}
			istringstream strm(subFormula);
			ReadFormula(strm, machine, indexes, index1, index2);
			++index1;
		}
		else if (ch == '*')
		{
			machine[indexes.size()][index1].insert(index2);
			machine[indexes.size()][index2].insert(index1);
		}
		else if (ch == '+')
		{
			machine[indexes.size()][index2].insert(index1);
		}
		else
		{
			SetSizeToNondet(machine, indexes.size() + 1, machine[0].size() + 1);
			machine[indexes[ch]][index2].insert(machine[0].size() - 1);
			++index2;
			index1 = index2 - 1;
		}
	}
}

NondetMachine ReadRegex(const string& fileName, vector<char>& chars)
{
	ifstream fIn(fileName);
	map<char, size_t> indexes;

	string charsStr;
	getline(fIn, charsStr);
	istringstream inStrm(charsStr);
	size_t i = 0;
	char ch;
	while (inStrm >> ch)
	{
		indexes.emplace(ch, i);
		chars.push_back(ch);
		++i;
	}

	NondetMachine machine;
	SetSizeToNondet(machine, indexes.size() + 1, 1);
	int index2 = 0;

	ReadFormula(fIn, machine, indexes, -1, index2);

	return machine;
}

int main(int argc, char* argv[])
{
	vector<char> chars;
	NondetMachine nondetMachine = ReadRegex(argv[1], chars);
	//DetMachine detMachine = DetermineMachine(nondetMachine);
	//PrintDetMachine(detMachine, termChars, argv[2]);
	//CreateGraph(detMachine, termChars);
}