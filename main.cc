/*******************************************************************

  SFML Simple Tetris
  By Adam Shepley
  June 2011

  This is a crappy Tetris game that I developed over an absrudely
  long amount of time.
  
  Of course, that's because I kept forgetting about it.
  The actual development time works out to a few hours altogether.


  Anyways, it works off of a 10x20 2-Dimensional Array.
  0 means empty, 3 through 9 are the tetris 'blocks', and anything
  above 10 is a moving block of a tetrimino - one of the 3 through
  9, with 10 added.
  
  Simple, but it works.
  
  Problem is, it means that I have to access the array in opposite
  order, so the array memory reading is rather inefficient as I'm
  jumping across memory by segments instead of linearly.
  
  I cannibalized most of my Snake code too. That can't be good.
  In truth, I'm rushing this out so I can get started on a real
  project.

  I copped out and flaked a LOT of the "Tetris Guidelines"
  Because this isn't a real product.
  
  For all fo the individual Tetriminos, we use a set of "current" 
  variables. These are the tetrimino block located to the top
  left of the tetrimino.
  
********************************************************************/



#define SFML_STATIC

#define NULL 0

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


#include <iostream>
#include <string>
#include <sstream>
#include <vector>


using std::vector;
using std::stringstream;
using std::cout;
using std::endl;
using std::string;

/*
//function used for multithreading
void ThreadFunction(void* UserData)
{
  //convert our userdata back to its original type

  int *testObject = static_cast<int*>(UserData);
  int test2 = *testObject;
}
*/


//This is our game board. We're keeping it global for simplicity's sake.
int gameBoard[10][22];

//This enum is used for our player inputs.
enum direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE, DIR_ROTATE };

enum rotation { flatDown, flatLeft, flatUp, flatRight };

//used for our game logic overall
bool Running = true;
bool playerKilled = false;
bool gamePaused = false;


//LARGER MEANS SLOWER.
//determines the rate of travel and collision checking
float timeSpeed = 0.7;

//this is used to check for user input, such that holding down the "right" direction doesn't send the block flying.
//In other words, Casual Modo
float movementSpeed = 0.025;


//deprecated.
float defaultSpeed = 0.1;


int wrapper = 20;

//we declare our score variables and direction enums
int currentScore = 0;
int highScore = 0;

//determines the distance our block travels.
float speed = 20;

///TESTING
//set this to true for test array data.
bool testBlock = false;


int defaultX = 5;
int defaultY = 19;
int testX = 0;
int testY = 0;
int testValue = 5;

int currBlockX = 5;
int currBlockY = 19;
rotation currBlockRotate;


// INITBOARD
//\ Initializes the board with the input number in the default position.

void initBoard()
{
  for(int i = 0; i < 10; i++)
  {
    for(int j = 0; j < 22; j++)
    {
      gameBoard[i][j] = 0;
    }
  }
}

void printBoardFromTop()
{
  cout << "Game Board Is: \n" << endl;
  for(int i = 19; i >= 0; i--)
  {
    for(int j = 0; j < 10; j++)
    {
      cout << gameBoard[j][i];
    }
    cout << endl;
  }
  
  cout << "\nEnd Game Board." << endl;
}

/// COLLISION TESTER
//we check to see if there's a collision at the bottom
bool checkHitVertical()
{
  //we check, horizontally, whether the block is above 
  for(int i = 0; i < 20; i++)
  {
    for(int j = 0; j < 10; j++)
    {
      if (gameBoard[j][i] > 10)
      {
	//prevent going out of bounds; if we're at the bottom, we have hit! May not be necessary.
	if (i == 0)
	  return true;
	
	//if we are above a piece that is solid but not moving, we have a collision
	if (gameBoard[j][i-1] > 0 && gameBoard[j][i-1] < 10)
	  return true;
      }
    }
  }
  
  //well, we're good.
  return false;
}


