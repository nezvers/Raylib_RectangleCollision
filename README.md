# Raylib_TrueTileCollision
Retro platformer collision paired with modern approach utilizing delta time.

Raylib is used for giving physics a context. Code is written in C with clear code.    
Retro aspect comes in form of collision solving utilizing bitwise operations and pixel movement, carrying sub pixel movement to next frames.    
For collision detection is used arbitrary collision box in form of 8 points around the "box" (corners and mid-points), that are checked against 1D tile map array.
Tile map array need only hold index for tile type.
