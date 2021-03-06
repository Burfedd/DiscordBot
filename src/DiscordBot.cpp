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

	FILE* f; // File ptr to use with "/record" command

	// Variables to hold .pcm files and voice_client ("/stream" command)
	uint8_t* source = nullptr;
	size_t source_size = 0;
	dpp::discord_voice_client* voice_client = nullptr;

	dpp::snowflake user_id;
	bool MODE_RECORD;

	dpp::cluster bot(botToken);
	bot.on_log(dpp::utility::cout_logger());



	bot.on_interaction_create([&bot, &f, &user_id, &MODE_RECORD, &source, &source_size, &voice_client](const dpp::interaction_create_t& event) {
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

			auto timer = new dpp::timer();
			*timer = bot.start_timer([&bot, timer, event, &f, &MODE_RECORD](){
				MODE_RECORD = false;
				fclose(f);
				event.from->disconnect_voice(event.command.guild_id);
				event.reply("Disconnected from a voice channel");
				bot.stop_timer(*timer);
			}, duration);
		}

		if (cmd == "stop") {
			fclose(f);
			MODE_RECORD = false;
			event.from->disconnect_voice(event.command.guild_id);
			event.reply("Disconnected from a voice channel");
		}

		if (cmd == "stream") {
			MODE_RECORD = false;
			user_id = event.command.usr.id;
			std::string name = "recording";		// default recording filename
			dpp::snowflake author = 0;
			if (std::holds_alternative<std::string>(event.get_parameter("name"))) {
				name = std::get<std::string>(event.get_parameter("name"));
			}
			if (std::holds_alternative<dpp::snowflake>(event.get_parameter("author"))) {
				author = std::get<dpp::snowflake>(event.get_parameter("author"));
			}

			std::string path = "saved_vc/" + std::to_string(author) + "/" + name + ".pcm";

			if (std::filesystem::exists(path)) {
				source = nullptr;
				source_size = 0;
				std::ifstream input(path, std::ios::in | std::ios::binary | std::ios::ate);
				std::cout << input.is_open();
				if (input.is_open()) {
					source_size = input.tellg();
					source = new uint8_t[source_size];
					input.seekg(0, std::ios::beg);
					input.read((char*)source, source_size);
					input.close();
				}

				dpp::guild* g = dpp::find_guild(event.command.guild_id);
				if (!g->connect_member_voice(user_id, false, true)) {
					bot.message_create(dpp::message(event.command.channel_id, "User must be on a voice channel"));
					return;
				}

				auto timer = new dpp::timer();
				*timer = bot.start_timer([&bot, timer, event, &voice_client](){
					if (voice_client != nullptr) {
						if (voice_client->get_secs_remaining() == 0) {
							event.from->disconnect_voice(voice_client->server_id);
							bot.stop_timer(*timer);
						}
					}
				}, 3); // Timer frequency - seconds

			} else {
				event.reply("Specified voice recording from specified author does not exist");
			}
		}
	});

	bot.on_voice_server_update([&bot, &MODE_RECORD](const dpp::voice_server_update_t& event) {
		if (!MODE_RECORD) {
			dpp::voiceconn* v = event.from->get_voice(event.guild_id);
		}
	});

	bot.on_voice_ready([&bot, &MODE_RECORD, &source, &source_size, &voice_client](const dpp::voice_ready_t& event) {
		if (!MODE_RECORD) {
			if (event.voice_client && event.voice_client->is_ready()) {
				voice_client = event.voice_client;
				event.voice_client->send_audio_raw((uint16_t*)source, source_size);
			}
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

			dpp::slashcommand vc_stream_cmd("stream", "Stream other user's recorded voice message", bot.me.id);
			vc_stream_cmd.add_option(dpp::command_option(dpp::co_user, "author", "Author", false));
			vc_stream_cmd.add_option(dpp::command_option(dpp::co_string, "name", "Recording name", false));
			bot.guild_command_create(vc_stream_cmd, GUILD_ID);
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
