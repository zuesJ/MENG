#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <verovio/c_wrapper.h>

#include "MENG.h"

typedef void* V_Toolkit;

bool init = false;

V_Toolkit tk = NULL;
MENG_Score score;

int MENG_Init(char* verovio_resource_path)
{
    tk = vrvToolkit_constructorResourcePath(verovio_resource_path);
    if (tk == NULL)
    {
        printf("MENG: Could not load resource path\n");
        return -1;
    }
    init = true;
    return 0;
}

int MENG_Destroy()
{
    vrvToolkit_destructor(tk);
    tk = NULL;
    init = false;
    return 0;
}

int MENG_LoadScore(char* new_score)
{
    bool success = vrvToolkit_loadData(tk, new_score);
    if (!success)
    {
        printf("Error in MENG_LoadScore\n");
        return -1;
    }
    score = (MENG_Score){.MEI = new_score};
    return 0;
}

int MENG_LoadScoreFromFile(char* path)
{
    bool success = vrvToolkit_loadFile(tk, path);
    if (!success)
    {
        printf("Error in MENG_LoadScoreFromFile\n");
        return -1;
    }
    score = (MENG_Score){vrvToolkit_getMEI(tk, "")};
    return 0;
}

int MENG_RenderSVG();

int MENG_RenderTimemap();

int MENG_RenderExpansionMap();

int MENG_RenderMIDI();
