#include "init.h"


int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    All_Init();

    // Main loop
    while (!quit) {
        Events_Handle();
        
        Grid_Update();

        if(timer == 0)
        {
            Screen_Clear();

            Screen_Draw();
        }
        
        SDL_Delay(1);
    }

    // Clean up
    All_Quit();

    return 0;
}
