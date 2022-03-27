#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <Windows.h>

#include "GridSQL.h"
#include "StateClass.h"

template <class MYCLASS>
int Run(MYCLASS& ClassRef) {
	int chosenRand;
	int chosenMove;
	unsigned int startingPosition;
	float accumalate;
	std::ofstream outFile;
	std::vector<std::tuple<int, int>> movesTaken;

	std::vector<std::tuple<int, float>> possibleMoves;

	startingPosition = ClassRef.Get_Current_State();

	if (ClassRef.Is_Finished()) {
		ClassRef.Record(ClassRef.Get_Current_State(), 5, 0);
		return 0;
	}
	std::cout << "Started Running" << std::endl;
	while (!ClassRef.Is_Finished()) {
		accumalate = 0;
		chosenRand = rand() % 100;
		possibleMoves = ClassRef.Get_Possible_Moves();

		for (unsigned int i = 0; i < possibleMoves.size(); ++i) {
			accumalate += std::get<1>(possibleMoves.at(i));
			if (accumalate >= chosenRand) {
				chosenMove = std::get<0>(possibleMoves.at(i));
				movesTaken.push_back(std::tuple<int, int>(ClassRef.Get_Current_State(), chosenMove));
				ClassRef.Perform_Move(std::get<0>(possibleMoves.at(i)));
				break;
			}
		}
	}
	outFile.open("TotalFile.txt", std::ios::out | std::ios::app);
	outFile << std::to_string(startingPosition) << " " << std::to_string(movesTaken.size()) << std::endl;
	outFile.close();
	std::cout << "Started Recording" << std::endl;
	ClassRef.Record_Vector(movesTaken);

	

	return 0;
}

std::vector<int> createNumberLocations()
{
	std::vector<int> NumberLocations;
	std::vector<int> Numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 0 };
	int chosenNumber;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 9; ++i) {
		chosenNumber = rand() % Numbers.size();
		NumberLocations.push_back(Numbers[chosenNumber]);
		Numbers.erase(Numbers.begin() + chosenNumber);
	}
	return NumberLocations;
}

int main()
{
	srand(time(NULL));
	std::vector<int> NumberLocations;

	for (int i = 0; i < 500; i++) {
		NumberLocations = createNumberLocations();
		for (unsigned int j = 0; j < NumberLocations.size(); j++) {
			std::cout << NumberLocations[j] << " ";
		}
		std::cout << std::endl;
		GridSQL TestGrid(3, NumberLocations, "3X3.DB");

		if (TestGrid.Is_Solvable()) {
			Run<GridSQL>(TestGrid);
		}
		else
		{
			std::cout << "Not Solvable! Waiting 30 secends\n";
			Sleep(30000);
			i--;
		}
	} 

	return 0;
}

