//Procedural Endless Dungeon Crawler
//Chris Haynes - 10748982
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <conio.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"
#include "ddutil.h"
#include "levelgenerator.h"
#include "enemyclass.h"
#define NAME          "Conan v1.1"
#define TITLE         "Conan v1.1"
#define SW            1920	//Screen Width in pixels
#define SH            1080	//Screen Height in pixels
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code)&0x8000) ? 1 : 0) //keyboard input definitions
#define KEY_UP  (vk_code) ((GetAsyncKeyState(vk_code)&0x8000) ? 0 : 1)

void DisplayFrameRate(void);			//Prints debug values to the screen
void CheckGameStates(void);				//Performs in game checks on multiple game objects
void EnemyMovement(void);				//Move and collide the enemies
void ArrowMovementAndCollision(void);	//Move and collide the arrows
void PlayerInputAndCollision(void);		//Receive player input and collide the player object
void DrawToScreen(void);				//Draw the next frame to the back surface
void EnemyPlayerCollisions(void);		//Collide enemies with the player
void EnemyAnimationSelector(int);		//Select the correct animation values for the enemy
void AnimationSelector(void);			//Select the correction animation values for the player
void AdvanceLevel(void);				//Reset all the level variables and initialise the next level
HRESULT RestoreAll(void);
static HRESULT InitApp(HINSTANCE hInstance, int nCmdShow);

LPDIRECTDRAW4         DD = NULL;		//DD object
LPDIRECTDRAWSURFACE4  DDP = NULL;		//DD primary surface
LPDIRECTDRAWSURFACE4  DDB = NULL;		//DD back surface
LPDIRECTDRAWSURFACE4  DD1 = NULL;		//Offscreen surface 1
LPDIRECTDRAWPALETTE   DDC = NULL;		// The primary surface palette
BOOL                  active = FALSE;	// Is application active?
static char           bmp[] = "ALL";

long   framecounter = 0;	//Variables to handle in game frames
float  dwFrameTime;
long   dwFrameCount;
float  dwFrames, dwFramesLast;
long   ssx = (TS*ROOMSIZE * 50) - (TS * 5), ssy = (TS*ROOMSIZE * 50) - TS; //Screen scroll position
long   sxmod = 0, symod = 0, sxnum = 0, synum = 0;	//Tile offset and current screen position tile reference
int    sspeed = 8;		//Speed of map scroll
int	   i, j, z;			//loop variables

int    destinationX = ssx, destinationY = ssy; //Screen destination position
int    loading = 0, levelChanging = 0, gameOver = 0, nextLevelText = 0; //Game state identifiers
float  size = 1.0;			//Size varient used for "Blt"ing text
int	   sizeDirection = 1;	//Determines whether "size" is positive or negative

RECT	hFull, hHalf, hEmpty;	//Bitmap positions for the full, half and empty heart container
RECT	arrowr, r, rTextSize;	//Bitmap positions used to hold the arrows, general cases and text co-ordinates
HRESULT	hRet;					//DirectX structure used for error tracking
DDBLTFX	fx, ddbltfx;			//Used to apply different effects when "Blt"ing
int		maxf = 1, f = 0;		//Maximum and current frame count for animations
int		t1, t2, delay = 50;			//Timing loop variables for animation
int		damageTime1, damageTime2;	//Timing loop varaibles for damage cool down

Level level;

struct Player {
	static const int width = 24, height = 52;
	static const int walkSpeed = 4;
	bool damaged;
	int animationState; // 0 - 3 standing (NWSE), 4 - 7 walking (NWSE), 8 - 11 shooting (NWSE), 12 dying/dead
	int xpos, ypos;
	int roomX, roomY;
	int tileX, tileY;
	int arrowCount, keyCount;
	int arrowMax, healthMax;
	int health;
} p;

struct Arrow {
	static const int speed = 12, delay = 200;
	static const int width = 8, height = 33;
	int active;
	int state; //1 = in flight, 2 = at rest
	int t1, t2, f;
	int tileX, tileY;
	int xpos, ypos;
	int direction; // 0 - 4 (NWSE)
} arrow[100];

void UpdateFrame(void) //Main game loop
{
	if (gameOver == 1 && KEY_DOWN(VK_SPACE)) { //Restart game if game over and space bar is pressed

		p.arrowCount = 5;
		p.health = 6;
		p.arrowMax = 10;
		p.healthMax = 6;

		level.levelNum = 0;
		roomMax = 5;

		gameOver = 0;
		size = 1, sizeDirection = 1;
		loading = 0;

		AdvanceLevel();
	}

	if (ssx + SW > MW*TS) ssx = (MW*TS) - SW;
	if (ssx < 0)        ssx = 0;
	sxmod = ssx%TS;//xoffset
	sxnum = (ssx - sxmod) / TS;

	if (ssy + SH > MH*TS) ssy = (MH*TS) - SH;
	if (ssy < 0)        ssy = 0;
	symod = ssy%TS;//xoffset
	synum = (ssy - symod) / TS;

	ArrowMovementAndCollision();	//Move and collide the arrow
			
	EnemyMovement();				//Move and collide the enemy

	if (p.damaged == false) EnemyPlayerCollisions();	//If the player is not in a damage cooldown, check player and enemy collisions

	PlayerInputAndCollision();		//Take user input for player control and collide the player

	CheckGameStates();				//Check and update various game states

	DrawToScreen();					//Draw the next frame to the back surface

	//DisplayFrameRate();			//Draw the debug values to the screen
}

