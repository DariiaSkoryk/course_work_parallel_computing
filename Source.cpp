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
	//structs

	struct Word : public std::string {
		bool operator<(std::string& anotherWord) {
			return compare(anotherWord) < 0;
		}

		bool operator>(std::string& anotherWord) {
			return compare(anotherWord) > 0;
		}

		bool operator<=(std::string& anotherWord) {
			return compare(anotherWord) <= 0;
		}

		bool operator>=(std::string& anotherWord) {
			return compare(anotherWord) >= 0;
		}

		bool operator==(std::string& anotherWord) {
			return compare(anotherWord) == 0;
		}

		bool operator!=(std::string anotherWord) {
			return compare(anotherWord) != 0;
		}

		Word() = default;

		Word(std::string str)
			:std::string(str)
		{

		};

	private:
		char compare(std::string& anotherWord) {
			char returnValue;
			size_t index{ 0 };
			auto thisWordLength = length();
			auto anotherWordLength = anotherWord.length();
			while ((index < thisWordLength) && (index < anotherWordLength) && (*this)[index] == anotherWord[index]) {
				index++;
			}
			if (thisWordLength == index || anotherWordLength == index) {
				returnValue = thisWordLength > anotherWordLength ? 1 : thisWordLength == anotherWordLength ? 0 : -1;
			}
			else {
				returnValue = (*this)[index] - anotherWord[index];
			}
			return returnValue;
		}
	};

	struct WordInfo {
		Word word;
		std::vector<std::pair<size_t, size_t>> position;

		bool operator<(WordInfo& anotherWord) {
			return word < anotherWord.word;
		}

		bool operator>(WordInfo& anotherWord) {
			return word > anotherWord.word;
		}

		bool operator<=(WordInfo& anotherWord) {
			return word <= anotherWord.word;
		}

		bool operator>=(WordInfo& anotherWord) {
			return word >= anotherWord.word;
		}

		bool operator==(WordInfo& anotherWord) {
			return word == anotherWord.word;
		}

		bool operator!=(WordInfo& anotherWord) {
			return word != anotherWord.word;
		}

		bool operator<(std::string& anotherWord) {
			return word < anotherWord;
		}

		bool operator>(std::string& anotherWord) {
			return word > anotherWord;
		}

		bool operator<=(std::string& anotherWord) {
			return word <= anotherWord;
		}

		bool operator>=(std::string& anotherWord) {
			return word >= anotherWord;
		}

		bool operator==(std::string& anotherWord) {
			return word == anotherWord;
		}

		bool operator!=(std::string& anotherWord) {
			return word != anotherWord;
		}

		WordInfo(std::string word, std::vector<std::pair<size_t, size_t>> position) {
			this->word = word;
			this->position = position;
		}

		WordInfo() = default;
	};

	struct Block {
		auto &operator[](size_t i) {
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
		auto& operator[](size_t i) {
			return dictionary.at(i);
		}

		auto begin() {
			return dictionary.cbegin();
		}

		auto end() {
			return dictionary.cend();
		}

		auto size() {
			return dictionary.size();
		}

		//TEST ME
		void invert() {
			std::sort(dictionary.begin(), dictionary.end());
			size_t fromPosition{ 1 };
			size_t toPosition{ dictionary.size() - 1 };
			while (toPosition) {
				if (dictionary[toPosition] == dictionary[toPosition - 1]) {
					//find interval
					fromPosition = toPosition - 1;
					while (fromPosition && dictionary[fromPosition] == dictionary[fromPosition - 1]) {
						fromPosition--;
					}
					//add all positions to one vector
					for (size_t i{ fromPosition + 1 }; i <= toPosition; i++) {
						dictionary[fromPosition].position.push_back(std::move(dictionary[i].position[0]));
					}
					//delete other vectord
					auto iteratorFrom{ std::next(dictionary.cbegin(), fromPosition + 1) };
					auto iteratorTo{ std::next(dictionary.cbegin(), toPosition + 1) };
					dictionary.erase(iteratorFrom, iteratorTo);
					toPosition = fromPosition ? fromPosition - 1 : 0;
				}
				else {
					toPosition--;
				}
			}
		}

		void push_back(std::string word, size_t indexOfFile, size_t positionInFile) {
			dictionary.push_back({ word, {std::make_pair(indexOfFile, positionInFile)} });
		}

		void clear() {
			dictionary.clear();
		}

	private:
		std::vector<WordInfo> dictionary;

		void invert(int from, int to) {							//done
			int left = from, right = to, middle = (left + right) / 2;
			while (left <= right) {
				while (dictionary[left] < dictionary[middle]) {
					left++;
				}
				while (dictionary[right] > dictionary[middle]) {
					right--;
				}
				if (left <= right) {
					std::swap(dictionary[left], dictionary[right]);
					left++;
					right--;
				}
			}
			if (left < to)
				invert(left, to);
			if (from < right)
				invert(from, right);
		}
	};

	//constants and variables

	const unsigned long long blockSize;
	std::vector<std::wstring> fileNames;
	char maxCountOfThreads;

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
				if (findFileData.cFileName[0] != L'.') {
					if (findFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
						directoryCopy = directory + findFileData.cFileName + L'\\';
						getFileNames(directoryCopy);
					}
					else {
						fileNames.push_back(directory + findFileData.cFileName);
					}
				}
				fileFound = FindNextFile(fileHandle, &findFileData);
			}
		}
	}

	unsigned long long getFileSize(std::wstring path) {
		WIN32_FIND_DATA findFileData;
		HANDLE fileHandle{ FindFirstFile(path.c_str(), &findFileData) };
		return (static_cast<unsigned long long>(MAXDWORD) + 1)* findFileData.nFileSizeHigh + findFileData.nFileSizeLow;
	}

	//C++11

	//	input from file (TAKES ABOUT 30 SECONDS ON TEST DATA)
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

	//	input from file && output to file
	bool mergeBlocks(size_t countOfBlocks) {
		std::ofstream fout("index");
		bool returnValue{ fout.is_open() };
		if (returnValue) {
			if (countOfBlocks > 1) {
				std::vector<std::ifstream> block;
				block.reserve(countOfBlocks);
				for (size_t i{ 0 }; i < countOfBlocks; i++) {
					auto s{ "data" + i};
					std::ifstream fin(s);
					
					if (!fin.is_open()) {
						returnValue = false;
					}
					else if(!fin.eof()) {
						block.push_back(std::move(fin));
					}
				}
				if (returnValue) {
					std::vector<Word> word;
					Word previousWord{ "" };
					word.resize(block.size());
					for (size_t i{ 0 }; i < word.size(); i++) {
						block[i] >> word[i];
					}

					size_t index;
					while (!block.empty()) {
						//find minimal word
						index = 0;
						for (size_t i{ 1 }; i < word.size(); i++) {
							if (word[i] < word[index]) {
								index = i;
							}
						}

						//write minimal word
						if (word[index] != previousWord) {
							fout << std::endl << word[index];
							previousWord = std::move(word[index]);
						}
						std::getline(block[index], word[index]);
						fout << ' ' << word[index];
						if (block[index].eof()) {
							block[index].close();
							block[index] = std::move(block.back());
							block.pop_back();
							word[index] = std::move(word[index]);
							word.pop_back();
						}
						else {
							block[index] >> word[index];
						}
					}
				}
			}
			else {
				std::ifstream fin("data0");
				if (fin.is_open()) {
					std::string line;
					while (!fin.eof()) {
						std::getline(fin, line);
						fout << line;
					}
					fin.close();
				}
				else {
					returnValue = false;
				}
			}
			fout.close();
		}
		return returnValue;
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

	//PARALLEL VARIANT

	void writeDictionaries(std::vector<Dictionary>& dictionaries, std::string outputFileName) {
		//find count of words we shouls write
		size_t remainingCount{ 0 };
		for (size_t i{ 0 }; i < dictionaries.size(); i++) {
			remainingCount += dictionaries[i].size();
		}

		//create index for each dictionary
		std::vector<size_t> index(dictionaries.size(), 0);

		std::ofstream fout(outputFileName);
		if (fout.is_open()) {
			size_t current;
			std::string previousWord{ "" };
			
			while (remainingCount) {
				//find minimal word to write
				current = 0;
				for (size_t i{ 1 }; i < dictionaries.size(); i++) {
					if (dictionaries[i][index[i]] < dictionaries[current][index[current]]) {
						current = i;
					}
				}

				//write one word from current dictionary
				if (dictionaries[current][index[current]] != previousWord) {
					fout << std::endl << dictionaries[current][index[current]].word;
					previousWord = std::move(dictionaries[current][index[current]].word);
				}
				for (const auto& entry : dictionaries[current][index[current]].position) {
					fout << ' ' << entry.first << ' ' << entry.second;
				}

				index[current]++;
				if (index[current] == dictionaries[current].size()) {
					//delete element from back to avoid moving elements from current to last, it works faster 
					dictionaries[current] = std::move(dictionaries.back());
					dictionaries.pop_back();
					index[current] = std::move(index.back());
					index.pop_back();
				}
				remainingCount--;
			}
			fout.close();
		}
	}

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

	void parallelBlockProcessing(Block &block, std::vector<Dictionary> &dictionaries, std::vector<std::thread> &threads, unsigned char countOfThreads) {
		if (countOfThreads < maxCountOfThreads) {
			std::thread newThread([&]() {
				addWordsToDictionaries(block, dictionaries, threads, countOfThreads);
			});
			threads[countOfThreads - 1] = std::move(newThread);
		}
		else {
			addWordsToDictionaries(block, dictionaries, threads, countOfThreads);
		}
	}

	void parallelIndexConstruntion(unsigned char countOfThreads) {
		//	prepare threads and dictionaries for work
		std::vector<std::thread> threads;
		threads.resize(countOfThreads - 1);
		std::vector<Dictionary> dictionaries;
		dictionaries.resize(countOfThreads);

		//	we are reading next block and processing current block at the same time
		//	we`re reading from hdd in current thread to reduce positioning time
		size_t indexOfFile{ 0 };
		auto currentBlock{ parseNextBlock(indexOfFile) };
		decltype(currentBlock) nextBlock;
		size_t blockNumber{ 0 };
		while (indexOfFile < fileNames.size()) {
			parallelBlockProcessing(currentBlock, dictionaries, threads, countOfThreads - 1);//???
			nextBlock = parseNextBlock(indexOfFile);
			threads.back().join();
			writeDictionaries(dictionaries, "data" + blockNumber);
			blockNumber++;
			currentBlock = std::move(nextBlock);
		}
		parallelBlockProcessing(currentBlock, dictionaries, threads, countOfThreads);
		writeDictionaries(dictionaries, "data" + blockNumber);
		mergeBlocks(blockNumber);
	}

	//SERIAL VARIANT
	
	void writeDictionary(Dictionary& dictionary, std::string outputFileName) {
		std::ofstream fout(outputFileName);
		if (fout.is_open()) {
			for (const auto& entry : dictionary) {
				fout << std::endl << entry.word;
				for (const auto& position : entry.position) {
					fout << ' ' << position.first << ' ' << position.second;
				}
			}
			fout.close();
		}
	}

	void serialIndexConstruction() {
		size_t indexOfFile{ 0 };
		Dictionary dictionary;
		Block block;
		size_t blockNumber{ 0 };
		while (indexOfFile < fileNames.size()) {
			block = parseNextBlock(indexOfFile);
			dictionary = std::move(getWords(block));
			dictionary.invert();
			writeDictionary(dictionary, "data" + blockNumber);
			blockNumber++;
		}
		mergeBlocks(blockNumber);
	}

public:
	Indexer(std::wstring directory, unsigned char maxCountOfThreads)
		:blockSize{getAvailableVirtualMemory() / 4 }	//for saving two blocks and other structures at the same time
	{
		this->maxCountOfThreads = maxCountOfThreads;
		getFileNames(directory);

		/*if (maxCountOfThreads > 1) {
			parallelIndexConstruntion(maxCountOfThreads);
		}
		else {
			serialIndexConstruction();
		}*/
	}

	//WRITE ME
	/*auto findWord() {
		std::vector<std::pair<size_t, size_t>> result;
		return result;
	}*/

	//DELETE ME
	void DEBUG() {
		size_t index{ 0 };
		auto block{ parseNextBlock(index) };
		std::vector<Dictionary> dictionary{ getWords(block, 0, block.size() / 2), getWords(block, block.size() / 2) };
		dictionary[0].invert();
		dictionary[1].invert();
		writeDictionaries(dictionary, "data0");
		writeDictionaries(dictionary, "data1");
		mergeBlocks(2);
		//writeDictionary(words, R"(D:\Programming\Programs\course_work_parallel_computing\Debug\Test_files\dictionary.txt)");
		/*for (int i = 0; i < words1.size(); i++) {
			std::cout << i << '\t' << words1[i].word;
			for (const auto& entry : words1[i].position) {
				std::cout << std::endl << '\t' << entry.first << ' ' << entry.second;
			}
			std::cout << std::endl << std::endl;
		}
		std::cout << std::endl << std::endl;
		for (int i = 0; i < words2.size(); i++) {
			std::cout << i << '\t' << words2[i].word;
			for (const auto& entry : words2[i].position) {
				std::cout << std::endl << '\t' << entry.first << ' ' << entry.second;
			}
			std::cout << std::endl << std::endl;
		}*/
	}
};

int main() {
	setlocale(0, "");
	
	Indexer indexer{ LR"(D:\Programming\Programs\course_work_parallel_computing\Debug\Test_files)", 1 };
	indexer.DEBUG();
	/*unsigned char countOfCores = std::thread::hardware_concurrency();
	for (; countOfCores; countOfCores--) {
		Indexer indexer{ LR"(C:\Users\PC\Desktop\Важна інфа\3 курс\2 sem\ПО\aclImdb)", countOfCores };
	}*/
	
	return 0;
}