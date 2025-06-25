#include "MENG.h"

#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

static xmlNode* __SearchNode(xmlNode* node, const xmlChar* node_name);
static int __LoadXmlNodes(char* file);
static MENG_Role __TranslateRoleByText(char* role_text);
static int __GetRespStmt(xmlNode* respStmt, MENG_PersName*** persName, unsigned int* persName_count, MENG_CorpName*** corpName, unsigned int* corpName_count);
static int __GetrevisionDesc(xmlNode* revisionDesc, MENG_Change*** changes, unsigned int* changes_count);

typedef struct MENG_MeiXml
{
    xmlDoc* mei_file;
    xmlNode* root;
    xmlNode* meiHead;
        xmlNode* fileDesc;
            xmlNode* titleStmt;
                xmlNode* title;
                xmlNode* respStmt;
            xmlNode* pubStmt;
                xmlNode* publisher;
                xmlNode* pubPlace;
                xmlNode* date;
                xmlNode* availability;
        xmlNode* encodingDesc;
            xmlNode* projectDesc;
                xmlNode* projectDesc_p;
        xmlNode* revisionDesc;
    xmlNode* music;
    xmlNode* body;
    xmlNode* mdiv;
    xmlNode* score;
        xmlNode* scoreDef;
            xmlNode* staffGrp;
                xmlNode* grpSym;
                xmlNode* label;
                xmlNode* labelAbbr;
                xmlNode* instrDef;
                xmlNode* staffDef1;
                    xmlNode* clef1;
                    xmlNode* keySig1;
                    xmlNode* meterSig1;
                xmlNode* staffDef2;
                    xmlNode* clef2;
                    xmlNode* keySig2;
                    xmlNode* meterSig2;

        xmlNode* section;
} MENG_MeiXml;

static MENG_MEI mei = {0};
static MENG_MeiXml xml = {0};