void CheckGameStates(void)
{
	//check if enough keys have been collected to open the exit 

	if (p.keyCount == level.keyRooms &&  map[exitXpos][exitYpos] != 10) { map[exitXpos][exitYpos] = 10; }


	//check if the current room still has enemies 

	if (floorGrid[p.roomX][p.roomY].enemyCount > 0) {

		if (floorGrid[p.roomX][p.roomY].southPath == 2) map[(p.roomX * ROOMSIZE) + (int)(ROOMSIZE / 2)][(p.roomY * ROOMSIZE) + 1] = 8;
		if (floorGrid[p.roomX][p.roomY].eastPath == 2) map[(p.roomX * ROOMSIZE) + ROOMSIZE - 2][(p.roomY * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 6;
		if (floorGrid[p.roomX][p.roomY].northPath == 2) map[(p.roomX * ROOMSIZE) + (int)(ROOMSIZE / 2)][(p.roomY * ROOMSIZE) + ROOMSIZE - 2] = 2;
		if (floorGrid[p.roomX][p.roomY].westPath == 2) map[(p.roomX * ROOMSIZE) + 1][(p.roomY * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 4;
	}

	if (floorGrid[p.roomX][p.roomY].enemyCount == 0) {

		if (floorGrid[p.roomX][p.roomY].southPath == 2) map[(p.roomX * ROOMSIZE) + (int)(ROOMSIZE / 2)][(p.roomY * ROOMSIZE) + 1] = 0;
		if (floorGrid[p.roomX][p.roomY].eastPath == 2) map[(p.roomX * ROOMSIZE) + ROOMSIZE - 2][(p.roomY * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
		if (floorGrid[p.roomX][p.roomY].northPath == 2) map[(p.roomX * ROOMSIZE) + (int)(ROOMSIZE / 2)][(p.roomY * ROOMSIZE) + ROOMSIZE - 2] = 0;
		if (floorGrid[p.roomX][p.roomY].westPath == 2) map[(p.roomX * ROOMSIZE) + 1][(p.roomY * ROOMSIZE) + (int)(ROOMSIZE / 2)] = 0;
	}

	//check for p damage cooldown 

	if (p.damaged == true) {
		damageTime2 = GetTickCount();
		if ((damageTime2 - damageTime1) > 1000)//wait 1 second
		{
			damageTime1 = damageTime2;
			p.damaged = false;
		}
	}

	//check if the p is dead 

	if (p.health <= 0) {

		p.health = 0;
		if (gameOver == 0) { gameOver = 1; f = 0; }
		p.animationState = 12;
		loading = 1;

		if (f == 6) f = 5;
	}

	//check for movement between rooms 
	//p moves north
	if (p.tileX == sxnum + 12 && p.tileY == synum + 1 && loading == 0 && levelChanging == 0) {

		int scroll = TS * ROOMSIZE;
		destinationY = ssy - scroll;
		loading = 1;
		p.ypos -= TS * 4;
		p.animationState = 0;
		p.tileY = (p.ypos + 30) / TS;
		p.roomY -= 1;
	}
	if (ssy > destinationY) {
		ssy -= sspeed;
		if (ssy <= destinationY) {

			ssy = destinationY;
			loading = 0;
			floorGrid[p.roomX][p.roomY].visited = true;
		}
	}
	//p moves east
	if (p.tileX == sxnum + 19 && p.tileY == synum + 8 && loading == 0 && levelChanging == 0) {

		int scroll = TS * ROOMSIZE;
		destinationX = ssx + scroll;
		loading = 1;
		p.xpos += TS * 4;
		p.animationState = 3;
		p.tileX = (p.xpos + 20) / TS;
		p.roomX += 1;
	}
	if (ssx < destinationX) {
		ssx += sspeed;
		if (ssx >= destinationX) {

			ssx = destinationX;
			loading = 0;
			floorGrid[p.roomX][p.roomY].visited = true;
		}
	}
	//p moves south
	if (p.tileX == sxnum + 12 && p.tileY == synum + 15 && loading == 0 && levelChanging == 0) {

		int scroll = TS * ROOMSIZE;
		destinationY = ssy + scroll;
		loading = 1;
		p.ypos += TS * 4;
		p.animationState = 2;
		p.tileY = (p.ypos + 30) / TS;
		p.roomY += 1;
	}
	if (ssy < destinationY) {
		ssy += sspeed;
		if (ssy >= destinationY) {

			ssy = destinationY;
			loading = 0;
			floorGrid[p.roomX][p.roomY].visited = true;
		}
	}
	//p moves west
	if (p.tileX == sxnum + 5 && p.tileY == synum + 8 && loading == 0 && levelChanging == 0) {

		int scroll = TS * ROOMSIZE;
		destinationX = ssx - scroll;
		loading = 1;
		p.xpos -= TS * 4;
		p.animationState = 1;
		p.tileX = (p.xpos + 20) / TS;
		p.roomX -= 1;
	}
	if (ssx > destinationX) {
		ssx -= sspeed;
		if (ssx <= destinationX) {

			ssx = destinationX;
			loading = 0;
			floorGrid[p.roomX][p.roomY].visited = true;
		}
	}
}

void EnemyMovement(void)
{
	// move and collide enemies ###########################################################

	if (loading == 0 && levelChanging == 0) {

		for (i = 0; i < 4; i++) {

			if (floorGrid[p.roomX][p.roomY].enemy[i].GetActive() == 1) { //for each active enemy in the current room

				if (floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() == floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationX() &&
					floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() == floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationY()) { //if the enemy has finished it's current movement

					floorGrid[p.roomX][p.roomY].enemy[i].SetState(1);

					if (floorGrid[p.roomX][p.roomY].enemy[i].chosenDirection == false) { //pick a new direction
						floorGrid[p.roomX][p.roomY].enemy[i].ChooseDirection();
						floorGrid[p.roomX][p.roomY].enemy[i].chosenDirection = true;
					}

					floorGrid[p.roomX][p.roomY].enemy[i].t2 = GetTickCount();
					if ((floorGrid[p.roomX][p.roomY].enemy[i].t2 - floorGrid[p.roomX][p.roomY].enemy[i].t1) > 1000)//Wait for up to 1 second;
					{
						floorGrid[p.roomX][p.roomY].enemy[i].t1 = floorGrid[p.roomX][p.roomY].enemy[i].t2;
						floorGrid[p.roomX][p.roomY].enemy[i].StartPatrol(); //start enemy movement
						floorGrid[p.roomX][p.roomY].enemy[i].chosenDirection = false;
					}
				}

				if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 0) {

					if (((p.xpos > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && p.xpos < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width)
						|| (p.xpos + p.width > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && p.xpos + p.width < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width))
						&& p.ypos < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos()) {

						if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].StartCharge(p.xpos, p.ypos);
					}
				}
				if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 1) {

					if (((p.ypos > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() && p.ypos < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height)
						|| (p.ypos + p.height > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() && p.ypos + p.height < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height))
						&& p.xpos < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos()) {

						if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].StartCharge(p.xpos, p.ypos);
					}
				}
				if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 2) {

					if (((p.xpos > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && p.xpos < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width)
						|| (p.xpos + p.width > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && p.xpos + p.width < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width))
						&& p.ypos > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos()) {

						if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].StartCharge(p.xpos, p.ypos);
					}
				}
				if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 3) {

					if (((p.ypos > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() && p.ypos < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height)
						|| (p.ypos + p.height > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() && p.ypos + p.height < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height))
						&& p.xpos > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos()) {

						if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].StartCharge(p.xpos, p.ypos);
					}
				}

				if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) { //patrol mode movement
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() < floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationX()) { //move the enemy to their new destination

						floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() > floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationX()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() < floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationY()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() > floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationY()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);
					}
				}
				if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 2) { //chase mode movement
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() < floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationX()) { //move the enemy to their new destination

						floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].chargeSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() > floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationX()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].chargeSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() < floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationY()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].chargeSpeed);
					}
					if (floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() > floorGrid[p.roomX][p.roomY].enemy[i].GetDestinationY()) {

						floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].chargeSpeed);
					}
				}
			}

			if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 0) {//walking up
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileX((floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + 20) / TS);
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileY((floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + 40) / TS);
				// wall collisions
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 7 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 7) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 8 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 8) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 9 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 9) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 14 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 14) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 15 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 15) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
			}
			if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 2) { // moving down
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileX((floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + 20) / TS);
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileY((floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + 10 + floorGrid[p.roomX][p.roomY].enemy[i].height) / TS);
				// wall collisions
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 1 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 1) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 2 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 2) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 3 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 3) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 12 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 12) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 13 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 13) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX() + floorGrid[p.roomX][p.roomY].enemy[i].width][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetYPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
			}
			if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 1) { //moving left
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileX((floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + 20) / TS);
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileY((floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + 40) / TS);
				// wall collisions
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 1 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 1) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 4 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 4) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 7 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 7) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 13 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 13) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 15 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 15) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 18) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
			}
			if (floorGrid[p.roomX][p.roomY].enemy[i].GetDirection() == 3) { //moving right
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileX((floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + 18 + floorGrid[p.roomX][p.roomY].enemy[i].width) / TS);
				floorGrid[p.roomX][p.roomY].enemy[i].SetTileY((floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + 40) / TS);
				// wall collisions
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 3 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 3) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 6 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 6) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 9 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 9) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 12 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 12) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 14 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 14) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());
				}
				if (map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY()] == 18 || map[floorGrid[p.roomX][p.roomY].enemy[i].GetTileX()][floorGrid[p.roomX][p.roomY].enemy[i].GetTileY() + floorGrid[p.roomX][p.roomY].enemy[i].height] == 18) {

					floorGrid[p.roomX][p.roomY].enemy[i].SetXPos(-floorGrid[p.roomX][p.roomY].enemy[i].walkSpeed);

					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationX(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos());
					floorGrid[p.roomX][p.roomY].enemy[i].SetDestinationY(floorGrid[p.roomX][p.roomY].enemy[i].GetYPos());

				}
			}
		}
	}
}

