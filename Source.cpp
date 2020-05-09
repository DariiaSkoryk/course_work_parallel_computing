#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <Windows.h>

class Indexer {
private:
	
	const int blockSize;
	std::map<std::string, std::vector<int>> dictionary;

	void getFileNames(std::wstring &directory, std::vector<std::wstring> &fileNames) {
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
						fileNames.push_back(directory + findFileData.cFileName);
					}
				}
			}
		}
	}

	int parseNextBlock(std::vector<std::wstring> path, int index) {
		std::string currentWord;
		int indexOfPath;
		while (dictionary.size() < blockSize && indexOfPath < path.size()) {
			std::ifstream inputFile(path[indexOfPath]);
			indexOfPath++;
			while (!inputFile.eof()) {
				inputFile >> currentWord;
				for (auto& entry : currentWord) {
					tolower(entry);
				}
				dictionary.insert(std::make_pair(currentWord, dictionary.size()));
			}
			inputFile.close();
		}
		return indexOfPath;
	}

	void writeIndex() {

	}

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
		:blockSize{65536}
	{

	}

	void BSBI(std::initializer_list<std::wstring> directory) {
		//parseNextBlock();

	}

	void search(std::string word) {

	}

	void DEBUG() {
		std::wstring str{ LR"(C:\Users\PC\Desktop\testdir)" };
		std::vector<std::wstring> fileNames;
		getFileNames(str, fileNames);
	}
};

int main() {
	Indexer indexer{};
	indexer.DEBUG();
	return 0;
}