#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "Game.h"


#define STUDENT_THD 0
#define STUDENT_BPS 1
#define STUDENT_BQN 2
#define STUDENT_MJ  3
#define STUDENT_MTV 4
#define STUDENT_MMONEY 5

#define NUM_ARCS 72
#define NUM_VERTICES 54

#define MAX_PATHS 150
#define MAX_CAMPUSES 75

#define CAMPUS_MULTIPLIER 10
#define G08_MULTIPLIER 20
#define ARC_MULTIPLIER 2
#define PATENT_MULTIPLIER 10
#define MOST_PUBS_BONUS 10
#define MOST_ARCS_BONUS 10

#define INITIAL_CAMPUS_A1 0
#define INITIAL_CAMPUS_B1 12
#define INITIAL_CAMPUS_C1 11
#define INITIAL_CAMPUS_A2 53
#define INITIAL_CAMPUS_B2 41
#define INITIAL_CAMPUS_C2 42

#define UP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3

#define UP_POSITION 3
#define DOWN_POSITION 0 
#define RIGHT_POSITION 7
#define LEFT_POSITION 1

typedef struct _region{
    int x;
    int y;
}region;

typedef struct _resources{
    int THD, BPS, BQN, MJ, MTV, MMONEY;
    int patents;
    int pubs;
    int arcGrants;
    int campuses;
    int GO8s;
}resources;

typedef struct _arcVertex {
	int A;
	int B;
	int C;
}arcVertex;

typedef struct _game{

    //For each player store certain resources
    resources *players;

    int turnNumber;

    int *disciplines;
    int *dice;

    //Store all the ARC and vertex positions of the game
    arcVertex *arcs;
    int *vertices;

}game;



//static int numberOfNewStudents(Game g, int regionID, int player);
int getPatents(Game g, int player);
region decodePath(path p);
region updateRegion(int a, region p);
int getArrayIndex(region r);
int whereMove(int a, path p, char previousMove);

//Edited by Paul 9/5
Game newGame (int degree[], int diceSeed[]) {

    //Allocate memory for the game
    Game g = malloc (sizeof (game));  
    assert (g != NULL);                

    //Begin Terra Nullius
    g->turnNumber = -1;              

    //Allocate memory for discipline and dice array
    g->disciplines = malloc(NUM_REGIONS*sizeof (int));
    g->dice = malloc (NUM_REGIONS*sizeof (int));

    int i = 0;
    while (i < NUM_REGIONS) {
        g->disciplines[i] = degree[i];
        g->dice[i] = diceSeed[i];
        i++;
    }
    //allocate memory for arc and vertices arrays
    g->arcs = malloc (NUM_ARCS*sizeof (arcVertex));         
    g->vertices = malloc (NUM_VERTICES*sizeof (int));

    //Set both arrays to zero, to make all arcs and vertices vacant    
    i = 0;
    while( i < NUM_VERTICES){
        g->arcs[i].A = 0;
        g->arcs[i].B = 0;
        g->arcs[i].C = 0;
        i++;
    }
    i = 0;
    while( i < NUM_VERTICES){
        g->vertices[i] = 0;
        i++;
    }


    g->vertices[INITIAL_CAMPUS_A1] = CAMPUS_A;
    g->vertices[INITIAL_CAMPUS_B1] = CAMPUS_B;
    g->vertices[INITIAL_CAMPUS_C1] = CAMPUS_C;
    g->vertices[INITIAL_CAMPUS_A2] = CAMPUS_A;
    g->vertices[INITIAL_CAMPUS_B2] = CAMPUS_B;
    g->vertices[INITIAL_CAMPUS_C2] = CAMPUS_C;

    //Allocate memory for the array which stores the players resources
    g->players = malloc ((1+NUM_UNIS)*sizeof(resources));  


    i = UNI_A;
    //Initialize the resources for every player
    while (i <= NUM_UNIS) {
        g->players[i].THD = 0; 
        g->players[i].BPS = 3;
        g->players[i].BQN = 3;
        g->players[i].MJ = 1;
        g->players[i].MTV = 1;
        g->players[i].MMONEY = 1;

        g->players[i].patents = 0;
        g->players[i].pubs = 0;

        g->players[i].arcGrants = 0;
        g->players[i].campuses = 2;
        g->players[i].GO8s = 0;

        i++;
    }

    return g;
}


// free all the memory we allocated for the game
// edited by paul 10/5
void disposeGame (Game g){
    assert( g != NULL);
    free( g->arcs);
    free( g->vertices);
    free( g->players);
    free( g->dice);
    free( g);
    g = NULL;
}