static int __LoadXmlNodes(char* file)
{
    xml.mei_file = xmlReadFile(file, NULL, 0);
    if (xml.mei_file == NULL) return -1;
    xml.root = xmlDocGetRootElement(xml.mei_file);
    if (xml.root == NULL) return -2;
    xml.meiHead = __SearchNode(xml.root, "meiHead");
    if (xml.meiHead == NULL) return -3;
    xml.fileDesc = __SearchNode(xml.meiHead, "fileDesc");
    if (xml.fileDesc == NULL) return -4;
    xml.titleStmt = __SearchNode(xml.fileDesc, "titleStmt");
    if (xml.titleStmt == NULL) return -5;
    xml.title = __SearchNode(xml.titleStmt, "title");
    if (xml.title == NULL) return -6;
    xml.respStmt = __SearchNode(xml.titleStmt, "respStmt");
    if (xml.respStmt == NULL) return -7;
    xml.pubStmt = __SearchNode(xml.fileDesc, "pubStmt");
    if (xml.pubStmt == NULL) return -8;
    xml.publisher = __SearchNode(xml.pubStmt, "publisher");
    if (xml.publisher == NULL) return -9;
    xml.pubPlace = __SearchNode(xml.pubStmt, "pubPlace");
    if (xml.pubPlace == NULL) return -10;
    xml.date = __SearchNode(xml.pubStmt, "date");
    if (xml.date == NULL) return -11;
    xml.availability = __SearchNode(xml.pubStmt, "availability");
    if (xml.availability == NULL) return -12;
    xml.encodingDesc = __SearchNode(xml.meiHead, "encodingDesc");
    if (xml.encodingDesc == NULL) return -13;
    xml.projectDesc = __SearchNode(xml.encodingDesc, "projectDesc");
    if (xml.projectDesc == NULL) return -14;
    xml.projectDesc_p = __SearchNode(xml.projectDesc, "p");
    if (xml.projectDesc_p == NULL) return -15;
    xml.revisionDesc = __SearchNode(xml.meiHead, "revisionDesc");
    if (xml.revisionDesc == NULL) return -16;
    xml.music = __SearchNode(xml.root , "music");
    if (xml.music == NULL) return -17;
    xml.body = __SearchNode(xml.music , "body");
    if (xml.body == NULL) return -18;
    xml.mdiv = __SearchNode(xml.body , "mdiv");
    if (xml.mdiv == NULL) return -19;
    xml.score = __SearchNode(xml.mdiv , "score");
    if (xml.score == NULL) return -20;
    xml.scoreDef = __SearchNode(xml.score , "scoreDef");
    if (xml.scoreDef == NULL) return -21;
    xml.staffGrp = __SearchNode(xml.scoreDef , "staffGrp");
    if (xml.staffGrp == NULL) return -22;
    xml.grpSym = __SearchNode(xml.staffGrp , "grpSym");
    if (xml.grpSym == NULL) return -23;
    xml.label = __SearchNode(xml.staffGrp , "label");
    if (xml.label == NULL) return -24;
    xml.labelAbbr = __SearchNode(xml.staffGrp , "labelAbbr");
    if (xml.labelAbbr == NULL) return -25;
    xml.instrDef = __SearchNode(xml.staffGrp , "instrDef");
    if (xml.instrDef == NULL) return -26;
    xml.section = __SearchNode(xml.score , "section");
    if (xml.section == NULL) return -27;
    bool foundanystaffdef = false;
    for (xmlNode* cur = xml.staffGrp->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"staffDef") == 0)
        {
            xmlChar* n_str = xmlGetProp(cur, (const xmlChar*)"n");
            if (xmlStrcmp(n_str, (const xmlChar*)"1") == 0)
            {
                xml.staffDef1 = cur;
                foundanystaffdef = true;
            }
            else if (xmlStrcmp(n_str, (const xmlChar*)"2") == 0)
            {
                xml.staffDef2 = cur;
                foundanystaffdef = true;
            }
            else
            {
                printf("[MENG Warning] MEI file has more than 2 staffs\n");
            }
            xmlFree(n_str);
        }
    }
    if (!foundanystaffdef) return -28;
    xml.clef1 = __SearchNode(xml.staffDef1, "clef");
    if (xml.clef1 == NULL) return -29;
    xml.keySig1 = __SearchNode(xml.staffDef1, "keySig");
    if (xml.keySig1 == NULL) return -30;
    xml.meterSig1 = __SearchNode(xml.staffDef1, "meterSig");
    if (xml.meterSig1 == NULL) return -31;
    xml.clef2 = __SearchNode(xml.staffDef2, "clef");
    if (xml.clef2 == NULL) return -32;
    xml.keySig2 = __SearchNode(xml.staffDef2, "keySig");
    if (xml.keySig2 == NULL) return -33;
    xml.meterSig2 = __SearchNode(xml.staffDef2, "meterSig");
    if (xml.meterSig2 == NULL) return -34;
    return 0;
}

