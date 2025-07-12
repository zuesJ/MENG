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
    xmlCleanupParser();
    vrvToolkit_destructor(tk);
    tk = NULL;
    init = false;
    return 0;
}