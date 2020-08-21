#include "p2Defs.h"
#include "p2Log.h"
#include "j1Map.h"
#include "j1Window.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Collision.h"

j1Map::j1Map() : j1Module()
{
	name.create("map");
}

j1Map::~j1Map()
{}

bool j1Map::Awake(pugi::xml_node& node)
{
	debug_metadata = false;
	LOG("Loading Map Parser");
	bool ret = true;

	folder.create(node.child("folder").child_value());
	LOG("folder name: %s", folder.GetString());

	return ret;
}

bool j1Map::Load(const char* file_name)
{
	bool ret = true;

	p2SString tmp(folder.GetString());
	tmp += file_name;
	LOG("Map File Path: %s", tmp.GetString());

	pugi::xml_parse_result result = map_file.load_file(tmp.GetString());

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	//Load general info -------------------------
	if ( ret == true )
		ret = LoadMap();

	//Load all tilesets info --------------------
	pugi::xml_node tileset_node;
	for (tileset_node = map_file.child("map").child("tileset"); tileset_node && ret; tileset_node = tileset_node.next_sibling("tileset"))
	{
		TilesetData* tileset_data = new TilesetData();

		if (ret == true)
		{
			ret = LoadTilesetDetails(tileset_node, tileset_data);
		}

		if (ret == true)
		{
			ret = LoadTilesetImage(tileset_node, tileset_data);
		}

		data.tilesets.add(tileset_data);
	}

	//Load layer info ----------------------------
	pugi::xml_node layer;
	for (layer = map_file.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if (ret == true)
			data.layers.add(lay);
	}

	if (ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
	}

	map_loaded = ret;

	return ret;
}

bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");
	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}

	else
	{
		data.map_version =		map.attribute("version").as_float();
		data.orientation =		map.attribute("orientation").as_string();
		data.render_order =		map.attribute("renderorder").as_string();
		data.map_width =		map.attribute("width").as_int();
		data.map_height =		map.attribute("height").as_int();
		data.tile_width =		map.attribute("tilewidth").as_int();
		data.tile_height =		map.attribute("tileheight").as_int();
		data.next_layer_id =	map.attribute("nextlayerid").as_int();
		data.next_object_id =	map.attribute("nextobjectid").as_int();

		/*background color?*/
	}

	if (data.orientation == "orthogonal")
	{
		data.map_type = MAPTYPE_ORTHOGONAL;
	}
	else if (data.orientation == "isometric")
	{
		data.map_type = MAPTYPE_ISOMETRIC;
	}
	else if (data.orientation == "staggered")
	{
		data.map_type = MAPTYPE_STAGGERED;
	}
	else
	{
		data.map_type = MAPTYPE_UNKNOWN;
	}

	//LOG map details---------
	if (App->allow_debug_log == true)
	{
		LOG("\n --------- MAP DETAILS ----------\n- map_version: %0.2f \n- orientation: %s \n- render_order: %s \n- map_width: %d \n- map_height: %d \n- tile_width: %d \n- tile_height: %d \n- next_layer_id: %d \n- next_object_id: %d",
			data.map_version,
			data.orientation.GetString(),
			data.render_order.GetString(),
			data.map_width,
			data.map_height,
			data.tile_width,
			data.tile_height,
			data.next_layer_id,
			data.next_object_id);

	}
	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TilesetData* set)
{
	bool ret = true;

	set->name.create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if (offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	//LOG Tileset details----------
	if (App->allow_debug_log == true)
	{
		LOG("\n --------- TILESET DETAILS ----------\n- name: %s \n- firstgid: %d \n- tile_width: %d \n- tile_height: %d \n- margin: %d \n- spacing: %d",
			set->name.GetString(),
			set->firstgid,
			set->tile_width,
			set->tile_height,
			set->margin,
			set->spacing);
	}
	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TilesetData* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");
	if (image == NULL)
	{
		LOG("Error parsng tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);

		set->tex_width = image.attribute("width").as_int();
		if (set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if (set->tex_height <= 0)
		{
			set->tex_height = h;
		}
	}

	set->num_tiles_width = set->tex_width / set->tile_width;
	set->num_tiles_height = set->tex_height / set->tile_height;
	
	//LOG Tileset Image details----------
	if (App->allow_debug_log == true)
	{
		LOG("\n --------- TILESET IMAGE DETAILS ----------\n- tex_width: %d \n- tex_height: %d \n- num_tiles_width: %d \n- num_tiles_height: %d",
			set->tex_width,
			set->tex_height,
			set->num_tiles_width,
			set->num_tiles_height);
	}
	return ret;
}

bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name =	node.attribute("name").as_string();
	layer->id =		node.attribute("id").as_int();
	layer->width =	node.attribute("width").as_int();
	layer->height =	node.attribute("height").as_int();
	LoadProperties(node, layer->properties);

	pugi::xml_node layer_data = node.child("data");

	if (layer_data == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layer);
	}
	else
	{
		layer->data = new uint[layer->width * layer->height];
		memset(layer->data, 0, layer->width * layer->height);

		int i = 0;
		for (pugi::xml_node tile = layer_data.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data[i++] = tile.attribute("gid").as_int(0);
		}
	}

	//LOG Layer details------------
	if (App->allow_debug_log == true)
	{
		LOG("\n --------- LAYER %d DETAILS ----------\n- name: %s \n- width: %d \n- height: %d",
			layer->id,
			layer->name.GetString(),
			layer->width,
			layer->height);
	}
	return ret;
}

bool j1Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if (data != NULL)
	{
		pugi::xml_node prop;

		for (prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list.add(p);
		}
	}
	return ret;
}
 