//this function shifts ALL blocks other than our current one down a vertical spot.
//use after clearing lines and such
void moveDullBlocksDown(int deletionCount, int *linesDeleted)
{
  int currBlock;
  
  for(int i = 0; i < 18; i++)
  {
    if(linesDeleted[i] == 2)
    {
      for(int x = 0; x < 10; x++)
      {
	for(int y = i; y < 19; y++)
	{
	  gameBoard[x][y] = gameBoard[x][y+1];
	}
      }
    }
  }
  
  /*
  int count = 0;
  for(int i = 0; i < 20; i++)
  {
   
  }
  cout << "DELETION COUNT IS " << deletionCount << endl;
  cout << "VALUE 1 IS " << linesDeleted[0] << " AND " << linesDeleted[1]  << " and " << linesDeleted[2] << endl;
  for(int i = 0; i < 10; i++)
  {
    for(int j = 1; j < 20; j++)
    {
      currBlock = gameBoard[i][j];
      if(currBlock > 0 && currBlock < 10)
      {
	if(gameBoard[i][j-1] == 0)
	{
	  gameBoard[i][j] = 0;
	  gameBoard[i][j-1] = currBlock;
	}
      }
    }
  }*/
}


int checkForFullLines()
{
  //we create an array to check what lines we need to get rid of
  int linesToDelete[20];
  
  bool somethingToDelete = false;
  int deletionCount = 0;
  for(int x = 0; x < 20; x++)
  {
    linesToDelete[x] = 2;
  }
  
  
  //we loop through each line and check to see if there is a 0; if so, the line has a space, and won't be deleted
  for(int i = 0; i < 20; i++)
  {
    for(int j = 0; j < 10; j++)
    {
      if(gameBoard[j][i] == 0)
      {
	linesToDelete[i] = 0;
      }
    }
    //this will save us time later
    if(linesToDelete[i] != 0)
    {
      somethingToDelete = true;
      deletionCount += 1;
    }
  }
  
  //why bother deleting nothing in a long loop?
  if(!somethingToDelete)
    return 0;
  
  for(int y = 0; y < 20; y++)
  {
    
    if(linesToDelete[y] > 0)
    {
      for(int i = 0; i < 10; i++)
      {
	gameBoard[i][y] = 0;
      }
    }
    
  }
  
  
  moveDullBlocksDown(deletionCount, linesToDelete);
  //we return the number of blocks we delete in order to give a score.
  return deletionCount;
  
}

//This is used to create all of the tetrimino blocks that go around the center block
//The trick is that the center block is ALWAYS center, despite rotations
//Returns true if there's immediately a hit afterwards - meaning our game has ended!
bool createTetrimino(int blockType)
{
  
  //we see if we're spawning on top of another block.
  for(int i = 3; i < 7; i++)
  {
    if(gameBoard[i][19] > 0 && gameBoard[i][19] < 10)
    {
      return true;
    }
  }
  
  switch(blockType)
  {
    //I block
    case 3:
      gameBoard[4][19] = blockType + 10;
      gameBoard[5][19] = blockType + 10;
      gameBoard[6][19] = blockType + 10;
      gameBoard[7][19] = blockType + 10;
      currBlockX = 4;
      currBlockY = 19;
      break;
      
    //J Block
    case 4:
      gameBoard[4][19] = blockType + 10;
      gameBoard[4][18] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      gameBoard[6][18] = blockType + 10;
      currBlockX = 4;
      currBlockY = 19;
      break;
      
    //L block
    case 5:
      gameBoard[4][18] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      gameBoard[6][18] = blockType + 10;
      gameBoard[6][19] = blockType + 10;
      currBlockX = 6;
      currBlockY = 19;
      break;
      
    //O square block
    case 6:
      gameBoard[4][19] = blockType + 10;
      gameBoard[5][19] = blockType + 10;
      gameBoard[4][18] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      currBlockX = 4;
      currBlockY = 19;
      break;
      
    //S block
    case 7:
      gameBoard[5][19] = blockType + 10;
      gameBoard[6][19] = blockType + 10;
      gameBoard[4][18] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      currBlockX = 5;
      currBlockY = 19;
      break;
      
    //T block
    case 8:
      gameBoard[5][19] = blockType + 10;
      gameBoard[4][18] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      gameBoard[6][18] = blockType + 10;
      currBlockX = 5;
      currBlockY = 19;
      break;
      
    //Z block
    case 9:
      gameBoard[4][19] = blockType + 10;
      gameBoard[5][19] = blockType + 10;
      gameBoard[5][18] = blockType + 10;
      gameBoard[6][18] = blockType + 10;
      currBlockX = 4;
      currBlockY = 19;
      break;
      
      
    default:
      return true;
      break;
  };
  
  //our blocks always start in "flat down" rotation
  currBlockRotate = flatDown;
  return checkHitVertical();
}

