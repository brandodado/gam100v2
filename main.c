// Entry point for the application.
#include "cprocessing.h"
#include <stdio.h>
#include <time.h>
#include "mainmenu.h"
#include "card.h"
#include "intro.h"

// Main execution function. Sets up the window, seeds RNG, loads static data (catalogue),
// and starts the CProcessing engine with the Intro state. Returns 0 on success.
int main(void)
{
    // Seed the random number generator
    CP_Random_Seed((unsigned int)time(NULL));

    // Set a safe window size first
    CP_System_SetWindowSize(1280, 720);

    // Load card catalogue ONCE at startup
    catalogue_size = LoadCatalogue("Assets/cath.txt", catalogue, 50);
    if (catalogue_size == 0) {
        printf("WARNING: Failed to load card catalogue!\n");
    }

    // Set the initial game state to the main menu
    CP_Engine_SetNextGameState(Intro_Init, Intro_Update, Intro_Exit);

    // Run the engine (no arguments)
    CP_Engine_Run(60);

    return 0;
}