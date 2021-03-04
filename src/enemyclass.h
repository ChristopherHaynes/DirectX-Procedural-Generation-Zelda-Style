#pragma once
class Enemy 
{
private:
	int active; // 0 inactive, 1 active
	int xpos, ypos, direction; //direction 0 - 3 (NWSE)
	int tileX, tileY;
	int destinationX, destinationY;
	int health;
	int behaviourState; // 0 NULL state, 1 patroling, 2 pursuing
public:
	static const int walkSpeed = 2, chargeSpeed = 5;
	static const int height = 50, width = 52;
	bool chosenDirection;
	int t1, t2, delay;
	int animT1, animT2, animDelay;
	int animationState; // 0 - 3 standing (NWSE), 4 - 7 walking (NWSE)
	int f, maxf;

	void Initalise(int xpos, int ypos);
	void Destroy(void);

	void ChooseDirection(void);
	void StartPatrol(void);
	void StartCharge(int playerX, int playerY);

	int GetActive(void) { return active; }
	int GetXPos(void) { return xpos; }
	int GetYPos(void) { return ypos; }
	int GetDestinationX(void) { return destinationX; }
	int GetDestinationY(void) { return destinationY; }
	int GetTileX(void) { return tileX; }
	int GetTileY(void) { return tileY; }
	int GetDirection(void) { return direction; }
	int GetHealth(void) { return health; }
	int GetState(void) { return behaviourState; }
	
	void SetActive(int value) { active = value; }
	void SetXPos(int xpos) { this->xpos += xpos; }
	void SetYPos(int ypos) { this->ypos += ypos; }
	void SetDestinationX(int xpos) { destinationX = xpos; }
	void SetDestinationY(int ypos) { destinationY = ypos; }
	void SetTileX(int tileX) { this->tileX = tileX; }
	void SetTileY(int tileY) { this->tileY = tileY; }
	void AdjustHealth(int healthChange);
	void SetState(int state);
};
