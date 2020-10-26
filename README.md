# Raylib TrueTileCollision
Retro platformer collision paired with modern approach utilizing delta time.

Raylib is used for giving physics a context. Code is written in C with clear code, so it's easily portable to C++ or other higher level language or framework.    
Retro aspect comes in form of collision solving utilizing bitwise operations and pixel movement, carrying sub pixel movement to next frames.    
For collision detection is used arbitrary collision box in form of 8 points around the "box" (corners and mid-points), that are checked against 1D tile map array (x + y*TILE_MAP_WIDTH).
The tile map array needs only hold an index for a tile type.

Modern touch is also in a form of object oriented programming using structs for Entity and Input, that can be used for player and enemies alike. 
Input instance is referenced and used as controller for Entity to control movement.    
Main influence is [Lizard](https://rainwarrior.itch.io/lizard) which source is an amazing learning material.

![](https://raw.githubusercontent.com/nezvers/Raylib_TrueTileCollision/main/preview.gif)
