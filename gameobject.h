#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <stdlib.h>
#include "controls.h"
#include "Sound.h"

using namespace std;
extern Controls* globalControls;
extern unsigned int game_score, current_level, collected;
static const float ANIM_SPEED = 0.04f;
extern bool level_complete;
extern Sound globalSound;
extern bool detonated;

//Parent gameobject
class GameObject
{
public:
	typedef vector<vector<GameObject*>> Level2D;
	
	//Vector2
	struct Vector2
	{
		int x, y;
		Vector2() :x(0), y(0) {}
		Vector2(int _x, int _y): x(_x), y(_y) {}
		Vector2 operator + (const Vector2& vec)
		{
			Vector2 vec2(this->x + vec.x, this->y + vec.y);
			return vec2;
		}
		Vector2 operator - (const Vector2& vec)
		{
			Vector2 vec2(this->x - vec.x, this->y - vec.y);
			return vec2;
		}
		Vector2 operator > (int n)
		{
			Vector2 vec2(this->x, this->y);
			int c;
			while (n-- > 0)
			{
				c = vec2.y;
				vec2.y = -vec2.x;
				vec2.x = c;
			}
			return vec2;
		}
		bool operator == (const Vector2& vec)
		{
			return (this->x == vec.x && this->y == vec.y);
		}

		bool operator != (const Vector2& vec)
		{
			return !(this->x == vec.x && this->y == vec.y);
		}
		
		static const Vector2 left, right, up, down, zero;
	};

	

	//Animation
	struct Animation
	{
		float start, length;
		bool isRandom;
		float index, speed;
		Animation() :start(0.0f), length(0.0f), isRandom(false), index(0), speed(0)
		{ 
			if (isRandom)index = start + (rand() % (int)(length));
		}
		Animation(float _start) :start(_start), length(0.0f), isRandom(false), index(_start), speed(0)
		{
			if (isRandom)index = start + (rand() % (int)(length));
		}
		Animation(float _start, float _length, bool _isRandom, float _speed): start(_start), length(_length), isRandom(_isRandom), index(_start), speed(_speed)
		{
			if (isRandom)index = start + (rand() % (int)(length));
		}
		void AnimUpdate(float t)
		{
			index += speed*t*ANIM_SPEED;
			if (index >= start + length)
				index = start;
		}
	};

	//Properties
	Vector2 position, velocity;
	Animation anim;
	GameObject* creator = this;
	bool active = true; //Has an update event
	bool visible = true; //Is rendered to the screen
	bool solid = false; //Cannot be destroyed/passed throught
	bool rounded = false; //Boulders will roll off it
	bool animating = false; //Animates
	bool pushable = false; //Can this object be pushed by thte player?
	int playerId = 0;//Player ID

	bool updated = false;
	char type = ' ';
	string travels = " ";

	virtual void Update() {}; //Update every step
	virtual void LateUpdate() {}; //LateUpdate, called once after all objects have been updated
	virtual void Animate(float t) { anim.AnimUpdate(t); }; //Continuous animation
	virtual void Landed() {}; //Was falling but has now landed (boulders)
	virtual void LandedOn() {}; //Landed on by a boulder
	virtual void Traveled(bool isPlayer) {}; //Traveled through by a player or other object, different to deconstructor
	virtual void Grow(float probability) {}; //For growing objects (amoeba, plasma, lava)
	virtual void Unlock() {}; //Called when quota of diamonds are collected
	virtual bool Pushed(Vector2 vec) { return false; }; //Called when an object is pushed, returns true if succesful
	virtual void Detonated() {}; //Cerating objects (TNT, Shellrad) can be detonated

