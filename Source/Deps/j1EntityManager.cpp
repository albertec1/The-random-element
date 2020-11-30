#include "j1EntityManager.h"
#include "j1Entity.h"
#include "j1EntityPlayer.h"
#include "AutonomousEntity.h"
#include "p2Log.h" 
#include "j1Map.h"
#include "j1Collision.h"
#include "j1Scene.h"

j1EntityManager::j1EntityManager()
{
	name.create("EntityManager");
}

j1EntityManager::~j1EntityManager()
{
	return;
}

bool j1EntityManager::Awake(pugi::xml_node& node)
{
	bool ret = true;

	player = (j1EntityPlayer*)CreateEntity(ENTITY_TYPE::PLAYER, { 0, 0 });

	//airEnemy = (AutonomousEntity*)CreateEntity(ENTITY_TYPE::AIR_ENEMY, { 750, 550 });

	groundEnemy = (AutonomousEntity*)CreateEntity(ENTITY_TYPE::GROUND_ENEMY, { 798, 865 });

	for (p2List_item<j1Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		ret = item->data->Awake(node);
	}
	return ret;
}

bool j1EntityManager::Start()
{
	bool ret = true;

	for (p2List_item<j1Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		ret = item->data->Start();
	}

	return true;
}

bool j1EntityManager::PreUpdate()
{
	bool ret = true;

	for (p2List_item<j1Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		ret = item->data->PreUpdate();
	}

	return true;
}

bool j1EntityManager::Update(float dt)
{
	bool ret = true;

	for (p2List_item<j1Entity*>* EntityIterator = entities.start; EntityIterator != nullptr; EntityIterator = EntityIterator->next)
	{
		ret = EntityIterator->data->Update(dt, false);
		if (ret == false)
		{
			LOG("Entity update error");
		}
	}
	return true;
}

bool j1EntityManager::PostUpdate()
{
	bool ret = true;

	for (p2List_item<j1Entity*>* EntityIterator = entities.start; EntityIterator != nullptr; EntityIterator = EntityIterator->next)
	{
		ret = EntityIterator->data->PostUpdate();
	}

	if (ret == true)
		ret = DrawEntity();

	return ret;
}

bool j1EntityManager::DrawEntity()
{
	bool ret = true;

	for (p2List_item<j1Entity*>* EntityIterator = entities.start; EntityIterator != nullptr; EntityIterator = EntityIterator->next)
	{
		ret = EntityIterator->data->Draw();
	}
	return true;
}

bool j1EntityManager::CleanUp()
{
	bool ret = true;

	for (p2List_item<j1Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		ret = entities.del(item);
	}
	return ret;
}

j1Entity* j1EntityManager::CreateEntity(ENTITY_TYPE type, iPoint initPos)
{
	static_assert((INT)ENTITY_TYPE::UNKNOWN_TYPE == 5, "CODE NEEDS UPDATE");
	j1Entity* ret = nullptr;
	switch (type)
	{
	case ENTITY_TYPE::PLAYER:			ret = new j1EntityPlayer(initPos, type); break;
	case ENTITY_TYPE::GROUND_ENEMY:		ret = new AutonomousEntity(initPos, type); break;
	case ENTITY_TYPE::AIR_ENEMY:		ret = new AutonomousEntity(initPos, type); break;
	case ENTITY_TYPE::UNKNOWN_TYPE:		ret = nullptr; break;
	}

	if (ret == nullptr)
	{
		LOG("Entity type WAS UNKNOWN_TYPE");
		return false;
	}

	entities.add(ret);
	return ret;
}

bool j1EntityManager::DestroyEntity(j1Entity* entity)
{
	p2List_item<j1Entity*>* item = nullptr; item->data = entity;

	item->data->CleanUp();
	entities.del(item);

	entity = nullptr;

	return true;
}

bool j1EntityManager::DestroyAllEntities()
{
	for (p2List_item<j1Entity*>* item = entities.start; entities.count() > 0; item = item->next)
	{
		item->data->CleanUp();
		entities.del(item);
	}
	return true;
}