int MENG_LoadMEIFile(char* file)
{
    if (!MENG_IsInit()) return -1;
    if (__LoadXmlNodes(file) < 0) return -2;

    xmlChar* titleText = xmlNodeGetContent(xml.title);
    mei.meiHead.fileDesc.titleStmt.title = strdup((char*)titleText);
    xmlFree(titleText);

    MENG_PersName** persName = NULL;
    unsigned int persName_count = 0;
    MENG_CorpName** corpName = NULL;
    unsigned int corpName_count = 0;

    if (__GetRespStmt(xml.respStmt, &persName, &persName_count, &corpName, &corpName_count) == -1) return -3;

    mei.meiHead.fileDesc.titleStmt.respStmt.persName = persName;
    mei.meiHead.fileDesc.titleStmt.respStmt.persName_count = persName_count;
    mei.meiHead.fileDesc.titleStmt.respStmt.corpName = corpName;
    mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count = corpName_count;

    xmlChar* publisher_text = xmlNodeGetContent(xml.publisher);
    xmlChar* pubPlace_text = xmlNodeGetContent(xml.pubPlace);
    xmlChar* date_text = xmlNodeGetContent(xml.date);
    xmlChar* availability_text = xmlNodeGetContent(xml.availability);
    if (publisher_text == NULL)
    {
        mei.meiHead.fileDesc.pubStm.publisher = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStm.publisher = strdup((char*)publisher_text);
    }

    if (pubPlace_text == NULL)
    {
        mei.meiHead.fileDesc.pubStm.pubPlace = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStm.pubPlace = strdup((char*)pubPlace_text);
    }

    if (date_text == NULL)
    {
        mei.meiHead.fileDesc.pubStm.date = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStm.date = strdup((char*)date_text);
    }

    if (availability_text == NULL)
    {
        mei.meiHead.fileDesc.pubStm.availability = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStm.availability = strdup((char*)availability_text);
    }
    xmlFree(publisher_text);
    xmlFree(pubPlace_text);
    xmlFree(date_text);
    xmlFree(availability_text);
    
    xmlChar* publisher_p_text = xmlNodeGetContent(xml.projectDesc_p);
    mei.meiHead.encodingDesc.projectDesc.desc = strdup((char*)publisher_p_text);
    xmlFree(publisher_p_text);

    unsigned int changes_count = 0;
    MENG_Change** changes = NULL;

    __GetrevisionDesc(xml.revisionDesc, &changes, &changes_count);
    
    mei.meiHead.revisionDesc.changes = changes;
    mei.meiHead.revisionDesc.changes_count = changes_count;

    xmlChar* barthru_text = xmlGetProp(xml.staffGrp, (const xmlChar*)"bar.thru");
    if (xmlStrcmp(barthru_text, (const xmlChar*)"true") == 0)
    {
        mei.music.scoreDef.barthru = true;
    }
    else
    {
        mei.music.scoreDef.barthru = false;
    }
    xmlFree(barthru_text);

    xmlChar* grpSym_text = xmlGetProp(xml.grpSym, (const xmlChar*)"symbol");
    mei.music.scoreDef.grpSym = strdup((char*)grpSym_text);
    xmlFree(grpSym_text);

    xmlChar* label_text = xmlNodeGetContent(xml.label);
    mei.music.scoreDef.label = strdup((char*)label_text);
    xmlFree(label_text);

    xmlChar* labelAbbr_text = xmlNodeGetContent(xml.labelAbbr);
    mei.music.scoreDef.labelAbbr = strdup((char*)labelAbbr_text);
    xmlFree(labelAbbr_text);

    xmlChar* midi_channel_text = xmlGetProp(xml.instrDef, (const xmlChar*)"midi.channel");
    xmlChar* midi_instrnum_text = xmlGetProp(xml.instrDef, (const xmlChar*)"midi.instrnum");
    xmlChar* midi_volume_text = xmlGetProp(xml.instrDef, (const xmlChar*)"midi.volume");

    mei.music.scoreDef.instrDef.midi_channel = (unsigned char)strtol((const char*)midi_channel_text, NULL, 10);
    mei.music.scoreDef.instrDef.midi_instrnum = (int)strtol((const char*)midi_instrnum_text, NULL, 10);
    mei.music.scoreDef.instrDef.midi_volume = (unsigned char)strtol((const char*)midi_volume_text, NULL, 10);

    xmlFree(midi_channel_text);
    xmlFree(midi_instrnum_text);
    xmlFree(midi_volume_text);

    xmlChar* n1_text = xmlGetProp(xml.staffDef1, (const xmlChar*)"n");
    xmlChar* lines1_text = xmlGetProp(xml.staffDef1, (const xmlChar*)"lines");
    mei.music.scoreDef.staffDef1.n = (unsigned int)strtol((const char*)n1_text, NULL, 10);
    mei.music.scoreDef.staffDef1.lines = (unsigned char)strtol((const char*)lines1_text, NULL, 10);
    xmlFree(n1_text);
    xmlFree(lines1_text);

    xmlChar* shape1_text = xmlGetProp(xml.clef1, (const xmlChar*)"shape");
    xmlChar* line1_text = xmlGetProp(xml.clef1, (const xmlChar*)"line");
    if (shape1_text == NULL || xmlStrlen(shape1_text) != 1) return -3;
    mei.music.scoreDef.staffDef1.clef.shape = (char)shape1_text[0];
    mei.music.scoreDef.staffDef1.clef.line = (unsigned char)strtol((const char*)line1_text, NULL, 10);
    xmlFree(shape1_text);
    xmlFree(line1_text);

    xmlChar* keySig1_text = xmlGetProp(xml.keySig1, (const xmlChar*)"sig");
    mei.music.scoreDef.staffDef1.keySig = strdup((char*)keySig1_text);
    xmlFree(keySig1_text);

    xmlChar* count1_text = xmlGetProp(xml.meterSig1, (const xmlChar*)"count");
    xmlChar* unit1_text = xmlGetProp(xml.meterSig1, (const xmlChar*)"unit");
    mei.music.scoreDef.staffDef1.meterSig.count = (unsigned char)strtol((const char*)count1_text, NULL, 10);
    mei.music.scoreDef.staffDef1.meterSig.unit = (unsigned char)strtol((const char*)unit1_text, NULL, 10);
    xmlFree(count1_text);
    xmlFree(unit1_text);

    xmlChar* n2_text = xmlGetProp(xml.staffDef2, (const xmlChar*)"n");
    xmlChar* lines2_text = xmlGetProp(xml.staffDef2, (const xmlChar*)"lines");
    mei.music.scoreDef.staffDef2.n = (unsigned int)strtol((const char*)n2_text, NULL, 10);
    mei.music.scoreDef.staffDef2.lines = (unsigned char)strtol((const char*)lines2_text, NULL, 10);
    xmlFree(n2_text);
    xmlFree(lines2_text);

    xmlChar* shape2_text = xmlGetProp(xml.clef2, (const xmlChar*)"shape");
    xmlChar* line2_text = xmlGetProp(xml.clef2, (const xmlChar*)"line");
    if (shape2_text == NULL || xmlStrlen(shape2_text) != 1) return -3;
    mei.music.scoreDef.staffDef2.clef.shape = (char)shape2_text[0];
    mei.music.scoreDef.staffDef2.clef.line = (unsigned char)strtol((const char*)line2_text, NULL, 10);
    xmlFree(shape2_text);
    xmlFree(line2_text);

    xmlChar* keySig2_text = xmlGetProp(xml.keySig2, (const xmlChar*)"sig");
    mei.music.scoreDef.staffDef2.keySig = strdup((char*)keySig2_text);
    xmlFree(keySig2_text);

    xmlChar* count2_text = xmlGetProp(xml.meterSig2, (const xmlChar*)"count");
    xmlChar* unit2_text = xmlGetProp(xml.meterSig2, (const xmlChar*)"unit");
    mei.music.scoreDef.staffDef2.meterSig.count = (unsigned char)strtol((const char*)count2_text, NULL, 10);
    mei.music.scoreDef.staffDef2.meterSig.unit = (unsigned char)strtol((const char*)unit2_text, NULL, 10);
    xmlFree(count2_text);
    xmlFree(unit2_text);
}

