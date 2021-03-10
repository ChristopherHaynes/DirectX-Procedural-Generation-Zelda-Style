# DirectX-Procedural-Generation-Zelda-Style-Game
Second year software development assignment to create a game using a Direct Draw environment, developing my own code for collision detection, animation and my own procedural dungeon generator. The concept for this game came from the idea of a mash-up between "Zelda: A Link to the Past" and "The Binding of Issac". This game renders as intended on 1920x1080 displays, but can produce unexpected results on displays of other sizes.

![Game Screenshot](https://github.com/ChristopherHaynes/DirectX-Procedural-Generation-Zelda-Style-Game/blob/master/res/game-screenshot.png?raw=true)

## Game Play
You will initially spawn on the 1st floor of the dungeon. A mini-map displaying your current position and the surrounding rooms is shown on the left of the screen and your health and arrow count will appear on the right. The goal on each floor of the dungeon is to find the required amount of keys in order to unlock the stairs to the next floor. The number of keys required will increase as you get deeper into the dungeon, and each floor will get larger consisting of more rooms. The remaining amount of keys required to unlock the stairs is represented by the number of padlocks underneath the mini-map. Rooms can contain health boosts, arrows, traps and enemies. If a room contains enemies, the doors will remain locked and you will not be able to progress until they have all been killed. There are also upgrades to be found that permanently upgrade your health and arrow count. How deep can you get into the dungeon... 

## Controls
W A S D - Directional Control

Space Bar - Hold to draw arrow. Once an arrow is fully drawn, release to fire

## Important Files
* __build/DungeonDave.exe__ - This is the compiled final version of the game. 
* __Dungeon Dave Project Report.pdf__ - This report contains a full write up on the design and creation of this game if more detail is required.
* __Room Grid.xlsx__ - This excel file contains a mock-up design of how rooms are laid out using the tile system.

## Dungeon Generation
Each floor of the dungeon is created through a bespoke procedural generation method. This process first determines the layout of the rooms in a floor, then works out the internal rooms structures before finally populating the rooms with items and enemies. In total there 15 different possible room types:

![Room Types](https://github.com/ChristopherHaynes/DirectX-Procedural-Generation-Zelda-Style-Game/blob/master/res/room-generation.png?raw=true)

The process for generating a floor is recursive and starts by placing a random room type into the center of a grid representing the floor. The recurrsive process then begins:
1. Examine all rooms which currently exist in the floor grid and record all currently open paths (E.G room type 1 would initially have a south open path).
1. Randomly select one of the open path positions.
1. Determine a short-list of viable room types which could fit into the open space.
1. Randomly select one of the viable room types and populate the space.
1. If the amount of remaining open spaces plus the amount of already generated rooms is equal to required floor size:
    1. FALSE - Clear the open paths list and start from step 1
    1. TRUE - Generate rooms to close off all remaining open paths
    
During this generation process three unique rooms are also determined using a weighted random selection; Spawn room, Key room and Exit room. These rooms all have unique structures and behaviours, however all other rooms are structured and populated procedurally. These other rooms first have one of 8 possible structures chosen, and then population of items, traps and enemies is done with another weighted random selection approach. The following image shows the 8 possible structures and the potential spawn points for items (blue), traps (grey), enemies (red):

![Room Structures](https://github.com/ChristopherHaynes/DirectX-Procedural-Generation-Zelda-Style-Game/blob/master/res/room-structures.png?raw=true)

There is much more detail on the procedural generation and rendering approach to these rooms within the project report, including:
* Covering edge cases in procedural generation
* Weighting approaches
* Tile mapping
* Screen scrolling

## License
Copyright (c) 2015-2021 Chris Haynes and others

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
