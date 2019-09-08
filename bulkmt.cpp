/*
 * main.cpp
 *
 *  Created on: 7 сент. 2019 г.
 *      Author: sveta
 */
#include "bulkmt.h"
#include <thread>
#include <mutex>
std::mutex cvMutex;
std::condition_variable fileCv;
std::condition_variable consoleCv;

void WriteToConsole(ConsoleWriter &consWriter, CommandProcessor &p) {
	while (p.GetState() != State::Finish) {
		std::unique_lock<std::mutex> lk(cvMutex);
		consoleCv.wait(lk,
				[&p]() {return (p.GetBulkSize() > 0 || p.GetState() == State::Finish);});
		consWriter.Write(p.GetCommands());
	}
}

void WriteToFile(FilerWriter &fileWriter, CommandProcessor &p) {
	while (p.GetState() != State::Finish) {
		std::unique_lock<std::mutex> lk(cvMutex);
		fileCv.wait(lk,
				[&p]() {return (p.GetBulkSize() > 0 || p.GetState() == State::Finish);});
		fileWriter.Write(p.GetCommands());
	}
}

int main(int, char *argv[]) {

	size_t n;

	n = std::atoi(argv[1]);
	if (n == 0)
		n = 2;

	CommandProcessor p(n);
	ConsoleWriter consWriter(p);
	FilerWriter fileWriter1(p);
	FilerWriter fileWriter2(p);

	std::thread logThread(WriteToConsole, std::ref(consWriter), std::ref(p));

	std::thread file1Thread(WriteToFile, std::ref(fileWriter1), std::ref(p));

	std::thread file2Thread(WriteToFile, std::ref(fileWriter2), std::ref(p));

	std::unique_lock<std::mutex> lk(cvMutex, std::defer_lock);
	int mainCommandCount = 0;
	int mainBlockCount = 0;
	int mainLinesCount = 0;

	std::string line;
	while (std::getline(std::cin, line)) {
		mainLinesCount++;
		lk.lock();
		p.ProcessCommand(line);
		lk.unlock();
		if (p.GetState() == State::Processed) {
			mainBlockCount++;
			mainCommandCount += p.GetBulkSize();
			lk.lock();
			fileCv.notify_all();
			consoleCv.notify_one();
			lk.unlock();
		}
	}
	if (p.GetState() != State::Processed) {
				mainBlockCount++;
				mainCommandCount += p.GetBulkSize();
	}
	p.ProcessCommand("", true);
	lk.lock();
	fileCv.notify_all();
	consoleCv.notify_one();
	lk.unlock();

	if (logThread.joinable()) {
		logThread.join();
	}
	if (file1Thread.joinable()) {
		file1Thread.join();
	}
	if (file2Thread.joinable()) {
		file2Thread.join();
	}

	std::cout << "lines count " << mainLinesCount << std::endl;
	std::cout << "main commands count " << mainCommandCount << std::endl;
	std::cout << "main blocks count " << mainBlockCount << std::endl;

	std::cout << "log commands count " << consWriter.commmandsCount
			<< std::endl;
	std::cout << "log blocks count " << consWriter.blocksCount << std::endl;

	std::cout << "file1 commands count " << fileWriter1.commmandsCount
			<< std::endl;
	std::cout << "file1 blocks count " << fileWriter1.blocksCount << std::endl;

	std::cout << "file2 commands count " << fileWriter2.commmandsCount
			<< std::endl;
	std::cout << "file2 blocks count " << fileWriter2.blocksCount << std::endl;

	return 0;
}


