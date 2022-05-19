#include <iostream>
#include <thread>
#include <vector>
#include <windows.h> //HAS THE SCREEN BUFFER METHODS IN HERE, C HEADER
#define PIECE_WIDTH 4
#define OFFSET 2
using namespace std;

string tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18; // PLAYING FIELD DIMENSIONS
unsigned char* pField = nullptr; //PLAYING FIELD 1D ARRAY (CHAR IS BASICALLY 8 BIT INT)
//UNSIGNED CHAR = 0 TO 255

int nScreenWidth = 80;
int nScreenHeight = 30; //80 x 30 = DEFAULT CMD SCREEN DIMENSIONS, ALSO WORKS AS A 1D ARRAY

int rotate(int px, int py, int width, int angle) { //ROTATING AN ARRAY IN STEPS BY 90 DEGREES
    switch(angle % 4) {                            //BASICALLY ONLY CHANGES THE INDEXING METHOD
        case 0:
            return py * width + px; //0 DEG
        case 1:
            return ((width - 1) * width) + py - (px * width); //90 DEG
        case 2:
            return ((width * width) - 1) - (py * width) - px; //180 DEG
        case 3:
            return 3 - py + (px * width); //270 DEG
    }
    return 0;
}

bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {

    for (int px = 0; px < PIECE_WIDTH; px++) {
        for (int py = 0; py < PIECE_WIDTH; py++) {

            int pi = rotate(px, py, PIECE_WIDTH, nRotation); //PIECE INDEX
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px); //FIELD INDEX

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pi] == 'X' && pField[fi] != 0) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

int main() {

    //CREATE ASSETS - TETRIS BLOCKS
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    
    tetromino[1].append("..X.");
    tetromino[1].append(".XX.");
    tetromino[1].append(".X..");
    tetromino[1].append("....");
    
    tetromino[2].append(".X..");
    tetromino[2].append(".XX.");
    tetromino[2].append("..X.");
    tetromino[2].append("....");
    
    tetromino[3].append("....");
    tetromino[3].append(".XX.");
    tetromino[3].append(".XX.");
    tetromino[3].append("....");
    
    tetromino[4].append("..X.");
    tetromino[4].append(".XX.");
    tetromino[4].append("..X.");
    tetromino[4].append("....");
    
    tetromino[5].append("....");
    tetromino[5].append(".XX.");
    tetromino[5].append("..X.");
    tetromino[5].append("..X.");
    
    tetromino[6].append("....");
    tetromino[6].append(".XX.");
    tetromino[6].append(".X..");
    tetromino[6].append(".X..");

    pField = new unsigned char[nFieldWidth * nFieldHeight]; //INITIALIZING THE FIELD
    for(int x = 0; x < nFieldWidth; x++) {
        for(int y = 0; y < nFieldHeight; y++) {
            pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
        }
    }
    //SETTING EVERYTHING BUT THE TOP WALL TO A WALL, AND EVERYTHING ELSE TO FREE

    char* screen = new char[nScreenWidth*nScreenHeight];
    for (int i = 0; i < nScreenWidth*nScreenHeight; i++) {
        screen[i] = ' '; //INITIALIZES THE SCREEN AND MAKES IT EMPTY
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole); //MAIN SCREEN BUFFER CODE, CREATES IT (HANDLE IS KIND OF A POINTER)
    DWORD dwBytesWritten = 0; //ON 32 BITS, THIS IS A 64 BIT NUMBER, ON 64 BITS, THIS IS A 128 BIT NUMBER (DOUBLE THE WORD SIZE)

    bool bGameOver = false; //MAIN LOOP RUNNING VARIABLE

    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    vector<int> vLines;

    while(!bGameOver) { //ENTER MAIN LOOP

        //GAME TIMING
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        //INPUT
        for (int k = 0; k < 4; k++) {
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;
        }

        //GAME LOGIC
        nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        if (bKey[3]) {
            nCurrentRotation += (doesPieceFit(nCurrentPiece, nCurrentRotation+1, nCurrentX, nCurrentY) && !bRotateHold) ? 1 : 0;
            bRotateHold = true;
        }
        else bRotateHold = false;

        if (bForceDown) {
            if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                nCurrentY++;
            }
            else {

                for (int px = 0; px < PIECE_WIDTH; px++) {
                    for (int py = 0; py < PIECE_WIDTH; py++) {
                        if(tetromino[nCurrentPiece][rotate(px, py, PIECE_WIDTH, nCurrentRotation)] == 'X') {
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }

                nPieceCount++;
                if (nPieceCount % 10 == 0) {
                    if (nSpeed >= 10) nSpeed--;
                }

                for(int py = 0; py < PIECE_WIDTH; py++) {
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth -1; px++) {
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
                        }

                        if (bLine) {
                            for (int px = 1; px < nFieldWidth - 1; px++) {
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            }

                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }

                nScore += 25;
                if(!vLines.empty()) nScore += (1 << vLines.size()) * 100;

                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;
                
                bGameOver = !doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }

            nSpeedCounter = 0;
        }

        //RENDER OUTPUT
        //DRAW FIELD
        for(int x = 0; x < nFieldWidth; x++) {
            for(int y = 0; y < nFieldHeight; y++) {
                screen[(y + OFFSET)*nScreenWidth + (x + OFFSET)] = " ABCDEFG=#"[pField[y*nFieldWidth + x]];
                //ITERATE THROUGH THE pField AND SET THE SCREEN TO ONE OF THE 10 VALUES IN THE STRING
                //OFFSET BY 2 FOR X AND Y FROM THE TOP LEFT
            }
        }

        //DRAW CURRENT PIECE
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if(tetromino[nCurrentPiece][rotate(px, py, PIECE_WIDTH, nCurrentRotation)] == 'X') {
                    screen[(nCurrentY + py + OFFSET)*nScreenWidth + (nCurrentX + px + OFFSET)] = nCurrentPiece + 'A';
                }
            }
        }

        if (!vLines.empty()) {
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : vLines) {
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--) {
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    }
                    pField[px] = 0;
                }
            }

            vLines.clear();
        }

        sprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, "SCORE %8d", nScore);


        //DISPLAY FRAME - WRITE BUFFER TO SCREEN
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, {0, 0}, &dwBytesWritten);

    }

    CloseHandle(hConsole);
    cout << "GAME OVER! Score: " << nScore << endl;
    system("pause");

    return 0; //YOU KNOW WHAT THIS DOES, DON'T YOU?
}