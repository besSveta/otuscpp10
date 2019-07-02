/*
 * bulkmt_lib.cpp
 *
 *  Created on: 2 июл. 2019 г.
 *      Author: sveta
 */
#include "bulkmt.h"


	std::string BulkInfo::GetBulkString() {
		return bulkString;
	}
	int BulkInfo::GetCommandCount() {
		return  commandCount;
	}
	std::string BulkInfo::GetTime() {
		return recieveTime;
	}
	bool BulkInfo::GetOutputed() {
		return outputed;
	}
	void BulkInfo::SetOutputed(bool val) {
		outputed = val;
	}
	bool BulkInfo::GetWritten() {
		return written;
	}
	void BulkInfo::SetWritten(bool val) {
		written = val;
	}
	CommandCollector::CommandCollector() {
			commandsCount = 0;
			recieveTime = "";
		}
		size_t CommandCollector::size() {
			return commandsCount;
		}
		void CommandCollector::AddCommand(std::string command, std::string time) {
			if (commandsCount == 0) {
				recieveTime = time;
				bulkString.str("");
				bulkString << " bulk:" << command;
			}
			else {
				bulkString << ", " << command;
			}
			commandsCount++;
		}

		// Выполнить команды и записать их в файл;
		BulkInfo CommandCollector::Process() {
			bulkString << std::endl;
			auto result = BulkInfo(bulkString.str(), commandsCount, recieveTime);
			bulkString.str("");
			commandsCount = 0;
			recieveTime = "";
			return result;
		}

		size_t CommandProcessor::GetBulkSize() {
			return commandCollector.size();
		}
		CommandProcessor::CommandProcessor(size_t n) :
			N(n) {
			openCount = 0;
			closeCount = 0;
			prevTime = sclock::now();
			processorState = State::WaitComand;
		}
		void CommandProcessor::SetState(State st) {
			processorState = st;
		}
		State CommandProcessor::GetState() {
			return processorState;
		}

		void CommandProcessor::ProcessCommand(std::string command) {

			auto currentTime = sclock::now();
			auto currentTime_t = sclock::to_time_t(currentTime);
			auto recieveTime = std::string(std::ctime(&currentTime_t));
			auto diff = std::chrono::duration_cast<std::chrono::seconds>(
				currentTime - prevTime).count();
			// выйти при вводе пустой строки и интервале между командами >2 секунд..
			if (command == "" && diff > 2) {
				processorState = State::Finish;
				return;
			}
			prevTime = currentTime;
			processorState = State::WaitComand;
			// новый блок.
			if (command == openBrace) {
				if (openCount == 0) {
					CallProcessor();
				}
				openCount++;
			}
			else {
				// закрывающая скобка.
				if (command == closeBrace && openCount>0) {
					closeCount++;
					// проверка на вложенность.
					if (closeCount == openCount) {
						CallProcessor();
						openCount = 0;
						closeCount = 0;
					}
				}
				else {
					commandCollector.AddCommand(command, std::to_string(diff));
					// если блок команд полностью заполнен и размер блока не был изменен скобкой.
					if (commandCollector.size() == N && openCount == 0) {
						CallProcessor();
					}

				}
			}
		}

