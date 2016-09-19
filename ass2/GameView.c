// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"

//used in action array in player struct to indicate meaning of the index
#define DRAC_ACTION_NEWS 2
#define MAX_NUM_ACTIONS 4

typedef struct player {
    // they all ascii
    unsigned char name;
    unsigned char moveHi;
    unsigned char moveLo;
    //encounter for drac placed trap
    //encounter for drac place immvam, could be trap off date or vampire matrued for drac
    unsigned char actions[4];
    unsigned char terminator;
}* Player;

typedef struct role{
    int health;
    int locationID; // where is this player
    int trailLen; //how many trail actually exit for this player
    Player trail; // store role's trail
}* Role;

struct gameView {
    Round round; //the round of current state
    int turn; //current turn
    int score;// overall score of the game
    struct player last;
    Role roles;
    Map map;
};

static void storePlayer (Player storeTo, Player infoFrom);

//finding the trail of the player at certain point, by go through the pastPlays back wards from start
static void findTrail (GameView current, char* pastPlays, Player start);
// used to calculate the life point of specific hunter
static void calHunterHealth(GameView current, char* pastPlays, int hunterID);
// used to calculate the blood of Dracula
static void calDraculaBloodAndScore(GameView current, char* pastPlays);



// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //initialization:
    GameView g = malloc(sizeof(struct gameView));

    //get turn number of the current game
    unsigned int lenOfPastPlays = (unsigned int)strlen(pastPlays) + 1;//actual length of the string include terminator NULL
    g->turn = lenOfPastPlays/8; // must be divisible by 8
    g->round = g->turn/NUM_PLAYERS; //current round

    g->score = GAME_START_SCORE;

    //make drac and hunter info
    g->roles = calloc(NUM_PLAYERS, sizeof(struct role));
    int i;
    for (i = 0; i < NUM_PLAYERS; i++) {
        g->roles[i].trail = calloc(7, sizeof(struct player)); //allocate memory for trail
    }

    //assign health points to all players
    for(i = 0; i < PLAYER_DRACULA; i++) {
        g->roles[i].health = GAME_START_HUNTER_LIFE_POINTS;
        g->roles[i].locationID = UNKNOWN_LOCATION; // at start no one know their location
    }
    g->roles[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;
    g->roles[PLAYER_DRACULA].locationID = UNKNOWN_LOCATION;

    g->map = newMap();

    //pastPlays[lenOfPastPlays] this should be the /0 for pastPlays so it minus 1 will give the info of last player
    Player temp = (Player)&pastPlays[lenOfPastPlays] - 1;
    //find the end of pastPlay
    storePlayer(&(g->last), temp); //make last become true last player

    //figure out the current score, health
    //update hunter's info
    for(i = 0; i < PLAYER_DRACULA; i++) calHunterHealth(g, pastPlays, i);
    calDraculaBloodAndScore(g,pastPlays);
    return g;
}
     
     
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    int i;
    for (i = 0; i < NUM_PLAYERS; i++)
        free(toBeDeleted->roles[i].trail);
    free(toBeDeleted->roles);
    free(toBeDeleted);
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
    //current player is the player next to the last player in the pastPlay
    switch (currentView->last.name) {
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
    return currentView->roles[player].health;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    return currentView->roles[player].locationID;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    int i;
    int actualSize = currentView->roles[player].trailLen;
    char place[3]; place[2] = '\0';
    for(i = 0; i <TRAIL_SIZE; i++){
        if(actualSize > 0){
            place[0] = currentView->roles[player].trail[i].moveHi;
            place[1] = currentView->roles[player].trail[i].moveLo;
            trail[i] = abbrevToID(place);
            if(place[0] == 'C' && place[1] == '?') trail[i] = CITY_UNKNOWN;
            if(place[0] == 'S' && place[1] == '?') trail[i] = SEA_UNKNOWN;
            if(place[0] == 'T' && place[1] == 'P') trail[i] = TELEPORT;
            if(place[0] == 'H' && place[1] == 'I') trail[i] = HIDE;
            if(place[0] == 'D'){
                switch (place[1]){
                    case '1': trail[i] = DOUBLE_BACK_1; break;
                    case '2': trail[i] = DOUBLE_BACK_2; break;
                    case '3': trail[i] = DOUBLE_BACK_3; break;
                    case '4': trail[i] = DOUBLE_BACK_4; break;
                    case '5': trail[i] = DOUBLE_BACK_5; break;
                    default:;
                }
            }
            actualSize--;
        } else {
            trail[i] = UNKNOWN_LOCATION;
        }
    }
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
//qwer - find fn
	// See if we're actually looking for something - sanity check
	if( (road == 0) && (rail == 0) && ( sea == 0 ) ) return NULL;

	// intialise variables
		// Array to return
	LocationID * connected[NUM_MAP_LOCATIONS];
		// See what to search
	int searchRoad, searchRail, searchSea;
	searchRoad = searchRail = searchSea = 0;

	// Determine what we're looking for -- probably redundant - can use road/rail/sea itself
	if( road ) searchRoad = 1;
	if( rail ) searchRail = 1;
	if( sea ) searchSea = 1;


	
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return *connected;
}

