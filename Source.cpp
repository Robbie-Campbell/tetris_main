#include <iostream>
#include <thread>
#include <vector>
#include <Windows.h>
using namespace std;
#include <stdio.h>


// Defines the width and height of the game screen
int nScreenWidth = 60;
int nScreenHeight = 30;

// Creates an array for all of the tetris shapes
wstring tetromino[7];

// Creates the boundarys for the game.
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;


// A function which rotates items in the tetromino array.
int Rotate(int px, int py, int r)
{
// circles through the different degrees
    switch (r % 4)
    {
    case 0: return py * 4 + px; // O deg
    case 1: return 12 + py - (px * 4); // 90 deg
    case 2: return 15 - (py * 4) - px; // 180 deg
    case 3: return 3 - py + (px * 4); // 270 deg
    }

    return 0;
}

// The function to see if a piece can access a space on the game screen
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{

    // all field cells over 0 are occupied
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
        {

            // Get index into place
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            // All of the conditions to check if a piece is can fit.
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
                    {
                        return false; // fail on first hit
                    }
                }
            }
        }

    return true;
}

int main()
{
    // All different tetris shapes
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"....");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");


    pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field buffer
    for (int x = 0; x < nFieldWidth; x++) // Board Boundary
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    // Create Screen Buffer
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
    
    // Checks if any moves are possible
    bool bGameOver = false;

    // Keeps track of each in use piece on the board
    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    // Variables to stop infinite rotation
    bool bKey[4];
    bool bRotateHold = false;

    // Conditions for the game states.
    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

// Keeps track of lines.
    vector<int> vLines;

    // The game itself
    while (!bGameOver)
    {
        // GAME TIMING ======================

        // The refresh speed of the game.
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        // INPUT ============================

        // Defines the different user keys
        for (int k = 0; k < 4; k++)
            bKey[k] = (0x8000 && GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
        // GAME LOGIC =======================

        // If possible, allows the user to move the piece left, right and down
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        // Allows the user to rotate a piece (and not rotate on holding down the button)
        if (bKey[3])
        {
            nCurrentRotation += (!bRotateHold && bKey[3] && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
        {
            bRotateHold = false;
        }
        
        // Moves the piece down periodically
        if (bForceDown)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
            {
                nCurrentY++;
            }
            else
            {
                //Lock the current piece in the field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                // Keeps a track of the pieces which have been put down
                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                // Check for lines
                for (int py = 0; py < 4; py++)
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine)
                        {
                            // Remove Line, set to =
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;

                            // Moves the lines down
                            vLines.push_back(nCurrentY + py);
                        }
                    }

                // Increments score and rewards riskier plays
                nScore += 25;
                if (!vLines.empty()) nScore +=(1 << vLines.size()) *100;

                // Choose Next Piece
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentPiece = rand() % 7;

                // Game over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }
        //RENDER OUTPUT =====================
        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

        // Draw a current piece
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

        // Draw the score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        // Tells the computer what to do if a full line is input
        if (!vLines.empty())
        {
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
                this_thread::sleep_for(400ms); // Satisfying delay for each line

            for (auto &v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = v; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }

            // Moves the pieces down
            vLines.clear();
        }

        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
        
    }

    // Game over screen

    CloseHandle(hConsole);
    cout << "GAME OVER, YOUR SCORE WAS: " << nScore << endl;
    system("pause");
    

    return 0;
}