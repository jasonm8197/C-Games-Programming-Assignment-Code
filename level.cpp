#include "level.h"

bool Level::CreateLevel()
{
	levelData["width"] = 32;
	levelData["height"] = 24;
	for (int x = 0; x < levelData["width"]; x++)
	{
		levelGrid.push_back(vector<GameObject*>());
		for (int y = 0; y < levelData["height"]; y++)
		{
			GameObject* object;
			object = new Boulder(levelGrid, GameObject::Vector2(x, y));
			levelGrid[x].push_back(object);
			SDL_Log("Created object %p at (%i,%i)", object, x, y);
		}
	}
	return true;
}

bool Level::CreateLevel(string fname, int levelId)
{
	levelGrid.clear();
	detonated = false;
	collected = 0;
	outOfTime = false;
	GameObject::GetMapOfObjects().clear();
	SDL_Log("Creating level %i", levelId);
	if (SetLevelData(fname, levelId))
	{
		CreateTiles(fname, levelId);
		return true;
	}
	else
	{
		SDL_Log("Game complete!");
		return false;
	}
}

void Level::CreateTiles(string fname, int levelId)
{
	levelData["width"] = 32;
	levelData["height"] = 24;
	SDL_Log("Creating objects. Level width: %i, Level height: %i", levelData["width"], levelData["height"]);
	int i = 0;
	for (int x = 0; x < levelData["width"]; x++)
	
	{
		levelGrid.push_back(vector<GameObject*>());
		for (int y = 0; y < levelData["height"]; y++)
		{
			i = x + y*levelData["width"];
			string c = string(1, levelTiles[i]);
			GameObject* object = GameObject::Create(c, levelGrid, GameObject::Vector2(x, y));
			levelGrid[x].push_back(object);
			//SDL_Log("Created object %p at (%i,%i)", object, x, y);
		}
	}
	SDL_Log("LevelGrid: %p", levelGrid);
	SDL_Log("Size: %i,%i",levelGrid.size(),levelGrid[0].size());

	//2 player mode: Can be set manually in options
	if (_2pMode)
	{
		GameObject* player2 = GameObject::Create("p", levelGrid, GameObject::Vector2(1, 1));
		levelGrid[1][1] = player2;
		player2->playerId = 1;
	}
}

bool Level::SetLevelData(string fname, int levelId)
{
	//The input stream (to a file)
	std::ifstream inStream;
	//Open the file for reading
	inStream.open(fname.c_str(), std::ios::in);
	if (inStream.fail())
	{
		SDL_Log("Could not open level file %s for deserialisation.", fname.c_str());
		SDL_Log("Setting defaults for options...");
		return false;
	}
	else
	{
		SDL_Log("Successfully opened level file %s for deserialisation.", fname.c_str());
		//Build an empty JSON object that we're going to save into
		json inObject;
		string levelName = "level" + to_string(levelId);

		try
		{
			//Read all JSON data into this object..
			//json.hpp will do the parsing!
			inStream >> inObject;
			//Now that it's parsed, set the instance variables of the
			//player to whatever is in the parsed JSON object
			SDL_Log("Reading in level data...");
			levelData["time"] = inObject["file"][levelName]["time"].get<int>();
			levelData["quota"] = inObject["file"][levelName]["quota"].get<int>();
			levelData["width"] = inObject["file"][levelName]["width"].get<int>();
			levelData["height"] = inObject["file"][levelName]["height"].get<int>();
			levelData["amoebaMax"] = inObject["file"][levelName]["amoebaMax"].get<int>();
			levelData["amoebaSpeed"] = inObject["file"][levelName]["amoebaSpeed"].get<int>();
			levelData["plasmaSpeed"] = inObject["file"][levelName]["plasmaSpeed"].get<int>();
			levelData["lavaSpeed"] = inObject["file"][levelName]["lavaSpeed"].get<int>();
			levelTiles = inObject["file"][levelName]["tiles"].get<string>();

			SDL_Log("LEVEL DATA\nTime: %i Quota: %i\nWidth: %i\nHeight: %i\nAmoeba Speed:%i\Amoeba Max Growth: %i\nPlasma Speed: %i\nLava Speed: %i\n\nTiles: %i",
				levelData["time"], levelData["quota"], levelData["width"], levelData["height"], levelData["amoebaMax"], levelData["amoebaSpeed"], levelData["plasmaSpeed"],
				levelData["lavaSpeed"], levelTiles);
			//And close the file
			SDL_Log("Closed %s.", fname.c_str());
			inStream.close();
			return true;
		}
		catch (exception e)
		{
			SDL_Log("Level %s not found. Assuming game is complete", levelName);
			return false;
		}
	}

}