static void storePlayer (Player storeTo, Player infoFrom) {
    assert(storeTo != NULL);
    assert(infoFrom != NULL);
    storeTo->name = infoFrom->name;
    storeTo->moveHi = infoFrom->moveHi;
    storeTo->moveLo = infoFrom->moveLo;
    storeTo->terminator = infoFrom->terminator;
    int i;
    for(i = 0; i < MAX_NUM_ACTIONS; i++) storeTo->actions[i] = infoFrom->actions[i];
}

static void findTrail (GameView current, char* pastPlays, Player start){
    //get the trail of all roles these name variables represent the actual trail index of each player
    int id;
    int size = 0; //the size of trail
    switch(start->name){
        case 'G': id = PLAYER_LORD_GODALMING; break;
        case 'S': id = PLAYER_DR_SEWARD; break;
        case 'H': id = PLAYER_VAN_HELSING; break;
        case 'M': id = PLAYER_MINA_HARKER; break;
        case 'D': id = PLAYER_DRACULA; break;
        default: assert(FALSE);
    }
    for(; start >= (Player)pastPlays; start -= 5){
        if(current->roles[id].trailLen == 7) break;
        storePlayer(&(current->roles[id].trail[size++]), start);
        current->roles[id].trailLen = size;
    }
}

/* need to do
 * score start with 366
 * score decrease by 6 each time hunter teleported to hospital                                                                                      yes
 * score decrease by 1 when it is dracula's turn
 * score decrease by 13 when vam matures
 *
 * dracula encounter hunter loss 10 blood                                                                                                           yes
 * dracula loss 2 blodd when he is at sea at end of his turn                                                                                        yes
 * dracula regen 10 blood if he is in castle dracula at end of his turn and has not been defeated even if he is automagically tele to there         yes
 * he can have more than 40 blood                                                                                                                   yes
 *
 * hunter has 9 life points max                                                                                                                     yes
 * hunter will encounter trap first then immature vampire and lastly face dracula                                                                   yes
 * a trap cause 2 life points, if there are more than one encounter hunter will face them one by one at time                                        yes
 * encounter dracula cost 4 life points                                                                                                             yes
 * regen 3 life points each time if they rest, (end their turn in the same city they ended their previous turn)                                     yes
 * when hunter telport to hospital they have zero life points (but this teleportation is not in history we need to deduce)                          yes
 *
 */