void ArrowMovementAndCollision(void)
{
	//move and collide the arrow ###################################################
	for (i = 0; i < 100; i++) {

		if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 0) {

			arrow[i].ypos -= arrow[i].speed;
			arrow[i].tileX = (arrow[i].xpos) / TS;
			arrow[i].tileY = (arrow[i].ypos + 5) / TS;
			if (map[arrow[i].tileX][arrow[i].tileY] == 7 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 7) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 8 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 8) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 9 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 9) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 14 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 14) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 15 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 15) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 18 || map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 18) { arrow[i].ypos += arrow[i].speed; arrow[i].state = 2; }

			for (j = 0; j < 4; j++) {
				if (arrow[i].xpos + (arrow[i].width / 2) > floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() && arrow[i].xpos + (arrow[i].width / 2) < floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[j].width
					&& arrow[i].ypos > floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() && arrow[i].ypos < floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[j].height) {

					floorGrid[p.roomX][p.roomY].enemy[j].AdjustHealth(-1);
					if (floorGrid[p.roomX][p.roomY].enemy[j].GetHealth() == 0) {

						floorGrid[p.roomX][p.roomY].enemy[j].Destroy();
						floorGrid[p.roomX][p.roomY].enemyCount--;
					}
				}
			}
		}
		if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 1) {

			arrow[i].xpos -= arrow[i].speed;
			arrow[i].tileX = (arrow[i].xpos) / TS;
			arrow[i].tileY = (arrow[i].ypos + 3) / TS;
			if (map[arrow[i].tileX][arrow[i].tileY] == 1 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 1) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 4 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 4) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 7 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 7) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 13 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 13) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 15 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 15) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 18 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 18) { arrow[i].xpos += arrow[i].speed; arrow[i].state = 2; }

			for (j = 0; j < 4; j++) {
				if (arrow[i].xpos > floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() && arrow[i].xpos < floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[j].width
					&& arrow[i].ypos + (arrow[i].width / 2) > floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() && arrow[i].ypos + (arrow[i].width / 2) < floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[j].height) {

					floorGrid[p.roomX][p.roomY].enemy[j].AdjustHealth(-1);
					if (floorGrid[p.roomX][p.roomY].enemy[j].GetHealth() == 0) {

						floorGrid[p.roomX][p.roomY].enemy[j].Destroy();
						floorGrid[p.roomX][p.roomY].enemyCount--;
					}
				}
			}
		}
		if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 2) {

			arrow[i].ypos += arrow[i].speed;
			arrow[i].tileX = (arrow[i].xpos + 3) / TS;
			arrow[i].tileY = (arrow[i].ypos + 30) / TS;
			if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 1 || map[arrow[i].tileX][arrow[i].tileY] == 1) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 2 || map[arrow[i].tileX][arrow[i].tileY] == 2) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 3 || map[arrow[i].tileX][arrow[i].tileY] == 3) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 12 || map[arrow[i].tileX][arrow[i].tileY] == 12) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 13 || map[arrow[i].tileX][arrow[i].tileY] == 13) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[(arrow[i].xpos + arrow[i].width) / TS][arrow[i].tileY] == 18 || map[arrow[i].tileX][arrow[i].tileY] == 18) { arrow[i].ypos -= arrow[i].speed; arrow[i].state = 2; }

			for (j = 0; j < 4; j++) {
				if (arrow[i].xpos + (arrow[i].width / 2) > floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() && arrow[i].xpos + (arrow[i].width / 2) < floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[j].width
					&& arrow[i].ypos > floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() && arrow[i].ypos < floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[j].height) {

					floorGrid[p.roomX][p.roomY].enemy[j].AdjustHealth(-1);
					if (floorGrid[p.roomX][p.roomY].enemy[j].GetHealth() == 0) {

						floorGrid[p.roomX][p.roomY].enemy[j].Destroy();
						floorGrid[p.roomX][p.roomY].enemyCount--;
					}
				}
			}
		}
		if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 3) {

			arrow[i].xpos += arrow[i].speed;
			arrow[i].tileX = (arrow[i].xpos + 33) / TS;
			arrow[i].tileY = (arrow[i].ypos + 3) / TS;
			if (map[arrow[i].tileX][arrow[i].tileY] == 3 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 3) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 6 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 6) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 9 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 9) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 12 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 12) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 14 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 14) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }
			else if (map[arrow[i].tileX][arrow[i].tileY] == 18 || map[arrow[i].tileX][(arrow[i].ypos + arrow[i].width) / TS] == 18) { arrow[i].xpos -= arrow[i].speed; arrow[i].state = 2; }

			for (j = 0; j < 4; j++) {
				if (arrow[i].xpos > floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() && arrow[i].xpos < floorGrid[p.roomX][p.roomY].enemy[j].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[j].width
					&& arrow[i].ypos + (arrow[i].width / 2) > floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() && arrow[i].ypos + (arrow[i].width / 2) < floorGrid[p.roomX][p.roomY].enemy[j].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[j].height) {

					floorGrid[p.roomX][p.roomY].enemy[j].AdjustHealth(-1);
					if (floorGrid[p.roomX][p.roomY].enemy[j].GetHealth() == 0) {

						floorGrid[p.roomX][p.roomY].enemy[j].Destroy();
						floorGrid[p.roomX][p.roomY].enemyCount--;
					}
				}
			}
		}
	}
}

