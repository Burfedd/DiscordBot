#include "DiscordBot.h"
#include <iostream>
#include <dpp/dpp.h>
#include <dpp/fmt/format.h>

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
		if (event.command.get_command_name() == "blep") {
			std::string animal = std::get<std::string>(event.get_parameter("animal"));
			event.reply(fmt::format("Blep! You chose {}", animal));
		}
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand newcommand("blep", "Send a random adorable animal photo", bot.me.id);
			newcommand.add_option(
				dpp::command_option(dpp::co_string, "animal", "The type of animal", true).
				add_choice(dpp::command_option_choice("Cat", std::string("animal_cat"))).
				add_choice(dpp::command_option_choice("Dog", std::string("animal_dog"))).
				add_choice(dpp::command_option_choice("Penguin", std::string("animal_penguin")))
			);

			bot.global_command_create(newcommand);
		}
	});

	bot.start(false);
}
