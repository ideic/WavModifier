#pragma once
#include <string>
class WavManipulator
{
public:
	WavManipulator();
	~WavManipulator();

	void InsertBeepIntoStream(std::string filename);
};