/*=================================================================*/
// make the specified action for the current player and update the 
// game state accordingly.  
// The function may assume that the action requested is legal.
// START_SPINOFF is not a legal action here
void makeAction (Game g, action a){

}
/*=================================================================*/


// edited by paul 9/5
void throwDice (Game g, int diceScore){
    int regionNum, uniNum, discipline, amount;
    amount = 0;
    regionNum = 0;
    //Check all regions
    while( regionNum < NUM_REGIONS) {

        //if that particular region is equal to the dice roll
        //enter a while loop and see which players get which student
        if( g->dice[regionNum] == diceScore){

            uniNum = UNI_A;
            while( uniNum <= NUM_UNIS){
                discipline = getDiscipline(g, regionNum);
                //check how many of that discipline the player gets
                //amount = numberOfNewStudents(g, regionNum, uniNum);

                if(discipline == STUDENT_THD){
                    g->players[uniNum].THD += amount;

                }else if( discipline == STUDENT_BPS){
                    g->players[uniNum].BPS += amount;

                }else if( discipline == STUDENT_BQN){
                    g->players[uniNum].BQN += amount;

                }else if( discipline == STUDENT_MJ){
                    g->players[uniNum].MJ += amount;

                }else if( discipline == STUDENT_MTV){
                    g->players[uniNum].MTV += amount;

                }else if( discipline == STUDENT_MMONEY){
                    g->players[uniNum].MMONEY += amount;
                }

                uniNum++;
            }
        }
        regionNum++;
    }
    //if a 7 was rolled change all MTVs and M$ to THD
    // for every player
    if( diceScore == 7) {

        uniNum = UNI_A;
        while(uniNum <= NUM_UNIS) {
            g->players[uniNum].THD += g->players[uniNum].MTV;
            g->players[uniNum].THD += g->players[uniNum].MMONEY;

            g->players[uniNum].MTV = 0;
            g->players[uniNum].MMONEY = 0;

            uniNum++;
        }
    }

    //increment turn number
    g->turnNumber++;
}

/*===============================================================*/
//Return the number of students a player should get
//depending on how many campuses/GO8s they have around
//a certain regionID used for function throwDice
int numberOfNewStudents(Game g, int regionID, int player){

    return 0;
}
/*===============================================================*/



/* **** Functions which GET data about the game aka GETTERS **** */

// what type of students are produced by the specified region?
// regionID is the index of the region in the newGame arrays (above) 
// see discipline codes above
// edited by paul 10/5
int getDiscipline (Game g, int regionID){
    int result;
    //check if region is valid, else set to -1
    if(TRUE){
        result = g-> disciplines[regionID];
    } else {
            result = 0;
    }
    return result;
}


// what dice value produces students in the specified region?
// 2..12
// edited by paul 10/5
int getDiceValue (Game g, int regionID){
    int result;

    //check if region is valid, else set to -1
    if(TRUE){
        result = g->dice[regionID];
    } else {
        result = -1;
    }

    return result;
}
// which university currently has the prestige award for the most ARCs?
// this is NO_ONE until the first arc is purchased after the game 
// has started.
// edited by paul 9/5
int getMostARCs (Game g) {
    int max, uniNum, mostARCs;

    uniNum = UNI_A;
    max = 0;
    mostARCs = NO_ONE;
    while( uniNum <= NUM_UNIS) {

        if( max < g->players[uniNum].arcGrants ){
            max = g->players[uniNum].arcGrants;
            mostARCs = uniNum;
        }
        uniNum++;

    }

    return mostARCs;
}


// which university currently has the prestige award for the most pubs?
// this is NO_ONE until the first publication is made.
// edited by paul 10/5
int getMostPublications (Game g){
    int uniNum, max, mostPubs;

    max = 0;
    mostPubs = NO_ONE;
    uniNum = UNI_A;
    while( uniNum <= NUM_UNIS) {

        if( max < g->players[uniNum].pubs ){
            max = g->players[uniNum].pubs;
            mostPubs = uniNum;
        }
        uniNum++;
    }
    return mostPubs;
}


//return the current turn number of the game
// used for testing
// edited by paul 10/5 
int getTurnNumber( Game g){
    int turn;
    turn = g->turnNumber;
    return turn;
}


