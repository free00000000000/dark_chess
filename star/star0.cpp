#include "float.h"
#include "star0.h"
#include "rand.cpp"
#include <iostream>

#define SIMULATE_COUNT_PER_CHILD 10
#define TIME_LIMIT 9.5
#define DEPTH_LIMIT 7
#define D_MIN -999999.0
#define D_MAX 999999.0
#define GET_X(s) (s[0]-96)  //c-'a'+1
#define GET_Y(s) (s[1]-48)  //c-'0'

// FILE* pfile;

MyAI::MyAI(){}

MyAI::~MyAI(void){}

bool MyAI::protocol_version(const char* data[], char* response){
	strcpy(response, "1.0.0");
  return 0;
}

bool MyAI::name(const char* data[], char* response){
	strcpy(response, "haha");
	return 0;
}

bool MyAI::version(const char* data[], char* response){
	strcpy(response, "1.0.0");
	return 0;
}

bool MyAI::known_command(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		if(!strcmp(data[0], commands_name[i])){
			strcpy(response, "true");
			return 0;
		}
	}
	strcpy(response, "false");
	return 0;
}

bool MyAI::list_commands(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		strcat(response, commands_name[i]);
		if(i < COMMAND_NUM - 1){
			strcat(response, "\n");
		}
	}
	return 0;
}

bool MyAI::quit(const char* data[], char* response){
  fprintf(stderr, "Bye\n"); 
	return 0;
}

