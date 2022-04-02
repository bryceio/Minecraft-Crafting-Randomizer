#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <new>
#include <random>
#include <ctime>
#include <filesystem>

std::string findReplace(std::string inputString, std::string findString, std::string replaceString) {
	if (inputString.find(findString) >= 250000) return inputString;
	return inputString.replace(inputString.find(findString), findString.length(), replaceString);
}

int main() {
	//Gets confirmation from user and datapack format number.
	std::cout << "Make sure all your files are located within a folder called \"recipes\" before continuing." << std::endl;
	std::cout << "WARNING: This process will overwrite the old recipes. Are you sure you want to continue?" << std::endl;
	bool confirmation = false;
	char response;
	do {
		std::cout << "Y/N?\n";
		std::cin >> response;
		if (response == 'Y' || response == 'y') confirmation = true;
		else if (response == 'N' || response == 'n') {
			std::cout << "Randomization process canceled." << std::endl;
			return 1;
		}
	} while (!confirmation);
	std::cout << "\nWhat is the datapack format number you would like to use? Please check the MC Wiki if you are unsure." << std::endl;
	int packFormat;
	std::cin >> packFormat;
	std::cout << "\nBeginning randomization process." << std::endl;

	//Seeds the randomization
	for (int i = 0; i <= time(nullptr) % 4206942; i++) rand();

	//Counts the files 
	int i = 0;
	std::ifstream inputStream;
	std::ofstream outputStream;
	std::cout << "Counting files." << std::endl;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("recipes")) {
		if (!dirEntry.is_directory()) i++;
	}

	//Sets up variables
	std::string* fileContent = new std::string[i+1];
	std::string* recipeOutput = new std::string[i+1];
	bool* needsComma = new bool[i+1];
	bool isCraftingTable;
	int returnStart;
	std::string currentLine;
	int fileCount = i;

	//Reads the files, separates the recipes from the output, and flags what recipes need to have a comma appended to their output.
	i = 0;
	std::cout << "Reading files." << std::endl;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("recipes")) {
		if (!dirEntry.is_directory()) {
			inputStream.open(dirEntry.path().string());
			getline(inputStream, currentLine);
			fileContent[i] = currentLine;
			isCraftingTable = false;
			do {
				getline(inputStream, currentLine);
				if ((currentLine.find("minecraft:crafting_shape") <= 500 || currentLine.find("minecraft:smithing") <= 500) && currentLine.find("\"type\"") <= 500) isCraftingTable = true;
				if (currentLine.find("\"result\"") <= 500) {
					if (isCraftingTable) {
						fileContent[i] += '\n' + currentLine;
						getline(inputStream, currentLine);
						returnStart = currentLine.find("\"minecraft:");
						fileContent[i] += '\n' + currentLine.substr(0, returnStart) + "&&&";
						recipeOutput[i] = findReplace(currentLine.substr(returnStart), ",", "");
						getline(inputStream, currentLine);
						if (currentLine.find('"') <= 500) {
							needsComma[i] = true;
							fileContent[i] += '\n' + currentLine;
						}
						else {
							needsComma[i] = false;
							fileContent[i] += '\n' + currentLine;
						}
					}
					else {
						returnStart = currentLine.find("\"minecraft:");
						fileContent[i] += '\n' + currentLine.substr(0, returnStart) + "&&&";
						needsComma[i] = true;
						recipeOutput[i] = findReplace(currentLine.substr(returnStart), ",", "");
					}
				}else fileContent[i] += '\n' + currentLine;
			} while (!inputStream.eof());
			i++;
			inputStream.close();
		}
	}

	//This creates a debug file for checking for any recipe files that need to be removed.
	//If this output file has a blank line in it, that means there is a file that needs to be removed.
	outputStream.open("RecipeDebugLog.txt");
	for (int i = 0; i < fileCount; i++) {
		outputStream << recipeOutput[i] << std::endl;
	}

	//Process of matching up recipes with outputs.
	std::cout << "Randomizing Files\n";
	i = 0;
	bool badChoice;
	int choice;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("recipes")) {
		if (dirEntry.is_directory()) continue;
		outputStream.open(dirEntry.path().string());
		badChoice = true;
		do {
			choice = rand() % fileCount;
			if (rand() % 1000 <= 5) std::cout << '.';
			if (recipeOutput[choice] != "&") badChoice = false;
		} while (badChoice);
		if (needsComma[i]) recipeOutput[choice] += ',';
		outputStream << findReplace(fileContent[i], "&&&", recipeOutput[choice]);
		recipeOutput[choice] = "&";
		outputStream.close();
		outputStream.clear();
		i++;
	}
	outputStream.close();
	outputStream.clear();
	std::cout << "\nRandomization Complete!" << std::endl;

	//Clears up the arrays and creates pack.mcmeta
	delete[] fileContent;
	delete[] recipeOutput;
	delete[] needsComma;
	outputStream.open("pack.mcmeta");
	outputStream << "{\n   \"pack\" : {\n      \"pack_format\": " << packFormat << ",\n";
	outputStream << "      \"description\": \"A randomized recipe pack made using bryceio\'s randomizer.\"\n   }\n}" << std::endl;
	outputStream.close();
	outputStream.clear();
	return 0;
}