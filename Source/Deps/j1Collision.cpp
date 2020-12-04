#include "j1Collision.h"
#include "j1Window.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Map.h"
#include "j1Entity.h"
#include "j1EntityManager.h"
#include "j1EntityPlayer.h"

j1Collision::j1Collision()
{
	matrix[WALL][WALL] = false;
	matrix[WALL][PLATFORM] = false;
	matrix[WALL][BONUS] = false;
	matrix[WALL][PLAYER] = true;
	matrix[WALL][ENEMY] = true;

	matrix[PLATFORM][WALL] = false;
	matrix[PLATFORM][PLATFORM] = false;
	matrix[PLATFORM][BONUS] = false;
	matrix[PLATFORM][PLAYER] = true;
	matrix[PLATFORM][ENEMY] = true;

	matrix[BONUS][WALL] = false;
	matrix[BONUS][PLATFORM] = false;
	matrix[BONUS][BONUS] = false;
	matrix[BONUS][PLAYER] = true;
	matrix[BONUS][ENEMY] = false;

	matrix[PLAYER][WALL] = true;
	matrix[PLAYER][PLATFORM] = true;
	matrix[PLAYER][BONUS] = true;
	matrix[PLAYER][PLAYER] = false;
	matrix[PLAYER][ENEMY] = true;

	matrix[ENEMY][WALL] = true;
	matrix[ENEMY][PLATFORM] = true;
	matrix[ENEMY][BONUS] = false;
	matrix[ENEMY][PLAYER] = true;
	matrix[ENEMY][ENEMY] = false;

	win_width = 0;
	win_height = 0;

	for (int i = 0; i < MAX_COLLIDERS; i++)
	{
		colliders[i] = new Collider;
	}
}

j1Collision::~j1Collision()
{}

bool j1Collision::Awake(pugi::xml_node& node)
{
	bool ret = true;

	name.create("collison"); 

	App->win->GetWindowSize(win_width, win_height);
	return ret;
}

bool j1Collision::PostUpdate()
{
	bool ret = true;

	// Remove all colliders scheduled for deletion 
	//exclude colliders that are not in camera and colliders that are not entities.
	/*for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i]->active == true && colliders[i]->to_delete == true)
		{
			colliders[i]->active = false;
		}
	}*/
	int width = App->map->metadata->width;
	int height = App->map->metadata->height;
	SDL_Rect cam = App->map->MapCulling({width, height }, 10, 10);
	int i = 0;
	for (int i = 0; i <= MAX_ENTITIES; i++)
	{
		if (colliders[i]->active == true)
			if(colliders[i]->to_delete == true)
			{
				colliders[i]->active = false;
			}
	}

	// Calculate collisions
	Collider* c1;
	Collider* c2;

	i = 0;
	int k = 0;
	bool entitiesChecked = false;
	bool entitiesChecked2 = false;

	////for (uint i = 0; i < MAX_COLLIDERS; ++i)
	//for (int y = cam.y; y < cam.h; y++)
	//	for (int x = 0; x < cam.w; x++)
	//{
	//		if (!entitiesChecked)
	//		{
	//			if (y == 0 && x == MAX_ENTITIES - 1)
	//			{
	//				x += cam.x;
	//				entitiesChecked = true;
	//			} //when entities finish, locate the x at the beggining of the screen
	//		}
	//		i = (y * width) + x; //check for every collider in the screen
	//		
	//			
	//	if (i >= MAX_COLLIDERS)
	//		continue;
	//	if (colliders[i] == nullptr || colliders[i]->active == false)// skip empty colliders
	//		continue;
	//	c1 = colliders[i];

	//	// avoid checking collisions already checked
	//	for (int y2 = cam.y; y2 < cam.h; y2++)
	//		for (int x2 = cam.x; x2 < cam.w; x2++)
	//	{
	//			if (!entitiesChecked2)
	//			{
	//				if (y2 == 0 && x2 == MAX_ENTITIES - 1)
	//				{
	//					x2 += cam.x;
	//					entitiesChecked2 = true;
	//				} //when entities finish, locate the x at the beggining of the screen
	//			}

	//		k = (y2 * width) + x2; //check for every collider in the screen
	//			
	//		if (k >= MAX_COLLIDERS)
	//			continue;
	//		if (colliders[k]->active == false || colliders[k] == nullptr )// skip empty or inactive colliders
	//			continue;
	//		if (x2 == x && y2 == y)
	//			continue;

	//		c2 = colliders[k];
	//		App->render->DrawQuad(c2->rect, 200, 50, 0, 255);

	//		if (c1->CheckCollision(c2->rect) == true) //what if we put the matrix check before looking if they actually collide? performance improvement?
	//		{
	//			if (c1->type == PLAYER || c2->type == PLAYER)
	//			{
	//				if (App->manager->player->god_mode == false)
	//				{
	//					App->manager->OnCollision(c1, c2);
	//				}
	//			}
	//			else
	//			{
	//				LOG("Collision!");
	//				/*c1->callback->OnCollision(c1, c2);
	//				c2->callback->OnCollision(c2, c1);*/
	//			}					
	//		}
	//	}
	//}

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		// skip empty colliders
		if (colliders[i] == nullptr)
			continue;
		c1 = colliders[i];

		// avoid checking collisions already checked
		for (uint k = i + 1; k < MAX_COLLIDERS; ++k)
		{
			// skip empty colliders
			if (colliders[k] == nullptr)
				continue;

			c2 = colliders[k];

			if (c1->CheckCollision(c2->rect) == true) //what if we put the matrix check before looking if they actually collide? performance improvement?
			{
				if (c1->type == PLAYER || c2->type == PLAYER)
				{
					if (App->manager->player->god_mode == false)
					{
						App->manager->OnCollision(c1, c2);
					}
				}
				else
				{
					LOG("Collision!");
					/*c1->callback->OnCollision(c1, c2);
					c2->callback->OnCollision(c2, c1);*/
				}
			}
		}
	}
	return ret;
}