void PlayerInputAndCollision(void)
{
	//input keyboard and collide player ###################################################

	if (loading == 0 && levelChanging == 0) {

		if (p.animationState != 8 && p.animationState != 9 && p.animationState != 10 && p.animationState != 11) {

			if (KEY_DOWN(0x57)) { //move up

				p.animationState = 4;
				p.ypos -= p.walkSpeed;
				p.tileX = (p.xpos + 20) / TS;
				p.tileY = (p.ypos + 40) / TS;
				// wall collisions
				if (map[p.tileX][p.tileY] == 7 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 7) p.ypos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 8 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 8) p.ypos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 9 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 9) p.ypos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 14 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 14) p.ypos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 15 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 15) p.ypos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 18 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 18) p.ypos += p.walkSpeed;
				else if (p.damaged == false && (map[p.tileX][p.tileY] == 19 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 19)) {
					p.damaged = true; p.health--; p.ypos += 3 * p.walkSpeed; damageTime1 = GetTickCount();
				}
				else if (map[p.tileX][p.tileY] == 20 || map[(p.xpos + p.width + 12) / TS][p.tileY] == 20) AdvanceLevel();

			}
			if (KEY_DOWN(0x53)) { //move down

				p.animationState = 6;

				p.ypos += p.walkSpeed;
				p.tileX = (p.xpos + 20) / TS;
				p.tileY = (p.ypos + 10 + p.height) / TS;
				if (map[(p.xpos + p.width) / TS][p.tileY] == 1 || map[p.tileX][p.tileY] == 1) p.ypos -= p.walkSpeed;
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 2 || map[p.tileX][p.tileY] == 2) p.ypos -= p.walkSpeed;
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 3 || map[p.tileX][p.tileY] == 3) p.ypos -= p.walkSpeed;
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 12 || map[p.tileX][p.tileY] == 12) p.ypos -= p.walkSpeed;
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 13 || map[p.tileX][p.tileY] == 13) p.ypos -= p.walkSpeed;
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 18 || map[p.tileX][p.tileY] == 18) p.ypos -= p.walkSpeed;
				else if (p.damaged == false && (map[(p.xpos + p.width + 12) / TS][p.tileY] == 19 || map[p.tileX][p.tileY] == 19)) {
					p.damaged = true; p.health--; p.ypos -= 3 * p.walkSpeed; damageTime1 = GetTickCount();
				}
				else if (map[(p.xpos + p.width + 12) / TS][p.tileY] == 20 || map[p.tileX][p.tileY] == 20) AdvanceLevel();
			}
			if (KEY_DOWN(0x41)) { //move left

				p.animationState = 5;

				p.xpos -= p.walkSpeed;
				p.tileX = (p.xpos + 20) / TS;
				p.tileY = (p.ypos + 40) / TS;
				if (map[p.tileX][p.tileY] == 1 || map[p.tileX][(p.ypos + p.height) / TS] == 1) p.xpos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 4 || map[p.tileX][(p.ypos + p.height) / TS] == 4) p.xpos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 7 || map[p.tileX][(p.ypos + p.height) / TS] == 7) p.xpos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 13 || map[p.tileX][(p.ypos + p.height) / TS] == 13) p.xpos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 15 || map[p.tileX][(p.ypos + p.height) / TS] == 15) p.xpos += p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 18 || map[p.tileX][(p.ypos + p.height) / TS] == 18) p.xpos += p.walkSpeed;
				else if (p.damaged == false && (map[p.tileX][p.tileY] == 19 || map[p.tileX][(p.ypos + p.height) / TS] == 19)) {
					p.damaged = true; p.health--; p.xpos += 3 * p.walkSpeed; damageTime1 = GetTickCount();
				}
				else if (map[p.tileX][p.tileY] == 20 || map[p.tileX][(p.ypos + p.height) / TS] == 20) AdvanceLevel();
			}
			if (KEY_DOWN(0x44)) { //move right

				p.animationState = 7;

				p.xpos += p.walkSpeed;
				p.tileX = (p.xpos + 18 + p.width) / TS;
				p.tileY = (p.ypos + 40) / TS;
				if (map[p.tileX][p.tileY] == 3 || map[p.tileX][(p.ypos + p.height) / TS] == 3) p.xpos -= p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 6 || map[p.tileX][(p.ypos + p.height) / TS] == 6) p.xpos -= p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 9 || map[p.tileX][(p.ypos + p.height) / TS] == 9) p.xpos -= p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 12 || map[p.tileX][(p.ypos + p.height) / TS] == 12) p.xpos -= p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 14 || map[p.tileX][(p.ypos + p.height) / TS] == 14) p.xpos -= p.walkSpeed;
				else if (map[p.tileX][p.tileY] == 18 || map[p.tileX][(p.ypos + p.height) / TS] == 18) p.xpos -= p.walkSpeed;
				else if (p.damaged == false && (map[p.tileX][p.tileY] == 19 || map[p.tileX][(p.ypos + p.height) / TS] == 19)) {
					p.damaged = true; p.health--; p.xpos -= 3 * p.walkSpeed; damageTime1 = GetTickCount();
				}
				else if (map[p.tileX][p.tileY] == 20 || map[p.tileX][(p.ypos + p.height) / TS] == 20) AdvanceLevel();
			}
			if (!KEY_DOWN(0x57) && p.animationState == 4) { f = 0; p.animationState = 0; }
			if (!KEY_DOWN(0x53) && p.animationState == 6) { f = 0; p.animationState = 2; }
			if (!KEY_DOWN(0x41) && p.animationState == 5) { f = 0; p.animationState = 1; }
			if (!KEY_DOWN(0x44) && p.animationState == 7) { f = 0; p.animationState = 3; }

			// arrow pickup
			for (i = 0; i < 100; i++) {
				if (p.tileX == arrow[i].tileX && p.tileY == arrow[i].tileY && p.arrowCount < p.arrowMax) {

					arrow[i].active = 0; arrow[i].state = 0;
					arrow[i].xpos = 0; arrow[i].ypos = 0; arrow[i].tileX = 0; arrow[i].tileY = 0;
					p.arrowCount++;
				}
			}

			// key pickup
			for (i = 0; i < 100; i++) {
				if (p.tileX == key[i].xpos / TS && p.tileY == key[i].ypos / TS && key[i].active == 1 && floorGrid[p.roomX][p.roomY].enemyCount == 0) {

					key[i].active = 0;
					key[i].xpos = 0; key[i].ypos = 0;
					p.keyCount++;
				}
			}

			//  collectables pickup
			for (i = 0; i < 2; i++) {
				if (p.tileX == floorGrid[p.roomX][p.roomY].pickupList[i].xpos / TS && p.tileY == floorGrid[p.roomX][p.roomY].pickupList[i].ypos / TS && floorGrid[p.roomX][p.roomY].pickupList[i].active == 1) {

					if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 0 && p.health < p.healthMax) {
						floorGrid[p.roomX][p.roomY].pickupList[i].active = 0;
						floorGrid[p.roomX][p.roomY].pickupList[i].xpos = 0; floorGrid[p.roomX][p.roomY].pickupList[i].ypos = 0;
						p.health++;
					}
					if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 1 && p.health < p.healthMax) {
						floorGrid[p.roomX][p.roomY].pickupList[i].active = 0;
						floorGrid[p.roomX][p.roomY].pickupList[i].xpos = 0; floorGrid[p.roomX][p.roomY].pickupList[i].ypos = 0;
						p.health += 2;
						if (p.health > p.healthMax) p.health = p.healthMax;
					}
					if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 2 && p.arrowCount < p.arrowMax) {
						floorGrid[p.roomX][p.roomY].pickupList[i].active = 0;
						floorGrid[p.roomX][p.roomY].pickupList[i].xpos = 0; floorGrid[p.roomX][p.roomY].pickupList[i].ypos = 0;
						p.arrowCount++;
					}
					if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 3 && p.healthMax < 9 && floorGrid[p.roomX][p.roomY].enemyCount == 0) {
						floorGrid[p.roomX][p.roomY].pickupList[i].active = 0;
						floorGrid[p.roomX][p.roomY].pickupList[i].xpos = 0; floorGrid[p.roomX][p.roomY].pickupList[i].ypos = 0;
						p.healthMax += 2; p.health += 2;
					}
					if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 4 && p.arrowMax < 17 && floorGrid[p.roomX][p.roomY].enemyCount == 0) {
						floorGrid[p.roomX][p.roomY].pickupList[i].active = 0;
						floorGrid[p.roomX][p.roomY].pickupList[i].xpos = 0; floorGrid[p.roomX][p.roomY].pickupList[i].ypos = 0;
						p.arrowMax += 4; p.arrowCount += 4;
					}
				}
			}
		}

		//shooting
		if (p.arrowCount > 0) {

			if (KEY_DOWN(VK_SPACE) && (p.animationState == 4 || p.animationState == 0)) { f = 0; p.animationState = 8; }
			if (KEY_DOWN(VK_SPACE) && (p.animationState == 6 || p.animationState == 2)) { f = 0; p.animationState = 10; }
			if (KEY_DOWN(VK_SPACE) && (p.animationState == 5 || p.animationState == 1)) { f = 0; p.animationState = 9; }
			if (KEY_DOWN(VK_SPACE) && (p.animationState == 7 || p.animationState == 3)) { f = 0; p.animationState = 11; }

			if (!KEY_DOWN(VK_SPACE) && p.animationState == 8 && f < 6) { f = 0; p.animationState = 0; }
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 10 && f < 6) { f = 0; p.animationState = 2; }
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 9 && f < 6) { f = 0; p.animationState = 1; }
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 11 && f < 6) { f = 0; p.animationState = 3; }

			if (KEY_DOWN(VK_SPACE) && p.animationState == 8 && f == 9) { f = 7; }
			if (KEY_DOWN(VK_SPACE) && p.animationState == 10 && f == 9) { f = 7; }
			if (KEY_DOWN(VK_SPACE) && p.animationState == 9 && f == 9) { f = 7; }
			if (KEY_DOWN(VK_SPACE) && p.animationState == 11 && f == 9) { f = 7; }

			if (!KEY_DOWN(VK_SPACE) && p.animationState == 8 && (f == 6 || f == 7 || f == 8)) {

				for (i = 0; i < 100; i++) {
					if (arrow[i].active == 0) {
						arrow[i].active = 1; arrow[i].state = 1;
						arrow[i].direction = 0;
						arrow[i].xpos = p.xpos + 29;
						arrow[i].ypos = p.ypos + 10;
						p.arrowCount--;
						f = 9;
						break;
					}
				}
			}
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 9 && (f == 6 || f == 7 || f == 8)) {

				for (i = 0; i < 100; i++) {
					if (arrow[i].active == 0) {
						arrow[i].active = 1; arrow[i].state = 1;
						arrow[i].direction = 1;
						arrow[i].xpos = p.xpos + 3;
						arrow[i].ypos = p.ypos + 29;
						p.arrowCount--;
						f = 9;
						break;
					}
				}
			}
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 10 && (f == 6 || f == 7 || f == 8)) {

				for (i = 0; i < 100; i++) {
					if (arrow[i].active == 0) {
						arrow[i].active = 1; arrow[i].state = 1;
						arrow[i].direction = 2;
						arrow[i].xpos = p.xpos + 29;
						arrow[i].ypos = p.ypos + 34;
						p.arrowCount--;
						f = 9;
						break;
					}
				}
			}
			if (!KEY_DOWN(VK_SPACE) && p.animationState == 11 && (f == 6 || f == 7 || f == 8)) {

				for (i = 0; i < 100; i++) {
					if (arrow[i].active == 0) {
						arrow[i].active = 1; arrow[i].state = 1;
						arrow[i].direction = 3;
						arrow[i].xpos = p.xpos + 28;
						arrow[i].ypos = p.ypos + 28;
						p.arrowCount--;
						f = 9;
						break;
					}
				}
			}
		}

		if (p.animationState == 8 && f == 11) { f = 0; p.animationState = 0; }
		if (p.animationState == 10 && f == 11) { f = 0; p.animationState = 2; }
		if (p.animationState == 9 && f == 11) { f = 0; p.animationState = 1; }
		if (p.animationState == 11 && f == 11) { f = 0; p.animationState = 3; }
	}
}

