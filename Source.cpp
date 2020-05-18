#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <Windows.h>
#include <thread>
#include <string>

class Indexer {
private:
	std::vector<std::wstring> fileNames;
	const unsigned long long blockSize;

	//WINAPI
	//OK
	auto getAvailableVirtualMemory() {
		MEMORYSTATUSEX memoryStatus;
		memoryStatus.dwLength = sizeof(memoryStatus);
		GlobalMemoryStatusEx(&memoryStatus);
		return static_cast<unsigned long long>(memoryStatus.ullAvailVirtual);
	}

	//OK
	void getFileNames(std::wstring &directory) {
		if (directory.length() != 0) {
			if (directory.back() != L'\\') {
				directory += L'\\';
			}
			std::wstring directoryCopy{directory + L'*' };
			WIN32_FIND_DATA findFileData;
			HANDLE fileHandle{ FindFirstFile(directoryCopy.c_str(), &findFileData) };
			BOOL fileFound{ true };
			fileFound = FindNextFile(fileHandle, &findFileData);
			while (fileFound) {
				fileFound = FindNextFile(fileHandle, &findFileData);
				if (findFileData.cFileName[0] != L'.') {
					if (findFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
						directoryCopy = directory + findFileData.cFileName + L'\\';
						getFileNames(directoryCopy);
					}
					else {
						fileNames.push_back(directory + findFileData.cFileName);
					}
				}
			}
		}
	}

	//OK
	unsigned long long getFileSize(std::wstring path) {
		WIN32_FIND_DATA findFileData;
		HANDLE fileHandle{ FindFirstFile(path.c_str(), &findFileData) };
		return (static_cast<unsigned long long>(MAXDWORD) + 1)* findFileData.nFileSizeHigh + findFileData.nFileSizeLow;
	}

	//C++11
	//OK (only for ENG)
	void tolower(std::string &word) {
		for (unsigned int i = 0; i < word.length(); i++) {
			if (word[i] >= 'A' && word[i] <= 'Z') {
				word[i] += 'a' - 'A';
			}
		}
	}

	
	//OK && CAN BE OPTIMIZED
	auto ignorePunctuation(std::string & data) {
		static const std::string availableCharacters{"abcdefghijklmnopqrstuvwzyz0123456789"};
		std::vector<std::string> words;
		auto firstPosition{ data.find_first_of(availableCharacters) };
		auto lastPosition{ data.find_first_not_of(availableCharacters, firstPosition) };
		while (firstPosition != std::string::npos) {
			words.push_back(std::move(data.substr(firstPosition, lastPosition - firstPosition)));
			firstPosition = data.find_first_of(availableCharacters, lastPosition);
			lastPosition = data.find_first_not_of(availableCharacters, firstPosition);
		}
		return words;
		
	}

	//OK
	auto parseNextInvertedBlock(unsigned int indexOfFile) {
		std::map < std::string, std::vector<std::pair<size_t, size_t>>> dictionary;
		std::string inputData;
		unsigned long long currentSize{ 0 };
		auto nextFileSize{ getFileSize(fileNames[indexOfFile]) };

		while (indexOfFile < fileNames.size() && currentSize + nextFileSize < blockSize) {
			int wordNumber{ 0 };
			std::ifstream inputFile(fileNames[indexOfFile]);
			std::thread inProcess([]() {});
			while (!inputFile.eof()) {
				std::getline(inputFile, inputData);
				inProcess.join();
				std::thread addToDictionary([this, &inputData, &dictionary, indexOfFile, &wordNumber]() {
					tolower(inputData);
					auto words = ignorePunctuation(inputData);
					for (auto& entry : words) {
						wordNumber++;
						dictionary[entry].push_back(std::make_pair(indexOfFile, wordNumber));
					}
				});
				inProcess = std::move(addToDictionary);
			}
			inProcess.join();

			inputFile.close();
			currentSize += nextFileSize;
			indexOfFile++;
		}
		return dictionary;
	}

	//WRITE ME
	void writeIndex() {

	}

public:
	Indexer()
		:blockSize{getAvailableVirtualMemory() / 2 }	//other memory for saved structure and other operations
	{

	}

	//DELETE ME
	void DEBUG() {
		setlocale(0, "");
		std::wstring d{ LR"(C:\Users\PC\Desktop\Важна інфа\3 курс\2 sem\ПО\aclImdb)" };
		getFileNames(d);
		
		auto f{ parseNextInvertedBlock(0) };
		for (const auto& entry : f) {
			std::cout << entry.first << "\t" << std::endl;
		}
		std::wcout << fileNames[f.rbegin()->second[0].first];
	}
};

void f(int a, std::string& b, int& c) {
	std::cout << b;
}

int main() {
	Indexer indexer{};
	indexer.DEBUG();
	return 0;
}