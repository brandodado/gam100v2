//---------------------------------------------------------
// file:	main.c
// author:	[MUHAMMAD AFIF JAZIMIN BIN JOHARI]
// email:	[m.binjohari]
//
// brief:	Main entry point for the sample project
//			of the CProcessing library
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"
#include "mainmenu.h"
#include <time.h>
#include <stdlib.h>

int main(void)
{
    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Set a safe window size first
    CP_System_SetWindowSize(1280, 720);

    // Set the initial game state to the main menu
    CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);

    // Run the engine (no arguments)
    CP_Engine_Run(60);

    return 0;
}