static void calHunterHealth(GameView current, char* pastPlays, int hunterID){
    unsigned int playLen = (unsigned int)strlen(pastPlays);
    if(playLen == 0) return;
    playLen++;
    char nameAbbre;
    Player temp;
    int dieFlag = FALSE;
    char place[3];
    int i;
    switch (hunterID){
        case PLAYER_LORD_GODALMING: nameAbbre = 'G'; break;
        case PLAYER_DR_SEWARD: nameAbbre = 'S'; break;
        case PLAYER_VAN_HELSING: nameAbbre = 'H'; break;
        case PLAYER_MINA_HARKER: nameAbbre = 'M'; break;
        case PLAYER_DRACULA: nameAbbre = 'D'; break;
        default: assert(FALSE);
    }
    //find first occurence of the hunter
    for(temp = (Player)pastPlays; temp->terminator != '\0'; temp++)
        if(temp->name == nameAbbre) break;
    if(temp->name != nameAbbre && temp->terminator == '\0')return;//as the hunter hasn't have his turn yet

    Player end = (Player)(&pastPlays[playLen]);
    //so temp->name is the name we want
    for(; temp < end; temp += 5){
        assert(temp->name == nameAbbre);
        if (dieFlag == TRUE) {
            current->roles[hunterID].health = GAME_START_HUNTER_LIFE_POINTS;
            dieFlag = FALSE;
        }
        findTrail(current,pastPlays,temp);//assign the trail of this hunter
        assert(current->roles[hunterID].trail[0].moveHi == temp->moveHi && current->roles[hunterID].trail[0].moveLo == temp->moveLo);
        //get the location of the player in this turn
        place[0] = temp->moveHi; place[1] = temp->moveLo; place[2] = '\0';
        current->roles[hunterID].locationID = abbrevToID(place);

        //regen 3
        if(current->roles[hunterID].trail[0].moveHi == current->roles[hunterID].trail[1].moveHi
           && current->roles[hunterID].trail[0].moveLo == current->roles[hunterID].trail[1].moveLo){
            current->roles[hunterID].health += LIFE_GAIN_REST;
            if(current->roles[hunterID].health > GAME_START_HUNTER_LIFE_POINTS)
                current->roles[hunterID].health = GAME_START_HUNTER_LIFE_POINTS;
        }

        //go through the action array to find out what hunter encounter
        for(i = 0; i < MAX_NUM_ACTIONS; i++){
            //for each different action do different things
            switch (temp->actions[i]){
                case 'T':
                    current->roles[hunterID].health -= LIFE_LOSS_TRAP_ENCOUNTER;
                    break;
                case 'V': // I think we don't need to deal with this
                    break;
                case 'D': // encounter Dracula
                    current->roles[hunterID].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    break;
                case '.': // we don't need to do anything with this
                    break;
                default:
                    assert(FALSE);
            }
            //check if he has zero health
            // so this guy should be at hospital but in the pastplay string,
            // it show he is in the place he was killed rather than hospital
            if(current->roles[hunterID].health <= 0) {
                current->roles[hunterID].health = 0;
                current->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                dieFlag = TRUE;
                assert(current->roles[hunterID].locationID != ST_JOSEPH_AND_ST_MARYS);
            }
        }
    }
}
static void calDraculaBloodAndScore(GameView current, char* pastPlays){
    //doesn't calculate hospital score loss
    unsigned int playLen = (unsigned int)strlen(pastPlays);
    if(playLen == 0) return;
    playLen++;
    Player end = (Player)(&pastPlays[playLen]);
    char actualHi = 0;
    char actualLo = 0;
    int actualId;
    char place[3];
    place[2] = '\0';
    Player temp;
    for(temp = (Player)pastPlays; temp < end; temp++){
        if(temp->name == 'D'){
            current->score -= SCORE_LOSS_DRACULA_TURN;
            findTrail(current,pastPlays,temp);//find the trail of dracula at this point
            place[0] = temp->moveHi; place[1] = temp->moveLo;
            actualId = abbrevToID(place);
            current->roles[PLAYER_DRACULA].locationID = actualId;

            //assign the current location ID of dracula
            if(temp->moveLo == '?'){
                if(temp->moveHi == 'C'){
                    current->roles[PLAYER_DRACULA].locationID = CITY_UNKNOWN;
                } else if(temp->moveHi == 'S') {
                    current->roles[PLAYER_DRACULA].locationID = SEA_UNKNOWN;
                }
            }
            if(temp->moveHi == 'H' && temp->moveLo == 'I') current->roles[PLAYER_DRACULA].locationID = HIDE;
            if(temp->moveHi == 'T' && temp->moveLo == 'P') current->roles[PLAYER_DRACULA].locationID = TELEPORT;
            if(temp->moveHi == 'D'){
                switch (temp->moveLo){
                    case '1': current->roles[PLAYER_DRACULA].locationID = DOUBLE_BACK_1; break;
                    case '2': current->roles[PLAYER_DRACULA].locationID = DOUBLE_BACK_2; break;
                    case '3': current->roles[PLAYER_DRACULA].locationID = DOUBLE_BACK_3; break;
                    case '4': current->roles[PLAYER_DRACULA].locationID = DOUBLE_BACK_4; break;
                    case '5': current->roles[PLAYER_DRACULA].locationID = DOUBLE_BACK_5; break;
                    default:;
                }
            }

            //find the actual type of land of current location
            actualHi = temp->moveHi;
            actualLo = temp->moveLo;
            if(current->roles[PLAYER_DRACULA].locationID == HIDE){
                actualHi = current->roles[PLAYER_DRACULA].trail[1].moveHi;
                actualLo = current->roles[PLAYER_DRACULA].trail[1].moveLo;
                if(actualHi == 'D') {
                    switch (actualLo) {
                        case '1':
                            actualHi = current->roles[PLAYER_DRACULA].trail[2].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[2].moveLo;
                            break;
                        case '2':
                            actualHi = current->roles[PLAYER_DRACULA].trail[3].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[3].moveLo;
                            break;
                        case '3':
                            actualHi = current->roles[PLAYER_DRACULA].trail[4].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[4].moveLo;
                            break;
                        case '4':
                            actualHi = current->roles[PLAYER_DRACULA].trail[5].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[5].moveLo;
                            break;
                        case '5':
                            actualHi = current->roles[PLAYER_DRACULA].trail[6].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[6].moveLo;
                            break;
                        default:;
                    }
                }
            } else if(actualHi == 'D'){
                switch (actualLo) {
                    case '1':
                        actualHi = current->roles[PLAYER_DRACULA].trail[1].moveHi;
                        actualLo = current->roles[PLAYER_DRACULA].trail[1].moveLo;
                        if(actualHi == 'H' && actualLo == 'I'){
                            actualHi = current->roles[PLAYER_DRACULA].trail[2].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[2].moveLo;
                        }
                        break;
                    case '2':
                        actualHi = current->roles[PLAYER_DRACULA].trail[2].moveHi;
                        actualLo = current->roles[PLAYER_DRACULA].trail[2].moveLo;
                        if(actualHi == 'H' && actualLo == 'I'){
                            actualHi = current->roles[PLAYER_DRACULA].trail[3].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[3].moveLo;
                        }
                        break;
                    case '3':
                        actualHi = current->roles[PLAYER_DRACULA].trail[3].moveHi;
                        actualLo = current->roles[PLAYER_DRACULA].trail[3].moveLo;
                        if(actualHi == 'H' && actualLo == 'I'){
                            actualHi = current->roles[PLAYER_DRACULA].trail[4].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[4].moveLo;
                        }
                        break;
                    case '4':
                        actualHi = current->roles[PLAYER_DRACULA].trail[4].moveHi;
                        actualLo = current->roles[PLAYER_DRACULA].trail[4].moveLo;
                        if(actualHi == 'H' && actualLo == 'I'){
                            actualHi = current->roles[PLAYER_DRACULA].trail[5].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[5].moveLo;
                        }
                        break;
                    case '5':
                        actualHi = current->roles[PLAYER_DRACULA].trail[5].moveHi;
                        actualLo = current->roles[PLAYER_DRACULA].trail[5].moveLo;
                        if(actualHi == 'H' && actualLo == 'I'){
                            actualHi = current->roles[PLAYER_DRACULA].trail[6].moveHi;
                            actualLo = current->roles[PLAYER_DRACULA].trail[6].moveLo;
                        }
                        break;
                    default:;
                }
            }
            place[0] = actualHi; place[1] = actualLo;
            actualId = abbrevToID(place);

            if(actualLo == '?'){
                if(actualHi == 'C'){
                    actualId = CITY_UNKNOWN;
                } else if(actualHi == 'S') {
                    actualId = SEA_UNKNOWN;
                }
            }
            if(actualHi == 'H' && actualLo == 'I') actualId = HIDE;
            if(actualHi == 'T' && actualLo == 'P') actualId = TELEPORT;




            if((actualId == CASTLE_DRACULA || actualId == TELEPORT) && current->roles[PLAYER_DRACULA].health > 0)
                current->roles[PLAYER_DRACULA].health += LIFE_GAIN_CASTLE_DRACULA;

            if((actualId == SEA_UNKNOWN) ||
                    (actualId <= MAX_MAP_LOCATION && actualId >= MIN_MAP_LOCATION && idToType(actualId) == SEA))
                current->roles[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;
            if(temp->actions[DRAC_ACTION_NEWS] == 'V')
                current->score -= SCORE_LOSS_VAMPIRE_MATURES;
            if(current->roles[PLAYER_DRACULA].health < 0)
                current->roles[PLAYER_DRACULA].health = 0;
        } else {
            int i;
            for(i = 0; i < MAX_NUM_ACTIONS; i++){
                if(temp->actions[i] == 'D') current->roles[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
            }
        }
    }
}