bool MyAI::boardsize(const char* data[], char* response){
  fprintf(stderr, "BoardSize: %s x %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::reset_board(const char* data[], char* response){
	Red_Time = -1; // unknown
	Black_Time = -1; // unknown
	initBoardState();
	return 0;
}

bool MyAI::num_repetition(const char* data[], char* response){
  return 0;
}

bool MyAI::num_moves_to_draw(const char* data[], char* response){
  return 0;
}

bool MyAI::move(const char* data[], char* response) {
	int src_x = GET_X(data[0]);
	int src_y = GET_Y(data[0]);
	int dst_x = GET_X(data[1]);
	int dst_y = GET_Y(data[1]);
	MakeMove(src_x, src_y, dst_x, dst_y);
	printf("# call move(): %s-%s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::flip(const char* data[], char* response) {
	int x = GET_X(data[0]);
	int y = GET_Y(data[0]);
	MakeFlip(x, y, toNumTable[(int)data[1][0]]);
	printf("# call flip(): %s(%c) = %d,%d(%d)\n", data[0], data[1][0], x, y, root.Board[y][x]);
	return 0;
}

bool MyAI::genmove(const char* data[], char* response){
	// set color
	if(!strcmp(data[0], "red")){
		Color = RED;
	}else if(!strcmp(data[0], "black")){
		Color = BLACK;
	}else{
		Color = 2;
	}
	// genmove
  char move[6];
	generateMove(move);
	sprintf(response, "%c%c %c%c", move[0], move[1], move[3], move[4]);
	return 0;
}

bool MyAI::game_over(const char* data[], char* response){
  fprintf(stderr, "Game Result: %s\n", data[0]); 
	return 0;
}

bool MyAI::ready(const char* data[], char* response){
  return 0;
}

bool MyAI::time_settings(const char* data[], char* response){
  return 0;
}

bool MyAI::time_left(const char* data[], char* response){
  if(!strcmp(data[0], "red")){
		sscanf(data[1], "%d", &(Red_Time));
	}else{
		sscanf(data[1], "%d", &(Black_Time));
	}
	fprintf(stderr, "Time Left(%s): %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::showboard(const char* data[], char* response) {
	return 0;
}


// ********************* AI FUNCTION ********************* //

void MyAI::initBoardState() {
	srand(time(NULL));
	step = 0;

	for (int i=0; i<10; ++i) {
		for (int j=0; j<6; ++j) {
			root.Board[i][j] = CHESS_COVER;
		}
	}

	short cover[] = {0, 1, 2, 2, 2, 2, 2, 5,
									 0, 1, 2, 2, 2, 2, 2, 5};
	for (int i=0; i<16; ++i)
		root.chessCover[i] = cover[i];

	root.depth = 0;
	root.isflip = false;
	root.parent = NULL;
}

short MyAI::getMove(short moves[][4], short Board[10][6], int color) {
	short count = 0;
	short dx[] = {1, 0, -1, 0};
	short dy[] = {0, 1, 0, -1};
	for (int y=1; y<=8; ++y) {
		for (int x=1; x<=4; ++x) {
			if (colorTable[Board[y][x]] == color){
				// move
				for (int i=0; i<4; ++i) {
					int next_x = x + dx[i];
					int next_y = y + dy[i];
					if (canMoveTable[Board[y][x]][Board[next_y][next_x]]) {
						moves[count][0] = x;
						moves[count][1] = y;
						moves[count][2] = next_x;
						moves[count][3] = next_y;
						count++;
					}
				}

				// ??????
				if (Board[y][x] == 6 || Board[y][x] == 14) {
					// printf("is C!\n");
					// fflush(stdout);
					for (int i=0; i<4; ++i) {
						int next_x = x + dx[i];
						int next_y = y + dy[i];
						bool ready = false;
						int opColor = !color;
						while (next_x>=1 && next_x<=4 && next_y>=1 && next_y <= 8) {
							// printf("(%d, %d)\n", next_x, next_y);
							if (ready && Board[next_y][next_x] != CHESS_EMPTY) {
								if (colorTable[Board[next_y][next_x]] == opColor) {
									moves[count][0] = x;
									moves[count][1] = y;
									moves[count][2] = next_x;
									moves[count][3] = next_y;
									count++;
									// printf("jump: (%d, %d) to (%d, %d)\n", x, y, next_x, next_y);
									// fflush(stdout);
									break;
								}
								break;
							}
							if (Board[next_y][next_x] != CHESS_EMPTY) ready = true;
							next_x += dx[i];
							next_y += dy[i];
						}
					}
				}

			}
		}
	}
	return count;
}

short MyAI::getFlip(short moves[][4], short Board[10][6], short chessCover[16], short move_count) {
	bool search[10][6] = {false};
	short no_search[32][2];  // X position: x, y
	short c_id = Color==RED ? 6:14;
	bool is_c = chessCover[c_id] > 0 ? true : false;

	for (int y=1; y<=8; ++y) {
		for(int x=1; x<=4; ++x) {
			// ?????????
			if (colorTable[Board[y][x]] != -1) {
				// ???????????????????????????
				if (is_c && colorTable[Board[y][x]] != Color) {
					if (y >= 3) search[y-2][x] = true;
					if (y <= 6) search[y+2][x] = true;
					if (x == 1) search[y][3] = true;
					else if (x == 2) search[y][4] = true;
					else if (x == 3) search[y][1] = true;
					else if (x == 4) search[y][2] = true;
				}
				

				// ??????????????????
				if (Board[y][x] == c_id) {
					if (y >= 3) search[y-2][x] = true;
					if (y <= 6) search[y+2][x] = true;
					if (x == 1) search[y][3] = true;
					else if (x == 2) search[y][4] = true;
					else if (x == 3) search[y][1] = true;
					else if (x == 4) search[y][2] = true;
				}
				
				// ????????????
				search[y+1][x] = true;
				search[y-1][x] = true;
				search[y][x+1] = true;
				search[y][x-1] = true;

				

			}
		}
	}

	short count = move_count;
	int count_no_search = 0;
	for (int y=1; y<=8; ++y) {
		for(int x=1; x<=4; ++x) {
			if (Board[y][x] == CHESS_COVER) {
				if (search[y][x]) {
					moves[count][0] = moves[count][2] = x;
					moves[count][1] = moves[count][3] = y;
					count++;
				} else {
					no_search[count_no_search][0] = x;
					no_search[count_no_search][1] = y;
					count_no_search += 1;
				}
			}
		}
	}

	// ????????????
	if (count_no_search > 0) {
		uint32_t id = randIndex(count_no_search);
		moves[count][0] = moves[count][2] = no_search[id][0];
		moves[count][1] = moves[count][3] = no_search[id][1];
		count++;
	}
	return count-move_count;
}

void MyAI::legalMove(short moves[][4], short &count, short Board[10][6], int color, short chessCover[16]) {

	short m = getMove(moves, Board, color);  // move count 
	short f = getFlip(moves, Board, chessCover, m);  // filp count

	count = m + f;
}

void MyAI::expansion(Node *node, short color) {
	// color: node ?????????
	assert(node->isflip == false);
	// if (node->isflip) return;

	// moves: 4*16+8 [from_x, from_y, to_x, to_y]
	short moves[72][4];
	short m = getMove(moves, node->Board, color);

	// ordering ???????????????
	int idx[72];
	for (int i=0; i<72; ++i) { idx[i] = i; }
	double moves_score[72];
	for (int i=0; i<m; ++i) {
		moves_score[i] = pieceScore[node->Board[moves[i][3]][moves[i][2]]];
	}
	std::sort(idx, idx+m, sort_indices(moves_score));

	// std::cout << "move count: " << m << std::endl;
	for (int i=0; i<m; ++i) {
		Node *next = new Node(*node);
		// printf("move: (%d, %d) to (%d, %d)\n", moves[i][0], moves[i][1], moves[i][2], moves[i][3]);
		MakeMove(moves[idx[i]], next->Board);
		next->depth += 1;
		memcpy(next->move, moves[idx[i]], 4*sizeof(short));
		next->parent = node;
		node->child[node->child_count++] = next;
	}

	// flips: 32 [x, y, x, y]
	short flips[32][4];
	short f = getFlip(flips, (*node).Board, (*node).chessCover, 0);  // filp count
	// std::cout << "flips count: " << f << std::endl;
	for (int i=0; i<f; ++i) {
		Node *next = new Node(*node);
		next->depth += 1;
		memcpy(next->move, flips[i], 4*sizeof(short));
		// expand next's child
		for (short j=1; j<16; ++j) {
			if (next->chessCover[j] > 0) {
				Node *next_open = new Node(*next);
				memcpy(next_open->move, next->move, 4*sizeof(short));
				MakeFlip(next_open->move, j, next_open->Board, next_open->chessCover);
				next_open->parent = next;
				next->child[next->child_count++] = next_open;
			}
		}
		assert(next->child_count<80);
		next->isflip = true;
		next->parent = node;
		node->child[node->child_count++] = next; 
		// std::cout << (*(*node).child[i]).move[0] << ' ' << (*(*node).child[i]).move[1] << std::endl;
	}

	assert(node->child_count<80);

	// std::cout << "move: " << node->child.size() << std::endl;
	// std::cout << "end expansion" << std::endl;
}

double MyAI::evaluation(short Board[10][6], short chessCover[16], short who_win) {

	double score = 0;
	short opColor = !Color;
	if (who_win == Color) return 1;
	else if (who_win == opColor) return -1;

	for (int y=1; y<=8; ++y) {
		for (int x=1; x<=4; ++x) {
			if (colorTable[Board[y][x]] == Color) {
				score += pieceScore[Board[y][x]];
			} else if (colorTable[Board[y][x]] == opColor) {
				score -= pieceScore[Board[y][x]];
			}
		}
	}

	for (int i=1; i<16; ++i) {
		if (chessCover[i] > 0) {
			if (colorTable[i] == Color) {
				score += pieceScore[i]*chessCover[i];
			} else {
				score -= pieceScore[i]*chessCover[i];
			}
		}
	}
	
	return score;
}

double MyAI::star(Node *node, short color, double alpha, double beta, int depth) {
	assert(node->isflip == true);
	
	if (time_flag) return -1;

	// node->alpha = alpha;
	// node->beta = beta;

	int total = 0;
	short w[15];  // ????????????
	for (int i=0; i<node->child_count; ++i) {
		short piece = node->child[i]->Board[node->child[i]->move[3]][node->child[i]->move[2]];
		w[i] = node->chessCover[piece];
		total += w[i];
	}

	double f_score = 0;  // chance node ??????

	for (int i=0; i<node->child_count; ++i) {
		double score = alphaBeta(node->child[i], color, D_MIN, D_MAX, depth-1);
		if (depth == 5) printf("%.4f (%d)\n", score, node->child[i]->Board[node->child[i]->move[3]][node->child[i]->move[2]]);
		node->child[i]->score = score;
		f_score += (double)w[i]/total * score;
	}
	
	if (depth == 5) printf("avg: %.4f\n", f_score);
	return f_score;
}

double MyAI::alphaBeta(Node *node, short color, double alpha, double beta, int depth) {
	// color: node's color

	// node->alpha = alpha;
	// node->beta = beta;

	if (time_flag) return -1;

	if ((double)(clock() - begin) / CLOCKS_PER_SEC >= TIME_LIMIT) {
		time_flag = true;
		return -1;
	}

	if (depth <= 0) {
		return evaluation(node->Board, node->chessCover, -1);
	}

	expansion(node, color);

	if (node->child_count == 0) {
		return evaluation(node->Board, node->chessCover, !color);
	}
	
	if (color == Color) {  // my turn
		double m = D_MIN;
		for (int i=0; i<node->child_count; ++i) {
			if (!node->child[i]->isflip) {
				node->child[i]->score = alphaBeta(node->child[i], !color, std::max(alpha, m), beta, depth-1);
			} else {
				node->child[i]->score = star(node->child[i], !color, std::max(alpha, m), beta, depth>>1);
			}
			m = std::max(m, node->child[i]->score);

			if (beta <= m) {
				node->clean_child();
				return m;
			}

		}
		node->clean_child();
		return m;

	} else {
		double m = D_MAX;
		for (int i=0; i<node->child_count; ++i) {
			if (!node->child[i]->isflip) {
				node->child[i]->score = alphaBeta(node->child[i], !color, alpha, std::min(beta, m), depth-1);
			} else {
				node->child[i]->score = star(node->child[i], !color, alpha, std::min(beta, m), depth>>1);	
			}
			
			m = std::min(m, node->child[i]->score);
			// beta = std::min(beta, child->score);


			if (m <= alpha) {
				node->clean_child();
				return m;
			}
		}
		node->clean_child();
		return m;
	}

}

void MyAI::generateMove(char move[6]) {
	begin = clock();
	time_flag = false;
	// pfile = fopen ("tree.txt","w");
	// fprintf(pfile, "");
	// fclose(pfile);

	printf("############# Generate Move #############\n");

	printBoard(root.Board);
	std::cout << "Color " << Color << std::endl;
	
	root.depth = 0;
	root.score = 0.;
	expansion(&root, Color);
	int it_depth = 2;
	short best_move[4];
	double best_score;
	
	while ((double)(clock() - begin) / CLOCKS_PER_SEC < TIME_LIMIT) {

		double best = D_MIN;
		short b_move[4];

		for (int i=0; i<root.child_count; ++i) {
			if (!root.child[i]->isflip) {
				root.child[i]->score = alphaBeta(root.child[i], !Color, std::max(D_MIN, best), D_MAX, it_depth-1);
			} else {
				root.child[i]->score = star(root.child[i], !Color, std::max(D_MIN, best), D_MAX, it_depth>>1);
			}
			

			printf("move: (%d, %d) to (%d, %d)  score: %.4f\n", root.child[i]->move[0], root.child[i]->move[1], root.child[i]->move[2], root.child[i]->move[3], root.child[i]->score);

			if (root.child[i]->score > best) {
				best = root.child[i]->score;
				memcpy(b_move, root.child[i]->move, 4*sizeof(short));
			}

		}

		if (time_flag == false) {
			memcpy(best_move, b_move, 4*sizeof(short));
			// best_score = best;
		}

		printf("max depth = %d, star depth = %d\n", it_depth, it_depth>>1);
		printf("best move: (%d, %d) to (%d, %d), score: %.4f\n\n", b_move[0], b_move[1], b_move[2], b_move[3], best);

		if (best == 1 || best == -1 || it_depth==11) break;

		it_depth += 2;
	}
	


	printf("time: %.3f\n", (double)(clock()-begin)/CLOCKS_PER_SEC);
	printf("final best move: (%d, %d) to (%d, %d)\n", best_move[0], best_move[1], best_move[2], best_move[3]);

	// pfile = fopen ("tree.txt","a");
	// printTree(&root, pfile);
	// fclose(pfile);

	fflush(stdout);

	char from[3] = {(char)(best_move[0]+96), (char)(48+best_move[1]), '\0'};
	char to[3] = {(char)(best_move[2]+96), (char)(48+best_move[3]), '\0'};
	
	sprintf(move, "%s-%s", from, to);
	printf("move: %s\n", move);

	printBoard(root.Board);
	printf("########### End Generate Move ###########\n\n");

	root.clean_child();

}

void MyAI::MakeMove(int src_x, int src_y, int dst_x, int dst_y) {
	root.Board[dst_y][dst_x] = root.Board[src_y][src_x];
	root.Board[src_y][src_x] = CHESS_EMPTY;

	step += 1;
}


void MyAI::MakeFlip(int x, int y, short pieceId) { 	
	root.Board[y][x] = pieceId;
	root.chessCover[pieceId]--;
	assert(root.chessCover[pieceId] >= 0);

	step += 1;
}

void MyAI::MakeMove(short move[4], short Board[10][6]) {
	// if (Board[move[3]][move[2]] != CHESS_EMPTY) {
	// 	chessDead[Board[move[3]][move[2]]]++;
	// }
	Board[move[3]][move[2]] = Board[move[1]][move[0]];
	Board[move[1]][move[0]] = CHESS_EMPTY;
}


void MyAI::MakeFlip(short move[4], short pieceId, short Board[10][6], short chessCover[16]) { 	
	Board[move[1]][move[0]] = pieceId;
	chessCover[pieceId]--;
	assert(chessCover[pieceId] >= 0);

}

bool MyAI::isEndgame(short B[10][6]) {
	int count = 0;
	for (short y=8; y>=1; --y) {
		for (short x=1; x<=4; ++x) {
			if (B[y][x] != CHESS_EMPTY) {
				count++;
				if (count > 6) return false;
			}
		}
	}
	return true;
}

void MyAI::printBoard(short B[10][6]) {
	printf("--- Board ---\n");
	for (short y=8; y>=1; --y) {
		printf("  %d ", y);
		for (short x=1; x<=4; ++x) {
			printf("%c ", toCharTable[B[y][x]]);
		}
		printf("\n");
	}
	printf("    a b c d\n");
	printf("-------------\n");

	fflush(stdout);
}

void MyAI::printTree(Node* node, FILE* pfile) {
	return;
	// if (node->depth != 0) {
	// 	for (int i=0; i<node->depth; ++i) fprintf(pfile, " ");
	// 	// if (node->parent->isflip) fprintf(pfile, " ");
	// 	for (int i=0; i<node->depth; ++i) fprintf(pfile, "-");
	// 	fprintf(pfile, "(%d, %d) to (%d, %d) (%c)| s: %.4f, a: %.4f, b: %.4f\n", node->move[0], node->move[1], node->move[2], node->move[3], toCharTable[node->Board[node->move[1]][node->move[0]]], node->score, node->alpha, node->beta);
	// }
	// else {
	// 	for (int i=0; i<node->depth; ++i) fprintf(pfile, " ");
	// 	for (int i=0; i<node->depth; ++i) fprintf(pfile, "-");
	// 	fprintf(pfile, "(%d, %d) to (%d, %d) | s: %.4f, a: %.4f, b: %.4f\n", node->move[0], node->move[1], node->move[2], node->move[3], node->score, node->alpha, node->beta);
	// }
	// for (auto& child : node->child) {
	// 	printTree(child, pfile);
	// }
}

// return range: [0, max)
uint32_t MyAI::randIndex(uint32_t max) {

	// range (unbiased)
	uint32_t mask = ~uint32_t(0);
	--max;
	mask >>= __builtin_clz(max|1);
	uint32_t x;
	do {
			x = pcg32_random_r() & mask;
	} while (x > max);

	return x;
}

uint32_t MyAI::pcg32_random_r() {
	uint64_t oldstate = rng.state;
	// Advance internal state
	rng.state = oldstate * 6364136223846793005ULL + (rng.inc|1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}