	//Constructor
	GameObject(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : levelGrid(_levelGrid), position(_position) { id = currentId++; objects[id] = this; }
	GameObject(vector<vector<GameObject*>> &_levelGrid, Vector2 _position, GameObject* _creator) : levelGrid(_levelGrid), position(_position) { id = currentId++; objects[id] = this; }
	GameObject(vector<vector<GameObject*>> &_levelGrid, Vector2 _position, char _exploType) : levelGrid(_levelGrid), position(_position) { id = currentId++; objects[id] = this; }
	virtual ~GameObject() { objects.erase(id); }
	static map<int, GameObject*>& GetMapOfObjects() 
	{
		return objects;
	}
	static GameObject* Create(string type, vector<vector<GameObject*>> &_levelGrid, Vector2 _position);

	
	
	

protected:
	void Move(Vector2 vec); //Move relative position
	GameObject* Get(Vector2 vec); //Get object at position
	GameObject* At(Vector2 vec); //Get if position is 'free'
	Vector2 Mod(Vector2 vec); //Vector2 in bounds of level
	void Explode(int size, char exploType = ' ', string exploInto = " ");//Explode script
	bool Adj(char c);//Is adjacent to object of type c
	void Dest();//Replace self with space object
	void AddScore(int n);//Adds to the score
	void AddCollected();//Increments diamonds collected
	static float RandomFloat(float min, float max);

	//Keep a record of objects so they can be deleted when not in the map
	static int currentId;
	static map<int, GameObject*> objects;
	vector<vector<GameObject*>> &levelGrid;
	int id;
};

class Space : public GameObject
{
public:
	Space(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		position = _position;
		//printf("This is space(%i,%i)\n", position.x, position.y);
		visible = false;
		solid = true;
		type = ' ';
	}

	Space(vector<vector<GameObject*>> &_levelGrid, Vector2 _position, GameObject* _creator) : GameObject(_levelGrid, position, _creator)
	{
		position = _position;
		creator = _creator;
		//printf("This is space(%i,%i)\n", position.x, position.y);
		visible = false;
		solid = true;
		type = ' ';
	}

	void LateUpdate()
	{
		//Solid for one frame
		solid = false;
		active = false;
	}
};



class Player : public GameObject
{
public:
	Player(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		type = 'p';
		travels = " d13u";
		position = _position;
		anim = Animation(120.0f, 8.0f, false, 0.5f);
		animating = true;	
	}

	void Update()
	{
		//Prioritise up/down controls over left/right
		velocity = Vector2(0,0);
		int px, py;
		Controls::Player* player = globalControls->players[playerId];
		py = player->keys["down"] - player->keys["up"];
		px = (py==0) ? player->keys["right"] - player->keys["left"] : 0;
		if (playerId == 1)
			SDL_Log("%i", player->keys["right"]);

		Vector2 p_move = Vector2(px, py);
		if (!At(p_move))
		{
			Get(p_move)->Traveled(true);
			if (!player->keys["action"])
				Move(p_move);
		}
		else
		{
			//Can only push boulders left/right
			if (p_move.x != 0 && (pushTimer++) >= 2)
			{
				if (Get(p_move)->Pushed(p_move))
					Move(p_move);
				pushTimer = 0;
			}
			else
				Get(p_move)->Pushed(Vector2(0, 0));
		}

		//Animation stuff
		bool pushing = (At(p_move));
		float lastStart = anim.start;
		if (p_move == Vector2(1, 0)) { anim.start = 96.0f; if (pushing) anim.start = 128.0f; }
		else if (p_move == Vector2(0, -1)) { anim.start = 104.0f;}
		else if (p_move == Vector2(-1, 0)) { anim.start = 112.0f; if (pushing) anim.start = 136.0f; }
		else if (p_move == Vector2(0, 1)) { anim.start = 120.0f; }
		if (lastStart != anim.start)
			anim.index = anim.start;
		if (p_move == Vector2(0, 0))
		{
			anim.index = anim.start;
			animating = false;
		}
		else
			animating = true;
	}

	void LandedOn()
	{
		printf("OH NOES! I HAVS BEEN LANDED ON!\n");
		Explode(1, '1');
	}
private:
	int pushTimer = 0;
};

class Dirt : public GameObject
{
public:
	Dirt(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		position = _position;
		//printf("This is dirt(%i,%i)\n", position.x, position.y);
		anim = Animation(168.0f + ((current_level%100)*4), 4.0f, true, 0.0f);
		type = 'd';
		active = false;
	}

