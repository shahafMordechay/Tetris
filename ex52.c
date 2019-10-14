#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define ERROR "Error in system call\n"
#define FRAME_SIZE 20
#define FRAME_HEIGHT 19
#define SHAPE_SIZE 3
#define START_X 9
#define START_Y 0

typedef struct Shape {
    short vertical;
    short x;
    short y;
}Shape;

char gameTable[FRAME_SIZE][FRAME_SIZE + 1];
Shape shape;
char key;
int isGameOver;

/**
 * create game borders on game table
 */
void createBorders() {
    // fill game table array with spaces
    int i = 0;
    for (; i < FRAME_SIZE; ++i) {
        memset(gameTable[i], ' ', FRAME_SIZE + 1);
    }

    // draw frame
    i = 0;
    for (; i < FRAME_HEIGHT; ++i) {
        gameTable[i][0] = gameTable[i][FRAME_SIZE - 1] = '*';
        gameTable[i][FRAME_SIZE] = '\n';
    }

    i = 0;
    for (; i < FRAME_SIZE; ++i) {
        gameTable[FRAME_HEIGHT][i] = '*';
    }
    gameTable[FRAME_HEIGHT][FRAME_SIZE] = '\n';
}

/**
 * draw one game frame
 */
void drawGameTable() {
    int i = 0, j = 0;
    for (; i < FRAME_SIZE; ++i) {
	    j = 0;
        for (; j < FRAME_SIZE + 1; ++j) {
            printf("%c", gameTable[i][j]);
        }
    }
}

/**
 * create new game shape
 */
void createShape() {
    shape.vertical = 1;
    shape.x = START_X;
    shape.y = START_Y;
}

/**
 *
 *
 * @return
 */
int drawShape() {
    gameTable[shape.y][shape.x] = '-';
    if (shape.vertical) {
        gameTable[shape.y + 1][shape.x] = '-';
        gameTable[shape.y + 2][shape.x] = '-';
    } else {
        gameTable[shape.y][shape.x + 1] = '-';
        gameTable[shape.y][shape.x + 2] = '-';
    }
}

/**
 * erase game shape
 */
void eraseShape() {
    if (shape.vertical) {
        int i = shape.y;
        for (; i < shape.y + SHAPE_SIZE; ++i) {
            gameTable[i][shape.x] = ' ';
        }
    } else {
        int i = shape.x;
        for (; i < shape.x + SHAPE_SIZE; ++i) {
            gameTable[shape.y][i] = ' ';
        }
    }
}

/**
 * detect shape collision
 *
 * @return collision direction or 0 if no collision
 */
char detectCollision() {

    // detect collision from below
    if ((shape.vertical && gameTable[shape.y + SHAPE_SIZE][shape.x] != ' ')
        || (!shape.vertical && (gameTable[shape.y + 1][shape.x] != ' '
                                 || gameTable[shape.y + 1][shape.x + 1] != ' '
                                 || gameTable[shape.y + 1][shape.x + 2] != ' '))) {
        return 'd';
    }

    // detect collision from right
    if ((!shape.vertical && gameTable[shape.y][shape.x + SHAPE_SIZE] != ' ')
        || (shape.vertical && (gameTable[shape.y][shape.x + 1] != ' '
                                || gameTable[shape.y + 1][shape.x + 1] != ' '
                                || gameTable[shape.y + 2][shape.x + 1] != ' '))) {
        return 'r';
    }

    // detect collision from left
    if ((!shape.vertical && gameTable[shape.y][shape.x - 1] != ' ')
        || (shape.vertical && (gameTable[shape.y][shape.x - 1] != ' '
                                || gameTable[shape.y + 1][shape.x - 1] != ' '
                                || gameTable[shape.y + 2][shape.x - 1] != ' '))) {
        return 'l';
    }

    return 0;
}

/**
 * detect if given row is full
 *
 * @param row - the row to check
 * @return 0 if row is NOT full, 1 otherwise
 */
int isRowFull(int row) {
    int i = 1;
    for (; i < FRAME_SIZE - 1; ++i) {
        if (gameTable[row][i] != '-') {
            return 0;
        }
    }

    return 1;
}

