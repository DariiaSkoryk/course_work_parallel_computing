#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <Windows.h>

class Indexer {
private:
	
	const unsigned long long blockSize;

	//WINAPI
	//OK
	size_t getAvailableVirtualMemory() {
		MEMORYSTATUSEX memoryStatus;
		memoryStatus.dwLength = sizeof(memoryStatus);
		GlobalMemoryStatusEx(&memoryStatus);
		return memoryStatus.ullAvailVirtual;
	}

	//OK
	void getFileNames(std::wstring &directory, std::vector<std::pair<std::wstring, unsigned long long>> &fileNames) {
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
						getFileNames(directoryCopy, fileNames);
					}
					else {
						fileNames.push_back(std::make_pair(directory + findFileData.cFileName,
							(static_cast<unsigned long long>(MAXDWORD) + 1)* findFileData.nFileSizeHigh + findFileData.nFileSizeLow));
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
		for (int i = 0; i < word.length(); i++) {
			if (word[i] >= 'A' && word[i] <= 'Z') {
				word[i] += 'A' - 'a';
			}
		}
	}

	//CHECK ME
	auto parseNextBlock(std::vector<std::wstring> fileNames, int index) {
		std::map<std::string, std::vector<int>> dictionary;
		std::string currentWord;
		int currentSize{ 0 };
		auto nextFileSize{ getFileSize(fileNames[index]) };

		while (index < fileNames.size() && currentSize + nextFileSize < blockSize) {
			std::ifstream inputFile(fileNames[index]);
			
			while (!inputFile.eof()) {
				inputFile >> currentWord;
				tolower(currentWord);
				dictionary.insert(std::make_pair(currentWord, dictionary.size()));
			}

			inputFile.close();
			currentSize += nextFileSize;
			index++;
		}
		return dictionary;
	}

	//WRITE ME
	void writeIndex() {

	}

	//FIX ME
	/*void invert() {
		//std::sort(dictionary.begin, dictionary.end, [](std::pair<std::string, std::vector<int>> &first, std::pair<std::string, std::vector<int>> &second) {return (bool)first.first.compare(second.first); });
		//We are searching from back to avoid iterator invalidation
		//auto and decltype are used to avoid std::map<std::string, std::vector<int>> 

		auto reverseFirstSimilarWord{ std::adjacent_find(dictionary.rbegin(), dictionary.rend()) };
		decltype(reverseFirstSimilarWord) reverseLastSimilarWord;

		while (reverseFirstSimilarWord != dictionary.rend()) {			
			reverseLastSimilarWord = std::make_reverse_iterator(dictionary.find(reverseFirstSimilarWord->first));
			
			//I don`t know if this loop works correctly. TEST IT !!!
			for (auto &entry = reverseFirstSimilarWord; entry != reverseLastSimilarWord; entry++) {
				//remember position of this word
				reverseLastSimilarWord->second.push_back(entry->second.at(0));
			}

			//delete similar words from list except one
			dictionary.erase((reverseLastSimilarWord - 1).base(), reverseFirstSimilarWord.base());
			//find another similar word
			reverseFirstSimilarWord = std::adjacent_find(dictionary.rbegin(), dictionary.rend());
		}
	}*/


public:
	Indexer()
		:blockSize{getAvailableVirtualMemory() / 2 }	//other memory for saved structure and other operations
	{

	}

	//WRITE ME
	void BSBI(std::initializer_list<std::wstring> directory) {
		//parseNextBlock();

	}

	//WRITE ME
	void search(std::string word) {

	}

	//DELETE ME
	void DEBUG() {
		
	}
};

int main() {
	Indexer indexer{};
	indexer.DEBUG();
	return 0;
}