#pragma once

#include <iostream>
#include <vector>
#include <string.h>
#include <fstream>
#include <sqlite3.h>
#include "StateClass.h"

class GridSQL : StateClass
{
public:
	enum POSSIBLEMOVES
	{
		UP = 1,
		DOWN = 2,
		LEFT = 3,
		RIGHT = 4,
		NONE = 5,
	};

	int GRID_SIZE;

	std::vector<int> NumberLocations;

	POSSIBLEMOVES PreviousMove;

	std::string FileName;

	std::string sqlCodeAfterSelect;

	sqlite3* db;

	GridSQL(int, std::vector<int>, std::string);

	~GridSQL();

	std::vector<POSSIBLEMOVES> Possible_Moves();

	bool Move(POSSIBLEMOVES);

	bool Is_Finished();

	bool Is_Solvable();

	double AverageMoves(std::string, unsigned int);

	// Inherited via StateClass
	virtual std::vector<std::tuple<int, float>> Get_Possible_Moves() override;
	virtual void Record(int State, int Move, unsigned int Moves_to_finish) override;
	virtual void Record_Vector(std::vector<std::tuple<int, int>>) override;
	virtual unsigned int Get_Current_State();
	virtual int Perform_Move(int);

	int selectAll();
private:
	static int createSQLDB(const char* s);
	static int createSQLTable(const char* s);
	static int selectElements(void* NotUsed, int argc, char** argv, char** azColName);
	static int printElements(void* NotUsed, int argc, char** argv, char** azColName);
	static int MoveCostsAfterSelect(void* moveVectorTuple, int argc, char** argv, char** azColName);
	int ExecCode();
};