void DrawToScreen(void)
{
	//clear the screen black############################################
	r.left = 0;
	r.top = 0;
	r.right = SW;
	r.bottom = SH;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));//Use the blt to do a color fill square
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0, 0, 0);//dwFillColor;
	DDB->Blt(&r, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	//Draw tiles  #######################################################
	if (levelChanging == 0) {

		for (i = 0; i < (SW / TS) + 1; i++)
		{
			for (j = 0; j < (SH / TS); j++)
			{
				int a = i + sxnum; if (a >= MW) a = a - MW;
				int b = j + synum; if (b >= MH) b = b - MH;
				z = map[a][b];//get map tile number

				r.left = TS*z;
				r.top = 0;
				r.right = (TS*z + TS);
				r.bottom = TS;

				DDB->BltFast(i*TS - sxmod, j*TS - symod, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
		}
	}
	//Draw Keys ##############################################################################

	for (i = 0; i < 100; i++) {

		if (key[i].active == 1 && key[i].xpos > ssx && key[i].xpos < ssx + SW && key[i].ypos > ssy && key[i].ypos < ssy + SH) {

			if (floorGrid[p.roomX][p.roomY].enemyCount == 0) {
				r.left = TS * 29;
				r.top = 0;
				r.right = TS * 29 + TS / 2;
				r.bottom = TS / 2;
				DDB->BltFast(key[i].xpos - ssx + TS / 4, key[i].ypos - ssy + TS / 4, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
		}
	}
	// Draw Pickups ##########################################################################

	for (i = 0; i < 2; i++) {

		if (floorGrid[p.roomX][p.roomY].pickupList[i].active == 1 && floorGrid[p.roomX][p.roomY].pickupList[i].xpos > ssx && floorGrid[p.roomX][p.roomY].pickupList[i].xpos < ssx + SW && floorGrid[p.roomX][p.roomY].pickupList[i].ypos > ssy && floorGrid[p.roomX][p.roomY].pickupList[i].ypos < ssy + SH) {

			if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 0) {
				r.left = TS * 29 + TS / 2;
				r.top = TS / 2;
				r.right = TS * 30;
				r.bottom = TS;
				DDB->BltFast(floorGrid[p.roomX][p.roomY].pickupList[i].xpos - ssx + TS / 4, floorGrid[p.roomX][p.roomY].pickupList[i].ypos - ssy + TS / 4, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 1) {
				r.left = TS * 29;
				r.top = TS / 2;
				r.right = TS * 29 + TS / 2;
				r.bottom = TS;
				DDB->BltFast(floorGrid[p.roomX][p.roomY].pickupList[i].xpos - ssx + TS / 4, floorGrid[p.roomX][p.roomY].pickupList[i].ypos - ssy + TS / 4, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 2) {
				r.left = 484;
				r.top = 666;
				r.right = 496;
				r.bottom = 703;
				DDB->BltFast(floorGrid[p.roomX][p.roomY].pickupList[i].xpos - ssx + 25, floorGrid[p.roomX][p.roomY].pickupList[i].ypos - ssy + TS / 4, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 3 && floorGrid[p.roomX][p.roomY].enemyCount == 0) {
				r.left = TS * 9;
				r.top = TS * 2;
				r.right = TS * 10;
				r.bottom = TS * 3;
				DDB->BltFast(floorGrid[p.roomX][p.roomY].pickupList[i].xpos - ssx, floorGrid[p.roomX][p.roomY].pickupList[i].ypos - ssy, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			if (floorGrid[p.roomX][p.roomY].pickupList[i].type == 4 && floorGrid[p.roomX][p.roomY].enemyCount == 0) {
				r.left = TS * 9;
				r.top = TS * 3;
				r.right = TS * 10;
				r.bottom = TS * 4;
				DDB->BltFast(floorGrid[p.roomX][p.roomY].pickupList[i].xpos - ssx, floorGrid[p.roomX][p.roomY].pickupList[i].ypos - ssy, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
		}
	}

	//draw arrow (BELOW p) #############################################################
	for (i = 0; i < 100; i++) {
		if (arrow[i].xpos > ssx && arrow[i].xpos < ssx + SW && arrow[i].ypos > ssy && arrow[i].ypos < ssy + SH) {

			if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 0) { // North, In-Flight
				arrowr.left = 424;
				arrowr.top = 671;
				arrowr.right = 431;
				arrowr.bottom = 704;
			}
			if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 1) { //West, In Flight
				arrowr.left = 434;
				arrowr.top = 681;
				arrowr.right = 467;
				arrowr.bottom = 688;
			}
			if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 2) { // South, In Flight
				arrowr.left = 414;
				arrowr.top = 671;
				arrowr.right = 422;
				arrowr.bottom = 704;
			}
			if (arrow[i].active == 1 && arrow[i].state == 1 && arrow[i].direction == 3) { // East, In Flight
				arrowr.left = 434;
				arrowr.top = 671;
				arrowr.right = 467;
				arrowr.bottom = 679;
			}

			arrow[i].t2 = GetTickCount();
			if ((arrow[i].t2 - arrow[i].t1) > arrow[i].delay)//Move to next frame;
			{
				arrow[i].t1 = arrow[i].t2;
				arrow[i].f++;
				if (arrow[i].f > 1) arrow[i].f = 0;
			}

			if (arrow[i].active == 1 && arrow[i].state == 2 && arrow[i].direction == 0) { // North, At Rest
				if (arrow[i].f == 0) {
					arrowr.left = 424;
					arrowr.top = 671;
					arrowr.right = 431;
					arrowr.bottom = 704;
				}
				if (arrow[i].f == 1) {
					arrowr.left = 484;
					arrowr.top = 666;
					arrowr.right = 496;
					arrowr.bottom = 703;
				}
			}
			if (arrow[i].active == 1 && arrow[i].state == 2 && arrow[i].direction == 1) { // West, At Rest
				if (arrow[i].f == 0) {
					arrowr.left = 434;
					arrowr.top = 681;
					arrowr.right = 467;
					arrowr.bottom = 688;
				}
				if (arrow[i].f == 1) {
					arrowr.left = 498;
					arrowr.top = 680;
					arrowr.right = 536;
					arrowr.bottom = 691;
				}
			}
			if (arrow[i].active == 1 && arrow[i].state == 2 && arrow[i].direction == 2) { // South, At Rest
				if (arrow[i].f == 0) {
					arrowr.left = 414;
					arrowr.top = 671;
					arrowr.right = 422;
					arrowr.bottom = 704;
				}
				if (arrow[i].f == 1) {
					arrowr.left = 470;
					arrowr.top = 666;
					arrowr.right = 482;
					arrowr.bottom = 703;
				}
			}
			if (arrow[i].active == 1 && arrow[i].state == 2 && arrow[i].direction == 3) { // East, At Rest
				if (arrow[i].f == 0) {
					arrowr.left = 434;
					arrowr.top = 671;
					arrowr.right = 467;
					arrowr.bottom = 679;
				}
				if (arrow[i].f == 1) {
					arrowr.left = 498;
					arrowr.top = 665;
					arrowr.right = 535;
					arrowr.bottom = 678;
				}
			}

			DDB->BltFast(arrow[i].xpos - ssx, arrow[i].ypos - ssy, DD1, &arrowr, DDBLTFAST_SRCCOLORKEY);
		}
	}
	//draw enemies############################################################

	for (i = 0; i < 4; i++) {

		if (floorGrid[p.roomX][p.roomY].enemy[i].GetActive() == 1 && floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() > ssx && floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() < ssx + SW && floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() > ssy && floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() < ssy + SH) {

			EnemyAnimationSelector(i);

			floorGrid[p.roomX][p.roomY].enemy[i].animT2 = GetTickCount();
			if ((floorGrid[p.roomX][p.roomY].enemy[i].animT2 - floorGrid[p.roomX][p.roomY].enemy[i].animT1) > floorGrid[p.roomX][p.roomY].enemy[i].animDelay)//Move to next frame;
			{
				floorGrid[p.roomX][p.roomY].enemy[i].animT1 = floorGrid[p.roomX][p.roomY].enemy[i].animT2;
				floorGrid[p.roomX][p.roomY].enemy[i].f++;
				if (floorGrid[p.roomX][p.roomY].enemy[i].f > floorGrid[p.roomX][p.roomY].enemy[i].maxf) floorGrid[p.roomX][p.roomY].enemy[i].f = 0;
			}

			DDB->BltFast(floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() - ssx, floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() - ssy, DD1, &r, DDBLTFAST_SRCCOLORKEY);
		}
	}

	//draw player############################################################

	if (p.xpos > ssx && p.xpos < ssx + SW && p.ypos > ssy && p.ypos < ssy + SH) {

		AnimationSelector();

		// Decide which frame will be blitted next
		t2 = GetTickCount();
		if ((t2 - t1) > delay)//Move to next frame;
		{
			t1 = t2;
			f++;
			if (f > maxf) f = 0;
		}

		DDB->BltFast(p.xpos - ssx, p.ypos - ssy, DD1, &r, DDBLTFAST_SRCCOLORKEY);
	}

	//black strips ############################################################

	//left
	r.left = 0;
	r.top = 0;
	r.right = TS * 5;
	r.bottom = SH;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));//Use the blt to do a color fill square
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0, 0, 0);//dwFillColor;
	DDB->Blt(&r, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	//right
	r.left = SW - (TS * 10);
	r.top = 0;
	r.right = SW;
	r.bottom = SH;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));//Use the blt to do a color fill square
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0, 0, 0);//dwFillColor;
	DDB->Blt(&r, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	//top
	r.left = 0;
	r.top = 0;
	r.right = SW;
	r.bottom = TS + TS / 2;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));//Use the blt to do a color fill square
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0, 0, 0);//dwFillColor;
	DDB->Blt(&r, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	//bottom
	r.left = 0;
	r.top = SH - TS*1.5;
	r.right = SW;
	r.bottom = SH;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));//Use the blt to do a color fill square
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(0, 0, 0);//dwFillColor;
	DDB->Blt(&r, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	//draw UI ##############################################################
	//draw minimap #########################################################

	r.left = 1270;
	r.top = 120;
	r.right = 1480;
	r.bottom = 330;
	DDB->BltFast(TS - 10, 4 * TS - 8, DD1, &r, DDBLTFAST_SRCCOLORKEY);

	for (i = -1; i < 2; i++) //y pos
	{
		for (j = -1; j < 2; j++) //x pos
		{
			int roomType = -1;

			if (floorGrid[p.roomX + j][p.roomY + i].occupied == true) {
				roomType = floorGrid[p.roomX + j][p.roomY + i].roomType;
			}

			if (roomType == -1) {
				r.left = TS * 15;
				r.top = TS;
				r.right = (TS * 15) + TS;
				r.bottom = TS + TS;
				DDB->BltFast((2 + j) * TS, (5 + i) * TS, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			else if (roomType != -1 && floorGrid[p.roomX + j][p.roomY + i].visited == true) {
				r.left = TS * roomType;
				r.top = TS;
				r.right = (TS * roomType) + TS;
				r.bottom = TS + TS;
				DDB->BltFast((2 + j) * TS, (5 + i) * TS, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
			else if (roomType != -1 && floorGrid[p.roomX + j][p.roomY + i].visited == false) {
				r.left = TS * roomType;
				r.top = TS * 11;
				r.right = (TS * roomType) + TS;
				r.bottom = (TS * 11) + TS;
				DDB->BltFast((2 + j) * TS, (5 + i) * TS, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}

			if (floorGrid[p.roomX + j][p.roomY + i].roomContents == 3 && floorGrid[p.roomX + j][p.roomY + i].visited == true) { //Mark the Exit Room if it has been visited
				r.left = 1892;
				r.top = 0;
				r.right = 1920;
				r.bottom = 26;
				DDB->BltFast(((2 + j) * TS) + 18, ((5 + i) * TS) + 18, DD1, &r, DDBLTFAST_SRCCOLORKEY);
			}
		}
	}
	//blit player marker ###########################################

	r.left = 83;
	r.top = 270;
	r.right = 83 + 25;
	r.bottom = 270 + 23;
	DDB->BltFast(2 * TS + 20, 5 * TS + 20, DD1, &r, DDBLTFAST_SRCCOLORKEY);

	//player health UI ##########################################################
	if (p.healthMax >= 8) DDB->BltFast(24 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	if (p.healthMax == 10) DDB->BltFast(25 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);

	if (p.health == 10) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(24 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(25 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 9) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(24 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(25 * TS, 4 * TS, DD1, &hHalf, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 8) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(24 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 7) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(24 * TS, 4 * TS, DD1, &hHalf, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 6) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 5) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hHalf, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 4) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 3) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hHalf, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 2) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hFull, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 1) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hHalf, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	}
	if (p.health == 0) {
		DDB->BltFast(21 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(22 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
		DDB->BltFast(23 * TS, 4 * TS, DD1, &hEmpty, DDBLTFAST_SRCCOLORKEY);
	}

	//player arrow count and padlock count UI ##########################################################
	r.left = TS * 9;
	r.top = TS * 3;
	r.right = TS * 10;
	r.bottom = TS * 4;

	for (i = 0; i < p.arrowCount; i++) {

		DDB->BltFast(21.2 * TS + ((TS / 4) * i), 6 * TS, DD1, &r, DDBLTFAST_SRCCOLORKEY); //draw each arrow a quater of a tile to the right of the last
	}

	r.left = TS * 9;
	r.top = TS * 4;
	r.right = TS * 10;
	r.bottom = TS * 5;

	for (i = 0; i < (level.spawnedKeyRooms - p.keyCount); i++) {

		DDB->BltFast(TS + ((TS / 4) * i), 8 * TS, DD1, &r, DDBLTFAST_SRCCOLORKEY); //draw each padlock a quater of a tile to the right of the last
	}

	//Draw Game Over Text #########################################################

	if (gameOver == 1) {

		r.left = 272;
		r.top = 787;
		r.right = 780;
		r.bottom = 890;

		size += (0.13 * sizeDirection);

		if (size > 25.0 && sizeDirection > 0) sizeDirection = -1;
		if (size < 0.3 && sizeDirection < 0) sizeDirection = 1;

		rTextSize.left = (SW / 2) - 254 - size - (TS * 2.5);
		rTextSize.top = (SH / 2) - 52 - size;
		rTextSize.right = ((SW / 2) + 254) + size - (TS * 2.5);
		rTextSize.bottom = ((SH / 2) + 52) + size;
		fx.dwSize = sizeof(fx);

		DDB->Blt(&rTextSize, DD1, &r, DDBLT_KEYSRC, &fx);
	}

	//Draw Next Level Text #########################################################

	if (nextLevelText == 1) {

		r.left = 281;
		r.top = 912;
		r.right = 785;
		r.bottom = 1017;

		size += (0.35 * sizeDirection);

		if (size > 55.0 && sizeDirection > 0) { nextLevelText = 0; size = 1; }

		rTextSize.left = (SW / 2) - 252 - size - (TS * 2.5);
		rTextSize.top = (SH / 2) - 52 - size;
		rTextSize.right = ((SW / 2) + 252) + size - (TS * 2.5);
		rTextSize.bottom = ((SH / 2) + 52) + size;
		fx.dwSize = sizeof(fx);

		DDB->Blt(&rTextSize, DD1, &r, DDBLT_KEYSRC, &fx);
	}

	// Flip the surfaces
	while (TRUE)
	{
		hRet = DDP->Flip(NULL, 0); if (hRet == DD_OK) break;
		if (hRet == DDERR_SURFACELOST) { hRet = RestoreAll(); if (hRet != DD_OK) break; }
		if (hRet != DDERR_WASSTILLDRAWING) break;
	}
}

void EnemyAnimationSelector(int i)
{
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 0) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 0;
		r.left = 0;
		r.top = TS * 15;
		r.right = TS;
		r.bottom = TS * 16;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 1) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 0;
		r.left = 0;
		r.top = TS * 13;
		r.right = TS;
		r.bottom = TS * 14;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 2) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 0;
		r.left = 0;
		r.top = TS * 12;
		r.right = TS;
		r.bottom = TS * 13;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 3) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 0;
		r.left = 0;
		r.top = TS * 14;
		r.right = TS;
		r.bottom = TS * 15;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 4) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 3;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 80;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 2) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 40;
		r.left = TS * floorGrid[p.roomX][p.roomY].enemy[i].f;
		r.top = TS * 15;
		r.right = TS + (TS * floorGrid[p.roomX][p.roomY].enemy[i].f);
		r.bottom = TS * 16;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 5) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 3;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 80;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 2) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 40;
		r.left = TS * floorGrid[p.roomX][p.roomY].enemy[i].f;
		r.top = TS * 13;
		r.right = TS + (TS * floorGrid[p.roomX][p.roomY].enemy[i].f);
		r.bottom = TS * 14;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 6) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 3;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 80;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 2) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 40;
		r.left = TS * floorGrid[p.roomX][p.roomY].enemy[i].f;
		r.top = TS * 12;
		r.right = TS + (TS * floorGrid[p.roomX][p.roomY].enemy[i].f);
		r.bottom = TS * 13;
	}
	if (floorGrid[p.roomX][p.roomY].enemy[i].animationState == 7) {

		floorGrid[p.roomX][p.roomY].enemy[i].maxf = 3;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 1) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 80;
		if (floorGrid[p.roomX][p.roomY].enemy[i].GetState() == 2) floorGrid[p.roomX][p.roomY].enemy[i].animDelay = 40;
		r.left = TS * floorGrid[p.roomX][p.roomY].enemy[i].f;
		r.top = TS * 14;
		r.right = TS + (TS * floorGrid[p.roomX][p.roomY].enemy[i].f);
		r.bottom = TS * 15;
	}
}

