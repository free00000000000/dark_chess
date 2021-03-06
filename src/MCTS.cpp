#include "float.h"
#include "MCTS.h"
#include "rand.cpp"
#include <iostream>

#define SIMULATE_COUNT_PER_CHILD 10
#define TIME_LIMIT 8
#define DEPTH_LIMIT 7
#define GET_X(s) (s[0]-96)  //c-'a'+1
#define GET_Y(s) (s[1]-48)  //c-'0'

double C;

MyAI::MyAI(double c){
	C = c;
}

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
	int cover_count = 0;
	short count = move_count;

	for (int i=1; i<16; ++i) cover_count += chessCover[i];

	if (cover_count == 32) {
		for (int y=1; y<=4; ++y) {
			for (int x=1; x<=2; ++x) {
				// flip
				moves[count][0] = moves[count][2] = x;
				moves[count][1] = moves[count][3] = y;
				count++;
			}
		}
		return count-move_count;
	}

	for (int y=1; y<=8; ++y) {
		for (int x=1; x<=4; ++x) {
			if (Board[y][x] == CHESS_COVER) {
				// flip
				moves[count][0] = moves[count][2] = x;
				moves[count][1] = moves[count][3] = y;
				count++;
			}
		}
	}
	return count-move_count;
}

void MyAI::legalMove(short moves[][4], short &count, short Board[10][6], int color, short chessCover[16]) {

	short m = getMove(moves, Board, color);  // move count 
	short f = getFlip(moves, Board, chessCover, m);  // filp count

	count = m + f;
}

void MyAI::expansion(Node *node) {
	assert(node->isflip == false);
	// if (node->isflip) return;

	short color;  // node ?????????
	if ((*node).depth%2 == 0) {
		color = Color;  // ??????
	} else {
		color = !Color;  // ??????
	}

	// moves: 4*16+8 [from_x, from_y, to_x, to_y]
	short moves[72][4];
	short m = getMove(moves, node->Board, color);
	// std::cout << "move count: " << m << std::endl;
	for (int i=0; i<m; ++i) {
		Node *next = new Node(*node);
		// printf("move: (%d, %d) to (%d, %d)\n", moves[i][0], moves[i][1], moves[i][2], moves[i][3]);
		MakeMove(moves[i], next->Board);
		next->depth += 1;
		memcpy(next->move, moves[i], 4*sizeof(short));
		next->parent = node;
		node->child.push_back(next);
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
				next->child.push_back(next_open);
			}
		}
		next->isflip = true;
		next->parent = node;
		node->child.push_back(next);
		// std::cout << (*(*node).child[i]).move[0] << ' ' << (*(*node).child[i]).move[1] << std::endl;
	}

	// std::cout << "move: " << node->child.size() << std::endl;
	// std::cout << "end expansion" << std::endl;
}


void MyAI::simulation(Node *node) {
	assert(node->isflip == false);

	short color;  // node's child ?????????
	if (node->depth%2 == 0) {  // ??????
		color = !Color;  // ??????
	} else {
		color = Color;  // ??????
	}

	if (node->child.size() == 0) {
		// printf("%d %d\n", node->Ntotal, node->score);
		// int score = node->score/node->Ntotal;
		
		node->Ntotal += (SIMULATE_COUNT_PER_CHILD<<4);
		node->score += node->avg_score*(SIMULATE_COUNT_PER_CHILD<<4);

		return;
	}

	for (auto& child : node->child) {
    // cout << it << endl;
		if (!child->isflip) {
			randomPlay(child, color, SIMULATE_COUNT_PER_CHILD);

		} else {
			// chance node
			for (auto& child_child : child->child) {
				randomPlay(child_child, color, SIMULATE_COUNT_PER_CHILD);
			}
		}

	}

	// std::cout << "end simulation" << std::endl;
}

double MyAI::evaluation(short Board[10][6], short chessCover[16], short who_win) {
	double score = 0;
	short opColor = !Color;
	if (who_win == Color) score = 1;
	else if (who_win == opColor) score = -1;

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
				score += pieceScore[i];
			} else {
				score -= pieceScore[i];
			}
		}
	}
	return score;
}

void MyAI::randomPlay(Node *node, short color, unsigned int times) {

	for (unsigned int t=0 ; t<times; ++t) {
		int count = 1;
		int c = color;
		short Board[10][6];
		short chessCover[16];
		memcpy(Board, (*node).Board, 10*6*sizeof(short));
		memcpy(chessCover, (*node).chessCover, 16*sizeof(short));

		while (true) {
			if (count >= 60) {
				// ???
				(*node).Ntotal += 1;
				node->score += evaluation(Board, chessCover, -1);
				break;
			}

			short moves[72][4];
			short m = getMove(moves, Board, c);
			
			short flips[32][4];
			short f = getFlip(flips, Board, chessCover, 0);  // filp count

			// c ???
			if (m==0 & f==0) {
				(*node).Ntotal += 1;
				node->score += evaluation(Board, chessCover, !c);
				break;
			}

			uint32_t id = randIndex(m+f);
			if (id < m) {
				// move
				if (Board[moves[id][3]][moves[id][2]] != CHESS_EMPTY) {
					count = -1;
				}

				MakeMove(moves[id], Board);
			} else {
				// flip
				count = -1;
				id -= m;
				int cover = 0;
				int piece[32];
				for (int i=1; i<16; ++i) {
					for (int j=0; j<chessCover[i]; ++j) {
						piece[cover] = i;
						cover++;
					}
				}
				uint32_t p_id = randIndex(cover);
				MakeFlip(flips[id], piece[p_id], Board, chessCover);
			}

			count++;
			c = !c;
		}
		
	}
}

