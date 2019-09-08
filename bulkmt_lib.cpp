#include <iostream>
#include <thread>
#include "bulkmt.h"
std::string trim(const std::string& str,
		const std::string& whitespace = " \t") {
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

void CommandsCollector::AddCommand(std::string && command) {
	if (commands.size() == 0) {
		auto currentTime = sclock::now();
		recieveTime = std::to_string(
				std::chrono::duration_cast<std::chrono::microseconds>(
						currentTime.time_since_epoch()).count());
		bulkString << " bulk:";
		bulkString << command;
	} else {
		bulkString << ", " << command;
	}
	commands.emplace_back(std::move(command));

}
std::size_t CommandsCollector::GetBulkSize() {
	return commands.size();
}

std::string CommandsCollector::GetRecieveTime() {
	return recieveTime;
}

std::deque<std::string>& CommandsCollector::GetCommands() {
	return commands;
}
std::string CommandsCollector::GetCommandsStr() {
	return bulkString.str();
}

void CommandsCollector::SetWrittenToFile(bool val) {
	writtenToFile = val;
}
void CommandsCollector::SetWrittenToConsole(bool val) {
	writtenToConsole = val;
}

bool CommandsCollector::GetWrittenToFile() {
	return writtenToFile;
}
bool CommandsCollector::GetWrittenToConsole() {
	return writtenToConsole;
}

void CommandsCollector::Clear() {
	commands.clear();
	bulkString.str("");
}
CommandWriter::CommandWriter(CommandProcessor& processor) :
		cmdProcessor(processor) {
	commmandsCount = 0;
	blocksCount = 0;

}

void ConsoleWriter::Write(CommandsCollector &commandsCollector) {

	if (!commandsCollector.GetWrittenToConsole()) {
		commmandsCount += commandsCollector.GetBulkSize();
		blocksCount++;
		std::cout << commandsCollector.GetCommandsStr() << std::endl;
		commandsCollector.SetWrittenToConsole(true);
		if (commandsCollector.GetWrittenToFile()) {
			// при полной записи очищается список команд.
			commandsCollector.Clear();
		}
	}
}
void FilerWriter::Write(CommandsCollector &commandsCollector) {

	std::stringstream filestring;
	if (!commandsCollector.GetWrittenToFile()) {
		filestring.clear();
		filestring << "bulk" << commandsCollector.GetRecieveTime()
				<< "_" << std::this_thread::get_id() << ".log";

		commmandsCount += commandsCollector.GetBulkSize();
		blocksCount++;
		std::ofstream ff(filestring.str(), std::ios_base::out);
		ff << commandsCollector.GetCommandsStr();
		ff.close();
		commandsCollector.SetWrittenToFile(true);
		if (commandsCollector.GetWrittenToConsole()) {
			// при полной записи очищается список команд.
			commandsCollector.Clear();
		}
	}

}

// получает команду и решает, что делать: выполнять или копить.

CommandsCollector & CommandProcessor::GetCommands() {
	return commands;
}

State CommandProcessor::GetState() {
	return processorState;
}

// Для выхода из цикла, который крутится в другом потоке. Вызывается когда команд больше нет.
void CommandProcessor::SetFinish() {
	if (commands.GetBulkSize() > 0) {
		commands.SetWrittenToConsole(false);
		commands.SetWrittenToFile(false);
	} else {
		commands.SetWrittenToConsole(true);
		commands.SetWrittenToFile(true);
	}
	processorState = State::Finish;
}

// Установить сигнал, что команды готовы к записи.
void CommandProcessor::Process() {
	// флаги, чтобы не выводить команды повторно.
	// также при полной записи эти флаги равны true, поэтому очищается список команд.
	commands.SetWrittenToConsole(false);
	commands.SetWrittenToFile(false);
	processorState = State::Processed;

}

/*void CommandProcessor::AddWriter(CommandWriter& someWriter) {
 writers.emplace_back(someWriter);
 }*/

CommandProcessor::CommandProcessor(size_t n) :
		N(n) {
	openCount = 0;
	closeCount = 0;
	processorState = State::WaitComand;
}

size_t CommandProcessor::GetBulkSize() {
	return commands.GetBulkSize();
}
void CommandProcessor::ProcessCommand(std::string command, bool isLast) {
	// новый блок.
	if (command == openBrace) {
		if (openCount == 0) {
			Process();
		}
		openCount++;
	} else {
		// закрывающая скобка.
		if (command == closeBrace && openCount > 0) {
			closeCount++;
			// проверка на вложенность.
			if (closeCount == openCount) {
				Process();
				openCount = 0;
				closeCount = 0;
			}
		} else {
			if (isLast) {
				SetFinish();
				return;
			}
			commands.AddCommand(std::move(command));
			// если блок команд полностью заполнен и размер блока не был изменен скобкой.
			if (commands.GetBulkSize() == N && openCount == 0) {
				Process();
			} else {
				processorState = State::WaitComand;
			}
		}
	}
}


