#pragma once;

#include <map>
#include <vector>
#include <string>
#include <stdlib.h>
#include <SDL.h>
#include "gameobject.h"
#include <fstream>

using namespace std;
extern unsigned int collected;
extern bool detonated;

class Level
{
public:
	int width, height;
	vector<vector<GameObject*>> levelGrid;
	map<string, int> levelData;
	string levelTiles;
	string fileData;
	bool outOfTime = false;
	bool _2pMode = false;

	//bool OpenFile(string filename);
	bool CreateLevel();
	bool CreateLevel(string fname, int levelId);
	bool SetLevelData(string fname, int levelId);
	void CreateTiles(string fname, int levelId);
	void Update();
	void Animate(float t);
	void Render(SDL_Renderer* renderer, SDL_Texture* tex, float t);
	
private:
	SDL_Rect RenderGetSrc(int imageIndex);
	SDL_Rect RenderGetDest(int x, int y, int dx, int dy, float t);
};

