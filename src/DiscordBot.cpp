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

	FILE* f;
	dpp::snowflake user_id;
	bool MODE_RECORD;

	dpp::cluster bot(botToken);
	bot.on_log(dpp::utility::cout_logger());



	bot.on_interaction_create([&bot, &f, &user_id, &MODE_RECORD](const dpp::interaction_create_t& event) {
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

		if (cmd == "record") {
			MODE_RECORD = true;
			user_id = event.command.usr.id;
			std::string name = "recording";		// default recording filename
			int64_t duration = 3;				// default recording duration
			if (std::holds_alternative<std::string>(event.get_parameter("name"))) {
				name = std::get<std::string>(event.get_parameter("name"));
			}
			if (std::holds_alternative<int64_t>(event.get_parameter("duration"))) {
				if (duration <= 10) {
					duration = std::get<int64_t>(event.get_parameter("duration"));
				}
			}

			std::string path_userid = std::to_string(user_id);
			if (!std::filesystem::exists("saved_vc" + path_userid)) {
				std::filesystem::create_directories("saved_vc/" + path_userid);
			}

			std::string fullpath = "./saved_vc/" + path_userid + "/" + name + ".pcm";
			f = fopen(fullpath.c_str(), "wb");

			dpp::guild* g = dpp::find_guild(event.command.guild_id);
			if (!g->connect_member_voice(user_id, true, false)) {
				event.reply("User must be on a voice channel");
				return;
			}
			event.reply("Connected to the voice channel, started recording: \"" + name + "\"");

			dpp::oneshot_timer(&bot, duration, [&event, &f, &MODE_RECORD](){
				MODE_RECORD = false;
				fclose(f);
				event.from->disconnect_voice(event.command.guild_id);
				event.reply("Disconnected from a voice channel");
			});
		}

		if (cmd == "stop") {
			fclose(f);
			MODE_RECORD = false;
			event.from->disconnect_voice(event.command.guild_id);
			event.reply("Disconnected from a voice channel");
		}
	});

	bot.on_voice_receive([&bot, &f, &user_id, &MODE_RECORD](const dpp::voice_receive_t& event) {
		if (MODE_RECORD) {
			if (event.user_id == user_id) {
				fwrite((char*)event.audio, 1, event.audio_size, f);
			}
		}
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand roll_cmd("roll", "Roll a random number between 1 and 100 or between specified boundaries", bot.me.id);
			roll_cmd.add_option(dpp::command_option(dpp::co_integer, "lower", "Lower boundary", false));
			roll_cmd.add_option(dpp::command_option(dpp::co_integer, "upper", "Upper boundary", false));
			bot.guild_command_create(roll_cmd, GUILD_ID);

			dpp::slashcommand vc_record_cmd("record", "Record you for a set amount of time and save the recorded .pcm", bot.me.id);
			vc_record_cmd.add_option(dpp::command_option(dpp::co_integer, "duration", "Duration of the recording", false));
			vc_record_cmd.add_option(dpp::command_option(dpp::co_string, "name", "Recording name", false));
			bot.guild_command_create(vc_record_cmd, GUILD_ID);

			dpp::slashcommand vc_stop_cmd("stop", "Stop recording", bot.me.id);
			bot.guild_command_create(vc_stop_cmd, GUILD_ID);
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