bool j1Collision::canCollide_right(uint tile_id) //we get this id from the x and y value of the collider and with the function get()
{
	bool ret = false;

	if (App->map->metadata->data[tile_id + 1] == NULL)
	{
		ret = true;
	}
	
	return ret;
}
bool j1Collision::canCollide_left(uint tile_id) //we get this id from the x and y value of the collider and with the function get()
{
	bool ret = false;
	
	if (App->map->metadata->data[tile_id - 1] == NULL)
	{
		ret = true;		
	}
	
	return ret;
}

bool j1Collision::canCollide_top(uint tile_id) //we get this id from the x and y value of the collider and with the function get()
{
	bool ret = false;

	if (App->map->metadata->data[tile_id - App->map->metadata->width] == NULL)
	{
		ret = true;		
	}

	return ret;
}

bool j1Collision::canCollide_bottom(uint tile_id) //we get this id from the x and y value of the collider and with the function get()
{
	bool ret = false;

	if (App->map->metadata->data[tile_id + App->map->metadata->width] == NULL)
	{
		ret = true;
	}

	return ret;
}

bool j1Collision::CleanUp()
{
	LOG("Freeing all colliders");

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	return true;
}

Collider* j1Collision::AddCollider(SDL_Rect _rect, COLLIDER_TYPE _type, j1Entity*_callback)
{
	Collider* ret = nullptr;

	/*for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
		{
			ret = colliders[i] = new Collider(rect, type, callback);
			break;
		}
	}*/

	if (_type == COLLIDER_TYPE::ENEMY || _type == COLLIDER_TYPE::PLAYER) //first ones are entity colliders
	{
		for (uint i = 0; i < MAX_ENTITIES; ++i)
		{
			if (colliders[i]->active == false)
			{
				colliders[i]->rect = _rect;
				colliders[i]->type = _type;
				colliders[i]->callback = _callback;
				colliders[i]->active = true;
				colliders[i]->to_delete = false;

				ret = colliders[i];
				break;
			}
		}
	}
	else
	{
		for (uint i = MAX_ENTITIES; i < MAX_COLLIDERS; ++i) //then normal colliders (walls, platforms, etc...)
		{
			if (colliders[i]->active == false)
			{
				colliders[i]->rect = _rect;
				colliders[i]->type = _type;
				colliders[i]->callback = _callback;
				colliders[i]->active = true;
				colliders[i]->to_delete = false;

				ret = colliders[i];
				break;
			}
		}
	}
	return ret;
}

void j1Collision::DebugDraw()
{
	/*if (App->input->GetKey(SDL_SCANCODE_F2) == j1KeyState::KEY_DOWN)
		debug = !debug;

	if (debug == false)
		return;*/

	debug = true;

	Uint8 alpha = 80;
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		if ((colliders[i]->rect.x + 32) > -(App->render->camera.x) && colliders[i]->rect.x <= (-(App->render->camera.x) + (int)win_width + 32) &&
			(colliders[i]->rect.y + 32) >= -(App->render->camera.y) && colliders[i]->rect.y <= (-(App->render->camera.y) + (int)win_height + 32))
		{
			switch (colliders[i]->type)
			{
			case COLLIDER_NONE: // white
				App->render->DrawQuad(colliders[i]->rect, 255, 255, 255, alpha);
				break;

			case WALL: // green
				App->render->DrawQuad(colliders[i]->rect, 0, 255, 0, alpha);
				break;

			case PLATFORM: // blue
				App->render->DrawQuad(colliders[i]->rect, 0, 0, 255, alpha);
				break;

			case ENEMY: // red
				App->render->DrawQuad(colliders[i]->rect, 255, 0, 0, alpha);
				break;

			case PLAYER: // yellow
				App->render->DrawQuad(colliders[i]->rect, 255, 255, 0, alpha);
				break;

			case BONUS: // pink
				App->render->DrawQuad(colliders[i]->rect, 255, 0, 255, alpha);
				break;
			}

		}
	}
}

Collider::Collider()
{
	rect = { 0,0,0,0 };
	type = COLLIDER_TYPE::COLLIDER_NONE;
	callback = nullptr;
	to_delete = false;
	active = false;
}

Collider::Collider(SDL_Rect rectangle, COLLIDER_TYPE type, j1Entity* callback) :
	rect(rectangle),
	type(type),
	callback(callback)
{
	to_delete = false;
	active = true;
}

bool Collider::CheckCollision(const SDL_Rect& r) const
{
	// Return true if there is an overlap
	// between argument "r" and property "rect"                                                                                                              

	if (((r.x + r.w) > (rect.x)) && ((r.x) < (rect.x + rect.w))
		&& ((r.y + r.h) > (rect.y)) && ((r.y) < (rect.y + rect.h)))
	{
		return true;
	}


	return false;
}
