#include "GridSQL.h"
#include <vector>
#include <tuple>
#include <string.h>
#include <sstream>
#include <sqlite3.h>
#include <Windows.h>

GridSQL::GridSQL(int gridSize, std::vector<int> vectorGot, std::string fileName)
{
	FileName = fileName;

	createSQLDB(FileName.c_str());
	createSQLTable(FileName.c_str());

	int exit = sqlite3_open(FileName.c_str(), &db);

	PreviousMove = GridSQL::POSSIBLEMOVES::NONE;

	if ((gridSize * gridSize) == vectorGot.size()) {
		NumberLocations = vectorGot;
		GRID_SIZE = gridSize;
	}
	else
	{
		GRID_SIZE = 0;
	}

	return;
}

GridSQL::~GridSQL()
{
	sqlite3_close(db);
}

std::vector<GridSQL::POSSIBLEMOVES> GridSQL::Possible_Moves()
{
	int zeroLocation = -1;

	for (unsigned int i = 0; i < NumberLocations.size(); ++i) {
		if (NumberLocations[i] == 0) zeroLocation = i;
	}
	if (zeroLocation == -1) return std::vector<GridSQL::POSSIBLEMOVES>();

	std::vector<GridSQL::POSSIBLEMOVES> moves;
	if (!(zeroLocation < GRID_SIZE) && PreviousMove != POSSIBLEMOVES::UP)
		moves.push_back(GridSQL::POSSIBLEMOVES::DOWN);
	if (!(zeroLocation >= (GRID_SIZE * GRID_SIZE) - GRID_SIZE) && PreviousMove != POSSIBLEMOVES::DOWN)
		moves.push_back(GridSQL::POSSIBLEMOVES::UP);
	if (!(zeroLocation % GRID_SIZE == 0) && PreviousMove != POSSIBLEMOVES::LEFT)
		moves.push_back(GridSQL::POSSIBLEMOVES::RIGHT);
	if (!((zeroLocation - GRID_SIZE + 1) % GRID_SIZE == 0) && PreviousMove != POSSIBLEMOVES::RIGHT)
		moves.push_back(GridSQL::POSSIBLEMOVES::LEFT);
	return moves;
}

//TODO - check if move is possible
bool GridSQL::Move(POSSIBLEMOVES move)
{
	int zeroLocation = -1;

	for (unsigned int i = 0; i < NumberLocations.size(); ++i) {
		if (NumberLocations[i] == 0) zeroLocation = i;
	}
	if (zeroLocation == -1) return false;

	int movingTile;
	switch (move)
	{
	case GridSQL::POSSIBLEMOVES::UP:
		movingTile = zeroLocation + 3;
		break;
	case GridSQL::POSSIBLEMOVES::DOWN:
		movingTile = zeroLocation - 3;
		break;
	case GridSQL::POSSIBLEMOVES::LEFT:
		movingTile = zeroLocation + 1;
		break;
	case GridSQL::POSSIBLEMOVES::RIGHT:
		movingTile = zeroLocation - 1;
		break;
	default:
		return false;
		break;
	}

	PreviousMove = move;
	NumberLocations[zeroLocation] = NumberLocations[movingTile];
	NumberLocations[movingTile] = 0;
	return this->Is_Finished();
}

bool GridSQL::Is_Finished()
{
	if (NumberLocations[NumberLocations.size() - 1] != 0) return false;
	for (unsigned int i = 0; i < NumberLocations.size() - 2; ++i) {
		if (NumberLocations[i] != i + 1) return false;
	}
	return true;
}

