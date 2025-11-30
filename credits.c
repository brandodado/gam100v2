#include "cprocessing.h"
#include <stdio.h>
#include <string.h>
#include "mainmenu.h"

// ------ Constants For Credits ------
#define HOLD_TIME           1.5
#define FADE_SPEED          150
#define MAX_SLIDES          6
#define MAX_LINE_PER_TEXTBLOCK  10
#define MAX_LINE_LENGTH     255
#define FILENAME            "Assets/credits.txt"
#define TEXTBLOCK_COUNT     5


// ------ Enums and Struct ------
typedef enum {
    GAME_AND_TEAM_NAME,
    TEAM_MEMBERS,
    PROFS,
    DIGIPEN_STRUCTURE,
    COPYRIGHT,
    THANK_YOU,
    END
}SlideIndex;

typedef enum {
    TEAM,
    PROGRAMMING,
    MATH,
    PRESIDENT,
    EXECUTIVE
}TextBlock;

typedef enum {
    FADE_IN,
    HOLD,
    FADE_OUT,
    NEXT_SLIDE,
    FINISH
}CreditFadeState;

typedef struct {
    char lines[MAX_LINE_PER_TEXTBLOCK][MAX_LINE_LENGTH];
    int line_cnt;
}CreditBlock;



// ------ Variables ------
CP_Font credit_font = NULL; 
SlideIndex current_slide;
CreditBlock text[TEXTBLOCK_COUNT]; // to hold textblock structs
CreditFadeState fade_state;

// variables to help with fade in logic
float alpha;
float timer;


// ------ Loading of the text blocks for the credits ------
void LoadCredit(const char* filename) {
    // open credit file
    FILE* fcredits = fopen(filename, "r");
    if (!fcredits) {
        // if we cannot load file we push notification to read that file loading unsucessful
        strcpy_s(text[0].lines[0], sizeof(text[0].lines[0]), "Error loading Credits file");
        return;
    }

    // declare varibale to hold lines, get the number of lines per textblock and the type of textblock 
    char buffer[MAX_LINE_LENGTH];
    int line_count = 0;
    TextBlock text_block = TEAM;

    while (fgets(buffer, MAX_LINE_LENGTH, fcredits)) {
        // get rid of breakline
        buffer[strcspn(buffer, "\n")] = '\0';
        buffer[strcspn(buffer, "\r")] = '\0';

        // textblock are seperated by break line in txt file
        if (strlen(buffer) == 0) {
            // set current line index to be the number of lines in the textblock
            text[text_block].line_cnt = line_count;
            //change to next textblock and reste the line_count
            text_block++;
            line_count = 0;
        }
        else if (line_count < MAX_LINE_PER_TEXTBLOCK) {
            // copy line into current textblock
            strcpy_s(text[text_block].lines[line_count], sizeof(text[text_block].lines[line_count]), buffer);
            line_count++;
        }
    }

    // capture the line count for the last Text Block
    if (line_count > 0 && line_count < MAX_LINE_PER_TEXTBLOCK) {
        text[text_block].line_cnt = line_count;
    }

    // close the file
    fclose(fcredits);
}


// ------ INIT ------
void Credits_Init(void)
{
    // load font if not loaded.
    if (credit_font == NULL) {
        credit_font = CP_Font_Load("Assets/Exo2-Regular.ttf");
    }
    current_slide = GAME_AND_TEAM_NAME; // set as 1st slide
    fade_state = FADE_IN; // set as 1st phase
    // init as 0 for fade in
    alpha = 0;
    timer = 0;

    LoadCredit(FILENAME);
}


