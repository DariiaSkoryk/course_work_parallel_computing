#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <Windows.h>
#include <thread>
#include <string>

class Indexer {
private:
	std::vector<std::wstring> fileNames;
	const unsigned long long blockSize;
	char maxCountOfThreads;

	template <typename T>
	struct Word {
		std::string word;
		T position;

		bool operator<(Word& anotherWord) {
			return compare(anotherWord) < 0;
		}

		bool operator>(Word& anotherWord) {
			return compare(anotherWord) > 0;
		}

		bool operator==(Word& anotherWord) {
			return compare(anotherWord) == 0;
		}

		Word(std::string word, T position) {
			this->word = word;
			this->position = position;
		}

		Word() = default;
	private:
		//count of symbols must be less than 127
		char compare(Word& anotherWord) {
			char returnValue;
			size_t index{ 0 };
			auto thisWordLength = word.length();
			auto anotherWordLength = anotherWord.word.length();
			while ((index < thisWordLength) && (index < anotherWordLength) && this->word[index] == anotherWord.word[index]) {
				index++;
			}
			if (thisWordLength == index || anotherWordLength == index) {
				returnValue = thisWordLength - anotherWordLength;
			}
			else {
				returnValue = word[index] - anotherWord.word[index];
			}
			return returnValue;
		}
	};

	struct Block {
		auto operator[](size_t i) {
			return block.at(i);
		}

		auto size() {
			return block.size();
		}

		void newFile(size_t indexOfFile, std::string word = "") {
			block.push_back(std::make_pair(indexOfFile, word));
		}

		//no checking for empty vector
		void push_back(std::string word) {
			block.back().second += ' ' + word;
		}

	private:
		std::vector<std::pair<size_t, std::string>> block;
	};

	struct Dictionary {
		auto operator[](size_t i) {
			return dictionary[i];
		}

		auto size() {
			return dictionary.size();
		}

		void invert() {
			std::sort(dictionary.begin(), dictionary.end());
		}

		void push_back(std::string word, size_t indexOfFile, size_t positionInFile) {
			dictionary.push_back({ word, {std::make_pair(indexOfFile, positionInFile)}});
		}
		
		void clear() {
			dictionary.clear();
		}

		/*void push_back(Dictionary dictionary) {
			this->dictionary.resize(this->dictionary.size() + dictionary.size());
			while (dictionary.size()) {
				//this->dictionary.push_back(dictionary.);
			}
		}*/
		
	private:
		std::vector<Word<std::vector<std::pair<size_t, size_t>>>> dictionary;
	};

	//WINAPI
	auto getAvailableVirtualMemory() {
		MEMORYSTATUSEX memoryStatus;
		memoryStatus.dwLength = sizeof(memoryStatus);
		GlobalMemoryStatusEx(&memoryStatus);
		return static_cast<unsigned long long>(memoryStatus.ullAvailVirtual);
	}

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

	unsigned long long getFileSize(std::wstring path) {
		WIN32_FIND_DATA findFileData;
		HANDLE fileHandle{ FindFirstFile(path.c_str(), &findFileData) };
		return (static_cast<unsigned long long>(MAXDWORD) + 1)* findFileData.nFileSizeHigh + findFileData.nFileSizeLow;
	}

	//C++11
	//OK && TAKES ABOUT 30 SECONDS ON TEST DATA
	auto parseNextBlock(size_t &indexOfFile) {
		Block block;
		std::string inputData;
		unsigned long long currentSize{ 0 };
		auto nextFileSize{ getFileSize(fileNames[indexOfFile]) };

		while (indexOfFile < fileNames.size() && currentSize + nextFileSize < blockSize) {
			std::ifstream inputFile(fileNames[indexOfFile]);
			size_t lineNumber{ 1 };
			if (!inputFile.eof()) {
				std::getline(inputFile, inputData);
				block.newFile(indexOfFile, std::move(inputData));
				while (!inputFile.eof()) {
					std::getline(inputFile, inputData);
					block.push_back(inputData);
				}
			}
			inputFile.close();
			currentSize += nextFileSize;
			indexOfFile++;
		}
		return block;
	}

	auto getWords(Block& block, size_t fromPosition = 0, size_t toPosition = 0) {
		Dictionary words;
		if (!toPosition) {
			toPosition = block.size();
		}
		std::string data;
		for (auto i = fromPosition; i < toPosition; i++) {
			size_t fromStringPosition{ 0 };
			size_t countOfLetters{ 0 };
			size_t countOfWords{ 0 };
			size_t dataLength{ block[i].second.length() };
			for (size_t j = 0; j < dataLength; j++) {
				if (block[i].second[j] >= 'A' && block[i].second[j] <= 'Z') {
					block[i].second[j] += 'a' - 'A';
					if (!countOfLetters) {
						fromStringPosition = j;
					}
					countOfLetters++;
				}
				else if ((block[i].second[j] >= 'a' && block[i].second[j] <= 'z') || (block[i].second[j] >= '0' && block[i].second[j] <= '9')) {
					if (!countOfLetters) {
						fromStringPosition = j;
					}
					countOfLetters++;
				}
				else if (countOfLetters) {
					countOfWords++;
					words.push_back(block[i].second.substr(fromStringPosition, countOfLetters), block[i].first, countOfWords);
					countOfLetters = 0;
				}
			}
		}
		return words;
	}