	void Traveled(bool isPlayer)
	{
		globalSound.PlaySound("dirt");
		Dest();
	}
};

class Wall : public GameObject
{
public:
	Wall(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		position = _position;
		anim = Animation(1388.0f+(current_level%10));
		rounded = true;
		active = false;
		type = 'w';
	}
};

//Detonator: Sets detonated to true when pushed
class Detonator : public GameObject
{
public:
	Detonator(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		position = _position;
		anim = Animation(144.0f);
		rounded = true;
		active = false;
		type = 'g';
	}

	bool Pushed(Vector2 vec)
	{
		anim.index = 145.0f;
		detonated = true;
		return false;
	}
};

class Solid : public GameObject
{
public:
	Solid(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		position = _position;
		anim = Animation(1273.0f, 32.0f, true, 0.0f);
		active = false;
		solid = true;
		type = 's';
	}
};

class Boulder : public GameObject
{
public:
	
	Boulder(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position)
	{
		pushable = true;
		rounded = true;
		position = _position;
		anim = Animation(72.0f, 12.0f, true, 0.0f);
		type = 'b';
		//printf("This is a boulder: (%i,%i)\n", position.x, position.y);
	}

	void Update()
	{
		//bool wasFalling = (velocity == Vector2(0, 1));
		bool wasFalling = (velocity != Vector2(0, 0));
		velocity = Vector2(0, 0);
		GameObject* below = At(Vector2(0, 1));
		if (!below)
		{
			Move(Vector2(0, 1));
			if (!wasFalling)
				globalSound.PlaySound(fallSnd);
		}
		else
		{
			if (wasFalling)
			{
				globalSound.PlaySound(landSnd);
				Landed();
				below->LandedOn();
			}
			if (rolloff && below->rounded)
			{
				if (!At(Vector2(1, 0)) && !At(Vector2(1, 1)))
					Move(Vector2(1, 0));
				else if (!At(Vector2(-1, 0)) && !At(Vector2(-1, 1)))
					Move(Vector2(-1, 0));
			}
		}
	}

	bool Pushed(Vector2 vec)
	{
		if (Get(vec)->type == ' ')
		{
			Move(vec);
			updated = true;
			globalSound.PlaySound("push");
			return true;
		}
		else
			return false;
	}
protected:
	bool rolloff = true;
	const char *fallSnd = "fall", *landSnd = "land";
};

class Diamond : public Boulder
{
public:
	Diamond(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Boulder(_levelGrid, position)
	{
		rounded = true;
		position = _position;
		anim = Animation(160.0f, 8.0f, true, 0.25f);
		animating = true;
		type = '1';
		landSnd = "diamond";
		//printf("This is a boulder: (%i,%i)\n", position.x, position.y);
	}

	void Traveled(bool isPlayer)
	{
		if (isPlayer)
		{
			globalSound.PlaySound("collect");
			AddScore(10);
			AddCollected();
		}
		Dest();
	}
};

class Bomb : public Boulder
{
public:
	Bomb(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Boulder(_levelGrid, position)
	{
		pushable = true;
		rounded = true;
		position = _position;
		anim = Animation(64.0f, 8.0f, true, 0.25f);
		animating = true;
		type = '2';
		//printf("This is a boulder: (%i,%i)\n", position.x, position.y);
	}

	void LandedOn() { Explode(1, '1'); globalSound.PlaySound("explosion"); }
	void Landed() { Explode(1, '1'); globalSound.PlaySound("explosion"); }

	~Bomb() { printf("!bomb\n"); Explode(1, '1'); }
};

class Fragile : public Boulder
{
public:
	Fragile(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Boulder(_levelGrid, position)
	{
		rounded = true;
		position = _position;
		anim = Animation(933.0f, 8.0f, true, 0.25f);
		animating = true;
		type = '3';
	}

	void LandedOn() { Explode(0, '2'); globalSound.PlaySound("breaking"); }
	void Landed() { Explode(0, '2'); globalSound.PlaySound("breaking"); rolloff = false; }

	void Traveled(bool isPlayer)
	{
		if (isPlayer)
		{
			globalSound.PlaySound("collect");
			AddScore(10);
			AddCollected();
		}
		Dest();
	}
};

class TNT : public Boulder
{
public:
	TNT(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Boulder(_levelGrid, position)
	{
		pushable = true;
		rounded = true;
		position = _position;
		anim = Animation(1321.0f);
		animating = true;
		type = '?'; // DONT' FORGET TO REPLACE WITH CORRECT TYPE
	}

	void Detonated() { Explode(1, '1'); globalSound.PlaySound("explosion"); }
	~TNT() {Explode(1, '1'); }
};

class Enemy : public GameObject
{
//This object is a parent for the different enemies in the game, you should never be spawning it directly hence everything is protected
protected:
	Vector2 direction;
	int exploSize = 1;
	char exploType = '1';
	string exploInto = " ";
	Enemy(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, position){}