//This function is used to set all current moving Tetriminos to normal blocks.
void tetriminoToBlock()
{
  for(int i = 0; i < 10; i++)
  {
    for(int j = 0; j < 20; j++)
    {
      if(gameBoard[i][j] > 10)
	gameBoard[i][j] -= 10;
    }
  }
}

//A simple function for moving blocks downward.
//Scans the entire array and moves a block down if it is an active tetrimino
void moveBlocksDown()
{
  //we'll just return if we run into an impact.
  if(checkHitVertical())
    return;
    
  for(int i = 0; i < 10; i++)
  {
    for(int j = 1; j < 20; j++)
    {
      int currValue = gameBoard[i][j];
      if(currValue > 10)
      {
	gameBoard[i][j-1] = currValue;
	gameBoard[i][j] = 0;
      }
    }
  }
  currBlockY--;
}


//A simple function for trying to move all active blocks to the right
//Scans entire array and moves the blocks if it can.
bool moveBlocksRight()
{

  int xpos[4], ypos[4], value[4], xcount = 0, ycount = 0;
  
  //we check for our own blocks in the array, and then if they are fine to move, we add their position to the list
  for(int i = 0; i < 20; i++)
  {
    for(int j = 8; j >= 0; j--)
    {
      if(gameBoard[j][i] > 10)
      {
	if(gameBoard[j+1][i] == 0 || gameBoard[j+1][i] > 10)	//check to see if the next block is a wall or invalid
	{
	  xpos[xcount] = j;
	  ypos[ycount] = i;
	  value[xcount] = gameBoard[j][i];
	  xcount++;
	  ycount++;
	}
	else
	{
	  return false;			//we can't actually move right, because a block has failed. Exit.
	}
      }
    }
  }
  
  //sanity check. Perhaps we tried moving with no Tetrimino on the game board.
  if(xcount < 4 || ycount < 4)
    return false;
  
  //if we got here, we can move our block
  for(int i = 0; i < 4; i++)
  {
      gameBoard[xpos[i]+1][ypos[i]] = value[i];
      gameBoard[xpos[i]][ypos[i]] = 0;
  }
  

  currBlockX++;
  return true;
}

//A function like above, to move blocks left
bool moveBlocksLeft()
{
  int xpos[4], ypos[4], value[4], xcount = 0, ycount = 0;
  
  //we check for our own blocks in the array, and then if they are fine to move, we add their position to the list
  for(int i = 0; i < 20; i++)
  {
    for(int j = 1; j < 10; j++)
    {
      if(gameBoard[j][i] > 10)
      {
	if(gameBoard[j-1][i] == 0 || gameBoard[j-1][i] > 10)	//check to see if the next block is a wall or invalid
	{
	  xpos[xcount] = j;
	  ypos[ycount] = i;
	  value[xcount] = gameBoard[j][i];
	  xcount++;
	  ycount++;
	}
	else
	{
	  return false;			//we can't actually move right, because a block has failed. Exit.
	}
      }
    }
  }
  
  //sanity check. Perhaps we tried moving with no Tetrimino on the game board.
  if(xcount < 4 || ycount < 4)
    return false;
  
  //if we got here, we can move our block
  for(int i = 0; i < 4; i++)
  {
      gameBoard[xpos[i]-1][ypos[i]] = value[i];
      gameBoard[xpos[i]][ypos[i]] = 0;
  }
  
  currBlockX--;
  return true;
}


///BLOCK ROTATER
//This function is responsible for rotating the Active Blocks -
//the current falling tetriminos.

