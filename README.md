# Raylib TrueTileCollision
Example showcases:
    * Use of 1D array for a tilemap and using it for collision;
    * Simple movement for a platformer (WASD + space) and top-down (TAB to switch between)
    * Rectangle based collision for movement and collectibles
    * Integer scaling with resizable window
    * Programmer/ procedural art using only built-in shape drawing
Main()
    * GameInit()
        * Reset()
    * GameLoop()
        * GameUpdate()
            * UpdateScreen();
            * UpdatePlayer();
                * RectangleCollisionUpdate(Rectangle *rect, Vector2 *velocity)
                * RectangleResize(Rectangle *rect, Vector2 *size)
                * RectangleListFromTiles(Rectangle *rect, Grid *grid)
                * RectangleTileCollision(Rectangle *rect, Vector2 *velocity, RectList *list)
            * UpdateCoin();
        * GameDraw()
            * DrawTileGrid()
            * DrawTileMap()
            * DrawCoins()
            * DrawPlayer()
            * DrawScoreText()