void EnemyPlayerCollisions(void)
{
	for (int i = 0; i < 4; i++) {

		if (p.xpos + p.width > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && //top right corner of player collision
			p.xpos + p.width < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width &&
			p.ypos > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() &&
			p.ypos < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height) {

			p.damaged = true; p.health--; damageTime1 = GetTickCount(); break;
		}

		if (p.xpos > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && //top left corner of player collision
			p.xpos < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width &&
			p.ypos > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() &&
			p.ypos < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height) {

			p.damaged = true; p.health--; damageTime1 = GetTickCount(); break;
		}

		if (p.xpos > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && //bottom left corner of player collision
			p.xpos < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width &&
			p.ypos + p.height > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() &&
			p.ypos + p.height < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height) {

			p.damaged = true; p.health--; damageTime1 = GetTickCount(); break;
		}

		if (p.xpos + p.width > floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() && //bottom right corner of player collision
			p.xpos + p.width < floorGrid[p.roomX][p.roomY].enemy[i].GetXPos() + floorGrid[p.roomX][p.roomY].enemy[i].width &&
			p.ypos + p.height > floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() &&
			p.ypos + p.height < floorGrid[p.roomX][p.roomY].enemy[i].GetYPos() + floorGrid[p.roomX][p.roomY].enemy[i].height) {

			p.damaged = true; p.health--; damageTime1 = GetTickCount(); break;
		}
	}
}