	//WRITE ME
	void writeIndex() {

	}

	//PARALLEL VARIANT
	//TEST ME
	void addWordsToDictionaries(Block& block, std::vector<Dictionary>& dictionaries, std::vector<std::thread>& threads, unsigned char countOfThreads) {
		size_t fromPosition{ 0 };
		size_t processingSize{ block.size() / (countOfThreads) };
		unsigned char rest{ block.size() % (countOfThreads) };

		for (unsigned char i{ 0 }; i < countOfThreads - 1; i++) {
			std::thread newThread([this, &block, &dictionaries, i](size_t fromPosition, size_t toPosition) {
				dictionaries[i] = std::move(getWords(block, fromPosition, toPosition));
				dictionaries[i].invert();
			}, fromPosition, fromPosition += (rest ? processingSize + 1 : processingSize));

			threads[i] = std::move(newThread);
			if (rest) {
				rest--;
			}
		}
		dictionaries[countOfThreads - 1] = std::move(getWords(block, fromPosition, fromPosition += processingSize));
		dictionaries[countOfThreads - 1].invert();
		for (unsigned char i{ 0 }; i < countOfThreads - 1; i++) {
			threads[i].join();
		}
	}

	//WRITE ME
	void mergeDictionaries(std::vector<Dictionary>& dictionaries, std::vector<std::thread>& threads, unsigned char countOfThreads) {
		/*for (unsigned char i{ 1 }; i < countOfThreads / 2; i++) {
			std::thread newThread([](Dictionary& firstDictionary, Dictionary& secondDictionary) {
				//merge
			}, dictionaries[i * 2], dictionaries[i * 2 + 1]);
			threads[i] = std::move(newThread);
		}*/
	}

	//TEST ME
	void parallelBlockProcessing(Block &block, std::vector<Dictionary> &dictionaries, std::vector<std::thread> &threads, unsigned char countOfThreads) {
		if (countOfThreads < maxCountOfThreads) {
			std::thread newThread([&]() {
				addWordsToDictionaries(block, dictionaries, threads, countOfThreads);
				mergeDictionaries(dictionaries, threads, countOfThreads);
			});
			threads[countOfThreads - 1] = std::move(newThread);
		}
		else {
			addWordsToDictionaries(block, dictionaries, threads, countOfThreads);
			mergeDictionaries(dictionaries, threads, countOfThreads);
		}
	}

	//TEST ME
	void parallelIndexConstruntion(unsigned char countOfThreads) {
		//	prepare threads and dictionaries for work
		std::vector<std::thread> threads;
		threads.resize(countOfThreads - 1);
		std::vector<Dictionary> dictionaries;
		dictionaries.resize(countOfThreads - 1);

		//	we are reading next block and processing current block at the same time
		//	we`re reading from hdd in current thread to reduce positioning time
		size_t indexOfFile{ 0 };
		auto currentBlock{ parseNextBlock(indexOfFile) };
		decltype(currentBlock) nextBlock;
		while (indexOfFile < fileNames.size()) {
			parallelBlockProcessing(currentBlock, dictionaries, threads, countOfThreads - 1);//???
			nextBlock = parseNextBlock(indexOfFile);
			threads.back().join();
			//writeBlockToDisk
			currentBlock = std::move(nextBlock);
		}
		parallelBlockProcessing(currentBlock, dictionaries, threads, countOfThreads);
		//writeBlockToDisk
		//mergeBlocks
	}

	//SERIAL VARIANT
	void serialIndexConstruction() {
		size_t indexOfFile{ 0 };
		Dictionary dictionary;
		Block block;
		while (indexOfFile < fileNames.size()) {
			block = parseNextBlock(indexOfFile);
			dictionary = std::move(getWords(block));
			dictionary.invert();
			//writeBlockToDisk
		}
		//mergeBlocks
	}

public:
	Indexer(std::wstring directory, unsigned char maxCountOfThreads)
		:blockSize{getAvailableVirtualMemory() / 4 }	//for saving two blocks and other structures at the same time
	{
		this->maxCountOfThreads = maxCountOfThreads;
		getFileNames(directory);

		if (maxCountOfThreads > 1) {
			parallelIndexConstruntion(maxCountOfThreads);
		}
		else {
			serialIndexConstruction();
		}
	}

	//WRITE ME
	auto findWord() {
		std::vector<std::pair<size_t, size_t>> result;
		return result;
	}

	//DELETE ME
	void DEBUG() {
		
	}
};

int main() {
	setlocale(0, "");
	
	unsigned char countOfCores = std::thread::hardware_concurrency();
	for (; countOfCores; countOfCores--) {
		Indexer indexer{ LR"(C:\Users\PC\Desktop\Важна інфа\3 курс\2 sem\ПО\aclImdb)", countOfCores };
		indexer.DEBUG();
	}
	return 0;
}