//TODO get values from file
std::vector<std::tuple<int, float>> GridSQL::Get_Possible_Moves()
{
	std::vector<std::tuple<int, float>> movesWithPercentages;
	std::vector<GridSQL::POSSIBLEMOVES> possibleMoves;
	std::vector<std::tuple<int, float>> moveCosts;
	int amountOfBlanks = 0;
	float totalCost = 0;
	float averageCost;
	bool Exists = false;

	sqlite3* DB;
	char* messageError;
	std::string sqlCode;

	int exit = sqlite3_open(FileName.c_str(), &DB);


	sqlCode = "SELECT MOVE, AVERAGECOST FROM STATESWITHMOVES WHERE STATE = " + std::to_string(Get_Current_State()) + ";";
	sqlite3_exec(DB, sqlCode.c_str(), GridSQL::MoveCostsAfterSelect, &moveCosts, NULL);

	possibleMoves = Possible_Moves();
	//-------------------Get Average Cost And Amount Of Blanks-------------------------------------------------
	for (std::vector<GridSQL::POSSIBLEMOVES>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it) {
		Exists = false;
		for (int i = 0; i < moveCosts.size(); i++) {
			if (std::get<0>(moveCosts.at(i)) == *it) {
				Exists = true;
				totalCost += std::get<1>(moveCosts.at(i));
			}
		}
		if (!Exists) {
			amountOfBlanks++;
		}
	}
	if (possibleMoves.size() == amountOfBlanks) {
		averageCost = 50;
	}
	else
	{
		averageCost = totalCost / (possibleMoves.size() - amountOfBlanks);
	}
	totalCost = averageCost * possibleMoves.size();
	//----------------------Reverse Move Count----------------------------------------------------------------
	for (int i = 0; i < moveCosts.size(); i++) {

		std::get<1>(moveCosts.at(i)) = std::get<1>(moveCosts.at(i)) + ((averageCost - std::get<1>(moveCosts.at(i))) * 2);
	}

	//----------------------Create Percents-------------------------------------------------------------------
	for (std::vector<GridSQL::POSSIBLEMOVES>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it) {
		Exists = false;
		for (int i = 0; i < moveCosts.size(); i++) {
			if (std::get<0>(moveCosts.at(i)) == *it) {
				Exists = true;
				if ((double)(std::get<1>(moveCosts.at(i)) / totalCost) * 100.0f > 90.0) {
					movesWithPercentages.clear();
					movesWithPercentages.push_back(std::make_tuple(*it, (std::get<1>(moveCosts.at(i)) / totalCost) * 100.0f));
					return movesWithPercentages;
				}
				movesWithPercentages.push_back(std::make_tuple(*it, (std::get<1>(moveCosts.at(i)) / totalCost)*100.0f));
			}
		}
		if (!Exists) {
			movesWithPercentages.push_back(std::make_tuple(*it, (averageCost / totalCost)*100.0f));
		}
	}
	sqlite3_close(DB);
	return movesWithPercentages;
}

void GridSQL::Record(int State, int Move, unsigned int Moves_to_finish)
{

	//sqlite3* DB;
	char* messageError;
	std::string sqlCode;

	int exit;// = sqlite3_open(FileName.c_str(), &DB);

	//Search file for same State&move
	sqlCode = "SELECT * FROM STATESWITHMOVES WHERE STATE = " + std::to_string(State) + " AND MOVE = " + std::to_string(Move) + ";";
	std::tuple<GridSQL*, int> passThroughTuple(this, Moves_to_finish);
	sqlite3_exec(db, sqlCode.c_str(), GridSQL::selectElements, &passThroughTuple, NULL);
	ExecCode();
	//If Not Found in file
	sqlCode = "INSERT INTO STATESWITHMOVES (STATE, MOVE, AVERAGECOST, AMOUNTOFTIMES) VALUES (" + std::to_string(State) + ", " + std::to_string(Move) + ", " + std::to_string(Moves_to_finish) + ", 1);";

	exit = sqlite3_exec(db, sqlCode.c_str(), NULL, 0, &messageError);
	if (exit != SQLITE_OK) {
		//std::cerr << "Error Inserting" << std::endl;
		sqlite3_free(messageError);
	}
	//sqlite3_close(DB);
	return;
}

void GridSQL::Record_Vector(std::vector<std::tuple<int, int>> MovesWithStates)
{
	std::vector<long long> visitedStatesAndMoves;
	std::tuple<int, int> tempTuple;
	unsigned int moves = 1;
	long long stateWithMove;
	unsigned int percentTracker = 10;

	//If not visited yet
	while (!MovesWithStates.empty())
	{
		tempTuple = MovesWithStates.back();
		MovesWithStates.pop_back();
		stateWithMove = (long long)(std::get<0>(tempTuple) + (std::get<1>(tempTuple) * pow(10, 9)));
		if (std::find(visitedStatesAndMoves.begin(), visitedStatesAndMoves.end(), stateWithMove) == visitedStatesAndMoves.end() && (visitedStatesAndMoves.empty() || stateWithMove != visitedStatesAndMoves.back())) {
			visitedStatesAndMoves.push_back(stateWithMove);
			Record(std::get<0>(tempTuple), std::get<1>(tempTuple), moves);
			if ((moves * 100 / (MovesWithStates.size() + moves)) >= percentTracker) {
				std::cout << percentTracker << "% finished" << std::endl;
				percentTracker += 10;
			}
		}
		else
		{
			Sleep(1);
		}
		moves++;
	}
}

unsigned int GridSQL::Get_Current_State()
{
	unsigned int state = 0;
	for (unsigned int i = 0; i < NumberLocations.size(); i++)
	{
		state += (int)(NumberLocations[i] * pow(10, NumberLocations.size() - 1 - i));
	}
	return state;
}

int GridSQL::Perform_Move(int Move_To_Perform)
{
	Move((POSSIBLEMOVES)Move_To_Perform);
	return Get_Current_State();
}

int GridSQL::selectAll()
{
	sqlite3* DB;
	std::string sqlCode;

	int exit = sqlite3_open(FileName.c_str(), &DB);
	sqlCode = "SELECT * FROM STATESWITHMOVES;";
	sqlite3_exec(DB, sqlCode.c_str(), GridSQL::printElements, NULL, NULL);
	return 0;
}

