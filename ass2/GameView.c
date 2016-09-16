// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"

// #include "Map.h" ... if you decide to use the Map ADT

typedef struct player {
    unsigned char name;
    unsigned char moveHi;
    unsigned char moveLo;
    unsigned char trap; //encounter for drac placed trap
    unsigned char immVam; //encounter for drac place immvam, could be trap off date or vampire matrued for drac
    unsigned char face_drac_or_drac_news;
    unsigned char addition;
}* Player;

typedef struct hunter{
    int health;
    int locationID;

}* Hunter;

typedef struct dracular {
    int health;
    int locationID;
}* Dracular;

struct gameView {
    Round round; //the round of current state
    int turn; //current turn
    int score;// overall score of the game
    Player last;
    Dracular dra;
    Hunter hunter[4];
    Map map;
};
     

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    GameView g = malloc(sizeof(struct gameView));
    g->last = (Player)pastPlays;
    g->dra = calloc(1,sizeof(struct dracular));
    g->map = newMap();
    int i;
    for (i = 0; i < 4; i++) {
        g->hunter[i] = calloc(1, sizeof(struct hunter));
    }
    for (i = 0; g->last->name != 0; g->last++) i++;
    g->last--;
    g->turn = i;
    g->round = g->turn/5;

    return g;
}
     
     
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    return currentView->round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    switch (currentView->last->name) {
        case 'G': return PLAYER_DR_SEWARD;
        case 'S': return PLAYER_VAN_HELSING;
        case 'H': return PLAYER_MINA_HARKER;
        case 'M': return PLAYER_DRACULA;
        default: return PLAYER_LORD_GODALMING;
    }
}

// Get the current score
int getScore(GameView currentView)
{
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    switch (player) {
        case PLAYER_DRACULA: return currentView->dra->health;
        default: return currentView->hunter[player]->health;
    }
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    switch (player) {
        case PLAYER_DRACULA: return currentView->dra->locationID;
        default: return currentView->hunter[player]->locationID;
    }
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return NULL;
}
