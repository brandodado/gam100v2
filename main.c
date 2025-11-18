#include "cprocessing.h"
#include "mainmenu.h"
#include <time.h>
// #include <stdlib.h> // No longer needed

int main(void)
{
    // Seed the random number generator
    CP_Random_Seed((unsigned int)time(NULL));

    // Set a safe window size first
    CP_System_SetWindowSize(1280, 720);

    // Set the initial game state to the main menu
    CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);

    // Run the engine (no arguments)
    CP_Engine_Run(60);

    return 0;
}