	//Makes the enemy move clockwise/anticlockwise
	void EnemyMove(int d)
	{
		velocity = Vector2(0, 0);
		Vector2 newDir = direction > d;

		if (!At(newDir)) { Move(newDir); direction = newDir; }
		else if (!At(direction)) { Move(direction); }
		else { direction = direction > (d+2)%4; if (!At(direction)) { Move(direction); } }
	}

	//Explodes if next to player or amoeba
	void AdjExplode()
	{
		if (Adj('p') || (Adj('m')))
			Explode(exploSize, exploType, exploInto);
	}

	//By default, update and LateUpdate functions will AdjExplode
	virtual void Update() { AdjExplode(); }
	virtual void LateUpdate() { AdjExplode(); }

	//If landed on it will also explode
	virtual void LandedOn(){ Explode(exploSize, exploType, exploInto); }

};

//Slimer: Moves anti-clockwise when possible
class Slimer : public Enemy
{
public:
	Slimer(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'A';
		position = _position;
		anim = Animation(576.0f, 16.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
	}

	void Update()
	{
		AdjExplode();
		EnemyMove(1);	
	}
	
};

//Gehu: Moves clockwise when possible
class Gehu : public Enemy
{
public:
	Gehu(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'B';
		position = _position;
		anim = Animation(592.0f, 16.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
	}

	void Update()
	{
		AdjExplode();
		EnemyMove(3);
	}

};

//Froward: Moves in straight lines
class Froward : public Enemy
{
public:
	Froward(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'C';
		position = _position;
		anim = Animation(608.0f);
		direction = Vector2(1, 0) > rand() % 4;
	}

