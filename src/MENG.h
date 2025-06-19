#ifndef MENG_H
#define MENG_H

#include <stdbool.h>

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
bool MENG_IsInit();
int MENG_Destroy();

int MENG_LoadScore(char* new_score);
int MENG_LoadScoreFromFile(char* path);

int MENG_RenderSVG();
int MENG_RenderTimemap();
int MENG_RenderExpansionMap();
int MENG_RenderMIDI();

// <------MEI Structure------>
typedef struct MENG_MEI MENG_MEI;

    typedef struct MENG_MeiHead MENG_MeiHead;
        typedef struct MENG_FileDesc MENG_FileDesc;
            typedef struct MENG_TitleStmt MENG_TitleStmt;
                typedef struct MENG_RespStmt MENG_RespStmt;
                typedef enum MENG_Role MENG_Role;
                typedef struct MENG_PersName MENG_PersName;
                typedef struct MENG_CorpName MENG_CorpName;
            typedef struct MENG_PubStmt MENG_PubStmt;
        typedef struct MENG_EncodingDesc MENG_EncodingDesc;
            typedef struct MENG_ProjectDesc MENG_ProjectDesc;
        typedef struct MENG_RevisionDesc MENG_RevisionDesc;
            typedef struct MENG_Change MENG_Change;

    typedef struct MENG_Music MENG_Music;

// <------MEI Methods------>
int MENG_LoadMEIFile(char* file);
void MENG_PrintMEI();
int MENG_SaveMEIFile(MENG_MEI mei, char* file);

// <------MEI------>
typedef struct MENG_Music
{
    int unused;
} MENG_Music;

typedef struct MENG_RespStmt
{
    MENG_PersName** persName;
    unsigned int persName_count;
    MENG_CorpName** corpName;
    unsigned int corpName_count;
} MENG_RespStmt;

typedef struct MENG_TitleStmt
{
    char* title;
    MENG_RespStmt respStmt;
} MENG_TitleStmt;

typedef struct MENG_PubStmt
{
    char* publisher;
    char* pubPlace;
    char* date;
    char* availability;
} MENG_PubStmt;

typedef struct MENG_FileDesc
{
    MENG_TitleStmt titleStmt;
    MENG_PubStmt pubStm;
} MENG_FileDesc;

typedef struct MENG_ProjectDesc
{
    char* desc;
} MENG_ProjectDesc;

typedef struct MENG_EncodingDesc
{
    MENG_ProjectDesc projectDesc;
} MENG_EncodingDesc;

typedef struct MENG_RevisionDesc
{
    MENG_Change** changes;
    unsigned int changes_count;
} MENG_RevisionDesc;

typedef struct MENG_MeiHead
{
    MENG_FileDesc fileDesc;
    MENG_EncodingDesc encodingDesc;
    MENG_RevisionDesc revisionDesc;
} MENG_MeiHead;

typedef struct MENG_MEI
{
    MENG_MeiHead meiHead;
    MENG_Music music;
} MENG_MEI;

typedef enum MENG_Role
{
    MENG_NOTDEFINED = -1,       // MEI format:
    MENG_ROLE_AUTHOR = 0,       // author
    MENG_ROLE_COMPOSER = 1,     // composer
    MENG_ROLE_ARRANGER = 2,     // arranger
    MENG_ROLE_EDITOR = 3,       // editor
    MENG_ROLE_CONTRIBUTOR = 4,  // contributor
    MENG_ROLE_FUNDER = 5,       // funder
    MENG_ROLE_LYRICIST = 6,     // lyricist
    MENG_ROLE_LIBRETTIST = 7,   // librettist
    MENG_ROLE_SPONSOR = 8       // sponsor
} MENG_Role;

typedef struct MENG_PersName
{
    char* name;
    MENG_Role role;
} MENG_PersName;

typedef struct MENG_CorpName
{
    char* name;
    MENG_Role role;
} MENG_CorpName;

typedef struct MENG_Change
{
    unsigned int n;
    char* isodate;
    MENG_RespStmt respStmt;
    char* changeDesc;
} MENG_Change;

#endif