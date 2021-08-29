#ifndef MYAI_INCLUDED
#define MYAI_INCLUDED 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <algorithm> 
#include <assert.h>
#include <unordered_set>
#include <string>
#include <vector>


#define RED 0
#define BLACK 1
#define CHESS_COVER 8  // X
#define CHESS_EMPTY 0  // -
#define COMMAND_NUM 18

// ['-','K','G','M','R','N','C','P','X','k','g','m','r','n','c','p']
const short toNumTable[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, -1, -1, -1, 2, -1, -1, -1, 1, -1, 3, 5, -1, 7, -1, 4, -1, -1, -1, -1, -1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, 10, -1, -1, -1, 9, -1, 11, 13, -1, 15, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1};

const char toCharTable[] = {'-','K','G','M','R','N','C','P','X','k','g','m','r','n','c','p'};

const bool canMoveTable[16][16] = {  // canMoveTable[from][to]
  // -  K  G  M  R  N  C  P  X  k  g  m  r  n  c  p
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // -
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0}, // K
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // G
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // M
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, // R
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}, // N
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // C
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // P
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // X
	{1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // k
	{1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // g
	{1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // m
	{1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // r
	{1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // n
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // c
	{1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // p
};

const short colorTable[] = {-1, 0, 0, 0, 0, 0, 0, 0, 
    						            -1, 1, 1, 1, 1, 1, 1, 1};

// const double pieceScore[] = {0, 810, 270, 90, 18, 6, 180, 1, 
// 														 0, 810, 270, 90, 18, 6, 180, 1};

const double pieceScore[] = {0, 0.125, 0.042, 0.014, 0.003, 0.001, 0.028, 0.0002, 
														 0, 0.125, 0.042, 0.014, 0.003, 0.001, 0.028, 0.0002};

class Node {
public:
	short Board[10][6];
	// ['-','K','G','M','R','N','C','P','X','k','g','m','r','n','c','p']
	short chessCover[16];

	Node *parent;
	std::vector<Node*>child;

	int depth; // depth, 0 for the root
	short move[4];  // move to this state
	bool isflip;  // chance node = true

	unsigned int Ntotal; // total # of simulations
	double score; 
	double avg_score;
	// float score;

	double alpha, beta;

	Node(){
		Ntotal = 0;
		score = 0.;
		avg_score = 0.;
	};
	
	Node(const Node &node) {
		memcpy(Board, node.Board, 10*6*sizeof(short));
		memcpy(chessCover, node.chessCover, 16*sizeof(short));
		depth = node.depth;
		isflip = false;
		Ntotal = 0;
		score = 0.;
		avg_score = 0.;
	};
};

class MyAI  
{
	const char* commands_name[COMMAND_NUM] = {
		"protocol_version",
		"name",
		"version",
		"known_command",
		"list_commands",
		"quit",
		"boardsize",
		"reset_board",
		"num_repetition",
		"num_moves_to_draw",
		"move",
		"flip",
		"genmove",
		"game_over",
		"ready",
		"time_settings",
		"time_left",
  	"showboard"
	};
public:
	MyAI();
	~MyAI(void);

	// commands
	bool protocol_version(const char* data[], char* response);// 0
	bool name(const char* data[], char* response);// 1
	bool version(const char* data[], char* response);// 2
	bool known_command(const char* data[], char* response);// 3
	bool list_commands(const char* data[], char* response);// 4
	bool quit(const char* data[], char* response);// 5
	bool boardsize(const char* data[], char* response);// 6
	bool reset_board(const char* data[], char* response);// 7
	bool num_repetition(const char* data[], char* response);// 8
	bool num_moves_to_draw(const char* data[], char* response);// 9
	bool move(const char* data[], char* response);// 10
	bool flip(const char* data[], char* response);// 11
	bool genmove(const char* data[], char* response);// 12
	bool game_over(const char* data[], char* response);// 13
	bool ready(const char* data[], char* response);// 14
	bool time_settings(const char* data[], char* response);// 15
	bool time_left(const char* data[], char* response);// 16
	bool showboard(const char* data[], char* response);// 17

private:
	short Color;
	int Red_Time, Black_Time;
	int step;
	Node root;

	// random
	uint32_t randIndex(uint32_t max);
	uint32_t pcg32_random_r();

	// Board
	void initBoardState();

	// move
	void MakeMove(int src_x, int src_y, int dst_x, int dst_y);
	void MakeFlip(int x, int y, short pieceId);
	void generateMove(char move[6]);
	void legalMove(short moves[][4], short &count, short Board[10][6], int color, short chessCover[16]);
	short getMove(short moves[][4], short Board[10][6], int color);
	short getFlip(short moves[][4], short Board[10][6], short chessCover[16], short move_count);

	// show
	void printBoard(short B[10][6]);
	void printTree(Node* node, FILE* pfile);

	void expansion(Node *node, short color);

	// search
	double alphaBeta(Node *node, short color, double alpha, double beta);
	double star(Node *node, short color, double alpha, double beta);

	bool isFinish(short Board[10][6]);
	bool isEndgame(short Board[10][6]);

	double evaluation(short Board[10][6], short chessCover[16], short who_win);

	void MakeMove(short move[4], short Board[10][6]);
	void MakeFlip(short move[4], short pieceId, short Board[10][6], short chessCover[16]);

	
};

#endif

