#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <verovio/c_wrapper.h>

#include "MENG.h"

typedef void* V_Toolkit;

bool init = false;

V_Toolkit tk = NULL;
MENG_Score score;

int MENG_Init(char* verovio_resource_path)
{
    xmlInitParser();
    tk = vrvToolkit_constructorResourcePath(verovio_resource_path);
    if (tk == NULL)
    {
        printf("MENG: Could not load resource path\n");
        return -1;
    }
    init = true;
    return 0;
}

bool MENG_IsInit()
{
    return init;
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
    if (!init) return -1;
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
    if (!init) return -1;
    bool success = vrvToolkit_loadFile(tk, path);
    if (!success)
    {
        printf("Error in MENG_LoadScoreFromFile\n");
        return -1;
    }
    score = (MENG_Score){vrvToolkit_getMEI(tk, "")};
    return 0;
}

int MENG_RenderSVG()
{
    if (!init) return -1;
    return 0;
}

int MENG_RenderTimemap()
{
    if (!init) return -1;
    return 0;
}

int MENG_RenderExpansionMap()
{
    if (!init) return -1;
    return 0;
}

int MENG_RenderMIDI()
{
    if (!init) return -1;
    return 0;
}
