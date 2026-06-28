#include <graphics.h>

#include <iostream>

char board[3][3] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

bool current_player = true; // true for X, false for O

bool check_win(char X){
	if (board[0][0] == X && board[0][1] == X && board[0][2] == X) return true;
	if (board[1][0] == X && board[1][1] == X && board[1][2] == X) return true;
	if (board[2][0] == X && board[2][1] == X && board[2][2] == X) return true;
	if (board[0][0] == X && board[1][0] == X && board[2][0] == X) return true;
	if (board[0][1] == X && board[1][1] == X && board[2][1] == X) return true;
	if (board[0][2] == X && board[1][2] == X && board[2][2] == X) return true;
	if (board[0][0] == X && board[1][1] == X && board[2][2] == X) return true;
	if (board[0][2] == X && board[1][1] == X && board[2][0] == X) return true;
	return false;
}

bool check_draw() {
	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			if(board[i][j] == ' ') return false;
		}
	}
	return true;
}

void draw_board() {
	line(200, 0, 200, 600);
	line(400, 0, 400, 600);
	line(0, 200, 600, 200);
	line(0, 400, 600, 400);
}

void draw_piece() {
	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			if(board[i][j] == 'x') {
				line(j * 200 + 50, i * 200 + 50, j * 200 + 150, i * 200 + 150);
				line(j * 200 + 150, i * 200 + 50, j * 200 + 50, i * 200 + 150);
			}
			else if(board[i][j] == 'o') {
				circle(j * 200 + 100, i * 200 + 100, 50);
			}
		}
	}
}


int main() {
	initgraph(600, 600);
	bool flag = true;
	ExMessage msg;
	BeginBatchDraw();

	while (flag) {
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg)) {
			if(msg.message == WM_LBUTTONDOWN) {
				int x = msg.x / 200;
				int y = msg.y / 200;

				if (board[y][x] == ' ') {
					board[y][x] = current_player ? 'x' : 'o';
					current_player = !current_player;
				}
			}
		}

		cleardevice();
		draw_board();
		draw_piece();
		FlushBatchDraw();

		if (check_win('x')) {
			MessageBox(GetHWnd(), _T("X player win!"), _T("Congratulations"), MB_OK);
			flag = false;
		}
		else if (check_win('o')) {
			MessageBox(GetHWnd(), _T("O player win!"), _T("Congratulations"), MB_OK);
			flag = false;
		}
		else if (check_draw()) {
			MessageBox(GetHWnd(), _T("Draw!"), _T("Game Over"), MB_OK);
			flag = false;
		}

		DWORD end_time = GetTickCount();
		DWORD elapsed_time = end_time - start_time;

		if(elapsed_time < 1000 / 60) {
			Sleep(1000 / 60 - elapsed_time);
		}
	}

	EndBatchDraw();

	return 0;
}