void j1EntityManager::OnCollision(Collider* c1, Collider* c2)
{
	
	if (c1->type == PLAYER)
	{
		iPoint wall_position = App->map->WorldToMap(c2->rect.x, c2->rect.y);
		uint tileid = App->map->metadata->GetLayerPositon(wall_position.x, wall_position.y);

		if (c1->rect.y < (c2->rect.y + c2->rect.h) && c1->rect.y < c2->rect.y
			&& (c1->rect.y + c1->rect.h) >= c2->rect.y && (c1->rect.y + c1->rect.h) < (c2->rect.y + c2->rect.h) && App->coll->canCollide_top(tileid)) //Player collides from top
		{
			if (c2->type == WALL || c2->type == PLATFORM)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c1->callback;
				callback->current_position.y = c2->rect.y - c2->rect.h;

				if (player->jumping == false)
				{
					if (player->jump_available == false)
						player->jump_available = true;
				}
			}
		}

		else if (c1->rect.x > c2->rect.x && c1->rect.x < (c2->rect.x + c2->rect.w) &&
			(c1->rect.x + c1->rect.w) > c2->rect.x && (c1->rect.x + c1->rect.w) > (c2->rect.x + c2->rect.w) && App->coll->canCollide_right(tileid)) //Player collides from right side
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c1->callback;
				callback->current_position.x = c2->rect.x + c2->rect.w;
				player->current_acceleration.x = 0;
			}
		}

		else if (c1->rect.x < c2->rect.x && c1->rect.x < (c2->rect.x + c2->rect.w) &&
			(c1->rect.x + c1->rect.w) > c2->rect.x && (c1->rect.x + c1->rect.w) < (c2->rect.x + c2->rect.w) && App->coll->canCollide_left(tileid))//Player collides from left side
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c1->callback;
				callback->current_position.x = c2->rect.x - c1->rect.w;
				player->current_acceleration.x = 0;	
			}		
		}			

		else if (c1->rect.y > c2->rect.y && c1->rect.y < (c2->rect.y + c2->rect.h) &&
			(c1->rect.y + c1->rect.h)>(c2->rect.y + c2->rect.h) && (c1->rect.y + c1->rect.h) > c2->rect.y && App->coll->canCollide_bottom(tileid))//Player collides from bottom
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c1->callback;
				callback->current_position.y = c2->rect.y + c2->rect.h;
			}
		}

	}

	if (c2->type == PLAYER)
	{
		iPoint wall_position = App->map->WorldToMap(c1->rect.x, c1->rect.y);
		uint tileid = App->map->metadata->GetLayerPositon(wall_position.x, wall_position.y);

		if (c2->rect.y < (c1->rect.y + c1->rect.h) && c2->rect.y < c1->rect.y &&
			(c2->rect.y + c2->rect.h) >= c1->rect.y && (c2->rect.y + c2->rect.h) < (c1->rect.y + c1->rect.h) && App->coll->canCollide_top(tileid)) //Player collides from top
		{
			if (c2->type == WALL || c2->type == PLATFORM)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c2->callback;
				callback->current_position.y = c1->rect.y - c1->rect.h;

				if (player->jumping == false)
				{
					if (player->jump_available == false)
						player->jump_available = true;

					if (player->jump_available == true)
					{
						player->current_velocity.y = 0;
						player->current_acceleration.y = 0;
					}

				}
			}
		}

		else if (c2->rect.x > c1->rect.x && c2->rect.x < (c1->rect.x + c1->rect.w) &&
			(c2->rect.x + c2->rect.w) > c1->rect.x && (c2->rect.x + c2->rect.w) > (c1->rect.x + c1->rect.w) && App->coll->canCollide_right(tileid)) //Player collides from right side
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c2->callback;
				callback->current_position.x = c1->rect.x + c1->rect.w;
				player->current_acceleration.x = 0;
			}
		}

		else if (c2->rect.x < c1->rect.x && c2->rect.x < (c1->rect.x + c1->rect.w) &&
			(c2->rect.x + c2->rect.w) > c1->rect.x && (c2->rect.x + c2->rect.w) < (c1->rect.x + c1->rect.w) && App->coll->canCollide_left(tileid))//Player collides from left side
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c2->callback;
				callback->current_position.x = c1->rect.x - c2->rect.w;
				player->current_acceleration.x = 0;
			}
		}

		else if (c2->rect.y > c1->rect.y && c2->rect.y < (c1->rect.y + c1->rect.h) &&
			(c2->rect.y + c2->rect.h)>(c1->rect.y + c1->rect.h) && (c2->rect.y + c2->rect.h) > c1->rect.y && App->coll->canCollide_bottom(tileid))//Player collides from bottom
		{
			if (c2->type == WALL)
			{
				j1EntityPlayer* callback = (j1EntityPlayer*)c2->callback;
				callback->current_position.y = c1->rect.y + c1->rect.h;
				player->current_velocity.y = 0;
				player->current_acceleration.y = 0;
			}
		}
	}	
} 

bool j1EntityManager::Save(pugi::xml_node& data)
{
	pugi::xml_node node = data.append_child("entity_manager");
	pugi::xml_node ent_node = node.append_child("entity");

	for (p2List_item<j1Entity*>* entity = entities.start; entity != nullptr; entity = entity->next)
	{
		pugi::xml_node data = ent_node.append_child("data");
		data.append_attribute("x").set_value(entity->data->current_position.x);
		data.append_attribute("y").set_value(entity->data->current_position.y);
		data.append_attribute("type").set_value((int)entity->data->type);

		/*pugi::xml_node restore = ent_node.append_child("restore");
		restore.append_attribute("to_delete").set_value(entity->to_delete);
		restore.append_attribute("health").set_value(entity->health);*/
	}

	pugi::xml_node end = ent_node.append_child("data");
	end.append_attribute("x").set_value(-1);	// after the last child, I create another one with the "x" attribute set to -1 
												//(which would never happen under normal circumstances) to indicate where to stop loading entities

	return true;
}

bool j1EntityManager::Load(pugi::xml_node& data)
{
	if (App->save_document_full == true)
	{
		pugi::xml_node node = data.child("entity_manager").child("entity");

		for (node; node != NULL; node = node.next_sibling("entity"))
		{
			pugi::xml_node data = node.child("data");

			float x = data.attribute("x").as_float();
			float y = data.attribute("y").as_float();
			ENTITY_TYPE type = (ENTITY_TYPE)data.attribute("type").as_int();
			
			if (x == -1)
			{
				break;
			}

			if (type == ENTITY_TYPE::PLAYER )
			{
				player->ResetPlayerAT(x, y);
			}
			//pugi::xml_node restore = node.child("restore");
			//entity->to_delete = restore.attribute("to_delete").as_bool;
			//entity->health = restore.attribute("health").as_bool;

		}
	}
	return true;
}