void MENG_PrintMEI()
{
    printf("[XML]\n");
    printf("├── mei_file        : %p\n", (void*)xml.mei_file);
    printf("├── root            : %p\n", (void*)xml.root);
    printf("├── meiHead         : %p\n", (void*)xml.meiHead);
    printf("│   ├── fileDesc        : %p\n", (void*)xml.fileDesc);
    printf("│   │   ├── titleStmt     : %p\n", (void*)xml.titleStmt);
    printf("│   │   │   ├── title        : %p\n", (void*)xml.title);
    printf("│   │   │   └── respStmt     : %p\n", (void*)xml.respStmt);
    printf("│   │   └── pubStmt       : %p\n", (void*)xml.pubStmt);
    printf("│   │       ├── publisher    : %p\n", (void*)xml.publisher);
    printf("│   │       ├── pubPlace     : %p\n", (void*)xml.pubPlace);
    printf("│   │       ├── date         : %p\n", (void*)xml.date);
    printf("│   │       └── availability : %p\n", (void*)xml.availability);
    printf("│   ├── encodingDesc     : %p\n", (void*)xml.encodingDesc);
    printf("│   │   └── projectDesc      : %p\n", (void*)xml.projectDesc);
    printf("│   │       └── projectDesc_p: %p\n", (void*)xml.projectDesc_p);
    printf("│   └── revisionDesc     : %p\n", (void*)xml.revisionDesc);
    printf("├── music           : %p\n", (void*)xml.music);
    printf("│   └── body            : %p\n", (void*)xml.body);
    printf("│       └── mdiv            : %p\n", (void*)xml.mdiv);
    printf("│           └── score           : %p\n", (void*)xml.score);
    printf("│               ├── scoreDef       : %p\n", (void*)xml.scoreDef);
    printf("│               │   └── staffGrp       : %p\n", (void*)xml.staffGrp);
    printf("│               │       ├── grpSym         : %p\n", (void*)xml.grpSym);
    printf("│               │       ├── label          : %p\n", (void*)xml.label);
    printf("│               │       ├── labelAbbr      : %p\n", (void*)xml.labelAbbr);
    printf("│               │       ├── instrDef       : %p\n", (void*)xml.instrDef);
    printf("│               │       ├── staffDef1      : %p\n", (void*)xml.staffDef1);
    printf("│               │       │   ├── clef1           : %p\n", (void*)xml.clef1);
    printf("│               │       │   ├── keySig1         : %p\n", (void*)xml.keySig1);
    printf("│               │       │   └── meterSig1       : %p\n", (void*)xml.meterSig1);
    printf("│               │       ├── staffDef2      : %p\n", (void*)xml.staffDef2);
    printf("│               │       │   ├── clef2           : %p\n", (void*)xml.clef2);
    printf("│               │       │   ├── keySig2         : %p\n", (void*)xml.keySig2);
    printf("│               │       │   └── meterSig2       : %p\n", (void*)xml.meterSig2);
    printf("│               └── section         : %p\n", (void*)xml.section);

    printf("[fileDesc]\n");
    printf("├──[Title]: %s\n", mei.meiHead.fileDesc.titleStmt.title);
    printf("├──[respStmt]: %ld %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.persName_count, mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count);
    for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.persName_count; i++)
    {
        if (mei.meiHead.fileDesc.titleStmt.respStmt.persName[i] != NULL)
        {
            printf("│   ├──[Pers] %s: %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->name, mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->role);
        }
    }
    for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count; i++)
    {
        if (mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i] != NULL)
        {
            printf("│   ├──[Corp] %s: %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->name, mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->role);
        }
    }
    printf("├──[PubStmt]\n│   ├──[publisher]: %s\n│   ├──[pubPlace]: %s\n│   ├──[date]: %s\n│   └──[availability]: %s\n", mei.meiHead.fileDesc.pubStm.publisher, mei.meiHead.fileDesc.pubStm.pubPlace, mei.meiHead.fileDesc.pubStm.date, mei.meiHead.fileDesc.pubStm.availability);
    printf("[encodingDesc]\n│   └──[projectDesc]: %s\n", mei.meiHead.encodingDesc.projectDesc.desc);
    printf("[revisionDesc]\n");
    printf("├──[Changes]: %ld\n", mei.meiHead.revisionDesc.changes_count);
    for (unsigned int i = 0; i < mei.meiHead.revisionDesc.changes_count; i++)
    {
        if (mei.meiHead.revisionDesc.changes[i] != NULL)
        {
            printf("│   ├──[Change n=%ld isodate='%s']\n│   │   ├──[changeDesc]: %s\n", mei.meiHead.revisionDesc.changes[i]->n, mei.meiHead.revisionDesc.changes[i]->isodate, mei.meiHead.revisionDesc.changes[i]->changeDesc);
            printf("│   │   ├──[respStmt]\n");
            for (unsigned int j = 0; j < mei.meiHead.revisionDesc.changes[i]->respStmt.persName_count; j++)
            {
                if (mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j] != NULL)
                {
                    printf("│   │   │   ├──[Pers] %s: %d\n", mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->name, mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->role);
                }
            }
            for (unsigned int j = 0; j < mei.meiHead.revisionDesc.changes[i]->respStmt.corpName_count; j++)
            {
                if (mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j] != NULL)
                {
                    printf("\t\t\t\t[Corp] %s: %d\n", mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j]->name, mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j]->role);
                }
            }
        }
    }
    printf("[Music]\n");
    printf("├──[ScoreDef bar.thru=%d]\n", mei.music.scoreDef.barthru);
    printf("│   ├──[grpSym]: %s\n", mei.music.scoreDef.grpSym);
    printf("│   ├──[label]: %s\n", mei.music.scoreDef.label);
    printf("│   ├──[labelAbbr]: %s\n", mei.music.scoreDef.labelAbbr);
    printf("│   ├──[instrDef midi.channel=%d midi.instrnum=%d midi_volume=\"%d%%\"]\n", mei.music.scoreDef.instrDef.midi_channel, mei.music.scoreDef.instrDef.midi_instrnum, mei.music.scoreDef.instrDef.midi_volume);
    printf("│   ├──[staffDef1 n=%d (should=1) lines=%d]\n", mei.music.scoreDef.staffDef1.n, mei.music.scoreDef.staffDef1.lines);
    printf("│   │   ├──[clef]: shape: %c line: %d\n", mei.music.scoreDef.staffDef1.clef.shape, mei.music.scoreDef.staffDef1.clef.line);
    printf("│   │   ├──[keySig]: %s\n", mei.music.scoreDef.staffDef1.keySig);
    printf("│   │   └──[meterSig count=%d unit=%d]\n", mei.music.scoreDef.staffDef1.meterSig.count, mei.music.scoreDef.staffDef1.meterSig.unit);
    printf("│   └──[staffDef2 n=%d (should=2) lines=%d]\n", mei.music.scoreDef.staffDef2.n, mei.music.scoreDef.staffDef2.lines);
    printf("│   │   ├──[clef]: shape: %c line: %d\n", mei.music.scoreDef.staffDef2.clef.shape, mei.music.scoreDef.staffDef2.clef.line);
    printf("│   │   ├──[keySig]: %s\n", mei.music.scoreDef.staffDef2.keySig);
    printf("│   │   └──[meterSig count=%d unit=%d]\n", mei.music.scoreDef.staffDef2.meterSig.count, mei.music.scoreDef.staffDef2.meterSig.unit);
    printf("└──[Section]\n");
}

