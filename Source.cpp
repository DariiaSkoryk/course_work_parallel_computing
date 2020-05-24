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
	template <typename FIRST_TYPE, typename SECOND_TYPE, typename THIRD_TYPE>
	struct triple{
		FIRST_TYPE first;
		SECOND_TYPE second;
		THIRD_TYPE third;
	};

	template <typename T>
	struct Word {
		std::string word;
		T position;

		bool operator<(Word &anotherWord) {
			return compare(anotherWord);
		}

		bool operator>(Word& anotherWord) {
			bool returnValue;
			size_t index{ 0 };
			auto thisWordLength = word.length();
			auto anotherWordLength = anotherWord.word.length();
			while ((index < thisWordLength) && (index < anotherWordLength) && this->word[index] == anotherWord.word[index]) {
				index++;
			}
			if (thisWordLength() == index || anotherWordLength == index) {
				returnValue = thisWordLength > anotherWordLength;
			}
			else {
				returnValue = word[index] > anotherWord.word[index];
			}
			return returnValue;
		}

		bool operator==(Word& anotherWord) {
			bool returnValue;
			size_t index{ 0 };
			auto thisWordLength = word.length();
			auto anotherWordLength = anotherWord.word.length();
			while ((index < thisWordLength) && (index < anotherWordLength) && this->word[index] == anotherWord.word[index]) {
				index++;
			}
			if (thisWordLength() == index || anotherWordLength == index) {
				returnValue = thisWordLength == anotherWordLength;
			}
			else {
				returnValue = word[index] == anotherWord.word[index];
			}
			return returnValue;
		}
	private:
		//count of symbols must be less than 127
		auto compare(Word& anotherWord) {
			char returnValue;
			size_t index{ 0 };
			auto thisWordLength = word.length();
			auto anotherWordLength = anotherWord.word.length();
			while ((index < thisWordLength) && (index < anotherWordLength) && this->word[index] == anotherWord.word[index]) {
				index++;
			}
			if (thisWordLength() == index || anotherWordLength == index) {
				returnValue = thisWordLength - anotherWordLength;
			}
			else {
				returnValue = word[index] - anotherWord.word[index];
			}
			return returnValue;
		}
	};

	struct Dictionary {
		std::vector<std::pair<std::string, triple<size_t, size_t, size_t>>> g;
		
		std::string operator[](int i) {
			return g[i].first;
		}

		bool operator<(std::pair)
	};

	std::vector<std::wstring> fileNames;
	const unsigned long long blockSize;
	char countOfThreads;

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
	//OK && OPTIMIZED
	auto getWords(std::vector<triple<size_t, size_t, std::string>>& block, size_t fromPosition = 0, size_t toPosition = 0) {
		std::map<size_t, std::vector<std::vector<std::string>>> words;
		if (!toPosition) {
			toPosition = block.size();
		}
		std::string data;
		for (auto i = fromPosition; i < toPosition; i++) {
			data = std::move(block[i].second);
			size_t fromStringPosition = 0;
			size_t countOfLetters = 0;
			size_t dataLength = data.length();
			for (size_t j = 0; j < dataLength; j++) {
				if (data[j] >= 'A' && data[j] <= 'Z') {
					data[j] += 'a' - 'A';
					if (!countOfLetters) {
						fromStringPosition = j;
					}
					countOfLetters++;
				}
				else if ((data[j] >= 'a' && data[j] <= 'z') || (data[j] >= '0' && data[j] <= '9')) {
					if (!countOfLetters) {
						fromStringPosition = j;
					}
					countOfLetters++;
				}
				else if (countOfLetters) {
					words[block[i].first].push_back(data.substr(fromStringPosition, countOfLetters));
					countOfLetters = 0;
				}
			}
		}
		return words;
	}

	//OR && TAKES ABOUT 30 SECONDS ON TEST DATA
	auto parseNextBlock(size_t &indexOfFile) {
		std::vector <std::pair<size_t, std::string>> block;
		std::string inputData;
		unsigned long long currentSize{ 0 };
		auto nextFileSize{ getFileSize(fileNames[indexOfFile]) };

		while (indexOfFile < fileNames.size() && currentSize + nextFileSize < blockSize) {
			std::ifstream inputFile(fileNames[indexOfFile]);
			size_t lineNumber{ 1 };
			if (!inputFile.eof()) {
				std::getline(inputFile, inputData);
				block.push_back(std::make_pair(indexOfFile, std::move(inputData)));
				while (!inputFile.eof()) {
					std::getline(inputFile, inputData);
					block.back().second += ' ' + inputData;
				}
			}
			inputFile.close();
			currentSize += nextFileSize;
			indexOfFile++;
		}
		return block;
	}

	//WRITE ME
	void addToDictionary(std::vector<triple<size_t, size_t, std::string>>& block, size_t fromPosition = 0, size_t toPosition = 0) {

	}

	//void invert();

	//WRITE ME
	void writeIndex() {

	}

	void parallelIndexConstruntion() {
		
			std::vector<std::thread> threads{ std::thread([]() {}) };
			std::cout << threads.capacity();
			threads.reserve(countOfThreads - 1);
			size_t indexOfFile{ 0 };
			auto currentBlock{ parseNextBlock(indexOfFile) };
			decltype(currentBlock) nextBlock;
			while (indexOfFile < fileNames.size()) {
				size_t fromPosition{ 0 };
				size_t processingSize{ currentBlock.size() / (countOfThreads - 1) };
				unsigned char rest{ currentBlock.size() % (countOfThreads - 1) };

				//parallel computing
				for (unsigned char i{ 0 }; i < countOfThreads; i++) {
					//create a new thread
					std::thread newThread([this, &currentBlock](size_t fromPosition, size_t toPosition) {
						addToDictionary(currentBlock, fromPosition, toPosition);
					}, fromPosition, fromPosition += (rest ? processingSize + 1 : processingSize));

					if (rest) {
						rest--;
					}
					threads.push_back(std::move(newThread));
				}
				//read from hdd in current thread to reduce positioning time
				nextBlock = parseNextBlock(indexOfFile);



			}
	}

	void serialIndexConstruction() {

	}

public:
	Indexer()
		:blockSize{getAvailableVirtualMemory() / 2 }	//other memory for saved structure and other operations
	{
		countOfThreads = std::thread::hardware_concurrency();
	}

	void indexConstruction(std::wstring directory) {
		getFileNames(directory);
		if (countOfThreads > 1) {
			parallelIndexConstruntion();
		}
		else {
			serialIndexConstruction();
		}
	}

	//DELETE ME
	void DEBUG() {
		setlocale(0, "");
		std::wstring d{ LR"(C:\Users\PC\Desktop\Важна інфа\3 курс\2 sem\ПО\aclImdb)" };
		indexConstruction(d);
		
		//auto f{ parseNextBlock() };
		/*for (const auto& entry : f) {
			std::cout << entry.first << "\t" << std::endl;
		}*/
		//std::wcout << fileNames[f.rbegin()->second[0].first];
	}

	//WRITE ME
	auto findWord() {
		std::vector<std::pair<size_t, size_t>> result;
		return result;
	}
};

int main() {
	Indexer indexer{};
	indexer.DEBUG();
	int cores_count = std::thread::hardware_concurrency();
	return 0;
}