void rotateActiveBlocks(int blockType)
{
  switch(blockType)
  {
    //I block
    case 3:
      if(currBlockRotate == flatDown)
      {/*
	//if the one above our target is full
	if(gameBoard[currBlockX+2][currBlockY+1] != 0)			//see if we even CAN rotate.
	{
	  if(!moveBlocksLeft())
	  {
	    if(!moveBlocksRight())
	    {
	      return;
	    }
	  }
	}*/
	//reset our old blocks
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY] = 0;
	gameBoard[currBlockX+3][currBlockY] = 0;
	
	gameBoard[currBlockX+2][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY-2] = blockType + 10;
	currBlockX += 2;
	currBlockY += 1;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-1] = 0;
	gameBoard[currBlockX][currBlockY-3] = 0;
	
	gameBoard[currBlockX-2][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-2] = blockType + 10;
	currBlockX -=2;
	currBlockY -=2;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+2][currBlockY] = 0;
	gameBoard[currBlockX+3][currBlockY] = 0;
	
	gameBoard[currBlockX+1][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY+2] = blockType + 10;
	currBlockX +=1;
	currBlockY +=2;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-2] = 0;
	gameBoard[currBlockX][currBlockY-3] = 0;
	
	gameBoard[currBlockX-1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY-1] = blockType + 10;
	currBlockX -=1;
	currBlockY -=1;
      }
	
      break;
      
    //J Block
    case 4:
      
      if(currBlockRotate == flatDown)
      {
	//reset our old blocks
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-1] = 0;
	gameBoard[currBlockX+2][currBlockY-1] = 0;
	
	gameBoard[currBlockX+1][currBlockY] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-2] = blockType + 10;
	currBlockX += 1;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-2] = 0;
	
	gameBoard[currBlockX-1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-2] = blockType + 10;
	currBlockX -=1;
	currBlockY -=1;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+2][currBlockY] = 0;
	gameBoard[currBlockX+2][currBlockY-1] = 0;
	
	gameBoard[currBlockX+1][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	currBlockX +=1;
	currBlockY +=1;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-2] = 0;
	gameBoard[currBlockX-1][currBlockY-2] = 0;
	
	gameBoard[currBlockX-1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	currBlockX -=1;
      }

      break;
      
    //L block
    case 5:
      
      if(currBlockRotate == flatDown)
      {
	//reset our old blocks
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-1] = 0;
	gameBoard[currBlockX-2][currBlockY-1] = 0;
	
	gameBoard[currBlockX-1][currBlockY] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX][currBlockY-2] = blockType + 10;
	currBlockX -= 1;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-2] = 0;
	gameBoard[currBlockX+1][currBlockY-2] = 0;
	
	gameBoard[currBlockX-1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	currBlockX -=1;
	currBlockY -=1;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-1] = 0;
	gameBoard[currBlockX+2][currBlockY] = 0;
	
	gameBoard[currBlockX][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	currBlockY +=1;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY-2] = 0;
	
	gameBoard[currBlockX+2][currBlockY] = blockType + 10;
	gameBoard[currBlockX][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY-1] = blockType + 10;
	currBlockX +=2;
      }

      break;
      
    //O square block
    case 6:

      break;
      
    //S block
    case 7:
      
      if(currBlockRotate == flatDown)
      {
	//reset our old blocks
	gameBoard[currBlockX+1][currBlockY] = 0;
	gameBoard[currBlockX-1][currBlockY-1] = 0;
	
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-2] = blockType + 10;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY-2] = 0;
	
	gameBoard[currBlockX][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY-2] = blockType + 10;
	currBlockY -=1;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX+1][currBlockY] = 0;
	gameBoard[currBlockX-1][currBlockY-1] = 0;
	
	gameBoard[currBlockX-1][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX-1][currBlockY] = blockType + 10;
	currBlockX -=1;
	currBlockY +=1;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY-2] = 0;
	
	gameBoard[currBlockX+1][currBlockY] = blockType + 10;
	gameBoard[currBlockX+2][currBlockY] = blockType + 10;
	currBlockX +=1;
      }

      break;
      
    //T block
    case 8:
      
      if(currBlockRotate == flatDown)
      {
	//reset our old blocks
	gameBoard[currBlockX-1][currBlockY-1] = 0;
	
	gameBoard[currBlockX][currBlockY-2] = blockType + 10;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	
	gameBoard[currBlockX-1][currBlockY-1] = blockType + 10;
	currBlockX -=1;
	currBlockY -=1;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX+2][currBlockY] = 0;
	
	gameBoard[currBlockX+1][currBlockY+1] = blockType + 10;
	currBlockX +=1;
	currBlockY +=1;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX][currBlockY-2] = 0;
	
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
      }

      break;
      
    //Z block
    case 9:
      
      if(currBlockRotate == flatDown)
      {
	//reset our old blocks
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX+1][currBlockY] = 0;
	
	gameBoard[currBlockX+2][currBlockY] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-2] = blockType + 10;
	currBlockX += 2;
      }
      else if(currBlockRotate == flatLeft)
      {
	gameBoard[currBlockX][currBlockY] = 0;
	gameBoard[currBlockX][currBlockY-1] = 0;
	
	gameBoard[currBlockX][currBlockY-2] = blockType + 10;
	gameBoard[currBlockX-2][currBlockY-1] = blockType + 10;
	currBlockX -=2;
	currBlockY -=1;
      }
      else if(currBlockRotate == flatUp)
      {
	gameBoard[currBlockX+1][currBlockY-1] = 0;
	gameBoard[currBlockX+2][currBlockY-1] = 0;
	
	gameBoard[currBlockX+1][currBlockY+1] = blockType + 10;
	gameBoard[currBlockX][currBlockY-1] = blockType + 10;
	currBlockX +=1;
	currBlockY +=1;
      }
      else if(currBlockRotate == flatRight)
      {
	gameBoard[currBlockX-1][currBlockY-1] = 0;
	gameBoard[currBlockX-1][currBlockY-2] = 0;

	
	gameBoard[currBlockX-1][currBlockY] = blockType + 10;
	gameBoard[currBlockX+1][currBlockY-1] = blockType + 10;
	currBlockX -=1;
      }

      break;
      
      
    default:
      return;
      break;
  };
  
  
  
}


