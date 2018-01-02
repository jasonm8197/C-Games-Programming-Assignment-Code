/**
* @file   main.cpp
* @author Jason Malcolm (jmalco@outlook.com)
* @brief  The main
*/

//C++ Headers
#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <ctime>
#include <map>
#include "game.h"
#include "gameobject.h"
#include "level.h"
#include "menu.h"
#include "Sound.h"
#include <direct.h>
#include "winner.h"

//Constants
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const char WINDOW_TITLE[] = "Assignment";
const float UPDATE_TIME = 133;
const int JOYSTICK_DEAD_ZONE = 8000;

//Global variables
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *tex_tiles, *tex_title, *tex_circleboy;
SDL_Texture *tex_ascii;
SDL_Joystick *joystick = NULL;

Menu globalMenu;
Controls* globalControls;
Sound globalSound;
unsigned int game_score = 0;
unsigned int game_lives = 3;
unsigned int collected = 0;
unsigned int current_level = 0;
unsigned int globalTimer = 0;
bool level_complete = false;
bool detonated = false;
vector<Emitter*> emitters;
vector<Particle*> particles;

//Alternate SDL_Log function
void DebugLogToScreen(void*           userdata,
	int             category,
	SDL_LogPriority priority,
	const char*     message)
{
	globalMenu.message = message;
	globalMenu.category = category;
}

void ToggleFullScreen()
{
	Uint32 flag = SDL_WINDOW_FULLSCREEN;
	bool isFullScreen = (SDL_GetWindowFlags(window) & flag);
	SDL_SetWindowFullscreen(window, isFullScreen ? 0 : flag);
	globalMenu.o_fullscreen = !isFullScreen;
}

void ToggleAudio()
{
	globalMenu.o_audioVolume += 0.1;
	if (globalMenu.o_audioVolume > 1)
		globalMenu.o_audioVolume = 0;
	globalSound.SetVolume(globalMenu.o_audioVolume);
}

void ToggleDifficulty()
{
	if (globalMenu.o_difficulty++ > 2)
		globalMenu.o_difficulty = 0;
}

