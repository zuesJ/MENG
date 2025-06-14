#ifndef MENG_H
#define MENG_H

typedef struct MENG_Score
{
    const char* MEI;
    char* TimeMap;
    char* ExpansionMap;
    char* MIDI;
} MENG_Score;

// Return 0 on success, < 0 on failure

/*Might return 0 on failure, but verovio will output an Error message*/
int MENG_Init(char* verovio_resource_path);
int MENG_Destroy();

int MENG_LoadScore(char* new_score);
int MENG_LoadScoreFromFile(char* path);

int MENG_RenderSVG();
int MENG_RenderTimemap();
int MENG_RenderExpansionMap();
int MENG_RenderMIDI();

#endif