int MENG_SaveMEIFile(MENG_MEI mei, char* file)
{

}

static int __GetrevisionDesc(xmlNode* revisionDesc, MENG_Change*** changes, unsigned int* changes_count)
{
    if (revisionDesc == NULL || changes == NULL || changes_count == NULL) return -1;
    *changes = NULL;
    *changes_count = 0;
    for (xmlNode* cur = revisionDesc->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"change") == 0)
        {
            xmlChar* n_str = xmlGetProp(cur, (const xmlChar*)"n");
            unsigned int n = (unsigned int)strtoul((const char*)n_str, NULL, 10);
            xmlChar* isodate = xmlGetProp(cur, (const xmlChar*)"isodate");
            xmlNode* changeDesc = __SearchNode(cur, "changeDesc");
            xmlNode* changeDesc_p = __SearchNode(changeDesc, "p");
            xmlChar* changeDesc_p_text = xmlNodeGetContent(changeDesc_p);

            xmlNode* respStmt = __SearchNode(cur, "respStmt");
            if (respStmt == NULL) return -19;
            MENG_PersName** persName = NULL;
            unsigned int persName_count = 0;
            MENG_CorpName** corpName = NULL;
            unsigned int corpName_count = 0;

            if (__GetRespStmt(respStmt, &persName, &persName_count, &corpName, &corpName_count) == -1) return -20;
            
            MENG_Change* change = (MENG_Change*)calloc(1, sizeof(MENG_Change));
            change->n = n;
            change->isodate = strdup(isodate);
            change->changeDesc = strdup(changeDesc_p_text);
            change->respStmt.persName = persName;
            change->respStmt.persName_count = persName_count;
            change->respStmt.corpName = corpName;
            change->respStmt.corpName_count = corpName_count;

            (*changes_count)++;

            if (*changes == NULL)
            {
                *changes = (MENG_Change**)calloc(1, sizeof(MENG_Change*));
            }
            else
            {
                *changes = (MENG_Change**)realloc(*changes, (*changes_count) * sizeof(MENG_Change*));
            }
            (*changes)[(*changes_count) - 1] = change;

            xmlFree(n_str);
            xmlFree(isodate);
            xmlFree(changeDesc_p_text);
        }
    }
}

