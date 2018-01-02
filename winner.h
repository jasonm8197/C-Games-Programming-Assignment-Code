#pragma once
#include <SDL.h>
#include <cmath>
#include <vector>
class Particle
{
public:
	struct Vector2
	{
		float x, y;
		Vector2() :x(0), y(0) {}
		Vector2(double _a) : x(cos(_a)), y(-sin(_a)) {}
		Vector2(float _x, float _y) : x(_x), y(_y) {}

		friend Vector2 operator * (Vector2 vec, const float &f)
		{
			return Vector2(vec.x*f, vec.y*f);
		}
	};

	SDL_Color Lerp(SDL_Color col1, SDL_Color col2, float t)
	{
		SDL_Color newCol;
		newCol.r = col1.r + (col2.r - col1.r)*t;
		newCol.g = col1.g + (col2.g - col1.g)*t;
		newCol.b = col1.b + (col2.b - col1.b)*t;
		return newCol;
	}

	float x, y, lifetime, life_max;
	Vector2 velocity;
	SDL_Color col, col1, col2;
	bool active;

	Particle() : x(.0f), y(.0f), lifetime(.0f), life_max(.0f), velocity(Vector2(.0f, .0f)), active(false) {}

	void SetParticle(float _x, float _y, float _lifetime, Vector2 _velocity, SDL_Color _col1, SDL_Color _col2)
	{
		x = _x;
		y = _y;
		lifetime = _lifetime;
		life_max = _lifetime;
		velocity = _velocity;
		active = true;
		col = _col1;
		col1 = _col1;
		col2 = _col2;
	}

	void Update(float dt)
	{
		x += velocity.x*dt;
		y += velocity.y*dt;
		lifetime -= dt;
		if (life_max != 0)
			col = Lerp(col1, col2, lifetime / life_max);
		if (lifetime <= .0f)
			active = false;
	}

	void Render(SDL_Renderer* renderer)
	{
		int _x = (int)x;
		int _y = (int)y;
		SDL_Rect rect = { x - 2,y - 2,4,4 };
		SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 1.0f);
		SDL_RenderDrawRect(renderer, &rect);
	}
};




using namespace std;
class Emitter
{
public:
	const float PI = 3.1415926;
	int x, y;
	SDL_Color col1, col2;
	float lifetime_min, lifetime_max;
	float speed_min, speed_max;

	Emitter() : x(0), y(0), col1({ 255,255,255 }), col2({ 0,0,0 }), lifetime_min(1.0f), lifetime_max(3.0f), speed_min(10.0f), speed_max(60.0f) {}
	Emitter(int _x, int _y, SDL_Color _col1 = { 255,255,255 }, SDL_Color _col2 = { 0,0,05 }, float _lifetime_min = 1.0f, float _lifetime_max = 3.0f, float _speed_min = 10.0f, float _speed_max = 60.0f)
	{
		x = _x;
		y = _y;
		col1 = _col1;
		col2 = _col2;
		lifetime_min = _lifetime_min;
		lifetime_max = _lifetime_max;
		speed_min = _speed_min;
		speed_max = _speed_max;
	}

	float RandomFloat(float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}

	void Emit(vector<Particle*> particles, int size, int start)
	{
		for (int i = start; i < particles.size(); i += size)
		{
			if (!particles[i]->active)
			{
				particles[i]->SetParticle((float)x, (float)y, RandomFloat(lifetime_min, lifetime_max), Particle::Vector2(RandomFloat(0, PI * 2)*RandomFloat(speed_min, speed_max)), col1, col2);
			}
		}
	}
};