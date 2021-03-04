#ifndef LEVELGENERATOR_H
#define LEVELGENERATOR_H
#include "enemyclass.h"
#define MW            1500	//Tile map width
#define MH            1500	//Tile map height
#define TS            64	//Tile square size
#define ROOMSIZE	  15	//Width and height of tiles per room (ROOMSIZE x ROOMSIZE)

extern int map[MW][MH];		//Defenition of the tile map	
extern int roomCount;		//Current amount of rooms generated
extern int roomMax;			//Desired number of rooms to be generated
extern int exitXpos, exitYpos;	//Tile position of the door blocking the exit

struct Level {
	int levelNum;			//Increases by 1 for each new floor
	int keyRooms;			//Intended number of key rooms
	int spawnedKeyRooms;	//Created number of key rooms
};

extern Level level;

struct Key {
	int active;		//0 = No key, 1 = key
	int xpos, ypos;	//Key position in world space
};

extern Key key[100];	

struct PickupList {
	int active;
	int type; //0 = half heart, 1 = full heart, 2 = arrow
	int xpos, ypos;
};

struct Room {  // Structure for each room
	PickupList pickupList[2];
	Enemy enemy[4];
	bool occupied;		//Marked true when a room is spawned
	bool examined;		//Marked true when a room has been checked by CheckSurroundingRooms method
	bool visited;		//Used to track player visted rooms for minimap
	int roomType;		//0-14 room types
	int roomContents;	//0 is normal room, 1 is starting room, 2 is key room, 3 is exit room
	int roomStructure;	//0-7 defines internal wall structure and spawn points for collectables and enemies
	int northPath;		//0 is no path(wall), 1 is empty path, 2 is already connected room path
	int eastPath;
	int southPath;
	int westPath;
	int enemyCount;		//Number of alive enemies in a room
};

extern Room floorGrid[100][100];

void StartGenerator(void);
void DrawRoom(int, int, int);
void StructureRoom(int xRoom, int yRoom);
void ChooseItemType(int roomX, int roomY, int i);
void SpawnRoom(int roomType, int roomX, int roomY);
void CloseRooms(void);
void ChooseRoom(void);
void CheckSurroundingRooms(int roomX, int roomY);
void SetRoomValues(int roomType, int roomX, int roomY);
void InitMap(void);

#endif