void Level::Update()
{
	//Update gameobjects 
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			levelGrid[x][y]->updated = false;
		}

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			GameObject* object = levelGrid[x][y];
			if (object->active && !object->updated)
			{
				object->Update();
				object->updated = true;

				//Special objects that need a bit more information...
				switch (object->type)
				{
				case 'n': {object->Grow((float)(levelData["plasmaSpeed"]) / 100); break; }
				case 'm': {object->Grow((float)(levelData["amoebaSpeed"]) / 100); break; }
				case 'l': {object->Grow((float)(levelData["lavaSpeed"]) / 100); break; }
				case 'e': {SDL_Log("quota==%i", levelData["quota"]); if (collected >= levelData["quota"]) object->Unlock(); break; }
				case 'p': {if (outOfTime)object->LandedOn(); break; }
				}

				if (detonated)
				{
					object->Detonated();
				}
			}
		}
		
	}

	//Late update (for any last minute changes during same frame)
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			GameObject* object = levelGrid[x][y];
			if (object->active)
			{
				object->LateUpdate();
			}
		}
	}

	//Delete objects that aren't in the levelGrid anymore
	vector<int> delets;
	map<int, GameObject*> &objects = GameObject::GetMapOfObjects();
	GameObject* obj;
	int _x, _y;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		obj = it->second; _x = obj->position.x; _y = obj->position.y;
		{
			if (obj != levelGrid[_x][_y])
				delets.push_back(it->first);
			//SDL_Log("LevelGrid[%i][%i]=%p obj=%p, same=%i",_x,_y, levelGrid[_x][_y], obj, obj == levelGrid[_x][_y]);
		}
	}
	for (int i = 0; i < delets.size(); i++)
	{
		delete objects[delets[i]];
	}
	//SDL_Log("Size of array: %i", objects.size());

	//char c; cin >> c;
}

void Level::Animate(float t)
{
	GameObject* obj;
	map<int, GameObject*> &objects = GameObject::GetMapOfObjects();
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		obj = it->second;
		if (obj->animating)
			obj->Animate(t);
	}
}

void Level::Render(SDL_Renderer* renderer, SDL_Texture* texture, float t)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			GameObject* object = levelGrid[x][y];			
			if (object->visible)
			{
				SDL_Rect src = RenderGetSrc((int)object->anim.index);
				SDL_Rect dest = RenderGetDest(object->position.x,object->position.y,object->velocity.x,object->velocity.y,t);
				//SDL_Rect dest = RenderGetDest(x,y,object->velocity.x,object->velocity.y,t);
				SDL_RenderCopy(renderer, texture, &src, &dest);
				//SDL_Log("rendering: %p", object);
			}
		}
	}
}

//Get source (known tiles format)
SDL_Rect Level::RenderGetSrc(int imageIndex)
{
	SDL_Rect rect;
	int rx = (imageIndex % 48) * 32;
	int ry = (imageIndex / 48) * 32;
	//printf("%i,%i,%i\n", rx, ry, imageIndex);
	rect = { rx,ry,32,32 };
	return rect;
}

//
SDL_Rect Level::RenderGetDest(int x, int y, int dx, int dy, float t)
{
	//x,y = Current position
	//xlast,ylast = Previous position
	//t = Time between previous frame and next update (normalised)
	SDL_Rect rect;
	float rx, ry;
	rx = (x - dx*(1 - t)) * 32;
	ry = (y - dy*(1 - t)) * 32;
	rect = { (int)rx,(int)ry,32,32 };

	return rect;
}