// ------ UPDATE ------
void Credits_Update(void)
{
    // for frame dependent animation
    float dt = CP_System_GetDt();

    //fade in fade out logic
    switch (fade_state) {
    case FADE_IN:
        // increase transparency by speed scaled by delta time and clamp it to 0 and 255
        alpha += FADE_SPEED * dt;
        alpha = CP_Math_ClampFloat(alpha, 0, 255);
        // if reached max opacity move to next stage
        if (alpha == 255) {
            // set the hold timer
            timer = HOLD_TIME;
            fade_state++;
        }
        break;

    case HOLD:
        timer -= dt;
        timer = CP_Math_ClampFloat(timer, 0, 5);
        if (timer <= 0) {
            // timer hits 0 moves to fade out stage
            fade_state++;
        }
        break;

    case FADE_OUT:
        // decrease transparency by speed scaled by delta time and clamp it to 0 and 255
        alpha -= FADE_SPEED * dt;
        alpha = CP_Math_ClampFloat(alpha, 0, 255);
        if (alpha == 0) {
            // if fully transparent move to next stage
            fade_state++;
        }
        break;

    case NEXT_SLIDE:
        // increment slide to move to next slide
        current_slide++;
        // if current slide is the last move to finish state
        if (current_slide == END) {
            fade_state++;
        }
        else {
            // reset the fade status
            fade_state = FADE_IN;
        }
        break;

    case FINISH:
        // if finished move back to main menu
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
        break;
    }

    // clear background and set as black
    CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

    if ((int)current_slide >= 0 && (int)current_slide < MAX_SLIDES)
    {
        // get window dimentions
        float screen_h = (float)CP_System_GetWindowHeight();
        float screen_w = (float)CP_System_GetWindowWidth();

        // set font size and spacing between lines
        float header_size = 80.0f;
        float body_size = 50.0f;
        float line_spacing = 50.0f;

        // default position for header
        float header_x = screen_w / 2;
        float header_y = 150; // padding from the top;
        int header_padding = 100;
        float body_y = header_y + header_padding;

        // set font color to white to be seen and controled by the alpha value for transparency
        CP_Settings_Fill(CP_Color_Create(255, 255, 255, (int)alpha));
        // allign text to the middle
        CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
        // set text size as header first
        CP_Settings_TextSize(header_size);


        switch (current_slide) {
        case GAME_AND_TEAM_NAME:
            header_y = (screen_h - header_size - body_size - line_spacing) / 2; // push it more to the middle of the screen
            CP_Font_DrawText("HEXHAND", header_x, header_y);
            CP_Settings_TextSize(body_size);
            CP_Font_DrawText("by salty fish", header_x, header_y + header_padding);
            break;

        case TEAM_MEMBERS:
            // use a for loop to go through all the lines for the textblock for this slide
            for (int i = 0; i < text[TEAM].line_cnt; i++) {
                if (i > 0) {
                    // set text size to body after the header which is the first line
                    CP_Settings_TextSize(body_size);
                }
                // draw the line. we only add header padding after the header
                CP_Font_DrawText(text[TEAM].lines[i], header_x, header_y + (i * line_spacing) + ((i > 0 ? 1 : 0) * header_padding));
            }
            break;

        case PROFS:
            // Draw header
            CP_Font_DrawText("FACULTY AND ADVISORS", header_x, header_y);

            int textbox_padding = 15;

            // draw programmer faculty

            //left half of the split 
            float text_box_x = (screen_w / 2) / 2; // minus half of textbox width as textbox draws from top left
            for (int i = 0; i < text[PROGRAMMING].line_cnt; i++) {
                if (i == 0) {
                    // use body size text for sub header
                    CP_Settings_TextSize(body_size);
                }
                else {
                    // smaller text to accomadate all profs
                    CP_Settings_TextSize(body_size - 15);
                }
                CP_Font_DrawText(text[PROGRAMMING].lines[i], text_box_x, header_y + (i * line_spacing) + header_padding);
            }

            // right half of the split
            text_box_x += screen_w / 2; // add half the screen to mirror right side coords

            for (int i = 0; i < text[MATH].line_cnt; i++) {
                if (i == 0) {
                    // use body size text for sub header
                    CP_Settings_TextSize(body_size);
                }
                else {
                    // smaller text to accomadate all profs
                    CP_Settings_TextSize(body_size - 15);
                }
                CP_Font_DrawText(text[MATH].lines[i], text_box_x, header_y + (i * line_spacing) + header_padding);
            }


            break;

        case DIGIPEN_STRUCTURE:
            // special slide
            CP_Settings_TextSize(body_size);
            CP_Font_DrawText("Created at", header_x, header_y);
            // set body y level
            body_y = header_y + line_spacing;
            CP_Font_DrawText("DigiPen Institute of Technology Singapore", header_x, body_y);
            // increase body y level to draw below previous line
            body_y += body_size / 2 + line_spacing;


            for (int i = 0; i < text[PRESIDENT].line_cnt; i++) {
                if (i == 0) {
                    CP_Settings_TextSize(header_size);
                }
                else {
                    CP_Settings_TextSize(body_size);
                }

                body_y += (i * line_spacing);
                CP_Font_DrawText(text[PRESIDENT].lines[i], header_x, body_y);
            }

            body_y += body_size / 2 + line_spacing;

            for (int i = 0; i < text[EXECUTIVE].line_cnt; i++) {
                if (i == 0) {
                    CP_Settings_TextSize(header_size);
                }
                else {
                    CP_Settings_TextSize(body_size);
                }

                body_y += (i * line_spacing);
                float text_box_padding = 20;
                // use textbox the size of the screen for auto wrapping
                CP_Font_DrawTextBox(text[EXECUTIVE].lines[i], text_box_padding, body_y, screen_w - 2 * text_box_padding);
            }

            float text_size = 20;
            // digipen copyright at the bottom
            CP_Settings_TextSize(text_size);
            body_y = screen_h - line_spacing - 15; // set to draw near the bottom with 15 pixels padding at the bottom
            CP_Font_DrawText("WWW.DIGIPEN.EDU", header_x, body_y);
            body_y += text_size / 2 + 10;
            CP_Font_DrawText("All Content (c) DigiPen Institute of Technology. All Rights Reserved", header_x, body_y);
            break;

        case COPYRIGHT:
            CP_Font_DrawText("Copyright", header_x, header_y);
            CP_Settings_TextSize(body_size);
            CP_Font_DrawText("Kenny Sound Assets (c) Kenny.nl (2010 - 2025)", header_x, body_y);
            break;

        case THANK_YOU:
            // push to close to middle of the screen
            header_y = (screen_h) / 2;
            CP_Font_DrawText("THANK YOU FOR PLAYING", header_x, header_y);
            break;
        }
    }

    // early exit
    // set to always showing
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
    CP_Settings_TextSize(40.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Font_DrawText("Press ESC to SKIP the credits", 50, 50);

    // exit if player want to skip
    if (CP_Input_KeyTriggered(KEY_ESCAPE)) {
        CP_Engine_SetNextGameState(Main_Menu_Init, Main_Menu_Update, Main_Menu_Exit);
    }
}


// ------ EXIT ------
void Credits_Exit(void)
{
    // free font used
    CP_Font_Free(credit_font);
}