///GameBoardPainter Class
//Game Board Drawing Class
//this class is responsible for drawing our game board array every cycle.

class GameBoardPainter
{
  public:
    
    GameBoardPainter()
    {
      //try to load our images
      if 
	(
	  !iCyanImage.LoadFromFile("blocks/Icyan.tga") ||
	  !jBlueImage.LoadFromFile("blocks/Jblue.tga") ||
	  !lOrangeImage.LoadFromFile("blocks/Lorange.tga") ||
	  !oYellowImage.LoadFromFile("blocks/Oyellow.tga") ||
	  !sGreenImage.LoadFromFile("blocks/Sgreen.tga") ||
	  !tPurpleImage.LoadFromFile("blocks/Tpurple.tga") ||
	  !zRedImage.LoadFromFile("blocks/Zred.tga")
	)
      {
          cout << "Failed to load a sprite! Made sure the .tga files are in the blocks directory!" << endl;
      }
      
      //assign the images to the sprites
      
      iCyanSprite.SetImage(iCyanImage);
      jBlueSprite.SetImage(jBlueImage);
      lOrangeSprite.SetImage(lOrangeImage);
      oYellowSprite.SetImage(oYellowImage);
      sGreenSprite.SetImage(sGreenImage);
      tPurpleSprite.SetImage(tPurpleImage);
      zRedSprite.SetImage(zRedImage);
      
      iCyanSprite.SetScale(0.6,0.6);
      jBlueSprite.SetScale(0.6,0.6);
      lOrangeSprite.SetScale(0.6,0.6);
      oYellowSprite.SetScale(0.6,0.6);
      sGreenSprite.SetScale(0.6,0.6);
      tPurpleSprite.SetScale(0.6,0.6);
      zRedSprite.SetScale(0.6,0.6);
    }
    
    
    //This function checks each block in the array and sees which one needs to be drawn
    
    //A BIT OF EXPLANATION
    //We are scaling our 50x50 block sprites to 70% of their size
    //This works out to be 35 pixels, hence the multipliers being by 35.
    void draw(sf::RenderWindow &App)
    {
      for(int i = 0; i < 10; i++)
      {
	for(int j = 0; j < 20; j++)
	{
	  int blockType = gameBoard[i][j];
	  if(blockType > 10)
	    blockType -= 10;
	  
	  switch(blockType)
	  {
	    //I block
	    case 3:
	      iCyanSprite.SetPosition(i * 30 + 250, -j * 30 + 725);
	      App.Draw(iCyanSprite);
	      break;
      
	    //J Block
	    case 4:
	      jBlueSprite.SetPosition(i * 30 + 250, -j * 30 + 725);
	      App.Draw(jBlueSprite);
	      break;
      
	    //L block
	    case 5:
	      lOrangeSprite.SetPosition(i * 30 + 250, -j * 30 + 725);
	      App.Draw(lOrangeSprite);
	      break;
      
	    //O square block
	    case 6:
	      oYellowSprite.SetPosition(i * 30 + 250, -j * 30 + 725);
	      App.Draw(oYellowSprite);
	      break;
      
	    //S block
	    case 7:
	      sGreenSprite.SetPosition( i * 30 + 250, -j * 30 + 725);
	      App.Draw(sGreenSprite);
	      break;
      
	    //T block
	    case 8:
	      tPurpleSprite.SetPosition(i * 30 + 250, -j * 30 + 725);
	      App.Draw(tPurpleSprite);
	      break;
      
	    //Z block
	    case 9:
	      zRedSprite.SetPosition(i * 30 + 250,-j * 30 + 725);
	      App.Draw(zRedSprite);
	      break;

	  };
	  
	}
      }
    }
    
    
  private:
    