// return the player id of the player whose turn it is 
// the result of this function is NO_ONE during Terra Nullis
// edited by paul 9/5
int getWhoseTurn (Game g){
    int result;

    if( g->turnNumber == -1) {
        result = NO_ONE;
    } else {
        result = ((g->turnNumber) % NUM_UNIS)+1;
    }

    return result;
}

/*===============================================================*/
// return the contents of the given vertex (ie campus code or 
// VACANT_VERTEX)
int getCampus(Game g, path pathToVertex){
    region r = decodePath(pathToVertex);

    int i;
    i = 0; 
    i = getArrayIndex(r);

    //printf("%d\n", i);

    return g->vertices[i];
}


/*===============================================================*/
//Code by Andrew 12/05.
//Considering the Hexagonal board as a series of rectangles, where the
//vertices of the hexagon correspond to the vertices of the rectangle,
//and the midpoint of the long sides. Use this to form an co-ordinate 
//system which we can keep track of to now what vertex we lie on.
region decodePath(path p) {
    int move = 0;
    region v = {0, 0}; //This is the intial position,

    int count = 0;
    //Take our intitial direction to be 0. If we turn 60 degrees to the
    //left, that new direction will be 1, and so on... till 5.
    //Take note that direction 6 == direction 0.
    int direction = 0; 

    while (p[count] == 'L' || p[count] == 'R' || p[count] == 'B') {

        move = whereMove(direction, &p[count], move);

        if (move == UP) {
            v.y ++;
        } else if (move == DOWN) {
            v.y--;
        } else if (move == RIGHT) {
            v.x++;
        } else if (move == LEFT) {
            v.x--;
        }

        //Account for change in direction:
        if (p[count] == 'R') {
            direction--;
        } else if (p[count] == 'L') {
            direction++;
        } else if (p[count] == 'B') {
        	direction += 3; //Going backwards flips our direction,
        				   //which can be achieved by adding 3 to 
        				   //our direction.
        }

        //We account for repetition of directions here.
        // 6 --> 0 and -1--> 5.
        if (direction == 6 || direction < 0) {
            direction = (direction+6)%6;
        }

        count ++;
    }

    return v;
}
// Test for B  move
//Using L, R and direction we face we can work out where we will 
//move, these possible combinations are stored in moves[].
//The formula for working out our array position which stores
//where we move is:
//arrayPos = direction*2 + (0 if right, 1 if left)

int whereMove(int direction, char *path, char previousMove) {

    int moves[12] = {DOWN, RIGHT, DOWN, UP, RIGHT, UP,
        UP, LEFT, UP, DOWN, LEFT, DOWN};
    int arrayPos = 0;

    if (path[0] == 'R') {
        arrayPos = direction*2; //See forumla above for explanation.
    } else if (path[0] == 'L') {
        arrayPos = direction*2 + 1;
    } else if (path[0] =='B') { //We do the opposite of the prvious move
    							//So we can go back to the prev. pos.
    	if (previousMove == UP) {
    		arrayPos = DOWN_POSITION;
    	} else if (previousMove == DOWN) {
    		arrayPos = UP_POSITION;
    	} else if (previousMove == DOWN) {
    		arrayPos = LEFT_POSITION;
    	} else if (previousMove == LEFT) {
    		arrayPos = RIGHT_POSITION;
    	}
    }

    return moves[arrayPos];
}

int getArrayIndex(region r){
    
    int loc = 0;

    if( r.y == 0){
        loc = 0 + (r.x + 0);

    } else if( r.y == -1){
        loc = 2 + (r.x +1);

    } else if( r.y == -2){
        loc = 6 + (r.x + 2);

    } else if( r.y == -3){
        loc = 12 + (r.x + 2);

    } else if( r.y == -4){
        loc = 18 + (r.x + 2);

    } else if( r.y == -5){
        loc = 24 + (r.x + 2);

    } else if( r.y == -6){
        loc = 30 + (r.x + 2);

    } else if( r.y == -7){
        loc = 36 + (r.x + 2);

    } else if( r.y == -8){
        loc = 42 + (r.x + 2);

    } else if( r.y == -9){
        loc = 48 + (r.x + 1);

    } else if( r.y == -10){
        loc = 52 + (r.x + 0);

    }

    return loc;
}

