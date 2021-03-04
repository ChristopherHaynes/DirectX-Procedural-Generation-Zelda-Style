#include<iostream>
#include<time.h>
#include "enemyclass.h"
#define TS 64

void Enemy::Initalise(int xpos, int ypos) 
{
	active = 1;
	health = 1;
	this->xpos = xpos;
	this->ypos = ypos;
	tileX = (xpos + 20) / TS;
	tileY = (ypos + 30) / TS;
	direction = rand() % 4;
	destinationX = xpos;
	destinationY = ypos;
	behaviourState = 1;
}
void Enemy::Destroy(void)
{
	active = 0;
	xpos = 0;
	ypos = 0;
	behaviourState = 0;
}
void Enemy::AdjustHealth(int healthChange)
{
	health += healthChange;
}
void Enemy::SetState(int state)
{
	behaviourState = state;
}
void Enemy::StartPatrol(void) 
{
	if (behaviourState == 1) {
	
		if (direction == 0) {
			destinationY = ypos - ((rand() % 5 + 1) * TS);
		}
		if (direction == 1) {
			destinationX = xpos - ((rand() % 5 + 1) * TS);
		}
		if (direction == 2) {
			destinationY = ypos + ((rand() % 5 + 1) * TS);
		}
		if (direction == 3) {
			destinationX = xpos + ((rand() % 5 + 1) * TS);
		}
	}
	animationState = direction + 4;
}
void Enemy::ChooseDirection(void) 
{
	if (behaviourState == 1) {

		int directionNew = rand() % 4;
		while (direction == directionNew) { directionNew = rand() % 4; }
		direction = directionNew;

		f = 0;
		animationState = direction;
	}
}
void Enemy::StartCharge(int playerX, int playerY) 
{
	if (direction == 0) {		
		destinationY = playerY - (TS * 12);
	}
	if (direction == 1) {
		destinationX = playerX - (TS * 12);
	}
	if (direction == 2) {
		destinationY = playerY + (TS * 12);
	}
	if (direction == 3) {
		destinationX = playerX + (TS * 12);
	}
	SetState(2);
	animationState = direction + 4;
}