	void Update()
	{
		velocity = Vector2(0, 0);
		if (!At(direction))
			Move(direction);
		else
		{
			direction = direction > rand() % 4;
		}

		if (velocity == Vector2(0, 0))anim.index = 608.0f;
		else if (velocity == Vector2(1, 0))anim.index = 609.0f;
		else if (velocity == Vector2(0, -1))anim.index = 610.0f;
		else if (velocity == Vector2(-1, 0))anim.index = 611.0f;
		else anim.index = 612.0f;
	}
};

//Crystalite: Clockwise, explodes into nine diamonds
class Crystalite : public Enemy
{
public:
	Crystalite(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'B';
		position = _position;
		anim = Animation(679.0f, 32.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
		exploType = '3';
		exploInto = "1";
	}

	void Update()
	{
		AdjExplode();
		EnemyMove(3);
	}

};

//Bouldrend: Anticlockwise, explodes into nine boulders
class Bouldrend : public Enemy
{
public:
	Bouldrend(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'B';
		position = _position;
		anim = Animation(711.0f, 16.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
		exploType = '4';
		exploInto = "b";
	}

	void Update()
	{
		AdjExplode();
		EnemyMove(1);
	}

};

//Shellrad: Explosive enemy, explodes when detonated.
class ShellRad : public Enemy
{
public:
	ShellRad(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'A';
		position = _position;
		anim = Animation(727.0f, 16.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		exploSize = 1;
		exploType = '1';
		exploInto = ' ';
		animating = true;
	}

	void Update()
	{
		AdjExplode();
		EnemyMove(1);
	}

	void Detonated()
	{
		printf("I HAS BEEN DETONATED");
		Explode(exploSize, exploType, exploInto); globalSound.PlaySound("explosion");
	}

	~ShellRad()
	{
		Detonated();
	}

};

//Mimic: Hides in diamonds
class Mimic : public Enemy
{
public:
	bool hidden;
	Mimic(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'J';
		position = _position;
		anim = Animation(782.0f, 30.0f, true, 0.5f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
		hidden = false;
	}

	void Update()
	{
		hidden = false;
		AdjExplode();
		EnemyMove(3);
		if (velocity == Vector2(0, 0))
		{
			int adj = 0;
			Vector2 vec = Vector2(1, 0);
			for (int i=0; i<4; i++)
			{
				
				adj += (Get(vec)->type == '1');
				if (adj == 1)
				{
					hidden = true;
					break;
				}
				vec = vec > 1;
			}
			
		}
	}

	void LateUpdate()
	{
		if (hidden)
		{
			anim.start = 933.0f;
			anim.length = 8.0f;
			anim.speed = 0.25f;
			if (anim.index < 933.0f)
				anim.index = 933.0f;
		}
		else
		{
			anim.start = 782.0f;
			anim.length = 30.0f;
			anim.speed = 0.5f;
			if (anim.index > 812.0f)
				anim.index = 782.0f;
		}
	}

};

//Seeker: Targets player
class Seeker : public Enemy
{
public:
	Seeker(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'G';
		position = _position;
		anim = Animation(613.0f, 8.0f, true, 0.25f);
		animating = true;
	}

	void Update()
	{
		AdjExplode();
		velocity = Vector2(0, 0);
		Vector2 player = GetPlayerPosition();
		Vector2 p_move = Vector2(0,0);
		if (!CanWrapHor())
		{
			p_move.x = sgn(player.x, position.x);
		}
		else
		{
			int w = levelGrid.size(), h = levelGrid[0].size();
			if (abs(player.x-position.x) < w/2-1)
				p_move.x = sgn(player.x, position.x);
			else
				p_move.x = -sgn(player.x, position.x);
		}
		if (!CanWrapVert())
		{
			if (At(p_move))
			{
				p_move.x = 0;
				p_move.y = sgn(player.y, position.y);
			}
		}

		if (!At(p_move))
			Move(p_move);
	}

private:
	bool CanWrapHor() { return !At(Vector2(-position.x, 0)) && !At(Vector2(-position.x - 1, 0)); }
	bool CanWrapVert() { return !At(Vector2(0, -position.y)) && !At(Vector2(0, -position.y - 1)); }
	Vector2 GetPlayerPosition()
	{
		for (int x = 0; x < levelGrid.size(); x++)
			for (int y = 0; y < levelGrid[0].size(); y++)
			{
				if (levelGrid[x][y]->type == 'p')
					return Vector2(x, y);
			}
	}
	int sgn(int x1, int x2)
	{
		return (int)copysign(x1 != x2, x1 - x2);
	}

};

//DeathBall: Just sits there... in waiting...
class DeathBall : public Enemy
{
public:
	DeathBall(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : Enemy(_levelGrid, position)
	{
		type = 'L';
		position = _position;
		anim = Animation(833.0f, 8.0f, true, 0.25f);
		direction = Vector2(1, 0) > rand() % 4;
		animating = true;
		solid = true;
		exploType = '5';
	}

	void Update()
	{
		AdjExplode();
	}

	void LandedOn()
	{
		//Won't explode when landed on
	}

};

//Plasma
class Plasma : public GameObject
{
public:
	Plasma(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, _position)
	{
		type = 'n';
		position = _position;
		anim = Animation(1207.0f, 64.0f, true, 0.1f);
		animating = true;
	}