bool GridSQL::Is_Solvable()
{
	int inv_count = 0;
	for (int i = 0; i < 9 - 1; i++)
		for (int j = i + 1; j < 9; j++)
			// Value 0 is used for empty space
			if (NumberLocations.at(j) && NumberLocations.at(i) && NumberLocations.at(i) > NumberLocations.at(j))
				inv_count++;
	return (inv_count % 2 == 0);
}

double GridSQL::AverageMoves(std::string Line, unsigned int Moves_To_Finish)
{
	unsigned int amountOfMoves = 1;
	unsigned int totalMoves = Moves_To_Finish;
	std::string parsedWord;
	std::stringstream lineStringstream(Line);
	while (lineStringstream >> parsedWord)
	{
		if (parsedWord == "" || parsedWord == " ")
			break;
		totalMoves += std::stoi(parsedWord);
		amountOfMoves++;
	}
	if (amountOfMoves == 0) return 0;
	return (double)totalMoves / amountOfMoves;
}

int GridSQL::selectElements(void* Grid_Class_AND_Moves_to_Finish, int argc, char** argv, char** azColName) {
	double averageCost = 0;
	int amount = 0;
	std::string state, move, columnName;
	for (int i = 0; i < argc; i++) {
		columnName = azColName[i];
		if (columnName == "AVERAGECOST") {
			columnName = argv[i];
			averageCost = std::stoi(columnName);
		}
		else if (columnName == "AMOUNTOFTIMES") {
			columnName = argv[i];
			amount = std::stoi(columnName);
		}
		else if (columnName == "STATE")
		{
			columnName = argv[i];
			state = columnName;
		}
		else if (columnName == "MOVE")
		{
			columnName = argv[i];
			move = columnName;
		}
	}

	if (averageCost != 0 && amount != 0) {
		averageCost = ((double)averageCost * amount) + std::get<1>(*(std::tuple<GridSQL*, int>*)Grid_Class_AND_Moves_to_Finish);
		amount++;
		averageCost = (double)averageCost / amount;
	}
	GridSQL* This = std::get<0>(*(std::tuple<GridSQL*, int>*)Grid_Class_AND_Moves_to_Finish);
	This->sqlCodeAfterSelect = "UPDATE STATESWITHMOVES SET AVERAGECOST = " + std::to_string(averageCost) + ", AMOUNTOFTIMES = " + std::to_string(amount) + " WHERE STATE = " + state + " AND MOVE = " + move + ";";
	return 1;
}

int GridSQL::printElements(void* NotUsed, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++) {
		std::cout << azColName[i] << ": " << argv[i] << std::endl;
	}

	std::cout << std::endl;
	return 0;
}

int GridSQL::MoveCostsAfterSelect(void* moveVectorTuple, int argc, char** argv, char** azColName) {
	std::string columnName;
	bool NoERROR = true;
	for (int i = 0; i < argc; i++) {
		columnName = azColName[i];
		if (columnName != "MOVE" && columnName != "AVERAGECOST") {
			NoERROR = true;
		}
	}
	if (NoERROR) {
		columnName = azColName[0];
		if (columnName == "MOVE") {
			((std::vector<std::tuple<int, float>>*)moveVectorTuple)->push_back(std::make_tuple(std::stoi(argv[0]), std::stod(argv[1])));
		}
		else
		{
			((std::vector<std::tuple<int, float>>*)moveVectorTuple)->push_back(std::make_tuple(std::stoi(argv[1]), std::stod(argv[0])));
		}
	}
	return 0;
}

int GridSQL::ExecCode()
{
	sqlite3* DB;
	char* messageError;

	sqlite3_open(FileName.c_str(), &DB);

	int exit = sqlite3_exec(DB, sqlCodeAfterSelect.c_str(), NULL, 0, &messageError);
	if (exit != SQLITE_OK) {
		//std::cerr << "Error After Select" << std::endl;
		sqlite3_free(messageError);
	}
	/*else
		std::cout << "Succesfull after select" << std::endl;*/
	sqlite3_close(DB);
	return 0;
}

int GridSQL::createSQLDB(const char* s) {
	sqlite3* DB;
	int exit;

	exit = sqlite3_open(s, &DB);

	sqlite3_close(DB);

	return exit;
}

int GridSQL::createSQLTable(const char* s) {
	sqlite3* DB;

	std::string sqlCode = "CREATE TABLE IF NOT EXISTS STATESWITHMOVES("
		"STATE INTEGER, "
		"MOVE INT NOT NULL, "
		"AVERAGECOST DOUBLE, "
		"AMOUNTOFTIMES INT, "
		"PRIMARY KEY(STATE, MOVE));";
	try
	{
		int exit = 0;
		exit = sqlite3_open(s, &DB);

		char* messageError;
		exit = sqlite3_exec(DB, sqlCode.c_str(), NULL, 0, &messageError);

		if (exit != SQLITE_OK) {
			std::cerr << "Error Creating Table" << std::endl;
			sqlite3_free(messageError);
		}
		/*else
			std::cout << "Table Created Successfully" << std::endl;*/
		sqlite3_close(DB);
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what();
	}
	sqlite3_close(DB);
	return 0;
}