    sf::Image iCyanImage;
    sf::Image jBlueImage;
    sf::Image lOrangeImage;
    sf::Image oYellowImage;
    sf::Image sGreenImage;
    sf::Image tPurpleImage;
    sf::Image zRedImage;
    
    sf::Sprite iCyanSprite;
    sf::Sprite jBlueSprite;
    sf::Sprite lOrangeSprite;
    sf::Sprite oYellowSprite;
    sf::Sprite sGreenSprite;
    sf::Sprite tPurpleSprite;
    sf::Sprite zRedSprite;
    

};

///GUI CLASS
//This class is used for drawing the entire game GUI, EXCEPT for the blocks and scoring system.
//Esentially, our background tiles.
class GUI
{
  public:
    
    GUI()
    {
      backgroundTile = sf::Shape::Rectangle(0,0,798,798,sf::Color(39,46,28));
      border = sf::Shape::Rectangle(0,0,10,10,sf::Color(147,112,219));
      border2 = sf::Shape::Rectangle(0,0,7.5,7.5,sf::Color(0,0,102));
      
      gameTile = sf::Shape::Rectangle(240,150,560,765, sf::Color(50,50,50));	//180,180,620,770
    }
    
    void draw(sf::RenderWindow& App)
    {
      App.Draw(backgroundTile);
      
      
      //Draw the first border
      border.SetPosition(0,0);
      border.SetScale(120,1);
      App.Draw(border);
      border.SetPosition(0,790);
      App.Draw(border);
      
      border.SetPosition(0,0);
      border.SetScale(1,120);
      App.Draw(border);
      border.SetPosition(790,0);
      App.Draw(border);
      
      
      //draw the second border2
      border2.SetPosition(0,0);
      border2.SetScale(120,1);
      App.Draw(border2);
      border2.SetPosition(0,793);
      App.Draw(border2);
      
      border2.SetPosition(0,0);
      border2.SetScale(1,120);
      App.Draw(border2);
      border2.SetPosition(793,0);
      App.Draw(border2);
      
      
      //draw's the central game's background
      App.Draw(gameTile);
      
      
      
      //draw the central game's background border
      
      
      
    }

    
  private:
    
    sf::Shape backgroundTile;
    sf::Shape border;
    sf::Shape border2;
    
    sf::Shape gameTile;
    sf::Shape gameBorder;
    
    
    

};