void AnimationSelector(void)
{
	if (p.animationState == 0) {

		maxf = 0;
		delay = 40;
		r.left = 0;
		r.top = TS * 2;
		r.right = TS;
		r.bottom = (TS * 2) + TS;
	}
	if (p.animationState == 1) {

		maxf = 0;
		delay = 40;
		r.left = 0;
		r.top = TS * 3;
		r.right = TS;
		r.bottom = (TS * 3) + TS;
	}
	if (p.animationState == 2) {

		maxf = 0;
		delay = 40;
		r.left = 0;
		r.top = TS * 4;
		r.right = TS;
		r.bottom = (TS * 4) + TS;
	}
	if (p.animationState == 3) {

		maxf = 0;
		delay = 40;
		r.left = 0;
		r.top = TS * 5;
		r.right = TS;
		r.bottom = (TS * 5) + TS;
	}
	if (p.animationState == 4) {

		maxf = 8;
		delay = 35;
		r.left = f * TS;
		r.top = TS * 2;
		r.right = TS + (f * TS);
		r.bottom = (TS * 2) + TS;
	}
	if (p.animationState == 5) {

		maxf = 8;
		delay = 35;
		r.left = f * TS;
		r.top = TS * 3;
		r.right = TS + (f * TS);
		r.bottom = (TS * 3) + TS;
	}
	if (p.animationState == 6) {

		maxf = 8;
		delay = 35;
		r.left = f * TS;
		r.top = TS * 4;
		r.right = TS + (f * TS);
		r.bottom = (TS * 4) + TS;
	}
	if (p.animationState == 7) {

		maxf = 8;
		delay = 35;
		r.left = f * TS;
		r.top = TS * 5;
		r.right = TS + (f * TS);
		r.bottom = (TS * 5) + TS;
	}
	if (p.animationState == 8) {

		maxf = 11;
		delay = 45;
		r.left = f * TS;
		r.top = TS * 6;
		r.right = TS + (f * TS);
		r.bottom = (TS * 6) + TS;
	}
	if (p.animationState == 9) {

		maxf = 11;
		delay = 45;
		r.left = f * TS;
		r.top = TS * 7;
		r.right = TS + (f * TS);
		r.bottom = (TS * 7) + TS;
	}
	if (p.animationState == 10) {

		maxf = 11;
		delay = 45;
		r.left = f * TS;
		r.top = TS * 8;
		r.right = TS + (f * TS);
		r.bottom = (TS * 8) + TS;
	}
	if (p.animationState == 11) {

		maxf = 11;
		delay = 45;
		r.left = f * TS;
		r.top = TS * 9;
		r.right = TS + (f * TS);
		r.bottom = (TS * 9) + TS;
	}
	if (p.animationState == 12) {

		maxf = 6;
		delay = 200;
		r.left = f * TS;
		r.top = TS * 10;
		r.right = TS + (f * TS);
		r.bottom = (TS * 10) + TS;
	}
}

void AdvanceLevel(void)
{
	if (levelChanging == 0) {

		levelChanging = 1;
		level.levelNum++;
		roomCount = 0;
		roomMax += 3;
		level.keyRooms = 1 + (int)(roomMax / 10);
		level.spawnedKeyRooms = 0;

		for (int i = 0; i < 100; i++) {

			key[i].active = 0;
			key[i].xpos = 0;
			key[i].ypos = 0;

			for (int j = 0; j < 100; j++) {

				arrow[i].active = 0;
				arrow[i].direction = 0;
				arrow[i].state = 0;
				arrow[i].xpos = 0;
				arrow[i].ypos = 0;
				arrow[i].tileX = 0;
				arrow[i].tileY = 0;

				floorGrid[i][j].occupied = false;
				floorGrid[i][j].examined = false; //Marked true when a room has been checked by CheckSurroundingRooms method
				floorGrid[i][j].visited = false;
				floorGrid[i][j].roomType = 0;
				floorGrid[i][j].roomContents = 0;  //0 is normal room, 1 is starting room, 2 is key room, 3 is exit room
				floorGrid[i][j].roomStructure = 0;
				floorGrid[i][j].northPath = 0; //0 is no path(wall), 1 is empty path, 2 is already connected room path
				floorGrid[i][j].eastPath = 0;
				floorGrid[i][j].southPath = 0;
				floorGrid[i][j].westPath = 0;
				floorGrid[i][j].enemyCount = 0;

				for (int k = 0; k < 2; k++) {

					floorGrid[i][j].pickupList[k].active = 0;
					floorGrid[i][j].pickupList[k].type = 0;
					floorGrid[i][j].pickupList[k].xpos = 0;
					floorGrid[i][j].pickupList[k].ypos = 0;
				}
				for (int k = 0; k < 4; k++) {

					floorGrid[i][j].enemy[k].Destroy();
				}
			}
		}
		for (int i = 0; i < MH; i++) {
			for (int j = 0; j < MW; j++) {

				map[i][j] = 5;
			}
		}

		ssx = (TS*ROOMSIZE * 50) - (TS * 5), ssy = (TS*ROOMSIZE * 50) - TS;
		destinationX = ssx; destinationY = ssy;
		p.xpos = ssx + SW / 2 - TS * 3, p.ypos = ssy + SH / 2 - TS;
		p.tileX = (p.xpos + 20) / TS;
		p.tileY = (p.ypos + 30) / TS;
		p.roomX = 50, p.roomY = 50;
		p.animationState = 2;
		p.keyCount = 0;

		StartGenerator();
		nextLevelText = 1;
		levelChanging = 0;
	}
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) //Initialization, message loop
{
	MSG                         msg;

	if (InitApp(hInstance, nCmdShow) != DD_OK) return FALSE;
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) return msg.wParam;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (active) { UpdateFrame(); }
		else { WaitMessage();/* Make sure we go to sleep if we have nothing else to do*/ }
	}
}

