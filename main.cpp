#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <iostream>
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

uintmax_t GetDirectorySize(const fs::path& dir_path)
{
	uintmax_t size = 0;

	for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
		if (fs::is_regular_file(entry.status())) {
			size += fs::file_size(entry.path());
		}
	}

	return size;
}

std::vector<std::string> GetSingleFilenames(std::string inputText)
{
	std::istringstream iss(inputText);

	std::vector<std::string> words;

	std::string word;
	while (iss >> word) {
		words.push_back(word);
	}

	return words;
}

bool CheckExcludedFilenames(std::string inputText, std::string fileName)
{
	std::vector<std::string> singleNames = GetSingleFilenames(inputText);
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
	bool firstTime = true;
	for (std::string singleName : singleNames)
	{
		std::transform(singleName.begin(), singleName.end(), singleName.begin(), ::tolower);

		size_t found = fileName.find(singleName);
		if (found != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

void HandleDirectories(const std::string& folderPath, const std::string folderName, bool& continueToDesktopIteration)
{
	if (!fs::exists(folderPath))
	{
		std::cout << "Creating " << folderName << " directory.." << std::endl;
		if (fs::create_directory(folderPath))
		{
			std::cout << folderName << " directory created." << std::endl << std::endl;
		}
		else
		{
			std::cout << "Error creating " << folderName << " directory." << std::endl << std::endl;
			continueToDesktopIteration = false;
		}
	}
	else
	{
		std::cout << folderName << " directory already exists." << std::endl << std::endl;
	}
}

void OrganizeDesktopFiles(std::string inputText)
{

	bool continueToDesktopIteration = true;
	PWSTR path;

	std::string desktopStr;

	std::string folderStoragePath;
	std::string folderStorageName = "Folders";

	std::string fileStoragePath;
	std::string fileStorageName = "Files";

	std::string textDocsStoragePath;
	std::string textDocsStorageName = "TextDocs";

	std::string photosStoragePath;
	std::string photosStorageName = "Photos";

	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &path)))
	{
		std::wstring wideString(path);
		std::string str(wideString.begin(), wideString.end());
		desktopStr = str;

		CoTaskMemFree(static_cast<void*>(path));

		folderStoragePath = desktopStr + "\\Folders";
		fileStoragePath = desktopStr + "\\Files";
		textDocsStoragePath = desktopStr + "\\TextDocs";
		photosStoragePath = desktopStr + "\\Photos";

		HandleDirectories(folderStoragePath, folderStorageName, continueToDesktopIteration);
		HandleDirectories(fileStoragePath, fileStorageName, continueToDesktopIteration);
		HandleDirectories(textDocsStoragePath, textDocsStorageName, continueToDesktopIteration);
		HandleDirectories(photosStoragePath, photosStorageName, continueToDesktopIteration);
	}
	else
	{
		std::cout << "Desktop not found." << std::endl;
		continueToDesktopIteration = false;
	}

	if (continueToDesktopIteration)
	{
		for (const auto& entry : fs::directory_iterator(desktopStr))
		{
			const fs::path& filePath = entry.path();
			fs::path fileName = filePath.filename();

			if (CheckExcludedFilenames(inputText, fileName.string()))
			{
				continue;
			}

			if (fs::is_directory(filePath))
			{


				if (fs::is_empty(filePath) && fileName != "TextDocs" && fileName != "Folders" && fileName != "Files" && fileName != "Photos")
				{

					try
					{
						fs::remove_all(filePath);
						std::cout << "Folder " << fileName << " deleted." << std::endl;
					}
					catch (const std::filesystem::filesystem_error& e)
					{
						std::cout << "Folder " << fileName << " couldnt be deleted. Error: " << e.what() << std::endl;
					}

				}
				else if (!fs::is_empty(filePath) && fileName != "TextDocs" && fileName != "Folders" && fileName != "Files" && fileName != "Photos")
				{
					uintmax_t size = GetDirectorySize(filePath); //1422091824 - biggest
					if (size > 50000000)
					{
						continue;
					}
					fs::path dirName = "Folders";
					fs::path sourcePath = desktopStr / fileName;
					fs::path destinationPath = desktopStr / dirName / fileName;

					fs::copy(sourcePath, destinationPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
					fs::remove_all(filePath);
				}
			}
			else if (fs::is_regular_file(filePath))
			{
				fs::path extension = fileName.extension();

				if (extension == ".txt")
				{
					fs::path dirName = "TextDocs";
					fs::rename(filePath, desktopStr / dirName / fileName);
					std::cout << "Text document " << fileName << " moved to TextDocs folder." << std::endl;
				}
				else if
					(
						extension == ".jpg" ||
						extension == ".png" ||
						extension == ".svg" ||
						extension == ".jpeg" ||
						extension == ".tiff" ||
						extension == ".tif" ||
						extension == ".gif" ||
						extension == ".raw"
						)
				{
					fs::path dirName = "Photos";
					fs::rename(filePath, desktopStr / dirName / fileName);
					std::cout << "Photo " << fileName << " moved to Photos folder." << std::endl;
				}

				else
				{
					fs::path dirName = "Files";
					fs::rename(filePath, desktopStr / dirName / fileName);
					std::cout << "Text document " << fileName << " moved to TextDocs folder." << std::endl;
				}
			}
		}
	}
}

int main() {
	int screenWidth = 1000;
	int screenHeight = 500;

	sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Desktop Cleaner", sf::Style::Close);

	sf::Font font;
	if (!font.loadFromFile("Kanit-SemiBold.ttf")) {
		std::cerr << "Error loading font file." << std::endl;
		return 1;
	}

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(18);
	text.setFillColor(sf::Color::Black);

	sf::Text dcText;
	dcText.setFont(font);
	dcText.setString("Desktop Cleaner");
	dcText.setCharacterSize(35);
	dcText.setFillColor(sf::Color::Black);
	dcText.setPosition(screenWidth / 2 - 120, 50);

	sf::Text cleanText;
	cleanText.setFont(font);
	cleanText.setString("Specify files you want to keep on desktop (e.g.: chrome MINECRAFT SpOtifY).");
	cleanText.setCharacterSize(25);
	cleanText.setFillColor(sf::Color::Black);
	cleanText.setPosition(20, 200);

	sf::String inputText;

	sf::RectangleShape button(sf::Vector2f(screenWidth / 4, screenHeight / 5));
	button.setFillColor(sf::Color::Green);
	button.setPosition((screenWidth - screenWidth / 4) / 2, screenHeight - 170);

	sf::Text buttonText("Start", font, 35);
	buttonText.setFillColor(sf::Color::White);
	buttonText.setPosition(button.getPosition().x + (button.getSize().x - buttonText.getLocalBounds().width) / 2,
		button.getPosition().y + (button.getSize().y - buttonText.getLocalBounds().height) / 2 - 10);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::TextEntered) {
				if (event.text.unicode < 128) {
					if (event.text.unicode == 8 && !inputText.isEmpty()) {
						inputText.erase(inputText.getSize() - 1, 1);
					}
					else if (event.text.unicode >= 32) {
						inputText += event.text.unicode;
					}
					text.setString(inputText);
				}
			}

			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::FloatRect buttonBounds = button.getGlobalBounds();
				if (buttonBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
					std::cout << "Button clicked! Entered text: " << inputText.toAnsiString() << std::endl;
					OrganizeDesktopFiles(inputText);
				}
			}

			if (event.type == sf::Event::MouseMoved) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::FloatRect buttonBounds = button.getGlobalBounds();

				if (buttonBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
					button.setFillColor(sf::Color::Cyan);
				}
				else {
					button.setFillColor(sf::Color::Green);
				}
			}
		}

		window.clear(sf::Color::Blue);

		sf::RectangleShape inputBox(sf::Vector2f(screenWidth - 20, 30));
		inputBox.setFillColor(sf::Color::Transparent);
		inputBox.setOutlineColor(sf::Color::Black);
		inputBox.setOutlineThickness(2);
		inputBox.setPosition(10, screenHeight - 250);

		sf::FloatRect textBounds = text.getLocalBounds();
		text.setPosition(20, screenHeight - 245);

		window.draw(inputBox);
		window.draw(text);
		window.draw(dcText);
		window.draw(cleanText);

		window.draw(button);
		window.draw(buttonText);

		window.display();
	}

	return 0;
}