// the contents of the given edge (ie ARC code or vacent ARC)
int getARC(Game g, path pathToEdge){
	int arcVertIndex, arcVertPrevIndex;
	region arcVertex = {0, 0};
	region arcVertexPrev = {0, 0};

	//This section creates a string with the last L,R,B removed.
	int pathLen = strlen(pathToEdge);
	char *pathToPrevious = malloc(sizeof(char)*pathLen);
	strncpy(pathToPrevious, pathToEdge, strlen(pathToEdge)-1);

	// path pathToPrevious
	// int pathLen = strlen(pathToEdge);
	// strcpy(pathToPrevious, pathToEdge);
	// pathToPrevious[pathLen-1] = '/0';

	//Find vertexes of the ARC.
	arcVertex = decodePath(pathToEdge);
	arcVertexPrev = decodePath(pathToPrevious);

	free(pathToPrevious); //pathToPrevious no longer used.

	//Get array index of the vertices.
	arcVertIndex = getArrayIndex(arcVertex);
	arcVertPrevIndex = getArrayIndex(arcVertexPrev);


	//Find out who owns it the ARC.	
	int whoOwns = NO_ONE;

	if (g->arcs[arcVertIndex].A == g->arcs[arcVertPrevIndex].A && 
	    g->arcs[arcVertIndex].A == TRUE) {
		whoOwns = UNI_A;
	} else if (g->arcs[arcVertIndex].B == g->arcs[arcVertPrevIndex].B && 
			   g->arcs[arcVertIndex].B == TRUE) {
		whoOwns = UNI_B;
	} else if (g->arcs[arcVertIndex].C == g->arcs[arcVertPrevIndex].C && 
			   g->arcs[arcVertIndex].C == TRUE) {
		whoOwns = UNI_C;
	} else {
		whoOwns = NO_ONE;
	}

	return whoOwns;
}


// returns TRUE if it is legal for the current
// player to make the specified action, FALSE otherwise.
//
// "legal" means everything is legal: 
//   * that the action code is a valid action code which is legal to 
//     be made at this time
//   * that any path is well formed and legal ie consisting only of 
//     the legal direction characters and of a legal length, 
//     and which does not leave the island into the sea at any stage.
//   * that disciplines mentioned in any retraining actions are valid 
//     discipline numbers, and that the university has sufficient
//     students of the correct type to perform the retraining
//
// eg when placing a campus consider such things as: 
//   * is the path a well formed legal path 
//   * does it lead to a vacent vertex?
//   * under the rules of the game are they allowed to place a 
//     campus at that vertex?  (eg is it adjacent to one of their ARCs?)
//   * does the player have the 4 specific students required to pay for 
//     that campus?
// It is not legal to make any action during Terra Nullis ie 
// before the game has started.
// It is not legal for a player to make the moves OBTAIN_PUBLICATION 
// or OBTAIN_IP_PATENT (they can make the move START_SPINOFF)
// you can assume that any pths passed in are NULL terminated strings.

// edited by paul 10/5
int isLegalAction (Game g, action a){
    int isLegal = FALSE;

    //check if actionCode is legal
    if( (a.actionCode >= PASS) && (a.actionCode <= RETRAIN_STUDENTS)){

        //check if game is in terra nullius
        if( g->turnNumber != -1){

            //Go through each actionCode and check
            //whether the requirements for it are met
            if( a.actionCode == PASS){
                //PASS is always legal if above conditions are met
                isLegal = TRUE;

            }else if( a.actionCode == BUILD_CAMPUS){
                /*====================================================
                  BUILD_CAMPUS is legal if
                 **the uni has enough resources
                 **the uni owns an ARC connecting to the vertex
                 **there are no other campuses on adjacent vertices
                 **and if the max amount of campuses haven't been built
                 ====================================================*/
                if(TRUE){
                    isLegal = TRUE;
                }
            }else if( a.actionCode == BUILD_GO8){
                /*=============================================
                  BUILD_G08 is legal if
                 **the uni already owns a campus at target vertex
                 **the uni has enough resources
                 **and if there are not too many G08s in the game
                 ==============================================*/
                if(TRUE){
                    isLegal = TRUE;
                }
            }else if( a.actionCode == OBTAIN_ARC){
                /*=========================================================
                  CREATE_ARC is legal if
                 **the arc is not already occupied
                 **the uni owns an ARC or campus connecting to the target ARC
                 **the uni has enough resources
                 ==========================================================*/
                if(TRUE){
                    isLegal = TRUE;
                }
            }else if( a.actionCode == START_SPINOFF){
                /*==============================
                  START_SPINOFF is legal if:
                 **the uni has enough resources
                 **(any other conditions)?
                 =============================*/
                if(TRUE){
                    isLegal = TRUE;
                }
            }else if( a.actionCode == RETRAIN_STUDENTS){
                /*=========================================================
                  RETRAIN_STUDENTS is legal if:
                 **retrain from a valid student (not THD)
                 **retrain to a valid student (not the same type)
                 **the uni has enough resources to retrain 
                 **(need to check the exchangeRate to see if enough resources)
                 ==========================================================*/
                if(TRUE){
                    isLegal = TRUE;
                }
            }
        }
    }
    return isLegal;
}