static xmlNode* __SearchNode(xmlNode* node, const xmlChar* node_name)
{
    for (xmlNode* cur = node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, node_name) == 0)
        {
            return cur;
        }
    }
    return NULL;
}

static MENG_Role __TranslateRoleByText(char* role_text)
{
    if (strcmp(role_text, "author") == 0)
    {
        return MENG_ROLE_AUTHOR;
    }
    else if (strcmp(role_text, "composer") == 0)
    {
        return MENG_ROLE_COMPOSER;
    }
    else if (strcmp(role_text, "arranger") == 0)
    {
        return MENG_ROLE_ARRANGER;
    }
    else if (strcmp(role_text, "editor") == 0)
    {
        return MENG_ROLE_EDITOR;
    }
    else if (strcmp(role_text, "contributor") == 0)
    {
        return MENG_ROLE_CONTRIBUTOR;
    }
    else if (strcmp(role_text, "funder") == 0)
    {
        return MENG_ROLE_FUNDER;
    }
    else if (strcmp(role_text, "lyricist") == 0)
    {
        return MENG_ROLE_LYRICIST;
    }
    else if (strcmp(role_text, "librettist") == 0)
    {
        return MENG_ROLE_LIBRETTIST;
    }
    else if (strcmp(role_text, "sponsor") == 0)
    {
        return MENG_ROLE_SPONSOR;
    }
    else
    {
        return -1;
    }
}

