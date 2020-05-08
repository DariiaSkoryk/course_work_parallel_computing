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
		
	}

	void writeIndex() {

	}

	void invert() {}


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
	std::ifstream in(LR"(C:\Users\PC\Desktop\aclImdb\test\neg\3750_2.txt)");
	if (in.is_open()) {
		std::cout << (char)in.get();
	}
	in.close();
	return 0;
}