// --- get data about a specified player ---

// return the number of KPI points the specified player currently has
// edited by paul 10/5
int getKPIpoints (Game g, int player){
    int total;

    total = 0;

    //Increase total depending on resources owned
    total += CAMPUS_MULTIPLIER*getCampuses(g, player);

    total += G08_MULTIPLIER*getGO8s(g, player);

    total += ARC_MULTIPLIER*getARCs(g, player);

    total += PATENT_MULTIPLIER*getPatents(g, player);

    if (getMostARCs(g) == player){
        total += MOST_ARCS_BONUS;
    }
    if( getMostPublications(g) == player){
        total += MOST_PUBS_BONUS;
    }

    return total;
}

int getPatents(Game g, int player){
    return g->players[player].patents;
}

// return the number of ARC grants the specified player currently has
// edited by paul 10/5
int getARCs (Game g, int player){
    return g->players[player].arcGrants;
    /*
       int result = 0;
       int index = 0;

    //check every arc to see how many ARC grants the player owns
    while( index < NUM_ARCS){
    //All ARC grants are stored in the arc array
    //0 represents that no one owns an ARC grant at that arc
    //1 represents an ARC grant owned by UNI_A, 2 by UNI_B, 3 by UNI_C
    if( g->arcs[index] == player){
    result++;
    }
    index++;
    }

    return result;
     */
}


// return the number of GO8 campuses the specified player currently has
// edited by paul 10/5
int getGO8s (Game g, int player){
    return g->players[player].GO8s;
    /*
       int result = 0;
       int index = 0;

    //check every vertex to see how many G08s the player owns
    while( index < NUM_VERTICES){
    //All G08s and Campuses are both stored in the vertices array
    //a 5  represents a G08 owned by UNI_A, 6 by UNI_B, 7 by UNI_C
    //and 4 means that no one owns a G08 at that vertex
    if( g->vertices[index] == player + 4){
    result++;
    }
    index++;
    }

    return result;
     */
}

// return the number of normal Campuses the specified player currently has
// edited by paul 10/5
int getCampuses (Game g, int player){
    return g->players[player].campuses;
    /*
       int result = 0;
       int index = 0;

    //check every vertex to see how many G08s the player owns
    while( index < NUM_VERTICES){
    //All G08s and Campuses are both stored in the vertices array
    //1 represents a campus owned by UNI_A, 2 by UNI_B and 3 by UNI_C
    //0 means that no one owns a campus at that vertex
    if( g->vertices[index] == player){
    result++;
    }
    index++;
    }
    return result;
     */
}

// return the number of IP Patents the specified player currently has
// edited by paul 10/5
int getIPs (Game g, int player){
    int result;

    result = g->players[player].patents;

    return result;
}


// return the number of Publications the specified player currently has
// edited by paul 10/5
int getPublications (Game g, int player){
    int result;

    result = g->players[player].pubs;

    return result;
}


// return the number of students of the specified discipline type 
// the specified player currently has
// edited by paul 10/5
int getStudents (Game g, int player, int discipline){
    int result;
    result = 0;
    if(       discipline == STUDENT_THD){
        result = g->players[player].THD;

    }else if( discipline == STUDENT_BPS){
        result = g->players[player].BPS;

    }else if( discipline == STUDENT_BQN){
        result = g->players[player].BQN;

    }else if( discipline == STUDENT_MJ){
        result = g->players[player].MJ;

    }else if( discipline == STUDENT_MTV){
        result = g->players[player].MTV;

    }else if( discipline == STUDENT_MMONEY){
        result = g->players[player].MMONEY;

    }
    return result;
}

// return how many students of discipline type disciplineFrom
// the specified player would need to retrain in order to get one 
// student of discipline type disciplineTo.  This will depend 
// on what retraining centers, if any, they have a campus at.
int getExchangeRate (Game g, int player, 
        int disciplineFrom, int disciplineTo){
    int rate = 3;

    //==================================================
    //Check if uni owns a campus near a retraining centre
    //to see if rate changes to 2;
    //==================================================
    int condition = FALSE;
    if(condition){
        rate = 2;
    }
    return rate;
}


