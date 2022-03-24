#include "DiscordBot.h"

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

	dpp::cluster bot(botToken);
	bot.on_log(dpp::utility::cout_logger());



	bot.on_interaction_create([&bot](const dpp::interaction_create_t& event) {
		std::string cmd = event.command.get_command_name();

		if (cmd == "dice" || cmd == "roll") {
			std::string result = std::to_string(cmd_roll());
			event.reply("Rolling dice... Result: " + result);
		}
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.guild_command_create(dpp::slashcommand("roll", "Rolling a random number between 1 and 100 or between specified boundaries", bot.me.id), GUILD_ID);
			bot.guild_command_create(dpp::slashcommand("dice", "Rolling a random number between 1 and 100 or between specified boundaries", bot.me.id), GUILD_ID);
		}
	});

	bot.start(false);
}

int cmd_roll() {
	return cmd_roll(0, 100);
}

int cmd_roll(int a, int b) {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(a, b);
	int random_integer = uni(rng);
	return random_integer;
}
