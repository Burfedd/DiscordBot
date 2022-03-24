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

		if (cmd == "roll") {
			int64_t lower = 0;
			int64_t upper = 100;
			if (std::holds_alternative<int64_t>(event.get_parameter("lower"))) {
				lower = std::get<int64_t>(event.get_parameter("lower"));
			}
			if (std::holds_alternative<int64_t>(event.get_parameter("upper"))) {
				upper = std::get<int64_t>(event.get_parameter("upper"));
			}
			if (lower < upper) {
				std::string result = std::to_string(cmd_roll(lower, upper));
				event.reply("Rolling [" + std::to_string(lower) + ";" + std::to_string(upper) + "]... Result: " + result);
			} else {
				event.reply("Upper boundary cannot be less or equal to lower boundary!");
			}
		}
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand roll_cmd("roll", "Roll a random number between 1 and 100 or between specified boundaries", bot.me.id);
			roll_cmd.add_option(dpp::command_option(dpp::co_integer, "lower", "Lower boundary", false));
			roll_cmd.add_option(dpp::command_option(dpp::co_integer, "upper", "Upper boundary", false));
			bot.guild_command_create(roll_cmd, GUILD_ID);
		}
	});

	bot.start(false);
}

int cmd_roll(int a, int b) {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(a, b);
	int random_integer = uni(rng);
	return random_integer;
}
