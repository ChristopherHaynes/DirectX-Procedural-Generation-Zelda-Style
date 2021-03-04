//Chris Haynes Dungeon Generator
#include<iostream>
#include<time.h>
#include "levelgenerator.h"

int map[MW][MH];
Room floorGrid[100][100];
Key key[100];

int roomMax = 5; //The Maximum rooms to be generated
int roomCount = 0;  //The current count of rooms generated
int exitXpos = 0, exitYpos = 0;

int potentialRooms = 0; //The amount of viable paths from the current floorgrid

struct RoomCoord {	//Structure for X and Y position of room which can connect to current room
	int roomX;
	int roomY;
} potentialRoom[1000];

void StartGenerator() {

	srand(time(NULL));
	int i = rand() % 15;
	InitMap();

	floorGrid[50][50].roomContents = 1;
	floorGrid[50][50].visited = true;
	DrawRoom(i, 50, 50);
	SetRoomValues(i, 50, 50);
	roomCount++;

	CheckSurroundingRooms(50, 50);
	ChooseRoom();
}

void DrawRoom(int rT, int xRoom, int yRoom) //populate the map array with the next generated room
{
	for (int i = 0; i < ROOMSIZE; i++) { //double loop to place every tile for the current room
		for (int j = 0; j < ROOMSIZE; j++) {

			if ((i == 1 || i == ROOMSIZE - 2) && j > 0 && j < ROOMSIZE - 1) { //build the horizontal walls at the top and bottom of the room (1 space outer boundary)

				if (j == 1 && i == 1) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 7; } //top left corner
				if (j == 1 && i == ROOMSIZE - 2) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 1; } //bottom left corner
				if (j == ROOMSIZE - 2 && i == 1) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 9; } //top right corner
				if (j == ROOMSIZE - 2 && i == ROOMSIZE - 2) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 3; } //bottom right corner

				if (i == 1 && j > 1 && j < ROOMSIZE - 2) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 8; } //top wall
				if (i == ROOMSIZE - 2 && j > 1 && j < ROOMSIZE - 2) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 2; } //bottom wall
			}
			else if (i > 1 && i < ROOMSIZE - 2 && j != 0 && j != ROOMSIZE - 1) { // build the floor and vertical walls line by line

				if (j == 1) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 4; } //left wall
				if (j == ROOMSIZE - 2) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 6; } //right wall


				if (j > 1 && j < ROOMSIZE - 2) { // floor condition

					map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 10;
				}
			}
			else{ // any unspecified tile will now be space between rooms, fill it up

				map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 5;
			}
		}
	}

	if (rT == 0 || rT == 3 || rT == 6 || rT == 8 || rT == 10 || rT == 11 || rT == 12 || rT == 14) { //north potentialRoom room types
	
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE)] = 4; //left flat
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE)] = 6; //right flat
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + 1] = 15; //left corner
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + 1] = 14; //right corner
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE)] = 0;
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + 1] = 0;
	}
	if (rT == 0 || rT == 2 || rT == 5 || rT == 8 || rT == 9 || rT == 11 || rT == 13 || rT == 14) { //east potentialRoom room types
	
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 8; //top flat
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 2; //bottom flat
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 2][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 14; //top corner
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 2][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 12; //bottom corner
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
		map[(xRoom * ROOMSIZE) + ROOMSIZE - 2][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
	}
	if (rT == 0 || rT == 1 || rT == 5 || rT == 6 || rT == 7 || rT == 12 || rT == 13 || rT == 14) { //south potentialRoom room types
	
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + ROOMSIZE - 1] = 4; //left flat
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + ROOMSIZE - 1] = 6; //right flat
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + ROOMSIZE - 2] = 13; //left corner
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + ROOMSIZE - 2] = 12; //right corner
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + ROOMSIZE - 1] = 0;
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + ROOMSIZE - 2] = 0;
	}
	if (rT == 0 || rT == 4 || rT == 7 || rT == 9 || rT == 10 || rT == 11 || rT == 12 || rT == 13) { //west potentialRoom room types
	
		map[(xRoom * ROOMSIZE)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 8; //top flat
		map[(xRoom * ROOMSIZE)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 2; //bottom flat
		map[(xRoom * ROOMSIZE) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 15; //top corner
		map[(xRoom * ROOMSIZE) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 13; //bottom corner
		map[(xRoom * ROOMSIZE)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
		map[(xRoom * ROOMSIZE) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
	}

	StructureRoom(xRoom, yRoom);
}

void StructureRoom(int xRoom, int yRoom) 
{
	int diceRoll = 0;

	if (floorGrid[xRoom][yRoom].roomStructure > 0) {
	
		if (floorGrid[xRoom][yRoom].roomStructure == 1) { // lateral centre cross ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if (i >= 4 && i <= 10 && j == 7) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }//spawn walls
					if (j >= 4 && j <= 10 && i == 7) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
	
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top left spikes
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 4] = 19; 
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 5] = 19;
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 6] = 19;
				map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 6] = 19;
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 6] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // top right spikes
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 4] = 19;
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 5] = 19;
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 6] = 19;
				map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 6] = 19;
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 6] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // bottom left spikes
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 10] = 19;
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 9] = 19;
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 8] = 19;
				map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 8] = 19;
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 8] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // bottom right spikes
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 10] = 19;
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 9] = 19;
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 8] = 19;
				map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 8] = 19;
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 8] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top left pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 5) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 5) * TS;	
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom right pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 9) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 9) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 3) * TS, ((yRoom * ROOMSIZE) + 3) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 11) * TS, ((yRoom * ROOMSIZE) + 3) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 3) * TS, ((yRoom * ROOMSIZE) + 11) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 11) * TS, ((yRoom * ROOMSIZE) + 11) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 2) { // diagonal centre cross ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if (i >= 4 && i <= 10 && j == i) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if (j >= 4 && j <= 10 && i + j == 14) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top left spikes
				map[(xRoom * ROOMSIZE) + 3][(yRoom * ROOMSIZE) + 3] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top right spikes
				map[(xRoom * ROOMSIZE) + 11][(yRoom * ROOMSIZE) + 3] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom left spikes
				map[(xRoom * ROOMSIZE) + 3][(yRoom * ROOMSIZE) + 11] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom right spikes
				map[(xRoom * ROOMSIZE) + 11][(yRoom * ROOMSIZE) + 11] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 6) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 8) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 4) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 10) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 4) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 10) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 3) { // inward facing corners ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if ((i == 5 || i == 9) && ((j >= 3 && j <= 5) || (j >= 9 && j <= 11))) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if ((j == 5 || j == 9) && ((i >= 3 && i <= 5) || (i >= 9 && i <= 11))) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top left spikes
				map[(xRoom * ROOMSIZE) + 2][(yRoom * ROOMSIZE) + 2] = 19;
				map[(xRoom * ROOMSIZE) + 2][(yRoom * ROOMSIZE) + 3] = 19;
				map[(xRoom * ROOMSIZE) + 3][(yRoom * ROOMSIZE) + 2] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // top right spikes
				map[(xRoom * ROOMSIZE) + 12][(yRoom * ROOMSIZE) + 2] = 19;
				map[(xRoom * ROOMSIZE) + 12][(yRoom * ROOMSIZE) + 3] = 19;
				map[(xRoom * ROOMSIZE) + 11][(yRoom * ROOMSIZE) + 2] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // bottom left spikes
				map[(xRoom * ROOMSIZE) + 2][(yRoom * ROOMSIZE) + 12] = 19;
				map[(xRoom * ROOMSIZE) + 2][(yRoom * ROOMSIZE) + 11] = 19;
				map[(xRoom * ROOMSIZE) + 3][(yRoom * ROOMSIZE) + 12] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { // bottom right
				map[(xRoom * ROOMSIZE) + 12][(yRoom * ROOMSIZE) + 12] = 19;
				map[(xRoom * ROOMSIZE) + 12][(yRoom * ROOMSIZE) + 11] = 19;
				map[(xRoom * ROOMSIZE) + 11][(yRoom * ROOMSIZE) + 12] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 10) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 4) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 4) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 10) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 4) { // outward facing corners ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if ((i == 3 || i == 11) && ((j >= 3 && j <= 5) || (j >= 9 && j <= 11))) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if ((j == 3 || j == 11) && ((i >= 3 && i <= 5) || (i >= 9 && i <= 11))) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if ((j == 7 && i >= 6 && i <= 8)) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if ((i == 7 && j >= 6 && j <= 8)) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top left spikes
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top right spikes
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom left spikes
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 10] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom right spikes
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 10] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 6) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 6) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 8) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 8) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 5) { // horizontal split ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if (i == 5 && j >= 2 && j <= 10) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if (i == 9 && j >= 4 && j <= 12) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top spikes
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 2] = 19;
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //middle spikes
				map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 6] = 19;
				map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 8] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom spikes
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 10] = 19;
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 12] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 4) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 10) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 3) * TS, ((yRoom * ROOMSIZE) + 3) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // mid right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // mid left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 11) * TS, ((yRoom * ROOMSIZE) + 11) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 6) { // vertical split ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if (j == 5 && i >= 2 && i <= 10) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if (j == 9 && i >= 4 && i <= 12) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //left spikes
				map[(xRoom * ROOMSIZE) + 2][(yRoom * ROOMSIZE) + 10] = 19;
				map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 10] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //middle spikes
				map[(xRoom * ROOMSIZE) + 6][(yRoom * ROOMSIZE) + 7] = 19;
				map[(xRoom * ROOMSIZE) + 8][(yRoom * ROOMSIZE) + 7] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //right spikes
				map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 4] = 19;
				map[(xRoom * ROOMSIZE) + 12][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 4) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 7) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 10) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 7) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 3) * TS, ((yRoom * ROOMSIZE) + 3) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // mid top enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // mid bottom enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 11) * TS, ((yRoom * ROOMSIZE) + 11) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 7) { // target pattern ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if ((i == 4 || i == 10) && j >= 4 && j <= 10) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if ((j == 4 || j == 10) && i >= 4 && i <= 10) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}
			map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 7] = 18;
			map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 4] = 0; map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 4] = 0;
			map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 7] = 0; map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 7] = 0;
			map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 10] = 0; map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 10] = 0;

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top left spikes
				map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top right spikes
				map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 4] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom left spikes
				map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 10] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom right spikes
				map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 10] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 6) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 8) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll >= 0) { // top enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 7) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 7) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
		if (floorGrid[xRoom][yRoom].roomStructure == 8) { // diagonal from corners ###################################################

			for (int i = 2; i < ROOMSIZE - 2; i++) { //double loop covering only the floor space of a room
				for (int j = 2; j < ROOMSIZE - 2; j++) {

					if (((i >= 2 && i <= 5) || (i >= 9 && i <= 12)) && j == i) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
					if (((j >= 2 && j <= 5) || (j >= 9 && j <= 12)) && i + j == 14) { map[(xRoom * ROOMSIZE) + j][(yRoom * ROOMSIZE) + i] = 18; }
				}
			}
			map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 7] = 19;

			diceRoll = rand() % 2;
			if (diceRoll == 0) { //top spikes
				map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 5] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //right spikes
				map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 7] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //left spikes
				map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 7] = 19;
			}
			diceRoll = rand() % 2;
			if (diceRoll == 0) { //bottom spikes
				map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 9] = 19;
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // top right pickup point
				floorGrid[xRoom][yRoom].pickupList[0].active = 1;
				floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 6) * TS;
				floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 7) * TS;
				ChooseItemType(xRoom, yRoom, 0);
			}
			diceRoll = rand() % 3;
			if (diceRoll == 0) { // bottom left pickup point
				floorGrid[xRoom][yRoom].pickupList[1].active = 1;
				floorGrid[xRoom][yRoom].pickupList[1].xpos = ((xRoom * ROOMSIZE) + 8) * TS;
				floorGrid[xRoom][yRoom].pickupList[1].ypos = ((yRoom * ROOMSIZE) + 7) * TS;
				ChooseItemType(xRoom, yRoom, 1);
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // top enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 5) * TS, ((yRoom * ROOMSIZE) + 3) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // right enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 11) * TS, ((yRoom * ROOMSIZE) + 5) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // left enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 3) * TS, ((yRoom * ROOMSIZE) + 9) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
			diceRoll = rand() % 4;
			if (diceRoll == 0) { // bottom enemy spawn setter
				floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 9) * TS, ((yRoom * ROOMSIZE) + 11) * TS);
				floorGrid[xRoom][yRoom].enemyCount++;
			}
		}
	}

	// designate a room for the starting room, key room and a room for the exit room.
	if (floorGrid[xRoom][yRoom].roomContents == 1) {
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 27;
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 18; //top left
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 18; //bottom left
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 18; //top right
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 18; //bottom right
	}
	if (floorGrid[xRoom][yRoom].roomContents == 2) {
		
		diceRoll = rand() % 3;
		if (diceRoll > 0) {

			map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0; //key spawn tile

			map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 5] = 28; //decorative tiles
			map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 7] = 28;
			map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 7] = 28;
			map[(xRoom * ROOMSIZE) + 7][(yRoom * ROOMSIZE) + 9] = 28;

			map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 5] = 18; //walls
			map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 5] = 18;
			map[(xRoom * ROOMSIZE) + 5][(yRoom * ROOMSIZE) + 9] = 18;
			map[(xRoom * ROOMSIZE) + 9][(yRoom * ROOMSIZE) + 9] = 18;

			floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 4) * TS, ((yRoom * ROOMSIZE) + 10) * TS); //always spawn two enemies
			floorGrid[xRoom][yRoom].enemyCount++;

			floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 10) * TS, ((yRoom * ROOMSIZE) + 4) * TS);
			floorGrid[xRoom][yRoom].enemyCount++;

			for (int i = 0; i < 100; i++) {

				if (key[i].active == 0) {
					key[i].active = 1;
					key[i].xpos = (xRoom * ROOMSIZE * TS) + (int)((ROOMSIZE / 2) * TS);
					key[i].ypos = (yRoom * ROOMSIZE * TS) + (int)((ROOMSIZE / 2) * TS);
					break;
				}
			}
		}
		if (diceRoll == 0) {

			map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 0; //key spawn tile and item spawn tile
			map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 0;

			for (int i = 4; i < 11; i++) { //decorative tiles
				for (int j = 4; j < 11; j++) {
				
					if (i == 4 || i == 10 || j == 4 || j == 10) {
						
					map[(xRoom * ROOMSIZE) + i][(yRoom * ROOMSIZE) + j] = 27; 
					} 
				}
			}

			for (int i = 3; i < 12; i++) { //spike traps
				for (int j = 3; j < 12; j++) {

					if ((i == 3 || i == 5 || i == 9 || i == 11) && ((j > 2 && j < 6) || (j > 8 && j < 12))) {

						map[(xRoom * ROOMSIZE) + i][(yRoom * ROOMSIZE) + j] = 19;
					} 
					if ((i == 4 || i == 10) && (j == 3 || j  == 5 || j == 9 || j == 11)) {
					
						map[(xRoom * ROOMSIZE) + i][(yRoom * ROOMSIZE) + j] = 19;
					}
				}
			}
			
			map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 4] = 18; //walls
			map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 4] = 18;
			map[(xRoom * ROOMSIZE) + 4][(yRoom * ROOMSIZE) + 10] = 18;
			map[(xRoom * ROOMSIZE) + 10][(yRoom * ROOMSIZE) + 10] = 18;

			floorGrid[xRoom][yRoom].enemy[0].Initalise(((xRoom * ROOMSIZE) + 6) * TS, ((yRoom * ROOMSIZE) + 6) * TS); //always spawn four enemies
			floorGrid[xRoom][yRoom].enemyCount++;

			floorGrid[xRoom][yRoom].enemy[1].Initalise(((xRoom * ROOMSIZE) + 8) * TS, ((yRoom * ROOMSIZE) + 6) * TS);
			floorGrid[xRoom][yRoom].enemyCount++;

			floorGrid[xRoom][yRoom].enemy[2].Initalise(((xRoom * ROOMSIZE) + 6) * TS, ((yRoom * ROOMSIZE) + 8) * TS);
			floorGrid[xRoom][yRoom].enemyCount++;

			floorGrid[xRoom][yRoom].enemy[3].Initalise(((xRoom * ROOMSIZE) + 8) * TS, ((yRoom * ROOMSIZE) + 8) * TS);
			floorGrid[xRoom][yRoom].enemyCount++;

			for (int i = 0; i < 100; i++) { //place key

				if (key[i].active == 0) {
					key[i].active = 1;
					key[i].xpos = (xRoom * ROOMSIZE * TS) + (int)((ROOMSIZE / 2) * TS);
					key[i].ypos = ((yRoom * ROOMSIZE + 1) * TS) + (int)((ROOMSIZE / 2) * TS);
					break;
				}
			}
	
			floorGrid[xRoom][yRoom].pickupList[0].active = 1;  // place one of the special items
			floorGrid[xRoom][yRoom].pickupList[0].xpos = ((xRoom * ROOMSIZE) + 7) * TS;
			floorGrid[xRoom][yRoom].pickupList[0].ypos = ((yRoom * ROOMSIZE) + 6) * TS;
			ChooseItemType(xRoom, yRoom, 0);
		}
	}
	if (floorGrid[xRoom][yRoom].roomContents == 3) {
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 20;
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 18; //top left
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 18; //top middle
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1] = 18; //top right
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 18; //middle left
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 18; //middle right
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) - 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 18; //bottom left
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2)][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 18; //bottom middle
		map[(xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1][(yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1] = 18; //bottom right
		exitXpos = (xRoom * ROOMSIZE) + (int)(ROOMSIZE / 2);
		exitYpos = (yRoom * ROOMSIZE) + (int)(ROOMSIZE / 2) + 1;
	}
}

void ChooseItemType(int roomX, int roomY, int i) 
{
	if (floorGrid[roomX][roomY].roomContents == 0) {
		
		int diceRoll = rand() % 3;
		floorGrid[roomX][roomY].pickupList[i].type = diceRoll;
	}
	if (floorGrid[roomX][roomY].roomContents == 2) {

		int diceRoll = (rand() % 2) + 3;
		floorGrid[roomX][roomY].pickupList[i].type = diceRoll;
	}
}

void SpawnRoom(int roomType, int roomX, int roomY) {

	if (floorGrid[roomX][roomY].occupied == false) {

		if (floorGrid[roomX][roomY].roomContents == 0) { floorGrid[roomX][roomY].roomStructure = rand() % 8 + 1; }

		DrawRoom(roomType, roomX, roomY);
		roomCount++;
		SetRoomValues(roomType, roomX, roomY);
	}
	else {
		std::cout << "ERROR - SPAWNING ON FULL GRID SPACE";
	}

	if (floorGrid[roomX][roomY].northPath == 1 && floorGrid[roomX][roomY + 1].southPath == 1) {
		floorGrid[roomX][roomY].northPath = 2;
		floorGrid[roomX][roomY + 1].southPath = 2;
	}

	if (floorGrid[roomX][roomY].eastPath == 1 && floorGrid[roomX + 1][roomY].westPath == 1) {
		floorGrid[roomX][roomY].eastPath = 2;
		floorGrid[roomX + 1][roomY].westPath = 2;
	}

	if (floorGrid[roomX][roomY].southPath == 1 && floorGrid[roomX][roomY - 1].northPath == 1) {
		floorGrid[roomX][roomY].southPath = 2;
		floorGrid[roomX][roomY - 1].northPath = 2;
	}

	if (floorGrid[roomX][roomY].westPath == 1 && floorGrid[roomX - 1][roomY].eastPath == 1) {
		floorGrid[roomX][roomY].westPath = 2;
		floorGrid[roomX - 1][roomY].eastPath = 2;
	}

	//Reset the examined bool for each room to false
	int reset = 0;
	int ycount = 0;
	do {
		floorGrid[reset][ycount].examined = false;
		ycount++;
		if (ycount == 99) {
			ycount = 0;
			reset++;
		}
	} while (reset < 100);

	//Check the current spawned room count against the Max and start spawning another if Max is not reached
	CheckSurroundingRooms(50, 50);
	
	if (roomCount + potentialRooms < roomMax) {

		ChooseRoom();
	}

	if (roomCount + potentialRooms >= roomMax) {

		CloseRooms();
	}
}

//Close off any open rooms
void CloseRooms() {

	//Whilst there is still open rooms, spawn a new room to close off a path
	if (potentialRooms != 0) {
		int i = rand() % potentialRooms + 1;
		int newRoomX = potentialRoom[i].roomX;
		int newRoomY = potentialRoom[i].roomY;

		//Reset all the potentialRooms and potentialRoom variables;
		int reset = 0;
		do {
			potentialRoom[reset].roomX = 0;
			potentialRoom[reset].roomX = 0;
			reset++;
		} while (reset < 100);
		potentialRooms = 0;

		// Check for potential connections to the new rooms location
		bool northConnection = false;
		bool eastConnection = false;
		bool southConnection = false;
		bool westConnection = false;

		if (floorGrid[newRoomX][newRoomY + 1].southPath == 1) { northConnection = true; }
		if (floorGrid[newRoomX + 1][newRoomY].westPath == 1) { eastConnection = true; }
		if (floorGrid[newRoomX][newRoomY - 1].northPath == 1) { southConnection = true; }
		if (floorGrid[newRoomX - 1][newRoomY].eastPath == 1) { westConnection = true; }

		// Whilst closing rooms designate a room for the key rooms and a room for the exit room.
		if (level.spawnedKeyRooms < level.keyRooms) {

			if (roomCount == roomMax - (level.keyRooms - level.spawnedKeyRooms) - 1) { 
				floorGrid[newRoomX][newRoomY].roomContents = 2; 
				level.spawnedKeyRooms++; 
			}
		}

		if (roomCount == roomMax - 1) { floorGrid[newRoomX][newRoomY].roomContents = 3; }

		//Single Path Cases
		if (northConnection == true && eastConnection == false && southConnection == false && westConnection == false) { SpawnRoom(1, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == true && southConnection == false && westConnection == false) { SpawnRoom(2, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == false && southConnection == true && westConnection == false) { SpawnRoom(3, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == false && southConnection == false && westConnection == true) { SpawnRoom(4, newRoomX, newRoomY); }

		//Double Path Cases
		if (northConnection == true && eastConnection == true && southConnection == false && westConnection == false) { SpawnRoom(5, newRoomX, newRoomY); }
		if (northConnection == true && eastConnection == false && southConnection == true && westConnection == false) { SpawnRoom(6, newRoomX, newRoomY); }
		if (northConnection == true && eastConnection == false && southConnection == false && westConnection == true) { SpawnRoom(7, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == true && southConnection == true && westConnection == false) { SpawnRoom(8, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == true && southConnection == false && westConnection == true) { SpawnRoom(9, newRoomX, newRoomY); }
		if (northConnection == false && eastConnection == false && southConnection == true && westConnection == true) { SpawnRoom(10, newRoomX, newRoomY); }

		//Triple Path Cases
		if (northConnection == false && eastConnection == true && southConnection == true && westConnection == true) { SpawnRoom(11, newRoomX, newRoomY); }
		if (northConnection == true && eastConnection == false && southConnection == true && westConnection == true) { SpawnRoom(12, newRoomX, newRoomY); }
		if (northConnection == true && eastConnection == true && southConnection == false && westConnection == true) { SpawnRoom(13, newRoomX, newRoomY); }
		if (northConnection == true && eastConnection == true && southConnection == true && westConnection == false) { SpawnRoom(14, newRoomX, newRoomY); }

		//Quad Path Case
		if (northConnection == true && eastConnection == true && southConnection == true && westConnection == true) { SpawnRoom(0, newRoomX, newRoomY); }
	}
}

//Select what rooms are viable to spawn
void ChooseRoom() {

	//Set the SpawnRoom variables from the data recovered from CheckSurroundingRooms
    int i = rand() % potentialRooms + 1;
	int newRoomX = potentialRoom[i].roomX;
	int newRoomY = potentialRoom[i].roomY;

	//Reset all the potentialRooms and potentialRoom variables;
	int reset = 0;
	do {
		potentialRoom[reset].roomX = 0;
		potentialRoom[reset].roomX = 0;
		reset++;
	} while (reset < 1000);
	potentialRooms = 0;

	//Check to see what surrounding spaces are already connected or blocked
	
	bool northConnection = false;
	bool eastConnection = false;
	bool southConnection = false;
	bool westConnection = false;

	bool northBlocked = false;
	bool eastBlocked = false;
	bool southBlocked = false;
	bool westBlocked = false;

	// Whilst choosing rooms designate a room for the key rooms.
	if (level.spawnedKeyRooms < level.keyRooms) {

		if (roomCount == roomMax - (level.keyRooms - level.spawnedKeyRooms) - 1) {
			floorGrid[newRoomX][newRoomY].roomContents = 2;
			level.spawnedKeyRooms++;
		}
	}

	//Check each occupied space for any required connections
	if (floorGrid[newRoomX][newRoomY + 1].southPath == 1) { northConnection = true; }
	if (floorGrid[newRoomX + 1][newRoomY].westPath == 1) { eastConnection = true; }		
	if (floorGrid[newRoomX][newRoomY - 1].northPath == 1) { southConnection = true; }	
	if (floorGrid[newRoomX - 1][newRoomY].eastPath == 1) { westConnection = true; }
	
	//Mark any directions which have an occupied room but no connecting path
	if (floorGrid[newRoomX][newRoomY + 1].occupied == 1 && northConnection == false) { northBlocked = true; }
	if (floorGrid[newRoomX + 1][newRoomY].occupied == 1 && eastConnection == false) { eastBlocked = true; }
	if (floorGrid[newRoomX][newRoomY - 1].occupied == 1 && southConnection == false) { southBlocked = true; }
	if (floorGrid[newRoomX -1][newRoomY].occupied == 1 && westConnection == false) { westBlocked = true; }

	//Determine the viable room connections based on the blocked path data and call SpawnRoom
	//One Connection Blocked
	//Case 1
	if (northBlocked == true && eastBlocked == false && southBlocked == false && westBlocked == false) {

		// Case 1 - East Connection
		if (eastConnection == true && southConnection == false && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(11, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
			}
		}
		//Case 1 - South Connection
		if (eastConnection == false && southConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(11, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
			}
		}
		//Case 1 - West Connection
		if (eastConnection == false && southConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(11, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
			}
		}
		//Case 1 - East and South Connection
		if (eastConnection == true && southConnection == true && westConnection == false) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(8, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
		}
		//Case 1 - East and West Connection
		if (eastConnection == true && southConnection == false && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(9, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
		}
		//Case 1 - South and West Connection
		if (eastConnection == false && southConnection == true && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(10, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
		}
		//Case 1 - East, South and West Connection
		if (eastConnection == true && southConnection == true && westConnection == true)
		{
			SpawnRoom(11, newRoomX, newRoomY);
		}
	}


	//Case 2
	if (northBlocked == false && eastBlocked == true && southBlocked == false && westBlocked == false) {

		// Case 2 - North Connection
		if (northConnection == true && southConnection == false && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(12, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(12, newRoomX, newRoomY); }
			}
		}
		//Case 2 - South Connection
		if (northConnection == false && southConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(12, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(12, newRoomX, newRoomY); }
			}
		}
		//Case 2 - West Connection
		if (northConnection == false && southConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(12, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(12, newRoomX, newRoomY); }
			}
		}
		//Case 2 - North and South Connection
		if (northConnection == true && southConnection == true && westConnection == false) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(12, newRoomX, newRoomY); }
		}
		//Case 2 - North and West Connection
		if (northConnection == true && southConnection == false && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(12, newRoomX, newRoomY); }
		}
		//Case 2 - South and West Connection
		if (northConnection == false && southConnection == true && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(10, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(12, newRoomX, newRoomY); }
		}
		//Case 2 - North, South and West Connection
		if (northConnection == true && southConnection == true && westConnection == true)
		{
			SpawnRoom(12, newRoomX, newRoomY);
		}
	}


	//Case 3
	if (northBlocked == false && eastBlocked == false && southBlocked == true && westBlocked == false) {
		// Case 3 - North Connection
		if (northConnection == true && eastConnection == false && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(13, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(13, newRoomX, newRoomY); }
			}
		}
		//Case 3 - East Connection
		if (northConnection == false && eastConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(13, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(13, newRoomX, newRoomY); }
			}
		}
		//Case 3 - West Connection
		if (northConnection == false && eastConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(13, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(13, newRoomX, newRoomY); }
			}
		}
		//Case 3 - North and East Connection
		if (northConnection == true && eastConnection == true && westConnection == false) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(13, newRoomX, newRoomY); }
		}
		//Case 3 - North and West Connection
		if (northConnection == true && eastConnection == false && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(13, newRoomX, newRoomY); }
		}
		//Case 3 - East and West Connection
		if (northConnection == false && eastConnection == true && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(9, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(13, newRoomX, newRoomY); }
		}
		//Case 3 - North, East and West Connection
		if (northConnection == true && eastConnection == true && westConnection == true)
		{
			SpawnRoom(13, newRoomX, newRoomY);
		}
	}


	//Case 4
	if (northBlocked == false && eastBlocked == false && southBlocked == false && westBlocked == true) {
		// Case 4 - North Connection
		if (northConnection == true && eastConnection == false && southConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(14, newRoomX, newRoomY); }
			}
		}
		//Case 4 - East Connection
		if (northConnection == false && eastConnection == true && southConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(14, newRoomX, newRoomY); }
			}
		}
		//Case 4 - South Connection
		if (northConnection == false && eastConnection == false && southConnection == true) {

			if (roomCount < roomMax / 2) {
				i = rand() % 3 + 1;
				if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 4 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(14, newRoomX, newRoomY); }
			}
		}
		//Case 4 - North and East Connection
		if (northConnection == true && eastConnection == true && southConnection == false) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(14, newRoomX, newRoomY); }
		}
		//Case 4 - North and South Connection
		if (northConnection == true && eastConnection == false && southConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(14, newRoomX, newRoomY); }
		}
		//Case 4 - East and South Connection
		if (northConnection == false && eastConnection == true && southConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(8, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(14, newRoomX, newRoomY); }
		}
		//Case 4 - North, East and South Connection
		if (northConnection == true && eastConnection == true && southConnection == true)
		{
			SpawnRoom(14, newRoomX, newRoomY);
		}
	}


	//Two Connections Blocked
	//Case 5
	if (northBlocked == true && eastBlocked == true && southBlocked == false && westBlocked == false) {

		//Case 5 - South Connection
		if (southConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(10, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
			}
		}
		//Case 5 - West Connection
		if (southConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(10, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(10, newRoomX, newRoomY); }
			}
		}
		//Case 5 - South and West Connection
		if (southConnection == true && westConnection == true) { SpawnRoom(10, newRoomX, newRoomY); }
	}


	//Case 6
	if (northBlocked == true && eastBlocked == false && southBlocked == true && westBlocked == false) {

		//Case 6 - East Connection
		if (eastConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(9, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
			}
		}
		//Case 6 - West Connection
		if (eastConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(9, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
			}
		}
		//Case 6 - East and West Connection
		if (eastConnection == true && westConnection == true) { SpawnRoom(9, newRoomX, newRoomY); }
	}


	//Case 7
	if (northBlocked == true && eastBlocked == false && southBlocked == false && westBlocked == true) {

		//Case 7 - East Connection
		if (eastConnection == true && southConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(8, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
			}
		}
		//Case 7 - South Connection
		if (eastConnection == false && southConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(8, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
			}
		}
		//Case 7 - East and South Connection
		if (eastConnection == true && southConnection == true) { SpawnRoom(8, newRoomX, newRoomY); }
	}


	//Case 8
	if (northBlocked == false && eastBlocked == true && southBlocked == true && westBlocked == false) {

		//Case 8 - North Connection
		if (northConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(7, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
			}
		}
		//Case 8 - West Connection
		if (northConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(7, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
			}
		}
		//Case 8 - North and West Connection
		if (northConnection == true && westConnection == true) { SpawnRoom(7, newRoomX, newRoomY); }
	}


	//Case 9
	if (northBlocked == false && eastBlocked == true && southBlocked == false && westBlocked == true) {

		//Case 9 - North Connection
		if (northConnection == true && southConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(6, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
			}
		}
		//Case 9 - South Connection
		if (northConnection == false && southConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(6, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
			}
		}
		//Case 9 - North and South Connection
		if (northConnection == true && southConnection == true) { SpawnRoom(6, newRoomX, newRoomY); }
	}


	//Case 10
	if (northBlocked == false && eastBlocked == false && southBlocked == true && westBlocked == true) {

		//Case 10 - North Connection
		if (northConnection == true && eastConnection == false) {

			if (roomCount < roomMax / 2) { SpawnRoom(5, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
			}
		}
		//Case 10 - East Connection
		if (northConnection == false && eastConnection == true) {

			if (roomCount < roomMax / 2) { SpawnRoom(5, newRoomX, newRoomY); }

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 2 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
			}
		}
		//Case 10 - North and East Connection
		if (northConnection == true && eastConnection == true) { SpawnRoom(5, newRoomX, newRoomY); }
	}


	//Three Connections Blocked
	//Case 11
	if (northBlocked == false && eastBlocked == true && southBlocked == true && westBlocked == true) { if (northConnection == true) { SpawnRoom(1, newRoomX, newRoomY); } }

	//Case 12
	if (northBlocked == true && eastBlocked == false && southBlocked == true && westBlocked == true) { if (eastConnection == true) { SpawnRoom(2, newRoomX, newRoomY); } }

	//Case 13
	if (northBlocked == true && eastBlocked == true && southBlocked == false && westBlocked == true) { if (southConnection == true) { SpawnRoom(3, newRoomX, newRoomY); } }

	//Case 14
	if (northBlocked == true && eastBlocked == true && southBlocked == true && westBlocked == false) { if (westConnection == true) { SpawnRoom(4, newRoomX, newRoomY); } }

	//Nothing Blocked
	//Case 15
	if (northBlocked == false && eastBlocked == false && southBlocked == false && westBlocked == false) {

		//Case 15 - Case 1 (North)
		if (northConnection == true && eastConnection == false && southConnection == false && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 7 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(0, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 8 + 1;
				if (i == 1) { SpawnRoom(1, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 8) { SpawnRoom(0, newRoomX, newRoomY); }
			}
		}
		//Case 15 - Case 2 (East)
		if (northConnection == false && eastConnection == true && southConnection == false && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 7 + 1;
				if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(0, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 8 + 1;
				if (i == 1) { SpawnRoom(2, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(5, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 8) { SpawnRoom(0, newRoomX, newRoomY); }
			}
		}
		//Case 15 - Case 3 (South)
		if (northConnection == false && eastConnection == false && southConnection == true && westConnection == false) {

			if (roomCount < roomMax / 2) {
				i = rand() % 7 + 1;
				if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(0, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 8 + 1;
				if (i == 1) { SpawnRoom(3, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(6, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(8, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(14, newRoomX, newRoomY); }
				if (i == 8) { SpawnRoom(0, newRoomX, newRoomY); }
			}
		}
		//Case 15 - Case 4 (West)
		if (northConnection == false && eastConnection == false && southConnection == false && westConnection == true) {

			if (roomCount < roomMax / 2) {
				i = rand() % 7 + 1;
				if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(0, newRoomX, newRoomY); }
			}

			if (roomCount >= roomMax / 2 && floorGrid[newRoomX][newRoomY].occupied == false) {

				i = rand() % 8 + 1;
				if (i == 1) { SpawnRoom(4, newRoomX, newRoomY); }
				if (i == 2) { SpawnRoom(7, newRoomX, newRoomY); }
				if (i == 3) { SpawnRoom(9, newRoomX, newRoomY); }
				if (i == 4) { SpawnRoom(10, newRoomX, newRoomY); }
				if (i == 5) { SpawnRoom(11, newRoomX, newRoomY); }
				if (i == 6) { SpawnRoom(12, newRoomX, newRoomY); }
				if (i == 7) { SpawnRoom(13, newRoomX, newRoomY); }
				if (i == 8) { SpawnRoom(0, newRoomX, newRoomY); }
			}
		}
		//Case 15 - Case 5 (North, East)
		if (northConnection == true && eastConnection == true && southConnection == false && westConnection == false) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(5, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(13, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 6 (North, South)
		if (northConnection == true && eastConnection == false && southConnection == true && westConnection == false) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(6, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(12, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 7 (North, West)
		if (northConnection == true && eastConnection == false && southConnection == false && westConnection == true) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(7, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(12, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(13, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 8 (East, South)
		if (northConnection == false && eastConnection == true && southConnection == true && westConnection == false) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(8, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(14, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 9 (East, West)
		if (northConnection == false && eastConnection == true && southConnection == false && westConnection == true) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(9, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(13, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 10 (South, West)
		if (northConnection == false && eastConnection == false && southConnection == true && westConnection == true) {

			i = rand() % 4 + 1;
			if (i == 1) { SpawnRoom(10, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(11, newRoomX, newRoomY); }
			if (i == 3) { SpawnRoom(12, newRoomX, newRoomY); }
			if (i == 4) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 11 (East, South, West)
		if (northConnection == false && eastConnection == true && southConnection == true && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(11, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 12 (North, South, West)
		if (northConnection == true && eastConnection == false && southConnection == true && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(12, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 13 (North, East, West)
		if (northConnection == true && eastConnection == true && southConnection == false && westConnection == true) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(13, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 14 (North, East, South)
		if (northConnection == true && eastConnection == true && southConnection == true && westConnection == false) {

			i = rand() % 2 + 1;
			if (i == 1) { SpawnRoom(14, newRoomX, newRoomY); }
			if (i == 2) { SpawnRoom(0, newRoomX, newRoomY); }
		}
		//Case 15 - Case 15 (North, East, South, West)
		if (northConnection == true && eastConnection == true && southConnection == true && westConnection == true) { SpawnRoom(0, newRoomX, newRoomY); }
	}
}


void CheckSurroundingRooms(int roomX, int roomY) {

	//Check the current room is marked as occupied for error tracking
	if (floorGrid[roomX][roomY].occupied == true) {

		//Check if there is a vacant path to the north
		if (floorGrid[roomX][roomY].northPath == 1) {

			potentialRooms++; //increment known amount of possible room spawn locations
			potentialRoom[potentialRooms].roomX = roomX; //store the x and y co-ordinates of the possible room spawn location with the matching potentialRooms number
			potentialRoom[potentialRooms].roomY = roomY + 1;
		}
		if (floorGrid[roomX][roomY].eastPath == 1) {

			potentialRooms++;
			potentialRoom[potentialRooms].roomX = roomX + 1;
			potentialRoom[potentialRooms].roomY = roomY;
		}
		if (floorGrid[roomX][roomY].southPath == 1) {

			potentialRooms++;
			potentialRoom[potentialRooms].roomX = roomX;
			potentialRoom[potentialRooms].roomY = roomY - 1;
		}
		if (floorGrid[roomX][roomY].westPath == 1) {

			potentialRooms++;
			potentialRoom[potentialRooms].roomX = roomX - 1;
			potentialRoom[potentialRooms].roomY = roomY;
		}
		//Mark room as examined, so the room checker doesn't get caught in a loop checking the same rooms
		floorGrid[roomX][roomY].examined = true;

		//Recursively call the function to check all connected rooms
		if (floorGrid[roomX][roomY].northPath == 2) {
			if (floorGrid[roomX][roomY + 1].examined == false) { CheckSurroundingRooms(roomX, roomY + 1); }
		}
		//Case 2
		if (floorGrid[roomX][roomY].eastPath == 2) {
			if (floorGrid[roomX + 1][roomY].examined == false) { CheckSurroundingRooms(roomX + 1, roomY); }
		}
		//Case 3
		if (floorGrid[roomX][roomY].southPath == 2) {
			if (floorGrid[roomX][roomY - 1].examined == false) { CheckSurroundingRooms(roomX, roomY - 1); }
		}
		//Case 4
		if (floorGrid[roomX][roomY].westPath == 2) {
			if (floorGrid[roomX - 1][roomY].examined == false) { CheckSurroundingRooms(roomX - 1, roomY); }
		}
	}
}

void SetRoomValues(int roomType, int roomX, int roomY) {

	floorGrid[roomX][roomY].occupied = true;
	floorGrid[roomX][roomY].roomType = roomType;

	if (roomType == 1) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 2) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 3) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 4) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 5) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 6) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 7) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 8) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 9) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 10) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 11) {
		floorGrid[roomX][roomY].northPath = 0;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 12) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 0;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 13) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 0;
		floorGrid[roomX][roomY].westPath = 1;
	}
	if (roomType == 14) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 0;
	}
	if (roomType == 0) {
		floorGrid[roomX][roomY].northPath = 1;
		floorGrid[roomX][roomY].eastPath = 1;
		floorGrid[roomX][roomY].southPath = 1;
		floorGrid[roomX][roomY].westPath = 1;
	}
}

void InitMap(void) 
{
	for (int i = 0; i < MW; i++) {
		for (int j = 0; j < MH; j++) {
		
			map[i][j] = 5;
		}
	}
}