void j1Map::Draw()
{
	if (map_loaded == false)
		return;

	p2List_item<MapLayer*>* current_layer = data.layers.start;
	int properties_count = current_layer->data->properties.list.count();

	SDL_Rect tile_rect;
	tile_rect.w = data.tile_width;
	tile_rect.h = data.tile_height;

	for (current_layer; current_layer != nullptr; current_layer = current_layer->next)
	{
		if (current_layer->data->properties.Get("Draw") == 1 || debug_metadata == true)
		{

			for (int y = 0; y < data.map_height; ++y)
			{
				for (int x = 0; x < data.map_width; ++x)
				{
					iPoint tileCoords = MapToWorld(x, y);
					int tile_id = current_layer->data->Get(x, y);

					TilesetData* tileset = GetTilesetFromTileId(tile_id);
					if (tileset != nullptr)
					{
						SDL_Rect r = tileset->GetTileRect(tile_id);
						iPoint pos = MapToWorld(x, y);

						App->render->Blit(tileset->texture, pos.x, pos.y, &r);
					}
				}
			}
		}

		if (current_layer->data->properties.Get("Navigation") == 1)
		{
			for (int y = 0; y < data.map_height; ++y)
			{
				for (int x = 0; x < data.map_width; ++x)
				{
					int tile_id = current_layer->data->Get(x, y);
					if (tile_id > 0)
					{
						TilesetData* tileset = GetTilesetFromTileId(tile_id);
						if (tileset != nullptr)
						{
							iPoint pos = MapToWorld(x, y);

							tile_rect.x = pos.x;
							tile_rect.y = pos.y;

							if (tile_id == WALL_ID)
								App->coll->AddCollider(tile_rect, COLLIDER_TYPE::WALL);

							else if (tile_id == PLATFORM_ID)
								App->coll->AddCollider(tile_rect, COLLIDER_TYPE::PLATFORM);

							else if (tile_id == BONUS_ID)
								App->coll->AddCollider(tile_rect, COLLIDER_TYPE::BONUS);
						}
					}
				}
			}
			App->coll->DebugDraw();
		}
	}
}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if (data.map_type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * data.tile_width;
		ret.y = y * data.tile_height;
	}
	else if (data.map_type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (data.tile_width * 0.5f);
		ret.y = (x + y) * (data.tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}
	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	if (data.map_type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / data.tile_width;
		ret.y = y / data.tile_height;
	}
	else if (data.map_type == MAPTYPE_ISOMETRIC)
	{

		float half_width = data.tile_width * 0.5f;
		float half_height = data.tile_height * 0.5f;
		ret.x = int((x / half_width + y / half_height) / 2);
		ret.y = int((y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

SDL_Rect TilesetData::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

TilesetData* j1Map::GetTilesetFromTileId(int id) const
{
	p2List_item<TilesetData*>* actual_tileset = data.tilesets.start;

	while (actual_tileset->next && id >= actual_tileset->next->data->firstgid)
	{
		actual_tileset = actual_tileset->next;
	}
	return actual_tileset->data;
}

int Properties::Get(const char* name, int default_value) const
{
	p2List_item<Property*>* item = list.start;

	while (item)
	{
		if (item->data->name == name)
			return item->data->value;
		item = item->next;
	}

	return default_value;
}

bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	p2List_item<TilesetData*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// Remove all layers
	p2List_item<MapLayer*>* item2;
	item2 = data.layers.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	data.layers.clear();

	// Clean up the pugui tree
	map_file.reset();

	return true;
}


