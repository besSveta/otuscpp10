/*
 * bulkmt.cpp
 *
 *  Created on: 30 июн. 2019 г.
 *      Author: sveta
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "bulkmt.h"

std::mutex cvMutex;
std::condition_variable cv;
std::condition_variable ConsoleCv;
std::string folder;
std::string trim(const std::string& str,
	const std::string& whitespace = " \t") {
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

void WriteToConsole(int &commandsCount, int & blockCount, CommandProcessor &p) {
	while (p.GetState() != State::Finish) {
		std::unique_lock<std::mutex> lk(cvMutex);
		ConsoleCv.wait(lk, [&p]() {return p.bulks.size() > 0; });
		for (std::size_t i = 0; i < p.bulks.size(); i++) {
			if (!p.bulks[i].GetOutputed()) {
				std::cout << p.bulks[i].GetBulkString();
				commandsCount += p.bulks[i].GetCommandCount();
				blockCount++;
				p.bulks[i].SetOutputed(true);
			}
		}
		lk.unlock();
	}
}

void WriteToFile(int &commandsCount, int & blockCount, CommandProcessor &p) {
	while (p.GetState() != State::Finish) {
		std::unique_lock<std::mutex> lk(cvMutex);
		cv.wait(lk, [&p]() {return p.bulks.size() > 0; });
		std::stringstream filestring;
		for (std::size_t i = 0; i < p.bulks.size(); i++) {
			if (!p.bulks[i].GetWritten()) {
				filestring.clear();
				filestring << folder << p.bulks[i].GetTime() << "_"
					<< std::this_thread::get_id() << ".log";
				std::ofstream file(filestring.str());
				file << p.bulks[i].GetBulkString();
				file.close();
				p.bulks[i].SetWritten(true);
				commandsCount += p.bulks[i].GetCommandCount();
				blockCount++;
				if (p.bulks[i].GetOutputed()) {
					std::swap(p.bulks[i], p.bulks.back());
					p.bulks.pop_back();
					i--;
				}
			}
		}
		lk.unlock();
	}
}
#ifdef _WIN32
#include <direct.h>
#endif
int main(int, char *argv[]) {

	size_t n;

	n = std::atoi(argv[1]);
	if (n == 0)
		n = 2;
#ifdef _WIN32
	auto status = _mkdir("bulkfiles");
#else
	auto status= mkdir("bulkfiles", S_IRWXU | S_IRWXG);;
#endif
	if (status == 0 || errno == static_cast<int>(std::errc::file_exists)) {
		folder = "bulkfiles/";
	}
	else
		folder = "";
	CommandProcessor p(n);
	int logCommandCount = 0;
	int logBlockCount = 0;
	std::thread logThread(WriteToConsole, std::ref(logCommandCount),
		std::ref(logBlockCount), std::ref(p));
	logThread.detach();

	int file1CommandCount = 0;
	int file1BlockCount = 0;
	std::thread file1Thread(WriteToFile, std::ref(file1CommandCount),
		std::ref(file1BlockCount), std::ref(p));

	int file2CommandCount = 0;
	int file2BlockCount = 0;
	std::thread file2Thread(WriteToFile, std::ref(file2CommandCount),
		std::ref(file2BlockCount), std::ref(p));
	std::unique_lock<std::mutex> lk(cvMutex, std::defer_lock);
	int mainCommandCount = 0;
	int mainBlockCount=0;
	int mainLinesCount=0;
	for (std::string line;
		(p.GetState() != State::Finish && std::getline(std::cin, line));) {
		mainLinesCount++;
		lk.lock();
		p.ProcessCommand(trim(line));
		lk.unlock();
		if (p.GetState() == State::Processed) {
			mainBlockCount++;
			mainCommandCount += p.bulks.back().GetCommandCount();
			cv.notify_all();
			ConsoleCv.notify_one();
		}
	}
	p.SetState(State::Finish);
	if (logThread.joinable()) {
		logThread.join();
	}
	if (file1Thread.joinable()) {
		file1Thread.join();
	}
	if (file2Thread.joinable()){
		file2Thread.join();
	}
	std::cout << "lines count " << mainLinesCount<<std::endl;
	std::cout << "main commands count " << mainCommandCount << std::endl;
	std::cout << "main blocks count " << mainBlockCount << std::endl;

	std::cout << "log commands count " << logCommandCount << std::endl;
	std::cout << "log blocks count " << logBlockCount << std::endl;

	std::cout << "file1 commands count " << file1CommandCount << std::endl;
	std::cout << "file1 blocks count " << file1BlockCount << std::endl;

	std::cout << "file2 commands count " << file2CommandCount << std::endl;
	std::cout << "file2 blocks count " << file2BlockCount << std::endl;
	return 0;
}