	void Grow(float probability)
	{
		Vector2 dir(1, 0);
		bool success = false;
		for (int i = 0; i < 4; i++)
		{
			if (!At(dir))
			{
				if (RandomFloat(0.0f, 1.0f) <= probability)
				{
					Vector2 pos = Mod(position + dir);
					GameObject* obj = levelGrid[pos.x][pos.y] = Create("n", levelGrid, pos);
					obj->updated = true;
					success = true;
				}
			}
			dir = dir > 1;
		}
		if (success)globalSound.PlaySound("plasma");
	}
private:
	float probability = 0.1f;
};

//Amoeba
class Amoeba : public GameObject
{
public:
	Amoeba(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, _position)
	{
		type = 'm';
		travels = " d";
		position = _position;
		anim = Animation(0.0f, 64.0f, true, 0.2f);
		animating = true;
	}

	void Grow(float probability)
	{
		Vector2 dir = Vector2(1,0) > rand() % 4;
		if (!At(dir))
		{
			if (RandomFloat(0.0f, 1.0f) <= probability)
			{
				Vector2 pos = Mod(position + dir);
				GameObject* obj = levelGrid[pos.x][pos.y] = Create("m", levelGrid, pos);
				obj->updated = true;
			}
		}
		dir = dir > 1;
	}

private:
	float probability = 0.1f;
};

//Lava
class Lava : public GameObject
{
public:
	Lava(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, _position)
	{
		type = 'l';
		travels = " b123ABDpe";
		position = _position;
		anim = Animation(998.0f, 64.0f, true, 0.2f);
		animating = true;
	}

	void Grow(float probability)
	{
		if (RandomFloat(0.0f, 1.0f) <= probability)
		{
			if (!TryAt(Vector2(0, 1)))
				if (!TryAt(Vector2(1, 0)) && !TryAt(Vector2(-1, 0)))
				{
					if (Get(Vector2(0, -1))->type == ' ')
						TryAt(Vector2(0, -1));
				}
		}
	}

	bool TryAt(Vector2 vec)
	{
		if (!At(vec))
		{
			Vector2 pos = Mod(position + vec);
			GameObject* obj = levelGrid[pos.x][pos.y] = Create("l", levelGrid, pos);
			obj->updated = true;
			return true;
		}
		else
			return false;
	}

private:
	float probability = 0.1f;
};

//Exit door
class ExitDoor : public GameObject
{
public:
	ExitDoor(vector<vector<GameObject*>> &_levelGrid, Vector2 _position) : GameObject(_levelGrid, _position)
	{
		type = 'e';
		position = _position;
		anim = Animation(901.0f);
	}

	void Unlock()
	{
		type = 'u';
		globalSound.PlaySound("unlocked");
		anim.index = 908.0f;
		printf("Exit door unlocked!\n");
	}

	void Traveled(bool isPlayer)
	{
		globalSound.PlaySound("win");
		level_complete = true;
		printf("Level complete!\n");
	}

};

//Explosion object
class Explosion : public GameObject
{
public:
	Explosion(vector<vector<GameObject*>> &_levelGrid, Vector2 _position, char _exploType, string _exploInto = " ") : GameObject(_levelGrid, position, _exploType)
	{
		position = _position;
		exploType = _exploType;
		exploInto = _exploInto;
		type = 'x';
		timer = 4;
		updated = true;
		switch (exploType)
		{
		case '2': {anim = Animation(88.0f, 8.0f, false, 0.25f); break; }
		case '3': {anim = Animation(885.0f, 8.0f, false, 0.25f); break; }
		case '4': {anim = Animation(877.0f, 8.0f, false, 0.25f); break; }
		case '5': {anim = Animation(917.0f, 8.0f, false, 0.25f); break; }
		default: {anim = Animation(909.0f, 8.0f, false, 0.25f); break; }
		}

		animating = true;
	}

	void Update()
	{
		if (--timer == 0)
			levelGrid[position.x][position.y] = Create(exploInto, levelGrid, Vector2(position.x, position.y));
	}
private:
	char exploType;
	string exploInto;
	int timer;
};