static int __GetRespStmt(xmlNode* respStmt, MENG_PersName*** persName, unsigned int* persName_count, MENG_CorpName*** corpName, unsigned int* corpName_count)
{
    if (respStmt == NULL || persName == NULL || persName_count == NULL || corpName == NULL || corpName_count == NULL) return -1;
    *persName = NULL;
    *persName_count = 0;
    *corpName = NULL;
    *corpName_count = 0;
    for (xmlNode* cur = respStmt->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"persName") == 0)
        {
            xmlChar* role = xmlGetProp(cur, (const xmlChar *)"role");
            xmlChar* text = xmlNodeGetContent(cur);
            if (role == NULL || text == NULL) return -4;

            MENG_PersName* curPersName = (MENG_PersName*)calloc(1, sizeof(MENG_PersName));
            if (curPersName == NULL) return -5;
            curPersName->name = strdup((char *)text);
            curPersName->role = __TranslateRoleByText((char*)role);
            if (curPersName->role == -1) return -6;
            
            (*persName_count)++;
            // Check and create persName**
            if (*persName == NULL)
            {
                *persName = (MENG_PersName**)calloc(*persName_count, sizeof(MENG_PersName*));
            }
            else
            {
                *persName = (MENG_PersName**)realloc(*persName, *persName_count * sizeof(MENG_PersName*));
            }
            (*persName)[*persName_count - 1] = curPersName;

            xmlFree(role);
            xmlFree(text);
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"corpName") == 0)
        {
            xmlChar* role = xmlGetProp(cur, (const xmlChar *)"role");
            xmlChar* text = xmlNodeGetContent(cur);
            if (role == NULL || text == NULL) return -7;

            MENG_CorpName* curCorpName = (MENG_CorpName*)calloc(1, sizeof(MENG_CorpName));
            if (curCorpName == NULL) return -8;
            curCorpName->name = strdup((char *)text);
            curCorpName->role = __TranslateRoleByText((char*)role);
            if (curCorpName->role == -1) return -9;
            (*corpName_count)++;

            if (*corpName == NULL)
            {
                *corpName = (MENG_CorpName**)calloc(*corpName_count, sizeof(MENG_CorpName*));
            }
            else
            {
                *corpName = (MENG_CorpName**)realloc(*corpName, *corpName_count * sizeof(MENG_CorpName*));
            }
            (*corpName)[*corpName_count - 1] = curCorpName;

            xmlFree(role);
            xmlFree(text);
        }
    }
    return 0;
}