//Constants
int main(int argc, char *argv[])
{
	//Show current directory
	char* path = getcwd(NULL, 0);
	if (path != NULL)
		SDL_Log("Current directory: %s", path);
	//Initialise SDL2
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		//Log an error
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialise SDL");

		//Return
		return -1;
	}

	//Initialise SDL image
	if (IMG_Init(IMG_INIT_PNG) < 0)
	{
		//Log an error
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialise PNG");

		//Return
		return -1;
	}

	//Check for joysticks
	if (SDL_NumJoysticks() < 1)
	{
		SDL_Log("No joystick connected.");
	}
	else
	{
		//Load joystick
		joystick = SDL_JoystickOpen(0);
		if (joystick == NULL)
		{
			SDL_Log("Joystick connected but cannot open. SDL Error: %s\n", SDL_GetError());
		}
	}

	
	globalControls = new Controls();
	globalControls->players.push_back(new Controls::Player("player1"));
	globalControls->players.push_back(new Controls::Player("player2"));
	globalControls->Deserialise();
	//globalControls->Serialise();
    //globalMenu.Serialise();
	globalMenu.Deserialise();
	bool configuring = false;
	int playerId = 0;
	string keyName = "";

	//Initialise window components
	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_SetWindowTitle(window, "16600592 Jason Malcolm CGP2011M Games Programming Assignment : Circleboy Diggers c++!");
	SDL_SetWindowResizable(window, SDL_bool(true));

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Surface* tilesImage = IMG_Load("../content/img/tiles.png");
	tex_tiles = SDL_CreateTextureFromSurface(renderer, tilesImage);
	SDL_Surface* titleImage = IMG_Load("../content/img/title.png");
	tex_title = SDL_CreateTextureFromSurface(renderer, titleImage);
	SDL_Surface* circleboyImage = IMG_Load("../content/img/circleboy.png");
	tex_circleboy = SDL_CreateTextureFromSurface(renderer, circleboyImage);

	globalMenu.window = window;
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "PRESS START", SDL_Color{ 255,255,0 }, 330, 600, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "START GAME", SDL_Color{ 0,255,255 }, 300, 100, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "CONTROLS", SDL_Color{ 0,255,255 }, 300, 160, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "OPTIONS", SDL_Color{ 0,255,255 }, 300, 220, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "EXIT", SDL_Color{ 0,255,255 }, 300, 280, 32);

	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "FULLSCREEN (F4)", SDL_Color{ 0,255,255 }, 300, 100, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "AUDIO VOL. (F5)", SDL_Color{ 0,255,255 }, 300, 160, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "DIFFICULTY (F6)", SDL_Color{ 0,255,255 }, 300, 220, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "BACK", SDL_Color{ 0,255,255 }, 300, 280, 32);

	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 JOY   ", SDL_Color{ 0,255,255 }, 300, 100, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 LEFT   ", SDL_Color{ 0,255,255 }, 300, 140, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 RIGHT  ", SDL_Color{ 0,255,255 }, 300, 180, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 UP     ", SDL_Color{ 0,255,255 }, 300, 220, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 DOWN   ", SDL_Color{ 0,255,255 }, 300, 260, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P1 ACTION ", SDL_Color{ 0,255,255 }, 300, 300, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 JOY   ", SDL_Color{ 0,255,255 }, 300, 340, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 LEFT   ", SDL_Color{ 0,255,255 }, 300, 380, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 RIGHT  ", SDL_Color{ 0,255,255 }, 300, 420, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 UP     ", SDL_Color{ 0,255,255 }, 300, 460, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 DOWN   ", SDL_Color{ 0,255,255 }, 300, 500, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "P2 ACTION ", SDL_Color{ 0,255,255 }, 300, 540, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "BACK", SDL_Color{ 0,255,255 }, 300, 580, 32);

	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "TIME", SDL_Color{ 0,255,255 }, 10, 10, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "QUOTA", SDL_Color{ 0,255,255 }, 10, 50, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "LIVES", SDL_Color{ 0,255,255 }, 10, 90, 32);
	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "SCORE", SDL_Color{ 0,255,255 }, 10, 130, 32);

	globalMenu.AddFont("../content/fonts/8-BIT WONDER.ttf", "T35ERFW", SDL_Color{ 255,0,0 }, 10, 730, 32);

	if (globalMenu.o_gameLog)
		SDL_LogSetOutputFunction(&DebugLogToScreen, NULL);

	
	Level myLevel;
	myLevel.width = 32;
	myLevel.height = 24;
	if (globalMenu.o_2Player)
		myLevel._2pMode = true;
	
	SDL_Log("Creating level from file...");
	myLevel.fileData = "../content/levels/levels.json";
	myLevel.CreateLevel(myLevel.fileData, current_level);

	//globalSound.PlayMusic();
	unsigned int lastQuota = 0;
	unsigned int lastLives = 0;
	unsigned int lastScore = 0;

	//Game win screen
	for (int i = 0; i < 250; i++)
	{
		particles.push_back(new Particle());
	}

	emitters.push_back(new Emitter(200, 200, { 189,212,53 }, { 30,200,144 }, 30.0f, 80.0f, 4.0f, 5.0f));
	emitters.push_back(new Emitter(600, 400, { 255,255,0 }, { 0,0,255 }, 10.0f, 100.0f, 1.0f, 10.0f));
	emitters.push_back(new Emitter(400, 300, { 255,0,255 }, { 128,0,255 }, 90.0f, 160.0f, 3.0f, 15.0f));

	//Game mode

	globalMenu.mode = 2;

	//Game loop
	bool quit = false;
	SDL_Event event;
	float time = 0, deltaTime = 0;
	float updateCounter = 0;

	int debug_time = 0, debug_fps=0;
	int secondTime = 0;

	while (!quit)
	{
		string ksym = "";
		//SDL_Log("Logging latency...");//Later, store these as variables to be examined
		//Get input from system/user
		debug_time = SDL_GetTicks();
		debug_fps = SDL_GetTicks();

		globalControls->mouse.lbutton = false;
		globalControls->p_return = false;
		globalControls->p_pause = false;
		globalControls->lastAction++;
		while (SDL_PollEvent(&event))
		for (int i = 0; i < globalControls->players.size(); i++)
		{
			Controls::Player* player = globalControls->players[i];
			
			{
				
				//Quit
				if (event.type == SDL_QUIT)
					quit = true;

				if (event.type == SDL_KEYDOWN)
				{
					ksym = SDL_GetKeyName(event.key.keysym.sym);
				}

				//Key down
				if (!player->useJoystick)
				{
					if (event.type == SDL_KEYDOWN)
					{
						{
							map<string, string> myMap = player->keyNames;
							for (auto& it = myMap.begin(); it != myMap.end(); ++it)
							{
								if (ksym == it->second)
								{
									player->keys[it->first] = true;
									//SDL_Log("%s %i ON ", it->first.c_str(), i);
								}
							}
						}
					}
					//Key up
					if (event.type == SDL_KEYUP)
					{
						{
							map<string, string> myMap = player->keyNames;
							for (auto& it = myMap.begin(); it != myMap.end(); ++it)
							{
								if (SDL_GetKeyName(event.key.keysym.sym) == it->second)
								{
									player->keys[it->first] = false;
									//SDL_Log("%s OFF", it->first.c_str());
								}
							}
						}
					}
				}

				//Joystick control
				else if (event.jaxis.which == player->joystickId)
				{
					if (event.type == SDL_JOYAXISMOTION)
					{
						//x axis
						if (event.jaxis.axis == 0)
						{
							player->keys["left"] = (event.jaxis.value < -JOYSTICK_DEAD_ZONE);
							player->keys["right"] = (event.jaxis.value > JOYSTICK_DEAD_ZONE);
						}
						//y axis
						if (event.jaxis.axis == 1)
						{
							player->keys["up"] = (event.jaxis.value < -JOYSTICK_DEAD_ZONE);
							player->keys["down"] = (event.jaxis.value > JOYSTICK_DEAD_ZONE);
						}
					}

					if (event.type == SDL_JOYBUTTONDOWN)
					{
						if (event.jbutton.button == 0)
							player->keys["action"] = true;
					}

					if (event.type == SDL_JOYBUTTONUP)
					{
						if (event.jbutton.button == 0)
							player->keys["action"] = false;
					}
				}

				//'lastAction' ensures these function keys aren't pressed within a given threshold
				if (globalControls->lastAction > 200)
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_RETURN: {globalControls->p_return = true; globalControls->lastAction = 0; break; }
						case SDLK_F4: {ToggleFullScreen(); globalControls->lastAction = 0; break; }
						case SDLK_F5: {ToggleAudio(); globalControls->lastAction = 0; break; }
						case SDLK_F6: {ToggleDifficulty(); globalControls->lastAction = 0; break; }
						case SDLK_F7: {if (globalMenu.mode == 0) { level_complete = true; }; globalControls->lastAction = 0; break; }//cheat
						case SDLK_p: {if (globalMenu.o_pausing)globalControls->p_pause = true; globalControls->lastAction = 0; break; }
					}
				}

				//Mouse
				if (event.type == SDL_MOUSEMOTION)
				{
					globalControls->mouse.x = event.motion.x;
					globalControls->mouse.y = event.motion.y;
				}

				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					if (event.button.button == 1)
						globalControls->mouse.lbutton = true;
				}
			}
		}
		//Controls input latency
		if (globalMenu.o_gameLatency)
			SDL_Log("Controls: Latency = %ims", SDL_GetTicks() - debug_time);

		//Mode 0 = gameplay
		if (globalMenu.mode == 0)
		{
			//Update
			debug_time = SDL_GetTicks();
			if (!globalMenu.paused)
			{
				deltaTime = (SDL_GetTicks() - time);
				time = SDL_GetTicks();
				updateCounter += deltaTime;
				if (updateCounter >= UPDATE_TIME)
				{
					updateCounter = 0;
					myLevel.Update();

					secondTime += UPDATE_TIME;
					if (secondTime >= 1000)
					{
						secondTime = 0;
						if (myLevel.levelData["time"] == 0)
							myLevel.outOfTime = true;
						else
							myLevel.levelData["time"]--;
						globalMenu.UpdateText({ 22 }, { " TIME " + to_string(myLevel.levelData["time"]) });
					}
					if (lastQuota != myLevel.levelData["quota"]) { lastQuota = myLevel.levelData["quota"]; globalMenu.UpdateText({ 23 }, { "QUOTA " + to_string(myLevel.levelData["quota"]), }); }
					if (lastScore != game_score) { lastQuota = game_score; globalMenu.UpdateText({ 24 }, { "SCORE " + to_string(game_score), }); 
					if (game_score % 200 == 0)game_lives++;//Gain a life for every 200 points
					}
					if (lastLives != game_lives) { lastQuota = game_lives; globalMenu.UpdateText({ 25 }, { "LIVES " + to_string(game_lives), }); }
				}
				
			}
			else
				updateCounter = 0;

			

			//Restart level
			if (globalControls->p_return)
			{
				myLevel.CreateLevel(myLevel.fileData, current_level);
				if (--game_lives <= 0)
				{
					SDL_Log("GAME OVER! Returning to menu");
					globalMenu.mode = 2;
				}
			}

			else if (globalControls->p_pause)
				globalMenu.paused = !globalMenu.paused;

			//Animation
			myLevel.Animate(deltaTime);

			//Update latency
			if (globalMenu.o_gameLatency)
				SDL_Log("Update   : Latency = %ims", SDL_GetTicks() - debug_time);

			

			//Rendering
			debug_time = SDL_GetTicks();
			SDL_RenderClear(renderer);
			myLevel.Render(renderer, tex_tiles, updateCounter / UPDATE_TIME);
			if (globalMenu.o_gameHud)
				globalMenu.DisplayText({ 22,23,24,25 });
			if (globalMenu.o_gameLog)
			{
				globalMenu.UpdateText({ 26 }, {globalMenu.message });
				globalMenu.DisplayText({ 26 });
			}
			SDL_RenderPresent(renderer);

			if (level_complete)
			{
				level_complete = false;
				if (!myLevel.CreateLevel(myLevel.fileData, ++current_level))
				{
					globalMenu.mode = 5;
					SDL_Log("Game completed successfully.");
				}
				else
					SDL_Log("game not complete...\ndfgg");
			}

		}
			
		
		//Main menu
		else if (globalMenu.mode == 1)
		{
			Controls::Player* player = globalControls->players[0];
			//Update menu
			deltaTime = (SDL_GetTicks() - time);
			time = SDL_GetTicks();
			updateCounter += deltaTime;
			if (updateCounter > UPDATE_TIME)
			{
				if (player->keys["down"])
				{
					globalMenu.UpdateText({ globalMenu.selected+1 }, { "off" });
					globalMenu.selected = (globalMenu.selected + 1) % 4;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected+1 }, { "selected" });
				}
				else if (player->keys["up"])
				{
					globalMenu.UpdateText({ globalMenu.selected+1 }, { "off" });
					globalMenu.selected = (globalMenu.selected + 3) % 4;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected+1 }, { "selected" });
				}
			}
			
			for (int i = 0; i < 4; i++)
			{
				if (globalMenu.GetMouseInText(i+1, globalControls->mouse.x, globalControls->mouse.y))
				{
					globalMenu.UpdateText({ globalMenu.selected + 1 }, { "off" });
					globalMenu.selected = i;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected + 1 }, { "selected" });
				}
			}
			
			//Take action dependent on selected
			if (player->keys["action"] || globalControls->p_return || globalControls->mouse.lbutton)
			{
				switch (globalMenu.selected)
				{
				//Start game
				case 0: 
				{
					current_level = 0;
					globalMenu.mode = 0;
					game_lives = 5;
					myLevel.CreateLevel(myLevel.fileData, current_level);
					break; 
				}

				//Controls
				case 1:
				{
					globalMenu.selected = 0;
					globalMenu.mode = 4;
					break;
				}

				//Option
				case 2:
				{
					globalMenu.selected = 0;
					globalMenu.mode = 3;
					break;
				}

				//Quit
				case 3:
				{
					quit = true;
					break;
				}
				}
				
			}

			//Rendering
			SDL_RenderClear(renderer);
			SDL_Rect src = { 0,64,32,32 };
			SDL_Rect dest = { 250,100 + globalMenu.selected * 60,32,32 };
			SDL_RenderCopy(renderer, tex_tiles, &src, &dest);
			globalMenu.DisplayText({ 1,2,3,4 });
			SDL_RenderPresent(renderer);
		}

		//Splash screen
		else if (globalMenu.mode == 2)
		{
			//Press ENTER to go to menu screen
			if (globalControls->p_return)
				globalMenu.mode = 1;

			//Rendering
			SDL_Rect src,dest;
			SDL_RenderClear(renderer);
			src = { 0,0,954, 768 };
			dest = { 0,0,954,768 };
			SDL_RenderCopy(renderer, tex_circleboy, &src, &dest);
			if (SDL_GetTicks() % 1000 < 500)//Flashing effect
				globalMenu.DisplayText({ 0 });
			src = { 0,0,541, 146}; dest = { 640,14,370,100 };
			SDL_RenderCopy(renderer, tex_title, &src, &dest);
			SDL_RenderPresent(renderer);		
		}

		//Controls
		else if (globalMenu.mode == 4)
		{
			Controls::Player* player = globalControls->players[0];

			if (globalTimer % 200 == 0)
			{
				globalMenu.UpdateText({ 9,10,11,12,13,14,15,16,17,18,19,20 },
				{
					"P1 JOY   " + to_string(globalControls->players[0]->useJoystick),
					"P1 LEFT  " + globalControls->players[0]->keyNames["left"],
					"P1 RIGHT " + globalControls->players[0]->keyNames["right"],
					"P1 UP    " + globalControls->players[0]->keyNames["up"],
					"P1 DOWN  " + globalControls->players[0]->keyNames["down"],
					"P1 ACTION" + globalControls->players[0]->keyNames["action"],
					"P2 JOY   " + to_string(globalControls->players[1]->useJoystick),
					"P2 LEFT  " + globalControls->players[1]->keyNames["left"],
					"P2 RIGHT " + globalControls->players[1]->keyNames["right"],
					"P2 UP    " + globalControls->players[1]->keyNames["up"],
					"P2 DOWN  " + globalControls->players[1]->keyNames["down"],
					"P2 ACTION" + globalControls->players[1]->keyNames["action"],
				});
			}

			if (!configuring)
			{
				//Update menu
				deltaTime = (SDL_GetTicks() - time);
				time = SDL_GetTicks();
				updateCounter += deltaTime;
				if (updateCounter > UPDATE_TIME)
				{
					if (player->keys["down"])
					{
						globalMenu.UpdateText({ globalMenu.selected + 1 }, { "off" });
						globalMenu.selected = (globalMenu.selected + 1) % 4;
						updateCounter = 0;
						globalMenu.UpdateText({ globalMenu.selected + 5 }, { "selected" });
					}
					else if (player->keys["up"])
					{
						globalMenu.UpdateText({ globalMenu.selected + 1 }, { "off" });
						globalMenu.selected = (globalMenu.selected + 3) % 4;
						updateCounter = 0;
						globalMenu.UpdateText({ globalMenu.selected + 5 }, { "selected" });
					}
				}

				for (int i = 8; i < 21; i++)
				{
					if (globalMenu.GetMouseInText(i + 1, globalControls->mouse.x, globalControls->mouse.y))
					{
						globalMenu.UpdateText({ globalMenu.selected + 9 }, { "off" });
						globalMenu.selected = i - 8;
						updateCounter = 0;
						globalMenu.UpdateText({ globalMenu.selected + 9 }, { "selected" });
					}
				}

				//Take action dependent on selected
				if (player->keys["action"] || globalControls->p_return || globalControls->mouse.lbutton)
				{
					//Go back
					if (globalMenu.selected == 12)
					{
						globalControls->Serialise(); globalMenu.selected = 0; globalMenu.mode = 1;
					}
					//configure
					else if (!configuring)
					{
						configuring = true;
						switch (globalMenu.selected)
						{
						case 0: {playerId = 0;keyName = "joy"; break;}
						case 1: {playerId = 0;keyName = "left"; break;}
						case 2: {playerId = 0;keyName = "right"; break;}
						case 3: {playerId = 0;keyName = "up"; break;}
						case 4: {playerId = 0;keyName = "down"; break;}
						case 5: {playerId = 0;keyName = "action"; break;}
						case 6: {playerId = 1;keyName = "joy"; break;}
						case 7: {playerId = 1;keyName = "left"; break;}
						case 8: {playerId = 1;keyName = "right"; break;}
						case 9: {playerId = 1;keyName = "up"; break;}
						case 10: {playerId = 1;keyName = "down"; break;}
						case 11: {playerId = 1;keyName = "action"; break;}
						}
					}
					
				}
			}

			//Configure
			else
			{
				if (ksym != "")
				{
					if (keyName != "joy")
					{
						SDL_Log("Set player %i %s to %s", playerId, keyName, ksym.c_str());
						globalControls->players[playerId]->keyNames[keyName] = ksym;
						configuring = false;
					}
					else
					{
						globalMenu.UpdateText({ 9 + 6 * playerId }, { "TYPE 1 OR 0" });
						if (ksym == "1")
						{
							globalControls->players[playerId]->useJoystick = true;
							configuring = false;
						}
						else if (ksym == "0")
						{
							globalControls->players[playerId]->useJoystick = false;
							configuring = false;
						}
					}
				}
			}
		
			
			

			//Rendering
			SDL_RenderClear(renderer);
			SDL_Rect src = { 0,64,32,32 };
			SDL_Rect dest = { 250,100 + (globalMenu.selected) * 40,32,32 };
			SDL_RenderCopy(renderer, tex_tiles, &src, &dest);
			globalMenu.DisplayText({ 9,10,11,12,13,14,15,16,17,18,19,20,21 });
			SDL_RenderPresent(renderer);
		}

		//Options
		else if (globalMenu.mode == 3)
		{
			Controls::Player* player = globalControls->players[0];
			//Update menu
			deltaTime = (SDL_GetTicks() - time);
			time = SDL_GetTicks();
			updateCounter += deltaTime;
			if (updateCounter > UPDATE_TIME)
			{
				if (player->keys["down"])
				{
					globalMenu.UpdateText({ globalMenu.selected + 1 }, { "off" });
					globalMenu.selected = (globalMenu.selected + 1) % 4;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected + 5 }, { "selected" });
				}
				else if (player->keys["up"])
				{
					globalMenu.UpdateText({ globalMenu.selected + 1 }, { "off" });
					globalMenu.selected = (globalMenu.selected + 3) % 4;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected + 5 }, { "selected" });
				}
			}

			for (int i = 4; i < 8; i++)
			{
				if (globalMenu.GetMouseInText(i + 1, globalControls->mouse.x, globalControls->mouse.y))
				{
					globalMenu.UpdateText({ globalMenu.selected + 5 }, { "off" });
					globalMenu.selected = i-4;
					updateCounter = 0;
					globalMenu.UpdateText({ globalMenu.selected + 5 }, { "selected" });
				}
			}

			//Take action dependent on selected
			if (player->keys["action"] || globalControls->p_return || globalControls->mouse.lbutton)
			{
				switch (globalMenu.selected)
				{
				case 0: {ToggleFullScreen(); break; }//Toggle fullscreen
				case 1: {ToggleAudio(); break; }//Toggle audio
				case 2: {ToggleDifficulty(); break; }//Toggle game difficulty
				case 3: {globalMenu.Serialise(); globalMenu.selected = 0; globalMenu.mode = 1; break; }//Toggle game difficulty
				}
				
				
			}
			string isFullscreen = (globalMenu.o_fullscreen) ? "ON" : "OFF";
			globalMenu.UpdateText({ 5,6,7 },
			{
				
				"FULLSCREEN (F4) " + isFullscreen,
				"AUDIO VOL. (F5) " + to_string(globalMenu.o_audioVolume),
				"DIFFICULTY (F6) " + to_string(globalMenu.o_difficulty),
			});

			//Rendering
			SDL_RenderClear(renderer);
			SDL_Rect src = { 0,64,32,32 };
			SDL_Rect dest = { 250,100 + (globalMenu.selected) * 60,32,32 };
			SDL_RenderCopy(renderer, tex_tiles, &src, &dest);
			globalMenu.DisplayText({ 5,6,7,8 });
			SDL_RenderPresent(renderer);
		}

		//Winner
		else if (globalMenu.mode == 5)
		{
			Controls::Player* player = globalControls->players[0];
			if (player->keys["action"] || globalControls->p_return || globalControls->mouse.lbutton)
			{
				globalMenu.Serialise(); globalMenu.selected = 0; globalMenu.mode = 1;
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);
			}
			else
			{

				int i = 0;
				for (auto &em : emitters)
				{
					em->Emit(particles, emitters.size(), i++);
				}

				for (auto &pt : particles)
				{
					if (pt->active)
						pt->Update(0.01);
				}

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				for (auto &pt : particles)
				{
					if (pt->active)
						pt->Render(renderer);
				}
				SDL_RenderPresent(renderer);
			}
			
		}
		
		if (globalMenu.o_gameLatency)
		{
			SDL_Log("Rendering: Latency = %ims", SDL_GetTicks() - debug_time);
			SDL_Log("Total    : Latency = %ims", SDL_GetTicks() - debug_fps);
		}
		globalTimer++;
	}

	//Destroy elements
	SDL_DestroyTexture(tex_tiles);
	SDL_FreeSurface(tilesImage);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_JoystickClose(joystick);
	joystick = NULL;
	IMG_Quit();
	SDL_Quit();
	
}