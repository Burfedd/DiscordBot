#include "DiscordBot.h"
#include <iostream>
#include <dpp/dpp.h>

int main()
{
	std::string botToken;
	std::ifstream file("../token.txt");
	if (file.is_open()) {
		std::getline(file, botToken);
	}
	file.close();
	if (botToken == "Paste_your_token_here") {
		std::cout << "You didn't specify the bot token! Aborting..." << std::endl;
		return 0;
	}

	std::cout << "Bot token: " << botToken << std::endl;
	return 0;
}
