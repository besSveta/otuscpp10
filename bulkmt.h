/*
 * bulkmt.h
 *
 *  Created on: 7 сент. 2019 г.
 *      Author: sveta
 */

/*
 * bulk.h
 *
 *  Created on: 30 июн. 2019 г.
 *      Author: sveta
 */
#pragma once
#include <deque>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <condition_variable>

using sclock=std::chrono::system_clock;
// сохраняет команды и  выполняет их при необходимости.
class CommandProcessor;
class CommandWriter;
class CommandsCollector {
private:
	std::deque<std::string> commands;
	std::string recieveTime;
	std::stringstream bulkString;
	bool writtenToFile;
	bool writtenToConsole;
public:
	void AddCommand(std::string && command);
	std::size_t GetBulkSize();

	std::string GetRecieveTime();

	std::deque<std::string>& GetCommands();
	std::string GetCommandsStr();
	void SetWrittenToFile(bool val);
	void SetWrittenToConsole(bool val);
	bool GetWrittenToFile();
	bool GetWrittenToConsole();
	void Clear();
};

enum class State {
	Processed, Finish, WaitComand,
};

//Интерфейс для записи команд.
class CommandWriter: private std::enable_shared_from_this<CommandWriter> {
protected:
	CommandProcessor& cmdProcessor;

public:
	CommandWriter(CommandProcessor& processor);
	virtual void Write(CommandsCollector& commandsCollector)=0;
	virtual ~CommandWriter() {
	};

	int commmandsCount;
		int blocksCount;
};
// Класс для записи в файл.
class FilerWriter: public CommandWriter {
public:
	FilerWriter(CommandProcessor& processor) :
			CommandWriter(processor) {
	}
	;
	void Write(CommandsCollector& commandsCollector) override;
};

// Класс для записи в консоль.
class ConsoleWriter: public CommandWriter {
public:
	ConsoleWriter(CommandProcessor& processor) :
			CommandWriter(processor){
	}
	;
	void Write(CommandsCollector& commandsCollector) override;
};
// получает команду и решает, что делать: выполнять или копить.
class CommandProcessor {
	const size_t N;
	int openCount;
	int closeCount;
	const std::string openBrace = "{";
	const std::string closeBrace = "}";

	CommandsCollector commands;
	// std::vector<std::reference_wrapper<CommandWriter>> writers;

	void Process();
	State processorState;
public:
	State GetState();
	void SetFinish();
/*	void AddWriter(CommandWriter& someWriter);*/

	CommandProcessor(size_t n);
	size_t GetBulkSize();
	void ProcessCommand(std::string command, bool isLast = false);

	CommandsCollector& GetCommands();

};


