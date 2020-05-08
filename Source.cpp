#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <Windows.h>

class Indexer {
private:
	struct Consideration {
		const std::wstring directory;
		const int fromFile;
		const int toFile;

		Consideration(std::wstring directory, int fromFile, int toFile)
			:	directory{directory}, 
				fromFile{fromFile}, 
				toFile{toFile}
		{
		}

		std::vector<std::wstring> getFileNames() {
			std::vector<std::wstring> result;

			if (this->directory.length() != 0) {
				const WCHAR* fileName{ this->directory.c_str() };
				WIN32_FIND_DATA findFileData;
				HANDLE fileHandle{ FindFirstFile(fileName, &findFileData) };
				int fileNumber{ 1 };
				BOOL fileFound{ true };
				while (fileNumber < this->fromFile && fileFound) {
					fileFound = FindNextFile(fileHandle, &findFileData);
					fileNumber++;
				}
				if (this->toFile) {
					while (fileNumber < this->toFile && fileFound) {
						fileFound = FindNextFile(fileHandle, &findFileData);
						result.push_back(findFileData.cFileName);
						fileNumber++;
					}
				}
				else {
					while (fileFound) {
						fileFound = FindNextFile(fileHandle, &findFileData);
						result.push_back(findFileData.cFileName);
						fileNumber++;
					}
				}
			}
			return std::move(result);
		}
	};
	
	const int blockSize;
	std::map<std::string, std::vector<int>> dictionary;
	std::vector<std::wstring> path;
	int indexOfPath;

	void parseNextBlock() {
		std::string currentWord;
		while (dictionary.size < blockSize && indexOfPath < path.size()) {
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
	}

	void writeIndex() {

	}

	void invert() {
		std::sort(dictionary.begin, dictionary.end);
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
	}


public:

	void BSBI(std::initializer_list<Consideration> filesInConsideration) {
		
	}

	void search(std::string word) {

	}
	
	Consideration addToConsideration(std::wstring directory) {
		return std::move(Consideration(directory, 0, 0));
	}

	Consideration addToConsideration(std::wstring directory, int fromFile, int toFile) {
		return std::move(Consideration(directory, fromFile, toFile));
	}
};

int main() {	
	return 0;
}