/*
 * bulkmt.h
 *
 *  Created on: 30 июн. 2019 г.
 *      Author: sveta
 */
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include<vector>

class BulkInfo {
	std::string bulkString;
	int commandCount;
	std::string recieveTime;
	bool outputed;
	bool written;
public:
	BulkInfo(std::string bulkStr, int commandCnt, std::string rTime) :
		bulkString(bulkStr), commandCount(commandCnt), recieveTime(rTime), outputed(
			false), written(false) {

	}

	std::string GetBulkString();
	int GetCommandCount();
	std::string GetTime();
	bool GetOutputed();
	void SetOutputed(bool val);
	bool GetWritten() ;
	void SetWritten(bool val);
};

using sclock = std::chrono::system_clock;
// сохраняет команды и  выполняет их при необходимости.
class CommandCollector {
	std::stringstream bulkString;
	std::string recieveTime;
	size_t commandsCount;
public:
	CommandCollector();
	size_t size();
	void AddCommand(std::string command, std::string time);

	// Выполнить команды и записать их в файл;
	BulkInfo Process() ;
};
enum class State {
	Processed, Finish, WaitComand,
};

// получает команду и решает, что делать: выполнять или копить.
class CommandProcessor {
	const size_t N;
	CommandCollector commandCollector;
	int openCount;
	int closeCount;
	sclock::time_point prevTime;
	const std::string openBrace = "{";
	const std::string closeBrace = "}";


	void CallProcessor() {
		if (commandCollector.size() > 0) {
			bulks.push_back(commandCollector.Process());
			processorState = State::Processed;
		}
	}
	State processorState;
public:
	CommandProcessor(size_t n);
	std::vector<BulkInfo> bulks;
	size_t GetBulkSize();
	void SetState(State st);
	State GetState();
	void ProcessCommand(std::string command) ;

};


