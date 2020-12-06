#pragma once

#include "j1Module.h"
#include "SDL/include/SDL.h"
#include "p2Point.h"

struct Collider;
struct SDL_Texture;

enum class ENTITY_TYPE
{
	PLAYER,
	AIR_ENEMY,
	GROUND_ENEMY,
	UNDERGROUND_ENEMY,
	DEBUG,
	UNKNOWN_TYPE,
};

class j1Entity : public j1Module
{
public:
	j1Entity(fPoint pos, ENTITY_TYPE type);

	~j1Entity();

	virtual bool Start();

	virtual bool PreUpdate();

	virtual bool Update(float dt, bool do_logic);

	virtual bool PostUpdate();

	virtual bool CleanUp();

	virtual bool Draw();

	virtual void OnCollision(Collider* c1, Collider* c2);

	virtual const Collider* GetCollider()
	{
		return entity_collider;
	}

	virtual const fPoint GetStartingPosition()
	{
		return starting_position;
	}

	virtual void SetTexture(SDL_Texture* texture) 
	{
		entity_texture = texture;
	}

public:
	ENTITY_TYPE		type;
	Collider*		entity_collider;

	fPoint			starting_position;
	fPoint			current_position;

	SDL_Texture*	entity_texture;
	SDL_Rect		entity_rect;
	iPoint			rect_size;

	iPoint			sprite_size;
	bool			flipped;
	//save and load?

	friend class j1EntityManager;
};