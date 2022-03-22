#include "DiscordBot.h"
#include <iostream>
#include <dpp/dpp.h>

const dpp::snowflake GUILD_ID = 889928376666710037;

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
	
	const std::string token = botToken;
	dpp::cluster bot(token);

	bot.on_interaction_create([](const dpp::interaction_create_t& event) {
		if (event.command.get_command_name() == "ping") {
			event.reply("Pong!");
		}
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.guild_command_create(dpp::slashcommand("ping", "Ping Pong!", bot.me.id), GUILD_ID);
		}
	});

	bot.start(false);
}
