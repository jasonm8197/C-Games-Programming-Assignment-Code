#pragma once
#include <string>
#include <fstream>
#include "SDL.h"
#include "SDL_image.h"

class ImageText
{
public:
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	int xt, yt;
	ImageText(SDL_Renderer* _renderer, SDL_Texture* _texture, int _xt, int _yt) :renderer(_renderer), texture(_texture), xt(_xt), yt(_yt) {}
};