/**
 * delete given raw and update game table accordingly
 *
 * @param row - the row to delete
 */
void deleteRow(int row) {
    eraseShape();

    int col = 0;
    for (; row > 0; row--) {
        col = 0;
        for (; col < FRAME_SIZE; ++col) {
            gameTable[row][col] = gameTable[row - 1][col];
        }
    }

    int i = 0;
    for (; i < FRAME_SIZE; ++i) {
        gameTable[0][i] = ' ';
    }

    drawShape();
}

/**
 * delete game full rows and update the game accordingly.
 * return 0 if game over, otherwise return 1.
 *
 * @return 1 if succeeded, 0 if row 0 is full
 */
int handleFullRows() {
    if (isRowFull(0)) {
        return 0;
    }

    int i = FRAME_HEIGHT;
    for (; i > 0; --i) {
        if (isRowFull(i)) {
            deleteRow(i);
        }
    }

    return 1;
}

/**
 * move shape to the right
 */
void moveRight() {
    if (detectCollision() == 'r') {
        return;
    }

    eraseShape();
    shape.x++;
}

/**
 * move shape to the left
 */
void moveLeft() {
    if (detectCollision() == 'l') {
        return;
    }

    eraseShape();
    shape.x--;
}

/**
 * move shape down
 */
void moveDown() {
    if (detectCollision() == 'd') {
        return;
    }

    eraseShape();
    shape.y++;
}

/**
 * rotate shape
 */
void rotate() {
    short x = shape.x, y = shape.y;

    // can't rotate from vertical to horizontal
    if (shape.vertical && (gameTable[y][x + 1] != ' '
                            || gameTable[y + 1][x - 1] != ' '
                            || gameTable[y + 1][x + 1] != ' '
                            || gameTable[y + 2][x - 1] != ' ')) {
        return;
    }

    // can't rotate from horizontal to vertical
    if (!shape.vertical && (gameTable[y - 1][x] != ' '
                             || gameTable[y + 1][x + 1] != ' '
                             || gameTable[y - 1][x + 1] != ' '
                             || gameTable[y + 1][x + 2] != ' ')) {
        return;
    }

    // rotate
    eraseShape();
    shape.vertical = (short) ((shape.vertical + 1) % 2);
}

/**
 * move the shape to the direction given by parameter 'key'
 * @param key
 */
void move(char key) {
    switch (key) {
        case 'a':
            moveLeft();
            break;
        case 'd':
            moveRight();
            break;
        case 's':
            moveDown();
            break;
        case 'w':
            rotate();
        default:
            break;
    }

    drawShape();
}

/**
 * move shape one step down
 * @return 1 if succeeded, ptherwise return 0.
 */
int updateShape() {

    // will collide if moved down
    if (detectCollision() == 'd') {
        return 0;
    }

    moveDown();

    return 1;
}

/**
 * initialize the game
 */
void initialize() {
    createBorders();
    createShape();
    key = '\0';
}

/**
 * read input and move shape accordingly
 *
 * @param sig - system signal
 */
void signal_hand (int sig)
{
    signal(SIGUSR1,signal_hand);

    if (read(0, &key, 1) < 0) {
        write(2, ERROR, sizeof(ERROR));
        _exit(1);
    }

    move(key);

    if (key == 'r' && isGameOver == 1) {
        isGameOver = 0;
        system("clear");
        initialize();
        alarm(1);
    }
}

void gameOver() {
    isGameOver = 1;
    system("clear");
    printf("GAME OVER\nPress r to play again\nPress q to quit\n");
}

/**
 * update game frame on alarm signal
 *
 * @param sig - system alarm signal
 */
void alarm_hand (int sig)
{
    if (!isGameOver) {
        signal(SIGALRM, alarm_hand);
        system("clear");
        handleFullRows();
        drawShape();
        drawGameTable();

        if (updateShape() == 0) {
            if (gameTable[2][START_X] == '-') {
                gameOver();
            }
            createShape();
        }
        alarm(1);
    }
}

int main() {
    isGameOver = 0;
    initialize();

    signal(SIGUSR2,signal_hand);
    signal(SIGALRM,alarm_hand);
    alarm(1);
    while (key != 'q');

    return 0;
}