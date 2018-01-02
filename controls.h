#ifndef CONTROLS
#define CONTROLS

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include "json.hpp"
#include <SDL.h>
using json = nlohmann::json;
using namespace std;

//Controls class
class Controls
{
public:

	struct Player
	{

		bool useJoystick = false;
		int joystickId = 0;
		map<string, bool> keys;
		map<string, string> keyNames;
		const char* name;
		Player(const char* playerName)
		{
			keys["left"] = false;
			keys["right"] = false;
			keys["up"] = false;
			keys["down"] = false;
			keys["action"] = false;

			keyNames["left"] = "Left";
			keyNames["right"] = "Right";
			keyNames["up"] = "Up";
			keyNames["down"] = "Down";
			keyNames["action"] = "Right Shift";

			name = playerName;
		}

		//string name;
	};

	struct Mouse
	{
		int x, y;
		bool lbutton;
		Mouse() : x(0), y(0), lbutton(false) {}
	};

	Mouse mouse;
	vector<Player*> players;
	bool p_return = false, p_pause = false;
	unsigned int lastAction = 0;

	void Serialise()
	{
		//The output stream (to a file)
		std::ofstream outStream;
		//Open the file for writing
		outStream.open(fname, std::ios::out);
		if (outStream.fail())
		{
			SDL_Log("Could not find %s for serialisation.", fname);
		}
		else
		{
			SDL_Log("Successfully opened options file %s for serialisation.", fname);
			//Create an empty json object that we're going to save
			json outObject;
			//Set up fields to save
			for (int i = 0; i < players.size(); i++)
			{
				Player* player = players[i];
				map<string, string>* myMap = &player->keyNames;
				for (auto& it = myMap->begin(); it != myMap->end(); ++it)
				{
					outObject["controls"][player->name][it->first] = it->second;
					SDL_Log("Set controls:%s:%s of player %i to %s", player->name , it->first.c_str(), i, it->second.c_str());
				}
				outObject["controls"][player->name]["useJoystick"] = player->useJoystick;
			}
			//Write to the file -- use dump to get the JSON text
			outStream << outObject.dump(3);

			//And close the file! Very important!
			SDL_Log("Closed %s", fname);
			outStream.close();
		}
	}

	//De-serialise (load)
	void Deserialise()
	{
		//The input stream (to a file)
		std::ifstream inStream;
		//Open the file for reading
		inStream.open(fname, std::ios::in);
		if (inStream.fail())
		{
			SDL_Log("Could not open options file %s for deserialisation.", fname);
			SDL_Log("Setting defaults for options...");
			//this->o_fullscreen = false;
			//this->o_audioVolume = 1.0;
			//this->o_difficulty = 0;
		}
		else
		{
			SDL_Log("Successfully opened controls file %s for deserialisation.", fname);
			//Build an empty JSON object that we're going to save into
			json inObject;
			//Read all JSON data into this object..
			//json.hpp will do the parsing!
			inStream >> inObject;
			//Now that it's parsed, set the instance variables of the
			//player to whatever is in the parsed JSON object
			SDL_Log("Reading in controls...");
			for (int i = 0; i < players.size(); i++)
			{
				Player* player = players[i];
				map<string, string>* myMap = &player->keyNames;
				for (auto& it = myMap->begin(); it != myMap->end(); ++it)
				{
					player->keyNames[it->first] = inObject["controls"][player->name][it->first].get<string>();
					SDL_Log("Set %s of player %i to %s", it->first.c_str(), i, it->second.c_str());
				}
				player->useJoystick = inObject["controls"][player->name]["useJoystick"].get<bool>();
			}

			//And close the file
			SDL_Log("Closed %s.", fname);
			inStream.close();
		}
	}

private:
	const char* fname = "../content/config/controls.json";
};


#endif