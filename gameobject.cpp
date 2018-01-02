#include "gameobject.h"
#include "level.h"

//Move position
void GameObject::Move(GameObject::Vector2 vec)
{
	velocity = vec;
	int x = position.x, y = position.y;
	levelGrid[position.x][position.y] = new Space(levelGrid, Vector2(position.x,position.y), this);
	position = GameObject::Mod(position + vec);
	levelGrid[position.x][position.y] = this;
}

//Destroy self
void GameObject::Dest()
{
	levelGrid[position.x][position.y] = new Space(levelGrid, Vector2(position.x, position.y), this);
}

//Get object at relative position
GameObject* GameObject::Get(GameObject::Vector2 vec)
{
	Vector2 vec2;
	vec2 = GameObject::Mod(position + vec);
	return levelGrid[vec2.x][vec2.y];
}

//Get if position is free
GameObject* GameObject::At(GameObject::Vector2 vec)
{
	GameObject* obj = Get(vec);
	if (obj->solid || travels.find(obj->type) == -1)
		return obj;
	else
		return NULL;
}

//Get vector2 within bounds of level
GameObject::Vector2 GameObject::Mod(GameObject::Vector2 vec)
{
	Vector2 vec2;
	vec2.x = (vec.x + levelGrid.size()) % levelGrid.size();
	vec2.y = (vec.y + levelGrid[0].size()) % levelGrid[0].size();
	return vec2;
}

//Explosion script
void GameObject::Explode(int size, char exploType, string exploInto)
{
	Vector2 pos;
	for (int x=-size; x<=size; x++)
		for (int y = -size; y <= size; y++)
		{
			SDL_Log("pos x:%i, y:%i", pos.x, pos.y);
			pos = GameObject::Mod(Vector2(position.x + x, position.y + y));
			GameObject* get = GameObject::Get(Vector2(x, y));
			if (!get->solid || get->type==' ')
				levelGrid[pos.x][pos.y] = new Explosion(levelGrid, Vector2(pos.x, pos.y), exploType, exploInto);
		}

	if (exploType == '0')globalSound.PlaySound("explosion");
	else if (exploType == '1')globalSound.PlaySound("explosion");
	else if (exploType == '3')globalSound.PlaySound("dcreate");
	else if (exploType == '4')globalSound.PlaySound("bcreate");
	else if (exploType == '5')globalSound.PlaySound("explosion2");
}

//Get if object is adjacent to object of type c
bool GameObject::Adj(char c)
{
	Vector2 dir(1, 0);
	for (int i = 0; i < 4; i++)
	{
		if (Get(Vector2(dir))->type == c)
			return true;
		else
			dir = dir > 1;
	}
	return false;
}

//Add score
void GameObject::AddScore(int n)
{
	game_score += n;
	printf("Added the score by %i! Score is now %i\n", n, game_score);
}

//Add score
void GameObject::AddCollected()
{
	collected++;
	printf("Incremented collected to %i\n", collected);
}

GameObject* GameObject::Create(string type, vector<vector<GameObject*>> &_levelGrid, Vector2 _position)
{
	GameObject* object;
	char c = type[0];
	switch (c)
	{
	case 'b': {object = new Boulder(_levelGrid, _position); break; }
	case '1': {object = new Diamond(_levelGrid, _position); break; }
	case '2': {object = new Bomb(_levelGrid, _position); break; }
	case '3': {object = new Fragile(_levelGrid, _position); break; }
	case 'A': {object = new Slimer(_levelGrid, _position); break; }
	case 'B': {object = new Gehu(_levelGrid, _position); break; }
	case 'C': {object = new Froward(_levelGrid, _position); break; }
	case 'D': {object = new Crystalite(_levelGrid, _position); break; }
	case 'E': {object = new Bouldrend(_levelGrid, _position); break; }
	case 'd': {object = new Dirt(_levelGrid, _position); break; }
	case 'w': {object = new Wall(_levelGrid, _position); break; }
	case 's': {object = new Solid(_levelGrid, _position); break; }
	case 'p': {object = new Player(_levelGrid, _position); break; }
	case 'e': {object = new ExitDoor(_levelGrid, _position); break; }
	case 'n': {object = new Plasma(_levelGrid, _position); break; }
	case 'm': {object = new Amoeba(_levelGrid, _position); break; }
	case 'l': {object = new Lava(_levelGrid, _position); break; }
	case 'g': {object = new Detonator(_levelGrid, _position); break; }
	//case 'j': {object = new KeyDoor(_levelGrid, _position); break; } 
	//case 'k': {object = new Key(_levelGrid, _position); break; }
	case 'F': {object = new ShellRad(_levelGrid, _position); break; }
	case 'G': {object = new Seeker(_levelGrid, _position); break; }
	//case 'H': {object = new Eater(_levelGrid, _position); break; }
	//case 'I': {object = new Digger(_levelGrid, _position); break; }
	case 'J': {object = new Mimic(_levelGrid, _position); break; }
	//case 'K': {object = new Ephorsu(_levelGrid, _position); break; }
	case 'L': {object = new DeathBall(_levelGrid, _position); break; }
	//case 'M': {object = new Spynoid(_levelGrid, _position); break; }
	//case 'x': {object = new HorizontalWall(_levelGrid, _position); break; }
	//case 'y': {object = new VerticalWall(_levelGrid, _position); break; }
	//case 'z': {object = new MagicWall(_levelGrid, _position); break; }
	//case '4': {object = new MegaBomb(_levelGrid, _position); break; }
	//case '5': {object = new Ore(_levelGrid, _position); break; }
	case '6': {object = new TNT(_levelGrid, _position); break; }
	//case 'a': {object = new Slipper(_levelGrid, _position); break; }
	//case 'c': {object = new Crate(_levelGrid, _position); break; }
	//case 't': {object = new Tube(_levelGrid, _position); break; }
	default: {object = new Space(_levelGrid, _position); break; }
	}

	return object;
}

float GameObject::RandomFloat(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;
	return (random*range) + min;
}

map<int, GameObject*> GameObject::objects;
int GameObject::currentId = 0;
GameObject::Vector2 left = GameObject::Vector2(-1, 0);
GameObject::Vector2 right = GameObject::Vector2(1, 0);
GameObject::Vector2 up = GameObject::Vector2(0, -1);
GameObject::Vector2 down = GameObject::Vector2(0, 1);
GameObject::Vector2 zero = GameObject::Vector2(0, 0);