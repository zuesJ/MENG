#include "MENG.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

static MENG_MEI mei = {0};

static xmlNode* __SearchNode(xmlNode* node, const xmlChar* node_name);
static MENG_Role __TranslateRoleByText(char* role_text);
static int __GetRespStmt(xmlNode* respStmt, MENG_PersName*** persName, unsigned int* persName_count, MENG_CorpName*** corpName, unsigned int* corpName_count);

int MENG_LoadMEIFile(char* file)
{
    if (!MENG_IsInit()) return -1;
    xmlDoc* mei_file = xmlReadFile(file, NULL, 0);
    if (mei_file == NULL) return -2;

    xmlNode* root = xmlDocGetRootElement(mei_file);
    if (root == NULL) return -3;

    xmlNode* meiHead = __SearchNode(root, "meiHead");
    xmlNode* fileDesc = __SearchNode(meiHead, "fileDesc");
    xmlNode* titleStmt = __SearchNode(fileDesc, "titleStmt");
    xmlNode* title = __SearchNode(titleStmt, "title");
    xmlChar* titleText = xmlNodeGetContent(title);
    mei.meiHead.fileDesc.titleStmt.title = strdup((char *)titleText);
    xmlFree(titleText);
    xmlNode* respStmt = __SearchNode(titleStmt, "respStmt");

    MENG_PersName** persName = NULL;
    unsigned int persName_count = 0;
    MENG_CorpName** corpName = NULL;
    unsigned int corpName_count = 0;
    
    if (__GetRespStmt(respStmt, &persName, &persName_count, &corpName, &corpName_count) == -1) return -4;

    mei.meiHead.fileDesc.titleStmt.respStmt.persName = persName;
    mei.meiHead.fileDesc.titleStmt.respStmt.persName_count = persName_count;
    mei.meiHead.fileDesc.titleStmt.respStmt.corpName = corpName;
    mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count = corpName_count;

    xmlNode* pubStmt = __SearchNode(fileDesc, "pubStmt");
    if (pubStmt == NULL) return -10;
    xmlNode* publisher = __SearchNode(pubStmt, "publisher");
    if (publisher == NULL) return -11;
    xmlNode* pubPlace = __SearchNode(pubStmt, "pubPlace");
    if (pubPlace == NULL) return -12;
    xmlNode* date = __SearchNode(pubStmt, "date");
    if (date == NULL) return -13;
    xmlNode* availability = __SearchNode(pubStmt, "availability");
    xmlChar* publisher_text = xmlNodeGetContent(publisher);
    xmlChar* pubPlace_text = xmlNodeGetContent(pubPlace);
    xmlChar* date_text = xmlNodeGetContent(date);
    xmlChar* availability_text = xmlNodeGetContent(availability);
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
    
    xmlNode* encodingDesc = __SearchNode(meiHead, "encodingDesc");
    if (encodingDesc == NULL) return -14;
    xmlNode* projectDesc = __SearchNode(encodingDesc, "projectDesc");
    if (projectDesc == NULL) return -15;
    xmlNode* projectDesc_p = __SearchNode(projectDesc, "p");
    if (projectDesc_p == NULL) return -16;
    xmlChar* publisher_p_text = xmlNodeGetContent(projectDesc_p);
    mei.meiHead.encodingDesc.projectDesc.desc = strdup((char*)publisher_p_text);
    xmlFree(publisher_p_text);

    xmlNode* revisionDesc = __SearchNode(meiHead, "revisionDesc");
    if (revisionDesc == NULL) return -17;

    unsigned int changes_count = 0;
    MENG_Change** changes = NULL;
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

            changes_count++;

            if (changes == NULL)
            {
                changes = (MENG_Change**)calloc(1, sizeof(MENG_Change*));
            }
            else
            {
                changes = (MENG_Change**)realloc(changes, changes_count * sizeof(MENG_Change*));
            }
            changes[changes_count - 1] = change;

            xmlFree(n_str);
            xmlFree(isodate);
            xmlFree(changeDesc_p_text);
        }
    }
    mei.meiHead.revisionDesc.changes = changes;
    mei.meiHead.revisionDesc.changes_count = changes_count;
}

void MENG_PrintMEI()
{
    printf("[fileDesc]\n");
    printf("\t[Title]: %s\n", mei.meiHead.fileDesc.titleStmt.title);
    printf("\t[respStmt]: %ld %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.persName_count, mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count);
    for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.persName_count; i++)
    {
        if (mei.meiHead.fileDesc.titleStmt.respStmt.persName[i] != NULL)
        {
            printf("\t\t[Pers] %s: %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->name, mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->role);
        }
    }
    for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count; i++)
    {
        if (mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i] != NULL)
        {
            printf("\t\t[Corp] %s: %ld\n", mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->name, mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->role);
        }
    }
    printf("\t[PubStmt]\n\t\t[publisher]: %s\n\t\t[pubPlace]: %s\n\t\t[date]: %s\n\t\t[availability]: %s\n", mei.meiHead.fileDesc.pubStm.publisher, mei.meiHead.fileDesc.pubStm.pubPlace, mei.meiHead.fileDesc.pubStm.date, mei.meiHead.fileDesc.pubStm.availability);
    printf("[encodingDesc]\n\t[projectDesc]: %s\n", mei.meiHead.encodingDesc.projectDesc.desc);
    printf("[revisionDesc]\n");
    printf("\t[Changes]: %ld\n", mei.meiHead.revisionDesc.changes_count);
    for (unsigned int i = 0; i < mei.meiHead.revisionDesc.changes_count; i++)
    {
        if (mei.meiHead.revisionDesc.changes[i] != NULL)
        {
            printf("\t\t[Change n=%ld isodate='%s']:\n\t\t\t[changeDesc]: %s\n", mei.meiHead.revisionDesc.changes[i]->n, mei.meiHead.revisionDesc.changes[i]->isodate, mei.meiHead.revisionDesc.changes[i]->changeDesc);
            printf("\t\t\t[respStmt]\n");
            for (unsigned int j = 0; j < mei.meiHead.revisionDesc.changes[i]->respStmt.persName_count; j++)
            {
                if (mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j] != NULL)
                {
                    printf("\t\t\t\t[Pers] %s: %d\n", mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->name, mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->role);
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
}

int MENG_SaveMEIFile(MENG_MEI mei, char* file)
{

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