int main(int argc, char** argv)
{
    int x = 800, y = 800;

    sf::RenderWindow App(sf::VideoMode(x, y, 32), "SFML Simple Tetris");
    App.SetFramerateLimit(90);
    App.Create(sf::VideoMode(x, y, 32), "SFML Simple Tetris", sf::Style::Resize|sf::Style::Close);


    //we create an event for...handling events
    sf::Event Event;

    //we grab our window's default input events
    const sf::Input& Input = App.GetInput();
    
    sf::Font MyFont;

    // Load from a font file on disk
    if (!MyFont.LoadFromFile("FIPPS.TTF"))
    {
      cout << "Something's gone wrong kitty! The default font could not be found." << endl;
      MyFont = sf::Font::GetDefaultFont();
    }


    //set up our text strings and such

    sf::String title("Simple Tetris", MyFont, 30);
    title.Move(250,50);
    title.SetColor(sf::Color::White);
    
    sf::String pauseText("  Game\nPaused!", MyFont, 30);
    pauseText.Move(20,400);
    pauseText.SetColor(sf::Color::Blue);

    sf::String scoreT("Score     High Score", MyFont, 20);
    scoreT.Move(220,90);
    scoreT.SetColor(sf::Color::White);

    sf::String currentScoreText("0", sf::Font::GetDefaultFont(), 15);
    currentScoreText.Move(220,120);
    currentScoreText.SetColor(sf::Color::White);

    sf::String highScoreText("0", sf::Font::GetDefaultFont(), 15);
    highScoreText.Move(365,120);
    highScoreText.SetColor(sf::Color::White);
    
    
    
    
    GUI * background = new GUI;
    GameBoardPainter * TetrisBoard = new GameBoardPainter();
    
  //  playerSnake * Snake = new playerSnake;
  //  powerPellet * pellet = new powerPellet;
    
    direction dir;
    sf::Clock clock;
    sf::Clock moveClock;
    sf::Clock pauseClock;
    float time;
    float moveTime;
    float pauseTime;
    
    int newBlock = 17; //= sf::Randomizer::Random(3, 9) + 10;
    
    //initialize our array.
    //...we could just have an initializer list, but 2D init lists are noooooooot pretty.
    initBoard();
    createTetrimino(newBlock - 10);
   
    //this is used to make sure the user has to press for each movement.
    bool buttonHeld = false;
    
    while (Running)
    {
      
	//used for synchronization and input checks.
	time = clock.GetElapsedTime();
	moveTime = moveClock.GetElapsedTime();
	
	pauseTime = pauseClock.GetElapsedTime();
	
        App.GetEvent(Event);
	string score;
        stringstream out;
        if (Event.Type == sf::Event::Closed)
            Running = false;

        App.Clear(sf::Color(0,0,0));


	///INPUT
	//This is where our input is registered and stored in an enum value.
	if(Input.IsKeyDown(sf::Key::Down))
	  dir = DIR_DOWN;
	else if(Input.IsKeyDown(sf::Key::Up))
	  dir = DIR_UP;
	else if(Input.IsKeyDown(sf::Key::Left))
	  dir = DIR_LEFT;
	else if(Input.IsKeyDown(sf::Key::Right))
	  dir = DIR_RIGHT;
	else if(Input.IsKeyDown(sf::Key::Space))
	  dir = DIR_ROTATE;
	else if(Input.IsKeyDown(sf::Key::Return))
	{
	  if(gamePaused && pauseTime > 1.0)
	  {
	    gamePaused = false;
	    pauseClock.Reset();
	  }
	  else if(!gamePaused && pauseTime > 1.0)
	  {
	    gamePaused = true;
	    pauseClock.Reset();
	  }
	}
	else
	{
	  buttonHeld = false;
	  dir = DIR_NONE;
	}
	
if(!gamePaused){
  
  
	///MOVEMENT AND LOGIC
	//Where we use our inputs and do our game logic.
	
	if(dir == DIR_UP)
	{
	  while(!checkHitVertical())
	  {
	    moveBlocksDown();
	  }
	}
	
	//check right away if we've lost.
	if(playerKilled == true)
	{
	  playerKilled = false;
						//  Snake->killed();
						//// delete Snake;
				      //	  playerSnake *Snake = new playerSnake;
	  currentScore = 0;
	  initBoard();
	  newBlock = sf::Randomizer::Random(3, 9) + 10;
	  createTetrimino(newBlock - 10);
	  clock.Reset();
	  moveClock.Reset();
	}
	
	//Move if the player has a key held down.
	if(moveTime >= movementSpeed)
	{
	  //no holding the button down!
	  if(!buttonHeld)
	  {
	    //gameBoard[currBlockX][currBlockY] = 0;
	    switch(dir)
	    {
	      case DIR_LEFT:			//needs to check related blocks for wall hits
		//if(currBlockX > 0)
		  //currBlockX -= 1;
		  moveBlocksLeft();
		buttonHeld = true;
		break;
	   
	      case DIR_RIGHT:
		//if(currBlockX < 9)		//needs to check related blocks for wall hits
		  //currBlockX += 1;
		moveBlocksRight();
		buttonHeld = true;
		break;
	    
	      case DIR_DOWN:			//needs to check related blocks for floor hits
		
		/*//We make sure that we can't override the blocks below!
		if(currBlockY >= 1 && gameBoard[currBlockX][currBlockY-1] == 0)
		if(!checkHitVertical() && currBlockY >= 1)
		  currBlockY -= 1;
		if(checkHitVertical())
		  currBlockY += 1;*/
//		currBlockY -= 1;
		moveBlocksDown();
		buttonHeld = true;
		break;
	    
	      case DIR_NONE:
		break;
		
	      case DIR_ROTATE:
		rotateActiveBlocks(newBlock - 10);
		if(currBlockRotate == flatDown)
		{
		  cout << "\nDOWN";
		  currBlockRotate = flatLeft;
		}
		else if(currBlockRotate == flatLeft)
		{
		  cout << "\nLEFT";
		  currBlockRotate = flatUp;
		}
		else if(currBlockRotate == flatUp)
		{
		  cout << "\nUP";
		  currBlockRotate = flatRight;
		}
		else if(currBlockRotate == flatRight)
		{
		  cout << "\nRIGHT";
		  currBlockRotate = flatDown;
		}
		
		buttonHeld = true;
	
		break;
	     
	      default:
		break;
	    };
	    if(dir != DIR_NONE)
	      printBoardFromTop();
	    
	   // gameBoard[currBlockX][currBlockY] = newBlock;
	  }
	  
	  moveClock.Reset();
	}
	
	
	//Force a movement downward once per 'tick', check collisions, generate new blocks
	if(time >= timeSpeed)
	{
	  if(checkHitVertical())
	  {
	    
	    //\	Random number for a random block.
	    newBlock = sf::Randomizer::Random(3, 9) + 10;
	    currBlockRotate = flatDown;
	    
	    //we set the current block status to a normal, nonmoving block
	    tetriminoToBlock();
	  //  gameBoard[currBlockX][currBlockY] -= 10;			//change to function that accounts for all parts of the tetrimino
	    
	    //we check for full lines, then delete them, then add to the score.
	    int lines = checkForFullLines();
	    
	    //tetris scoring system
	    switch(lines)
	    {
	      case 1:
		currentScore += 40;
		break;
	      case 2:
		currentScore += 100;
		break;
	      case 3:
		currentScore += 300;
		break;
	      case 4:
		currentScore += 1200;
		break;
	      default:
		break;
		
	    }
	    
	    currentScore += 10;
	    
	    ///END GAME
	    //if we crete a new tetrimino and there's a collision, that means we've lost the game
	    if(createTetrimino(newBlock - 10))
	      playerKilled = true;
	    
	    cout << "\nNEW BLOCK CREATED." << endl;
	  }
	  else
	  {
	    //else we move the block(s) down
	    moveBlocksDown();
	    //gameBoard[currBlockX][currBlockY] = 0;			//needs to be functions that change previous to 0 and next to blocks
	    //currBlockY -= 1;
	    //gameBoard[currBlockX][currBlockY] = newBlock;
	  }
	  
	//  if(Input.IsKeyDown(sf::Key::Up))
	  //{
	    printBoardFromTop();
	  //}
	  
	  //moveTestBlock();
	  
	  clock.Reset();
	}
	
	//cout << "CubeX is " << Snake->getSnakeHead().getXPos() << " and CubeY is " << Snake->getSnakeHead().getYPos() << endl;
	//cout << "PelletX is " << pellet->getXPos() << " and pelletY is " << pellet->getYPos() << endl;
	//if we eat a pellet
	
/*	if(collisionTest(*Snake, *pellet))
	{
	  pellet->reset();
	  currentScore += 100;
	  
	  Snake->eat();
	  out << currentScore;
	  score = out.str();
	  score1.SetText(score);
	  timeSpeed -= 0.0025;
	}
*/	
	//Our drawing phase
	
//	Snake->draw(App);
//	pellet->draw(App);

}
	///DRAWING AND SOUND
	//Here is where all of our sounds and sprites are drawn
	
	background->draw(App);
	TetrisBoard->draw(App);

	/*sf::Image testImage;
	testImage.LoadFromFile("blocks/Icyan.tga");
	sf::Sprite alpha; alpha.SetImage(testImage);*/
	
	//App.Draw(alpha);
	
	if(gamePaused)
	  App.Draw(pauseText);
	
        App.Draw(title);
	App.Draw(scoreT);
	App.Draw(currentScoreText);
	App.Draw(highScoreText);

	
        App.Display();
	
	out << currentScore;
	score = out.str();
	currentScoreText.SetText(score);
	out.str("");
	
	
	if(highScore <= currentScore)
	{
	  highScore = currentScore;
	  out << highScore;
	  score = out.str();
	  highScoreText.SetText(score);
	}
	/*
	std::cerr << "loop # " << loopnum << endl;
	loopnum++;
	
*/
    }

  //  delete Snake;

    return EXIT_SUCCESS;
}
