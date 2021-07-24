

#include <iostream>
#include <Windows.h> //Need this to write to screen buffer directly instead of couting which takes longer
#include <math.h> // used for angles, sinf(), cosf()
#include <chrono>
#include <vector>
#include <algorithm>

int nScreenWidth = 120; //int of screen width, columns
int nScreenHeight = 40; // int of screen height, rows

// Aerial view
float fPlayerXpos = 14.0f; // player x position in float because its less clunky
float fPlayerYpos = 14.0f; // player y position in float because its less clunky
float fPlayerT = 0.0f; // player facing angle( theta ) in float because its less clunky

int nMapHeight = 16; // Map height, used for 2d array
int nMapWidth = 16; // Map width

float fFov = 3.141549 /4; // FOV field of view cone
float fDepth = 16.0f;

bool gameRunning = true;


int main() 
{
    //By using screen buffers, the screen won't flicker and it will be smoother and faster than cout as it is more than one screen, its screens other than the active screen
    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight]; // 2D array of the screen, unicode wchar_t, screen buffer
    // https://docs.microsoft.com/en-us/windows/console/console-screen-buffers
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); // https://docs.microsoft.com/en-us/windows/console/createconsolescreenbuffer
    //Handle is a type that is an address that points to a windows item, like a button https://en.wikibooks.org/wiki/Windows_Programming/Handles_and_Data_Types#HANDLE
    SetConsoleActiveScreenBuffer(hConsole); // SETS THE BUFFER TO THE hCOnsole we made ( the active screen we want)
    DWORD dwBytesWritten = 0;

    std::wstring map; // wstring because its for unicode, error at first because i did not add std:: namespace

    // MAP IS SIDEWAYS THE TOP RIGHT CORNER IS BOTTOM LEFT, TOP LEFT IS TOP LEFT, the bottom right is bottom right and the bottom left is top right
    map += L"################"; // trying to add boundaries for map but it is saying it has no storage class or type sciefier
    map += L"#..............#"; // Fixed it by putting it into the function, had it outside the main function
    map += L"#...........X..#"; // 16 x 16 map
    map += L"#..............#"; // (column selection) changed the middle # symbols by highlighting square text then replacing with .'s
    map += L"#.......########"; // https://stackoverflow.com/questions/43502589/vscode-column-selection-with-keyboard
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"######.........#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#........#######";
    map += L"#..............#";
    map += L"################";



    //auto type is for complicated types, local lifetime, uses the type of the initialization
    // it guarantees no conversion if changed
    // automatically chooses a type
    // https://docs.microsoft.com/en-us/cpp/cpp/auto-cpp?view=msvc-160
    // 
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now(); 
    // there are 2 time points to measure a duration of time

    while (gameRunning) 
    { // Game loop

        tp2 = std::chrono::system_clock::now(); // for each game loop, grab the current system time
        std::chrono::duration<float> elapsedTime = tp2- tp1; // calculate the current system - the last system time
        tp1 = tp2; // make the previous system time to the current system time, update
        float fElapsedTime = elapsedTime.count(); // turns elapsed time into floating point, so its less clunky 

        // Controls
        // Handle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000){ //gets input from keyboard and the 0x8000 is 1000000... in binary and it shows a high bit set so if the key is down, then its true
            fPlayerT -= 1.0f * fElapsedTime; // changes the direction the player is facing so makes the player spin left 
        } // first problem: it moves too fast, so we must add timings to the code as external sources can change how fast it runs
        // we will use chrono library to time, makes it so much smoother 
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000){
            fPlayerT += 1.0f * fElapsedTime; // spins right
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000){
            fPlayerXpos += sinf(fPlayerT) * 5.0f * fElapsedTime; // moves forward using the unit vector, increasing its magnitude
            fPlayerYpos += cosf(fPlayerT) * 5.0f * fElapsedTime; // fElapsed time to make it smoother again
            // collision detection for w and s because they can move into walls
            // turn the player position into integers so u can check with map 2d array, if it is a wall '#'
            // undo the move you just did
            if (map[(int)fPlayerXpos * nMapWidth + (int)fPlayerYpos] == '#')
            {
                fPlayerXpos -= sinf(fPlayerT) * 5.0f * fElapsedTime; // moves backward using the unit vector, increasing its magnitude
                fPlayerYpos -= cosf(fPlayerT) * 5.0f * fElapsedTime;
            }
            if (map[(int)fPlayerXpos * nMapWidth + (int)fPlayerYpos] == 'X')
            {
                gameRunning = false;

            }
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000){
            fPlayerXpos -= sinf(fPlayerT) * 5.0f * fElapsedTime; // moves backward using the unit vector, increasing its magnitude
            fPlayerYpos -= cosf(fPlayerT) * 5.0f * fElapsedTime; // fElapsed time to make it smoother again
            // collision detection for w and s because they can move into walls
            // turn the player position into integers so u can check with map 2d array, if it is a wall '#'
            // undo the move you just did
            if (map[(int)fPlayerXpos * nMapWidth + (int)fPlayerYpos] == '#')
            {
                fPlayerXpos += sinf(fPlayerT) * 5.0f * fElapsedTime; // moves forward using the unit vector, increasing its magnitude
                fPlayerYpos += cosf(fPlayerT) * 5.0f * fElapsedTime;
            }
            if (map[(int)fPlayerXpos * nMapWidth + (int)fPlayerYpos] == 'X')
            {
                gameRunning = false;

            }
        }

        
        for (int x = 0; x < nScreenWidth; x++ )
        {
            
            float fRayAngle = (fPlayerT - fFov / 2.0f) + ((float)x / (float) nScreenWidth) * fFov; 
            //ray angle uses fov divided by 2 because player angle is in the middle of the fov then adds the rest of the fov

            float fDistanceToWall = 0.0f;
            // distance from the player to wall, used for the ray angle
            bool wallHit = false; //boolean value if the the ray has hit a wall
            bool boundaryHit = false; // boolean value if the edge of a cell is hit

            float fEyeX = sinf(fRayAngle); // these are the angles that the player is looking towards unit vector
            float fEyeY = cosf(fRayAngle); // unit vector used to specify the directoin the player is facing, using sin and cos of an angle

            while(!wallHit && fDistanceToWall < fDepth) // Depth is created here in case a wall is never hit
            {
                fDistanceToWall += .01f; //the ray increments til it hits the wall


                // With the unit vector created above, this creates a line 
                // extended until collision with a wall
                int nTestX = (int)(fPlayerXpos + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerYpos + fEyeY * fDistanceToWall);

                // if the line created with the unit vector goes out of bounds, out of the map 
                // 
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapWidth) 
                {
                    wallHit = true;
                    fDistanceToWall = fDepth; //when it is out of bounds, make the distance to the walls, 
                }
                else // if in bound and line hits a wall
                {
                    if (map[nTestX * nMapWidth + nTestY] == '#') 
                    {  // x coord * width + y coord why is it x * wdith and not y * width
                        
                        wallHit = true;
                        // once wall is hit we need to check where the edge of a cell is
                        // checking where the corners are based on the angle of the corner to the ray

                        std::vector<std::pair<float, float>> p; //distance, dot , vector to accumulate the 4 corners

                        for (int tx = 0; tx < 2; tx++)
                        {
                            for (int ty = 0; ty < 2; ty++) 
                            {
                                float vy = (float)nTestY + ty - fPlayerYpos;  // uses nTestY because its an integer to get the exact corner of the cell
                                float vx = (float)nTestX + tx - fPlayerXpos; // another vector 
                                float d = sqrt(vx*vx + vy*vy); // distance/magnitude of the vector
                                float dot = (fEyeX  * vx/d) + (fEyeY * vy/d); // dot product of the vector 
                                // representation of the angle of the ray hitting the wall
                                p.push_back(std::make_pair(d, dot)); // push it onto the vector

                            }
                        //sorts the vector from the beginniging, to the end, with a lambda function as sort function
                        // lambda takes a type of pair, sorts from closest point to furthest, 
                        // simple boolean test, one smaller than the other, using distance 
                            std::sort(p.begin(), p.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) { return left.first < right.first;});

                            float fBound = .01;
                            //use the inverse cosign of the dot product to find the angle between the two rays
                            // the ray thats hitting the wall and the angle towards the corner
                            // fBound is just a small angle to see if its close enough to the corner
                            if (acos(p.at(0).second) <= fBound) boundaryHit = true;
                            if (acos(p.at(1).second) <= fBound) boundaryHit = true;
                            
                        }



                    }
                     
                }

            }
            int nCeiling = (float)(nScreenHeight /2.0) - nScreenHeight / ((float)fDistanceToWall); 
            // The ceiling length is the midpoint of the screen - by the screenheight in proportion to the distance of the wall
            // if distance to wall gets larger, the height of the ceiling gets larger as ur farther away

            int nFloor  = nScreenHeight - nCeiling;
            // floor  is the rest of the screen, the screen - ceiling height
            
            short nShade = 0x2591;
            short floorShade = 0;
            if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588; // shading is super dark if close 
            // closer ones first
            else if (fDistanceToWall <= fDepth /3.0f) nShade = 0x2593; // lighter as its further
            else if (fDistanceToWall <= fDepth /2.0f) nShade = 0x2592; // lighter
            else if (fDistanceToWall <= fDepth) nShade = 0x2591; // lighter
            else nShade = ' '; //lightest theres nothing

            if (boundaryHit) nShade = ' ';

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling) // THen this is the ceiling, less than ceiling
                {
                    screen[y*nScreenWidth + x] = ' ';
                }
                else if (y > nCeiling && y <= nFloor) { // greater than ceiling and less than floor, then this is a wall
                    screen[y*nScreenWidth + x] = nShade; // just shading wall with one hash doesnt add enough perspective/depth 
                                                        // created a nShade variable to shade in the wall
                }
                //else if( y > nCeiling && y > (nFloor*1.5)) { // closer floor
                    // this is the floor, 
                    //screen[y*nScreenWidth + x] = 'x';
                //}
                else // this is the floor
                {
                    // variable to find the distance of the floor, created with proportion of the screen height, higher height = farther
                    float b = 1.0f - (((float) y - nScreenHeight/ 2.0f) / ((float) nScreenHeight/2.0f)); // 

                    if (b <.25)  floorShade = '#'; // the closest flooring
                    else if (b <.5) floorShade = 'x';
                    else if (b <.75) floorShade = '-';
                    else if (b <.9) floorShade = '.';
                    else floorShade = ' '; // farthest floor

                    screen[y*nScreenWidth + x] =floorShade;
                }
                
            }

        }
        swprintf(screen, 40, L"X = %3.2f, Y = %3.2f, A = %3.2f, FPS = %3.2f", fPlayerYpos, fPlayerXpos, fPlayerT, 1.0f/fElapsedTime);
        // stores as a string into screen, 40 is the number of characters, the last one is the string, these stats have no coords os its dispalyed at 0,0

        // Display map, go thorugh map coords and then directly put them into screen buffer, 16x16
        for (int nx = 0; nx < nMapWidth; nx++) 
        {
            for (int ny = 0; ny < nMapHeight; ny++) 
            {
                screen[(ny+1)* nScreenWidth + nx] = map[ny*nMapWidth+nx];
            }
        }
        screen[((int)fPlayerXpos+1)*nScreenWidth + (int)fPlayerYpos] = 'P';

        screen[nScreenWidth * nScreenHeight - 1] = '\0'; // Sets teh end of the array or it won't stop
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);
        // ^ writes to the screen, it has the handle(address) for the screen buffer, the 2d array of the screen thats going to be written, the dimensions, the location (top left) , and the amount of bytes thats going to be written
        // Console must have the generic_write in permissions
        // https://docs.microsoft.com/en-us/windows/console/writeconsoleoutputcharacter
    }
    while(!gameRunning) 
    {
        wchar_t *screen1 = new wchar_t[nScreenWidth*nScreenHeight];
        screen1[0] = 'G';
        screen1[1] = 'a';
        screen1[2] = 'm';
        screen1[3] = 'e';
        screen1[4] = ' ';
        screen1[5] = 'O';
        screen1[6] = 'v';
        screen1[7] = 'e';
        screen1[8] = 'r';


        screen1[nScreenWidth * nScreenHeight - 1] = '\0'; // Sets teh end of the array or it won't stop
        WriteConsoleOutputCharacterW(hConsole, screen1, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);


    }

    

    return 0;
}  