static void ReleaseAllObjects(void)
{
	if (DD != NULL)
	{
		if (DDP != NULL) { DDP->Release(); DDP = NULL; }
		if (DD1 != NULL) { DD1->Release(); DD1 = NULL; }
		DD->Release();
		DD = NULL;
	}
}

HRESULT InitFail(HWND hWnd, HRESULT hRet, LPCTSTR szError, ...)
{
	char                        szBuff[128];
	va_list                     vl;

	va_start(vl, szError);
	vsprintf_s(szBuff, szError, vl);
	ReleaseAllObjects();
	MessageBox(hWnd, szBuff, TITLE, MB_OK);
	DestroyWindow(hWnd);
	va_end(vl);
	return hRet;
}

HRESULT RestoreAll(void)
{
	HRESULT                     hRet;

	hRet = DDP->Restore();
	if (hRet == DD_OK)
	{
		hRet = DD1->Restore();
		if (hRet == DD_OK) { DDReLoadBitmap(DD1, bmp); }
	}
	return hRet;
}

void DisplayFrameRate(void) //every second
{
	HDC                  hdc;
	char  buf[256];
	float time, time2;

	framecounter++;
	dwFrameCount++;
	time = (float)GetTickCount();
	time2 = time - dwFrameTime;
	if (time2 > 1000)
	{
		dwFrames = (dwFrameCount * 1000) / time2;
		dwFrameTime = time;
		dwFrameCount = 0;
	}
	if (dwFrames == 0) return;
	if (dwFrames != dwFramesLast) dwFramesLast = dwFrames;
	if (DDB->GetDC(&hdc) == DD_OK)//print stuff on screen
	{
		//set text colour
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(0, 255, 0));
		//print title
		sprintf_s(buf, "Procedural Dungeon Crawler v0.4, FPS=%.1f, level=%1d, keyRooms=%1d, spawnedKeyRooms=%1d, keyCount=%1d", dwFrames, level.levelNum, level.keyRooms, level.spawnedKeyRooms, p.keyCount); TextOut(hdc, 0, SH - 64, buf, strlen(buf));
		//print title
		sprintf_s(buf, "pHealth=%1d, DamageCooldown=%4d, EnemyCount=%1d, Pickup[0]Type=%1d, Size=%.1f", p.health, damageTime2 - damageTime1, floorGrid[p.roomX][p.roomY].enemyCount, floorGrid[p.roomX][p.roomY].pickupList[0].type, size); TextOut(hdc, 0, SH - 48, buf, strlen(buf));
		//print room count
		sprintf_s(buf, "RoomCount=%.1d, Room x(%.1d),y(%.1d), RoomContents=%1d RoomVisted=%4d, arrowMax=%.1d, arrowCount=%.1d, loading=%.1d", roomCount, p.roomX, p.roomY, floorGrid[p.roomX][p.roomY].roomContents, floorGrid[p.roomX][p.roomY].visited, p.arrowMax, p.arrowCount, loading); TextOut(hdc, 0, SH - 32, buf, strlen(buf));
		//print p co-ordinates
		sprintf_s(buf, "xpos=%1d ypos=%1d  ssx=%4d ssy=%4d sxnum=%4d synum=%4d  pTileX=%2d pTileY=%2d", p.xpos, p.ypos, ssx, ssy, sxnum, synum, p.tileX, p.tileY); TextOut(hdc, 0, SH - 16, buf, strlen(buf));

		DDB->ReleaseDC(hdc);
	}
}

long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) //The Main Window Procedure
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
	}break;
	case WM_ACTIVATEAPP:
		// Pause if minimized or not the top window
		active = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
		return 0L;

	case WM_DESTROY:
		// Clean up and close the app
		ReleaseAllObjects();
		PostQuitMessage(0);
		return 0L;

	case WM_KEYDOWN:
		// Handle any non-accelerated key commands
		switch (wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0L;
		}
		break;

	case WM_SETCURSOR:
		// Turn off the cursor since this is a full-screen app
		SetCursor(NULL);
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static HRESULT InitApp(HINSTANCE hInstance, int nCmdShow)
{
	HWND                        hWnd;
	WNDCLASS                    wc;
	DDSURFACEDESC2              ddsd;
	DDSCAPS2                    ddscaps;
	HRESULT                     hRet;
	LPDIRECTDRAW                pDD;

	// Set up and register window class
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NAME;
	wc.lpszClassName = NAME;
	RegisterClass(&wc);
	//Create a window
	hWnd = CreateWindowEx
	(WS_EX_TOPMOST,
		NAME,
		TITLE,
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hWnd) return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	//Create the main DirectDraw object
	hRet = DirectDrawCreate(NULL, &pDD, NULL);
	if (hRet != DD_OK) return InitFail(hWnd, hRet, "DirectDrawCreate FAILED");
	//Fetch DirectDraw4 interface
	hRet = pDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&DD);
	if (hRet != DD_OK) return InitFail(hWnd, hRet, "QueryInterface FAILED");
	//Get exclusive mode
	hRet = DD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet != DD_OK)  return InitFail(hWnd, hRet, "SetCooperativeLevel FAILED");
	//Set the video mode to SWxSHx32
	hRet = DD->SetDisplayMode(SW, SH, 32, 0, 0);
	if (hRet != DD_OK)return InitFail(hWnd, hRet, "SetDisplayMode FAILED");
	// Create the primary surface with 1 back buffer
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;
	hRet = DD->CreateSurface(&ddsd, &DDP, NULL);
	if (hRet != DD_OK) return InitFail(hWnd, hRet, "CreateSurface FAILED");
	// Get a pointer to the back buffer
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	hRet = DDP->GetAttachedSurface(&ddscaps, &DDB);
	if (hRet != DD_OK) return InitFail(hWnd, hRet, "GetAttachedSurface FAILED");
	// Create the offscreen surface, by loading our bitmap.
	DD1 = DDLoadBitmap(DD, bmp, 0, 0);
	if (DD1 == NULL) return InitFail(hWnd, hRet, "DDLoadBitmap FAILED");
	// Set the color key for this bitmap (pink!)
	DDSetColorKey(DD1, RGB(255, 0, 255));

	// INITILISING VALUES ###########################################################

	p.xpos = ssx + SW / 2 - TS * 3, p.ypos = ssy + SH / 2 - TS;
	p.tileX = (p.xpos + 20) / TS; p.tileY = (p.ypos + 30) / TS;
	p.roomX = 50, p.roomY = 50;
	p.animationState = 2;
	p.arrowCount = 5;
	p.health = 6;
	p.arrowMax = 10;
	p.healthMax = 6;

	hFull.left = TS * 16;
	hFull.top = TS;
	hFull.right = TS * 17;
	hFull.bottom = TS * 2;

	hHalf.left = TS * 17;
	hHalf.top = TS;
	hHalf.right = TS * 18;
	hHalf.bottom = TS * 2;

	hEmpty.left = TS * 18;
	hEmpty.top = TS;
	hEmpty.right = TS * 19;
	hEmpty.bottom = TS * 2;

	level.levelNum = 1;
	level.keyRooms = 1;
	level.spawnedKeyRooms = 0;

	StartGenerator();

	t1 = GetTickCount();
	return DD_OK;
}