// Node* MyAI::selection(Node* node, bool end) {
Node* MyAI::selection(Node* node) {
	// std::cout << "selection" << std::endl;
	Node* best_node;

	while (node->child.size() > 0) {
		double best_ucb = -99999999.;

		if (!node->isflip) {
			// ??? chance node ??????
			if (node->depth % 2 == 0) {
				for (auto& child : node->child) {
					// printf("score: %.4f\n", child->avg_score);
					double ucb = child->avg_score + C*sqrt(log(node->Ntotal)/child->Ntotal);
					// double ucb = child->avg_score;
					// if (!end) ucb += C*sqrt(log(node->Ntotal)/child->Ntotal);
					// if (child->isflip) ucb -= 100;
					// printf("(%d, %d): %f, %f \n", child->move[0], child->move[1], ucb, child->avg_score);
					if (ucb > best_ucb) {
						best_ucb = ucb;
						best_node = child;
					}
				}
			} else {
				best_ucb = 99999999.;
				for (auto& child : node->child) {
					// printf("score: %.4f\n", child->avg_score);
					double ucb = child->avg_score - C*sqrt(log(node->Ntotal)/child->Ntotal);
					// double ucb = child->avg_score;
					// if (!end) ucb -= C*sqrt(log(node->Ntotal)/child->Ntotal);
					// if (child->isflip) ucb += 100;
					// printf("(%d, %d): %f, %f t:%d \n", child->move[0], child->move[1], ucb, child->avg_score, child->Ntotal);
					if (ucb < best_ucb) {
						best_ucb = ucb;
						best_node = child;
					}
				}
			}

		} else {
			// chance node ?????????
			int cover = 0;
			int piece[32];
			int idx = 0;  // child id
			for (int i=1; i<16; ++i) {
				if (node->chessCover[i] > 0) {
					for (int j=0; j<node->chessCover[i]; ++j) {
						piece[cover] = idx;
						cover++;
					}
					idx++;
				}
			}
			uint32_t c_id = randIndex(cover);
			best_node = node->child[piece[c_id]];
			// if (node->child.size() == 1) best_node = node->child[0];
			// else return node;

			// std::cout << "c_id: " << piece[c_id] << std::endl;
		}

		node = best_node;
		// printf("move %c (%d, %d) to (%d, %d)  win rate: %.4f\n", toCharTable[node->Board[node->move[3]][node->move[2]]], node->move[0], node->move[1], node->move[2], node->move[3], node->avg_score);
		// std::cout << "ucb: " << best_ucb << std::endl;
		
	}
	
	// printBoard(node->Board);
	// std::cout << "end selection" << std::endl;
	if (node->Ntotal >= 10000) {
		// printf("score: %.5f \n", node->avg_score);
		return NULL;
	}
	return node;
}

void MyAI::backpropagation(Node* node) {
	unsigned int total = 0;
	double score = 0;
	Node* now = node;
	if (now->child.size() == 0) {
		total = now->Ntotal;
		score = now->score;
		now->avg_score = (double)score/total;
		now = now->parent;
		
	} else {
		for (auto& child : node->child) {
			if (child->isflip) {  // chance node
				unsigned int Ntotal = 0;
				int cover_idx[14];
				for (int i=1, j=0; i<16; ++i) {
					if (child->chessCover[i] > 0) {
						Ntotal += child->chessCover[i];
						cover_idx[j] = i;
						j++;
					}
				}
				int i = 0;
				child->avg_score = 0;
				for (auto& child_child : child->child) {
					double p = (double)child->chessCover[cover_idx[i]]/Ntotal;
					child_child->avg_score = (double)child_child->score/child_child->Ntotal;
					child->avg_score += child_child->avg_score * p;
					child->Ntotal += child_child->Ntotal;
					i++;
				}
				child->score = (int)child->Ntotal*child->avg_score;
				total += child->Ntotal;
				score += child->score;

			} else {
				total += child->Ntotal;
				score += child->score;
				child->avg_score = (double)child->score/child->Ntotal;
			}
		}
	}
	
	// std::cout << score << ' ' << total << std::endl;
	while (now != NULL) {
		now->Ntotal += total;
		now->score += score;
		now->avg_score = (double)now->score/now->Ntotal;
		// if (now->avg_score >= WIN_SCORE) {
		// 	printf("(%d, %d): score: %.5f \n", now->move[0], now->move[1], now->avg_score);
		// }
		now = now->parent;
	}
}

