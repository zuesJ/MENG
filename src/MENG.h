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

// <------MEI Methods------>
int MENG_LoadMEIFile(char* file);
int MENG_SaveMEIFile(MENG_MEI mei, char* file);



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
            typedef struct MENG_ProjectDesc;
        typedef struct MENG_RevisionDesc MENG_RevisionDesc;
            typedef struct MENG_Change MENG_Change;

    typedef struct MENG_Music MENG_Music;

// <------MEI------>
typedef struct MENG_MEI
{
    MENG_MeiHead meiHead;
    MENG_Music music;
} MENG_MEI;

    // <------MeiHead------>
    typedef struct MENG_MeiHead
    {
        MENG_FileDesc fileDesc;
        MENG_EncodingDesc encodingDesc;
        MENG_RevisionDesc revisionDesc;
    } MENG_MeiHead;

        // <------FileDesc------>
        typedef struct MENG_FileDesc
        {
            MENG_TitleStmt titleStmt;
            MENG_PubStmt pubStm;
        } MENG_FileDesc;

            // <------MENG_TitleStmt------>
            typedef struct MENG_TitleStmt
            {
                char* title;
                MENG_RespStmt respStmt;
            } MENG_TitleStmt;

                typedef struct MENG_RespStmt
                {
                    MENG_PersName** persName;
                    unsigned int persName_count;
                    MENG_CorpName** corpName;
                    unsigned int corp_count;
                } MENG_RespStmt;

                    typedef enum MENG_Role
                    {
                        MENG_ROLE_AUTHOR,
                        MENG_ROLE_COMPOSER,
                        MENG_ROLE_ARRANGER,
                        MENG_ROLE_EDITOR,
                        MENG_ROLE_CONTRIBUTOR,
                        MENG_ROLE_FUNDER,
                        MENG_ROLE_LYRICIST,
                        MENG_ROLE_LIBRETTIST,
                        MENG_ROLE_SPONSOR
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

            // <------MENG_PubStmt------>
            typedef struct MENG_PubStmt
            {
                char* publisher;
                char* pubPlace;
                char* date;
                char* availability;
            } MENG_PubStmt;
        
        // <------EncodingDesc------>
        typedef struct MENG_EncodingDesc
        {
            MENG_ProjectDesc projectDesc;
            MENG_AppInfo appInfo;
        } MENG_EncodingDesc;

            typedef struct MENG_ProjectDesc
            {
                char* desc;
            } MENG_ProjectDesc;

        // <------RevisionDesc------>
        typedef struct MENG_RevisionDesc
        {
            MENG_Change** changes;
        } MENG_RevisionDesc;

            typedef struct MENG_Change
            {
                char* date;
                MENG_RespStmt respStmt;
                char* changeDesc;
            } MENG_Change;
    // <------Music------>


#endif