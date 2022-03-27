#pragma once

#include <vector>
#include<tuple>

class StateClass
{
public:
	virtual unsigned int Get_Current_State() = 0;

	virtual std::vector<std::tuple<int, float>> Get_Possible_Moves() = 0;

	virtual int Perform_Move(int) = 0;

	virtual bool Is_Finished() = 0;

	virtual void Record(int State, int Move, unsigned int Moves_to_finish) = 0;

	virtual void Record_Vector(std::vector<std::tuple<int, int>>) = 0;
protected:
	virtual ~StateClass();
};