void MyAI::generateMove(char move[6]) {
	clock_t begin = clock();
	// FILE* pfile = fopen ("tree.txt","w");
	// fprintf(pfile, "");
	// fclose(pfile);

	printf("############# Generate Move #############\n");

	printBoard(root.Board);
	std::cout << "Color " << Color << std::endl;
	
	root.depth = 0;
	root.score = 0.;
	root.Ntotal = 0;
	
	// bool end = isEndgame(root.Board);

	while((double)(clock() - begin) / CLOCKS_PER_SEC < TIME_LIMIT) {
	// int r = 0;
	// while((r++)<10) {
		Node* node = selection(&root);
		// printf("--> (%d, %d) to (%d, %d)  win rate: %.4f\n", (*node).move[0], (*node).move[1], (*node).move[2], (*node).move[3], (*node).WR);
		if (node == NULL) {
			std::cout << "break" << std::endl;
			break;
		}
		expansion(node);
		simulation(node);
		backpropagation(node);

		// for (auto& child : root.child) {
		// 	printf("move: (%d, %d) to (%d, %d)  win rate: %.4f\n", (*child).move[0], (*child).move[1], (*child).move[2], (*child).move[3], (*child).WR);
		// 	printf("total: %d  wins: %d\n", (*child).Ntotal, (*child).Wins);
		// }
		// std::cout << std::endl;
	}

	short best_move[4];
	double best = -99999999.;
	for (auto& child : root.child) {
		printf("move: (%d, %d) to (%d, %d)  avg score: %.4f\n", (*child).move[0], (*child).move[1], (*child).move[2], (*child).move[3], (*child).avg_score);
		printf("total: %d  score: %f\n", (*child).Ntotal, (*child).score);

		if ((*child).avg_score > best) {
			best = (*child).avg_score;
			memcpy(best_move, (*child).move, 4*sizeof(short));
		}
	}


	printf("time: %.3f\n", (double)(clock()-begin)/CLOCKS_PER_SEC);

	


	/*
	expansion(&root);

	std::cout << root.child.size() << std::endl;

	short best_move[4];
	if (Color != 2) {
		simulation(&root);

		double best = -1.;
		for (auto& child : root.child) {
			printf("move: (%d, %d) to (%d, %d)  win rate: %.2f\n", (*child).move[0], (*child).move[1], (*child).move[2], (*child).move[3], (*child).WR);
			printf("total: %d  wins: %d\n", (*child).Ntotal, (*child).Wins);

			if ((*child).WR > best) {
				best = (*child).WR;
				memcpy(best_move, (*child).move, 4*sizeof(short));
			}
		}

	} else {
		uint32_t m = randIndex(root.child.size());
		memcpy(best_move, (*root.child[m]).move, 4*sizeof(short));
	}
	*/

	// moves: 4*16+8 [from_x, from_y, to_x, to_y]
	// flips: 16 [x, y, x, y]
	// short total_moves[88][4];
	// short count = 0;

	// legalMove(total_moves, count, root.Board, Color, root.chessCover);

	// int m = rand() % count;
	// short best_move[4] = {total_moves[m][0], total_moves[m][1], total_moves[m][2], total_moves[m][3]};


	std::cout << "total simulation: " << root.Ntotal << std::endl;
	printf("final best move: (%d, %d) to (%d, %d)\n", best_move[0], best_move[1], best_move[2], best_move[3]);

	// pfile = fopen ("tree.txt","a");
	// printTree(&root, pfile);
	// fclose(pfile);

	fflush(stdout);

	char from[3] = {(char)(best_move[0]+96), (char)(48+best_move[1]), '\0'};
	char to[3] = {(char)(best_move[2]+96), (char)(48+best_move[3]), '\0'};
	
	sprintf(move, "%s-%s", from, to);
	printf("move: %s\n", move);

	root.child.clear();
	root.Ntotal = 0;

	printBoard(root.Board);
	printf("########### End Generate Move ###########\n\n");
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
	for (int i=0; i<node->depth; ++i) fprintf(pfile, " ");
	for (int i=0; i<node->depth; ++i) fprintf(pfile, "-");
	if (node->depth != 0)
		fprintf(pfile, "(%d, %d) to (%d, %d) (%c)| s: %.4f t: %d\n", node->move[0], node->move[1], node->move[2], node->move[3], toCharTable[node->Board[node->move[1]][node->move[0]]], node->avg_score, node->Ntotal);
	else 
		fprintf(pfile, "(%d, %d) to (%d, %d) | s: %.4f t: %d\n", node->move[0], node->move[1], node->move[2], node->move[3], node->avg_score, node->Ntotal);
	for (auto& child : node->child) {
		printTree(child, pfile);
	}
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