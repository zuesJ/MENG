#include "MENG.h"

#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

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

static xmlNode* __SearchNode(xmlNode* node, const xmlChar* node_name);
static int __LoadXmlNodes(char* file, MENG_MeiXml* xml);
static void __PrintXml(MENG_MeiXml xml);
static MENG_Role __TranslateRoleByText(char* role_text);
static char* __TranslateRoleToText(MENG_Role role);
static int __GetRespStmt(xmlNode* respStmt, MENG_PersName*** persName, unsigned int* persName_count, MENG_CorpName*** corpName, unsigned int* corpName_count);
static int __GetrevisionDesc(xmlNode* revisionDesc, MENG_Change*** changes, unsigned int* changes_count);
static xmlChar* __uintIntoXmlChar(unsigned int n);
static void __AnalyzeMeasure(xmlNode* measure_node, MENG_Measure* measure, MENG_MEI* mei);
static void __AnalyzeStaff(xmlNode* staff_node, MENG_Staff* staff);
static void __AnalyzeLayer(xmlNode* layer_node, MENG_Layer* layer);
static MENG_Chord* __AnalyzeChord(xmlNode* chord_node);
static MENG_Beam* __AnalyzeBeam(xmlNode* beam_node);
static MENG_TempoPlacement __TranslateTempoPlacementByText(const char* place_text);
static MENG_Duration __TranslateDurationByText(const char* dur_text);
static MENG_Octave __TranslateOctaveByText(const char* oct_text);
static MENG_Pitch __TranslatePitchByText(const char* pitch_text);
static void __PrintLayerElements(MENG_Layer* layer);

MENG_MEI MENG_LoadMEIFile(char* file)
{
    MENG_MEI mei = {0};
    if (!MENG_IsInit()) return mei;
    MENG_MeiXml xml = {0};
    if (__LoadXmlNodes(file, &xml) < 0) return mei;
    //__PrintXml(xml);

    xmlChar* titleText = xmlNodeGetContent(xml.title);
    mei.meiHead.fileDesc.titleStmt.title = strdup((char*)titleText);
    xmlFree(titleText);

    MENG_PersName** persName = NULL;
    unsigned int persName_count = 0;
    MENG_CorpName** corpName = NULL;
    unsigned int corpName_count = 0;

    if (__GetRespStmt(xml.respStmt, &persName, &persName_count, &corpName, &corpName_count) == -1) return mei;

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
        mei.meiHead.fileDesc.pubStmt.publisher = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStmt.publisher = strdup((char*)publisher_text);
    }

    if (pubPlace_text == NULL)
    {
        mei.meiHead.fileDesc.pubStmt.pubPlace = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStmt.pubPlace = strdup((char*)pubPlace_text);
    }

    if (date_text == NULL)
    {
        mei.meiHead.fileDesc.pubStmt.date = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStmt.date = strdup((char*)date_text);
    }

    if (availability_text == NULL)
    {
        mei.meiHead.fileDesc.pubStmt.availability = "";
    }
    else
    {
        mei.meiHead.fileDesc.pubStmt.availability = strdup((char*)availability_text);
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
    if (shape1_text == NULL || xmlStrlen(shape1_text) != 1) return mei;
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
    if (shape2_text == NULL || xmlStrlen(shape2_text) != 1) return mei;
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

    // Go through all sections
    //   - For each section check for 2 staffs, ties and tempo
    //   - Add ties to the section.ties attrib and the tempo attrib to the corresponding measure.tempo
    //   - For each staff in section look out for layers and allocate mem for them.
    //   - For each layer in staff, look for notes, rests, chords, beams...
    //   - Go on a recusive loop until the layer has no more elements.
    for (xmlNode* cur = xml.section->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"measure") == 0)
        {
            xmlChar* id = xmlGetNsProp(cur, (const xmlChar*)"id", (const xmlChar*)"http://www.w3.org/XML/1998/namespace");
            if (id == NULL) return mei;
            bool found_m = false;
            unsigned int index = 0;
            for (int i = 0; i < xmlStrlen(id); i++)
            {
                if (id[i] == (xmlChar)'m')
                {
                    xmlChar* cropped = xmlStrdup(id + i + 1);
                    index = strtol((const char*)cropped, NULL, 10);
                    index--;
                    xmlFree(cropped);
                    found_m = true;
                    break;
                }
            }
            if (!found_m) return mei;
            if (mei.music.section.measures == NULL)
            {
                mei.music.section.measures = (MENG_Measure**)calloc(index + 1, sizeof(MENG_Measure*));
                mei.music.section.measures_count = index + 1;
            }
            else if (mei.music.section.measures_count < index + 1)
            {
                mei.music.section.measures = (MENG_Measure**)realloc(mei.music.section.measures, (index + 1) * sizeof(MENG_Measure*));
                for (unsigned int i = mei.music.section.measures_count; i < (index + 1); i++)
                {
                    mei.music.section.measures[i] = NULL;
                }
                mei.music.section.measures_count = index + 1;
            }

            mei.music.section.measures[index] = (MENG_Measure*)calloc(1, sizeof(MENG_Measure));
            __AnalyzeMeasure(cur, mei.music.section.measures[index], &mei);
            xmlFree(id);
        }
    }

    return mei;
}
static void __AnalyzeMeasure(xmlNode* measure_node, MENG_Measure* measure, MENG_MEI* mei)
{
    if (measure == NULL || measure_node == NULL) return;
    for (xmlNode* cur = measure_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"tempo") == 0)
        {
            xmlChar* bpm = xmlGetProp(cur, (const xmlChar*)"midi.bpm");
            if (bpm == NULL) return;
            xmlChar* tstamp = xmlGetProp(cur, (const xmlChar*)"tstamp");
            if (tstamp == NULL) return;
            xmlChar* place = xmlGetProp(cur, (const xmlChar*)"place");
            if (place == NULL) return;
            xmlChar* staff = xmlGetProp(cur, (const xmlChar*)"staff");
            if (staff == NULL) return;
            measure->tempo.bpm = strtol((const char*)bpm, NULL, 10);
            measure->tempo.tstamp = strtol((const char*)tstamp, NULL, 10);
            measure->tempo.place = __TranslateTempoPlacementByText((char*)place);
            measure->tempo.staff = strtol((const char*)staff, NULL, 10);
            xmlFree(bpm);
            xmlFree(tstamp);
            xmlFree(place);
            xmlFree(staff);
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"tie") == 0)
        {
            if (mei->music.section.ties == NULL)
            {
                mei->music.section.ties = (MENG_Tie**)calloc(1, sizeof(MENG_Tie*));
                mei->music.section.ties_count = 1;
            }
            else
            {
                mei->music.section.ties_count++;
                mei->music.section.ties = (MENG_Tie**)realloc(mei->music.section.ties, mei->music.section.ties_count * sizeof(MENG_Tie*));
            }
            mei->music.section.ties[mei->music.section.ties_count - 1] = (MENG_Tie*)calloc(1, sizeof(MENG_Tie));
            xmlChar* startid = xmlGetProp(cur, (xmlChar*)"startid");
            if (startid == NULL) return;
            xmlChar* endid = xmlGetProp(cur, (xmlChar*)"endid");
            if (endid == NULL) return;
            mei->music.section.ties[mei->music.section.ties_count - 1]->startid = strdup(startid);
            mei->music.section.ties[mei->music.section.ties_count - 1]->endid = strdup(endid);
            xmlFree(startid);
            xmlFree(endid);
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"staff") == 0)
        {
            xmlChar* n = xmlGetProp(cur, (xmlChar*)"n");
            if (n == NULL) return;
            unsigned int n_num = strtol((const char*)n, NULL, 10);
            if (n_num == 1)
            {
                __AnalyzeStaff(cur, &measure->staffs[0]);
            }
            else if (n_num == 2)
            {
                __AnalyzeStaff(cur, &measure->staffs[1]);
            }
            else
            {
                return;
            }
        }
    }
}
static void __AnalyzeStaff(xmlNode* staff_node, MENG_Staff* staff)
{
    if (staff_node == NULL || staff == NULL) return;
    for (xmlNode* cur = staff_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"layer") == 0)
        {
            unsigned int index = strtol((const char*)xmlGetProp(cur, (xmlChar*)"n"), NULL, 10) - 1;
            if (staff->layers == NULL)
            {
                staff->layers = (MENG_Layer**)calloc(index + 1, sizeof(MENG_Layer*));
                staff->layers_count = index + 1;
            }
            else if (staff->layers_count < index + 1)
            {
                staff->layers = (MENG_Layer**)realloc(staff->layers, (index + 1) * sizeof(MENG_Layer*));
                for (unsigned int i = staff->layers_count; i < (index + 1); i++)
                {
                    staff->layers[i] = NULL;
                }
                staff->layers_count = index + 1;
            }
            staff->layers[index] = (MENG_Layer*)calloc(1, sizeof(MENG_Layer));
            __AnalyzeLayer(cur, staff->layers[index]);
        }
    }
}
static void __AnalyzeLayer(xmlNode* layer_node, MENG_Layer* layer)
{
    if (layer_node == NULL || layer == NULL) return;
    unsigned int notes_count = 0;
    unsigned int rests_count = 0;
    unsigned int chords_count = 0;
    unsigned int beams_count = 0;
    for (xmlNode* cur = layer_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            notes_count++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"rest") == 0)
        {
            rests_count++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"chord") == 0)
        {
            chords_count++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"beam") == 0)
        {
            beams_count++;
        }
    }
    layer->notes_count = notes_count;
    layer->rests_count = rests_count;
    layer->chords_count = chords_count;
    layer->beams_count = beams_count;
    if (notes_count > 0)
    {
        layer->notes = (MENG_Note**)calloc(notes_count, sizeof(MENG_Note*));
    }
    if (rests_count > 0)
    {
        layer->rests = (MENG_Rest**)calloc(rests_count, sizeof(MENG_Rest*));
    }
    if (chords_count > 0)
    {
        layer->chords = (MENG_Chord**)calloc(chords_count, sizeof(MENG_Chord*));
    }
    if (beams_count > 0)
    {
        layer->beams = (MENG_Beam**)calloc(beams_count, sizeof(MENG_Beam*));
    }

    unsigned int notes_i = 0;
    unsigned int rests_i = 0;
    unsigned int chords_i = 0;
    unsigned int beams_i = 0;

    unsigned int total_count = notes_count + rests_count + chords_count + beams_count;
    if (total_count > 0)
    {
        layer->order_count = total_count;
        layer->order = (MENG_Order**)calloc(total_count, sizeof(MENG_Order*));
    }
    for (xmlNode* cur = layer_node->children; cur != NULL; cur = cur->next)
    {
        unsigned int total_i = notes_i + rests_i + chords_i + beams_i;
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            xmlChar* dur_text = xmlGetProp(cur, (xmlChar*)"dur");
            if (dur_text == NULL) return;
            xmlChar* oct_text = xmlGetProp(cur, (xmlChar*)"oct");
            if (oct_text == NULL) return;
            xmlChar* pname_text = xmlGetProp(cur, (xmlChar*)"pname");
            if (pname_text == NULL) return;
            layer->notes[notes_i] = (MENG_Note*)calloc(1, sizeof(MENG_Note));
            layer->notes[notes_i]->dur = __TranslateDurationByText((const char*)dur_text);
            layer->notes[notes_i]->oct = __TranslateOctaveByText((const char*)oct_text);
            layer->notes[notes_i]->pname = __TranslatePitchByText((const char*)pname_text);
            xmlFree(oct_text);
            xmlFree(pname_text);
            layer->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            layer->order[total_i]->type = MENG_TYPE_NOTE;
            layer->order[total_i]->element.note = layer->notes[notes_i];
            notes_i++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"rest") == 0)
        {
            xmlChar* dur_text = xmlGetProp(cur, (xmlChar*)"dur");
            if (dur_text == NULL) return;
            layer->rests[rests_i] = (MENG_Rest*)calloc(1, sizeof(MENG_Rest));
            layer->rests[rests_i]->dur = __TranslateDurationByText((const char*)dur_text);
            xmlFree(dur_text);
            layer->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            layer->order[total_i]->type = MENG_TYPE_REST;
            layer->order[total_i]->element.rest = layer->rests[rests_i];
            rests_i++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"chord") == 0)
        {
            layer->chords[chords_i] = __AnalyzeChord(cur);
            layer->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            layer->order[total_i]->type = MENG_TYPE_CHORD;
            layer->order[total_i]->element.chord = layer->chords[chords_i];
            chords_i++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"beam") == 0)
        {
            layer->beams[beams_i] = __AnalyzeBeam(cur);
            layer->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            layer->order[total_i]->type = MENG_TYPE_BEAM;
            layer->order[total_i]->element.beam = layer->beams[beams_i];
            beams_i++;
        }
    }
    
}
static MENG_Chord* __AnalyzeChord(xmlNode* chord_node)
{
    if (chord_node == NULL) return NULL;
    xmlChar* dur_text = xmlGetProp(chord_node, (xmlChar*)"dur");
    if (dur_text == NULL) return NULL;
    MENG_Chord* new_chord = (MENG_Chord*)calloc(1, sizeof(MENG_Chord));
    new_chord->dur = __TranslateDurationByText((const char*)dur_text);
    unsigned int notes_count = 0;
    for (xmlNode* cur = chord_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            notes_count++;
        }
    }
    new_chord->notes = (MENG_Note**)calloc(notes_count, sizeof(MENG_Note*));
    new_chord->notes_count = notes_count;
    unsigned int notes_i = 0;
    for (xmlNode* cur = chord_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            xmlChar* oct_name = xmlGetProp(cur, (xmlChar*)"oct");
            xmlChar* pname_name = xmlGetProp(cur, (xmlChar*)"pname");
            new_chord->notes[notes_i] = (MENG_Note*)calloc(1, sizeof(MENG_Note));
            new_chord->notes[notes_i]->dur = 0;
            new_chord->notes[notes_i]->oct = __TranslateOctaveByText((const char*) oct_name);
            new_chord->notes[notes_i]->pname = __TranslatePitchByText((const char*) pname_name);
            xmlFree(oct_name);
            xmlFree(pname_name);
            notes_i++;
        }
    }
    xmlFree(dur_text);
    return new_chord;
}
static MENG_Beam* __AnalyzeBeam(xmlNode* beam_node)
{
    if (beam_node == NULL) return NULL;
    MENG_Beam* new_beam = (MENG_Beam*)calloc(1, sizeof(MENG_Beam));
    unsigned int notes_count = 0;
    unsigned int rests_count = 0;
    unsigned int chords_count = 0;
    for (xmlNode* cur = beam_node->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            notes_count++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"rest") == 0)
        {
            rests_count++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"chord") == 0)
        {
            chords_count++;
        }
    }
    new_beam->notes_count = notes_count;
    new_beam->rests_count = rests_count;
    new_beam->chords_count = chords_count;
    if (notes_count > 0)
    {
        new_beam->notes = (MENG_Note**)calloc(notes_count, sizeof(MENG_Note*));
    }
    if (rests_count > 0)
    {
        new_beam->rests = (MENG_Rest**)calloc(rests_count, sizeof(MENG_Rest*));
    }
    if (chords_count > 0)
    {
        new_beam->chords = (MENG_Chord**)calloc(chords_count, sizeof(MENG_Chord*));
    }
    unsigned int notes_i = 0;
    unsigned int rests_i = 0;
    unsigned int chords_i = 0;
    unsigned int total_count = notes_count + rests_count + chords_count;
    if (total_count > 0)
    {
        new_beam->order_count = total_count;
        new_beam->order = (MENG_Order**)calloc(total_count, sizeof(MENG_Order*));
    }
    for (xmlNode* cur = beam_node->children; cur != NULL; cur = cur->next)
    {
        unsigned int total_i = notes_i + rests_i + chords_i;
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"note") == 0)
        {
            xmlChar* dur_text = xmlGetProp(cur, (xmlChar*)"dur");
            if (dur_text == NULL) return new_beam;
            xmlChar* oct_text = xmlGetProp(cur, (xmlChar*)"oct");
            if (oct_text == NULL) return new_beam;
            xmlChar* pname_text = xmlGetProp(cur, (xmlChar*)"pname");
            if (pname_text == NULL) return new_beam;
            new_beam->notes[notes_i] = (MENG_Note*)calloc(1, sizeof(MENG_Note));
            new_beam->notes[notes_i]->dur = __TranslateDurationByText((const char*)dur_text);
            new_beam->notes[notes_i]->oct = __TranslateOctaveByText((const char*)oct_text);
            new_beam->notes[notes_i]->pname = __TranslatePitchByText((const char*)pname_text);
            xmlFree(oct_text);
            xmlFree(pname_text);
            new_beam->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            new_beam->order[total_i]->type = MENG_TYPE_NOTE;
            new_beam->order[total_i]->element.note = new_beam->notes[notes_i];
            notes_i++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"rest") == 0)
        {
            xmlChar* dur_text = xmlGetProp(cur, (xmlChar*)"dur");
            if (dur_text == NULL) return new_beam;
            new_beam->rests[rests_i] = (MENG_Rest*)calloc(1, sizeof(MENG_Rest));
            new_beam->rests[rests_i]->dur = __TranslateDurationByText((const char*)dur_text);
            xmlFree(dur_text);
            new_beam->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            new_beam->order[total_i]->type = MENG_TYPE_REST;
            new_beam->order[total_i]->element.rest = new_beam->rests[rests_i];
            rests_i++;
        }
        else if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"chord") == 0)
        {
            new_beam->chords[chords_i] = __AnalyzeChord(cur);
            new_beam->order[total_i] = (MENG_Order*)calloc(1, sizeof(MENG_Order));
            new_beam->order[total_i]->type = MENG_TYPE_CHORD;
            new_beam->order[total_i]->element.chord = new_beam->chords[chords_i];
            chords_i++;
        }
    }
    return new_beam;
}


char* MENG_OutputMeiAsChar(MENG_MEI mei)
{
    xmlDoc* doc = xmlNewDoc((xmlChar*)"1.0");
    xmlNode* root = xmlNewNode(NULL, (xmlChar*)"mei");
    xmlDocSetRootElement(doc, root);

    // Set proccessing instruct. to the document
    xmlNode* pi1 = xmlNewPI((xmlChar*)"xml-model", (xmlChar*)"href=\"https://music-encoding.org/schema/5.1/mei-all.rng\" ""type=\"application/xml\" ""schematypens=\"http://relaxng.org/ns/structure/1.0\"");
    xmlNode* pi2 = xmlNewPI((xmlChar*)"xml-model", (xmlChar*)"href=\"https://music-encoding.org/schema/5.1/mei-all.rng\" ""type=\"application/xml\" ""schematypens=\"http://purl.oclc.org/dsdl/schematron\"");;
    xmlAddPrevSibling(root, pi1);
    xmlAddPrevSibling(root, pi2);

    // Add namespace and mei version
    xmlNs* ns = xmlNewNs(root, (xmlChar*)"http://www.music-encoding.org/ns/mei", NULL);
    xmlSetNs(root, ns);
    xmlNewProp(root, (xmlChar*)"meiversion", (xmlChar*)"5.1");

    // Fill MeiHead
    {
        xmlNode* meiHead = xmlNewChild(root, NULL, (xmlChar*)"meiHead", NULL);
        xmlNode* fileDesc = xmlNewChild(meiHead, NULL, (xmlChar*)"fileDesc", NULL);
        xmlNode* titleStmt = xmlNewChild(fileDesc, NULL, (xmlChar*)"titleStmt", NULL);
        xmlNode* title = xmlNewChild(titleStmt, NULL, (xmlChar*)"title", (char*)mei.meiHead.fileDesc.titleStmt.title);
        xmlNode* respStmt = xmlNewChild(titleStmt, NULL, (xmlChar*)"respStmt", NULL);
        if (mei.meiHead.fileDesc.titleStmt.respStmt.persName != NULL && mei.meiHead.fileDesc.titleStmt.respStmt.persName_count > 0)
        {
            for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.persName_count; i++)
            {
                if (mei.meiHead.fileDesc.titleStmt.respStmt.persName[i] != NULL)
                {
                    xmlNode* pers = xmlNewChild(respStmt, NULL, (xmlChar*)"persName", mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->name);
                    xmlNewProp(pers, (xmlChar*)"role", (xmlChar*)__TranslateRoleToText(mei.meiHead.fileDesc.titleStmt.respStmt.persName[i]->role));
                }
                else
                {
                    printf("[MENG] Corrupted MEI struct\n");
                    return NULL;
                }
            }
        }
        if (mei.meiHead.fileDesc.titleStmt.respStmt.corpName != NULL && mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count > 0)
        {
            for (unsigned int i = 0; i < mei.meiHead.fileDesc.titleStmt.respStmt.corpName_count; i++)
            {
                if (mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i] != NULL)
                {
                    xmlNode* corp = xmlNewChild(respStmt, NULL, (xmlChar*)"corpName", mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->name);
                    xmlNewProp(corp, (xmlChar*)"role", (xmlChar*)__TranslateRoleToText(mei.meiHead.fileDesc.titleStmt.respStmt.corpName[i]->role));
                }
                else
                {
                    printf("[MENG] Corrupted MEI struct\n");
                    return NULL;
                }
            }
        }
        xmlNode* pubStmt = xmlNewChild(fileDesc, NULL, (xmlChar*)"pubStmt", NULL);
        xmlNode* publisher = xmlNewChild(pubStmt, NULL, (xmlChar*)"publisher", (xmlChar*)mei.meiHead.fileDesc.pubStmt.publisher);
        xmlNode* pubPlace = xmlNewChild(pubStmt, NULL, (xmlChar*)"pubPlace", (xmlChar*)mei.meiHead.fileDesc.pubStmt.pubPlace);
        xmlNode* date = xmlNewChild(pubStmt, NULL, (xmlChar*)"date", (xmlChar*)mei.meiHead.fileDesc.pubStmt.date);
        xmlNode* availability = xmlNewChild(pubStmt, NULL, (xmlChar*)"availability", (xmlChar*)mei.meiHead.fileDesc.pubStmt.availability);
        xmlNode* encodingDesc = xmlNewChild(meiHead, NULL, (xmlChar*)"encodingDesc", NULL);
        xmlNode* projectDesc = xmlNewChild(encodingDesc, NULL, (xmlChar*)"projectDesc", NULL);
        xmlNode* projectDesc_p = xmlNewChild(projectDesc, NULL, (xmlChar*)"p", (xmlChar*)mei.meiHead.encodingDesc.projectDesc.desc);
        xmlNode* revisionDesc = xmlNewChild(meiHead, NULL, (xmlChar*)"revisionDesc", NULL);
        if (mei.meiHead.revisionDesc.changes != NULL)
        {
            for (unsigned int i = 0; i < mei.meiHead.revisionDesc.changes_count; i++)
            {
                if (mei.meiHead.revisionDesc.changes[i] != NULL)
                {
                    xmlNode* change = xmlNewChild(revisionDesc, NULL, (xmlChar*)"change", NULL);
                    xmlChar* n_text = __uintIntoXmlChar(mei.meiHead.revisionDesc.changes[i]->n);
                    xmlNewProp(change, (xmlChar*)"n", n_text);
                    xmlFree(n_text);
                    xmlNewProp(change, (xmlChar*)"isodate", (xmlChar*)mei.meiHead.revisionDesc.changes[i]->isodate);
                    xmlNode* change_respStmt = xmlNewChild(change, NULL, (xmlChar*)"respStmt", NULL);
                    if (mei.meiHead.revisionDesc.changes[i]->respStmt.persName != NULL && mei.meiHead.revisionDesc.changes[i]->respStmt.persName_count > 0)
                    {
                        for (unsigned int j = 0; j < mei.meiHead.revisionDesc.changes[i]->respStmt.persName_count; j++)
                        {
                            if (mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j] != NULL)
                            {
                                xmlNode* pers = xmlNewChild(change_respStmt, NULL, (xmlChar*)"persName", mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->name);
                                xmlNewProp(pers, (xmlChar*)"role", (xmlChar*)__TranslateRoleToText(mei.meiHead.revisionDesc.changes[i]->respStmt.persName[j]->role));
                            }
                        }
                    }
                    if (mei.meiHead.revisionDesc.changes[i]->respStmt.corpName != NULL && mei.meiHead.revisionDesc.changes[i]->respStmt.corpName_count > 0)
                    {
                        for (unsigned int j = 0; j < mei.meiHead.revisionDesc.changes[i]->respStmt.corpName_count; j++)
                        {
                            if (mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j] != NULL)
                            {
                                xmlNode* corp = xmlNewChild(change_respStmt, NULL, (xmlChar*)"corpName", mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j]->name);
                                xmlNewProp(corp, (xmlChar*)"role", (xmlChar*)__TranslateRoleToText(mei.meiHead.revisionDesc.changes[i]->respStmt.corpName[j]->role));
                            }
                        }
                    }
                    xmlNode* changeDesc = xmlNewChild(change, NULL, (xmlChar*)"changeDesc", NULL);
                    xmlNode* changeDesc_p = xmlNewChild(changeDesc, NULL, (xmlChar*)"p", (xmlChar*)mei.meiHead.revisionDesc.changes[i]->changeDesc);
                }
            }
        }
        else
        {
            printf("[MENG] Corrupted MEI struct\n");
        }
    }

    // Store Mei in a char buffer
    xmlChar* xmlbuff = NULL;
    int buffersize = 0;
    xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &buffersize, "UTF-8", 1);

    xmlFreeDoc(doc);

    return (char*)xmlbuff;
}

static void __PrintXml(MENG_MeiXml xml)
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
}

void MENG_PrintMEI(MENG_MEI mei)
{
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
    printf("├──[PubStmt]\n│   ├──[publisher]: %s\n│   ├──[pubPlace]: %s\n│   ├──[date]: %s\n│   └──[availability]: %s\n", mei.meiHead.fileDesc.pubStmt.publisher, mei.meiHead.fileDesc.pubStmt.pubPlace, mei.meiHead.fileDesc.pubStmt.date, mei.meiHead.fileDesc.pubStmt.availability);
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
    printf("    ├──[measures count:%ld]: %p\n", mei.music.section.measures_count, mei.music.section.measures);
    if (mei.music.section.measures != NULL)
    {
        for (unsigned int i = 0; i < mei.music.section.measures_count; i++)
        {
            if (mei.music.section.measures[i] != NULL)
            {
                printf("    │   ├──[measure n=%ld]: %p ; tempo_bpm: %d ; tstamp: %d ; place: %d ; staff: %d\n", i + 1, mei.music.section.measures[i], mei.music.section.measures[i]->tempo.bpm, mei.music.section.measures[i]->tempo.tstamp, mei.music.section.measures[i]->tempo.place, mei.music.section.measures[i]->tempo.staff);
                for (unsigned int j = 0; j < 2; j++)
                {
                    printf("    │   │   ├──[staff n=\"%d\"]: %p ; layers: %p ; layers_count: %d\n", j+1, &mei.music.section.measures[i]->staffs[j], mei.music.section.measures[i]->staffs[j].layers, mei.music.section.measures[i]->staffs[j].layers_count);
                    for (unsigned int k = 0; k < mei.music.section.measures[i]->staffs[j].layers_count; k++)
                    {
                        if (mei.music.section.measures[i]->staffs[j].layers[k] != NULL)
                        {
                            printf("    │   │   │   ├──[layer n=\"%d\"] (np: %p, nc: %d) ; (rp: %p, rc: %d) ; (cp: %p, cc: %d) ; (bp: %p, bc: %d)\n", k + 1,  mei.music.section.measures[i]->staffs[j].layers[k]->notes, mei.music.section.measures[i]->staffs[j].layers[k]->notes_count, mei.music.section.measures[i]->staffs[j].layers[k]->rests, mei.music.section.measures[i]->staffs[j].layers[k]->rests_count, mei.music.section.measures[i]->staffs[j].layers[k]->chords, mei.music.section.measures[i]->staffs[j].layers[k]->chords_count, mei.music.section.measures[i]->staffs[j].layers[k]->beams, mei.music.section.measures[i]->staffs[j].layers[k]->beams_count);
                            __PrintLayerElements(mei.music.section.measures[i]->staffs[j].layers[k]);
                        }
                        else
                        {
                            printf("    │   │   │   ├──[layer n=\"%d\"]: null", k);
                        }
                    }
                }
            }
            else
            {
                printf("    │   ├──[measure index=%ld]: %p\n", i, mei.music.section.measures[i]);
            }
        }
    }
    printf("    └──[ties count:%ld]: %p\n", mei.music.section.ties_count, mei.music.section.ties);
    if (mei.music.section.ties != NULL)
    {
        for (unsigned int i = 0; i < mei.music.section.ties_count; i++)
        {
            if (mei.music.section.ties[i] != NULL)
            {
                printf("        ├──[tie n=%ld]: %p ; startid: %s ; endid: %s\n", i + 1, mei.music.section.ties[i], mei.music.section.ties[i]->startid, mei.music.section.ties[i]->endid);
            }
            else
            {
                printf("        ├──[tie index=%ld]: %p\n", i, mei.music.section.ties[i]);
            }
        }
    }
}
static void __PrintLayerElements(MENG_Layer* layer)
{
    for (unsigned int i = 0; i < layer->order_count; i++)
    {
        if (layer->order[i] != NULL)
        {
            printf("    │   │   │   │   ├──[Order i=%d type=%d ", i, layer->order[i]->type);
            switch (layer->order[i]->type)
            {
                case MENG_TYPE_UNDEFINED:
                    printf("Undefined");
                    break;
                case MENG_TYPE_NOTE:
                    printf("Note: %p", layer->order[i]->element.note);
                    break;
                case MENG_TYPE_REST:
                    printf("Rest: %p", layer->order[i]->element.rest);
                    break;
                case MENG_TYPE_CHORD:
                    printf("Chord: %p", layer->order[i]->element.chord);
                    break;
                case MENG_TYPE_BEAM:
                    printf("Beam: %p", layer->order[i]->element.beam);
                    break;
                default:
                    printf("Undefined (big error)");
                    break;
            }
            printf("]: ptr: %p\n", layer->order[i]);
        }
        else
        {
            printf("    │   │   │   │   ├──[Order i=%d]: ptr: %p\n", i, layer->order[i]);
        }
    }
    for (unsigned int i = 0; i < layer->notes_count; i++)
    {
        if (layer->notes[i] != NULL)
        {
            printf("    │   │   │   │   ├──[Note slot=\"%d\" dur=\"%d\" oct=\"%d\" pname=\"%d\"]: ptr: %p\n", i, layer->notes[i]->dur, layer->notes[i]->oct, layer->notes[i]->pname, layer->notes[i]);
        }
        else
        {
            printf("    │   │   │   │   ├──[Note slot=\"%d\"]: %p\n", i, layer->notes[i]);
        }
    }
    for (unsigned int i = 0; i < layer->rests_count; i++)
    {
        if (layer->rests[i] != NULL)
        {
            printf("    │   │   │   │   ├──[Rest slot=\"%d\" dur=\"%d\"]: %p\n", i, layer->rests[i]->dur, layer->rests[i]);
        }
        else
        {
            printf("    │   │   │   │   ├──[Rest slot=\"%d\"]: %p\n", i, layer->rests[i]);
        }
    }
    
    for (unsigned int i = 0; i < layer->chords_count; i++)
    {
        if (layer->chords[i] != NULL)
        {
            printf("    │   │   │   │   ├──[Chord slot=\"%d\" dur=\"%d\"]: %p\n", i, layer->chords[i]->dur, layer->chords[i]);
            for (unsigned int j = 0; j < layer->chords[i]->notes_count; j++)
            {
                if (layer->chords[i]->notes[j] != NULL)
                {
                    printf("    │   │   │   │   │   ├──[Note slot=%d dur=\"%d oct=\"%d\" pname=\"%d\"]: ptr: %p\n", j, layer->chords[i]->notes[j]->dur, layer->chords[i]->notes[j]->oct, layer->chords[i]->notes[j]->pname, layer->chords[i]->notes[j]);
                }
                else
                {
                    printf("    │   │   │   │   │   ├──[Note slot=%d]: ptr: %p\n", j, layer->chords[i]->notes[j]);
                }
            }
        }
        else
        {
            printf("    │   │   │   │   ├──[Chord slot=\"%d\"]: %p\n", i, layer->chords[i]);
        }
    }
    for (unsigned int i = 0; i < layer->beams_count; i++)
    {
        if (layer->beams[i] != NULL)
        {
            printf("    │   │   │   │   ├──[Beam slot=\"%d\"]: %p\n", i, layer->beams[i]);
            for (unsigned int j = 0; j < layer->beams[i]->order_count; j++)
            {
                if (layer->beams[i]->order[j] != NULL)
                {
                    printf("    │   │   │   │   │   ├──[Order j=%d type=%d ", j, layer->beams[i]->order[j]->type);
                    switch (layer->beams[i]->order[j]->type)
                    {
                        case MENG_TYPE_UNDEFINED:
                            printf("Undefined");
                            break;
                        case MENG_TYPE_NOTE:
                            printf("Note: %p", layer->beams[i]->order[j]->element.note);
                            break;
                        case MENG_TYPE_REST:
                            printf("Rest: %p", layer->beams[i]->order[j]->element.rest);
                            break;
                        case MENG_TYPE_CHORD:
                            printf("Chord: %p", layer->beams[i]->order[j]->element.chord);
                            break;
                        default:
                            printf("Undefined (big error)");
                            break;
                    }
                    printf("]: %p\n", layer->beams[i]->order[j]);
                }
                else 
                {
                    printf("    │   │   │   │   │   ├──[Order j=%d] %p\n", j, layer->beams[i]->order[j]);
                }
                
            }
            for (unsigned int j = 0; j < layer->beams[i]->notes_count; j++)
            {
                if (layer->beams[i]->notes[j] != NULL)
                {
                    printf("    │   │   │   │   │   ├──[Note slot=\"%d\" dur=\"%d\" oct=\"%d\" pname=\"%d\"]: ptr: %p\n", j, layer->beams[i]->notes[j]->dur, layer->beams[i]->notes[j]->oct, layer->beams[i]->notes[j]->pname, layer->beams[i]->notes[j]);
                }
                else
                {
                    printf("    │   │   │   │   │   ├──[Note slot=\"%d\"]: %p\n", j, layer->beams[i]->notes[j]);
                }
            }
            for (unsigned int j = 0; j < layer->beams[i]->rests_count; j++)
            {
                if (layer->beams[i]->rests[j] != NULL)
                {
                    printf("    │   │   │   │   │   ├──[Rest slot=\"%d\" dur=\"%d\"]: ptr: %p\n", j, layer->beams[i]->rests[j]->dur, layer->beams[i]->rests[j]);
                }
                else
                {
                    printf("    │   │   │   │   │   ├──[Rest slot=\"%d\"]: %p\n", j, layer->beams[i]->rests[j]);
                }
            }
            for (unsigned int j = 0; j < layer->beams[i]->chords_count; j++)
            {
                if (layer->beams[i]->chords[j] != NULL)
                {
                    printf("    │   │   │   │   │   ├──[Chord slot=\"%d\" dur=\"%d\"]: ptr: %p\n", j, layer->beams[i]->chords[j]->dur, layer->beams[i]->chords[j]);
                    for (unsigned int k = 0; k < layer->beams[i]->chords[j]->notes_count; k++)
                    {
                        if (layer->beams[i]->chords[j]->notes[k] != NULL)
                        {
                            printf("    │   │   │   │   │   │   ├──[Note slot=%d dur=\"%d oct=\"%d\" pname=\"%d\"]: ptr: %p\n", k, layer->beams[i]->chords[j]->notes[k]->dur, layer->beams[i]->chords[j]->notes[k]->oct, layer->beams[i]->chords[j]->notes[k]->pname, layer->beams[i]->chords[j]->notes[k]);
                        }
                        else
                        {
                            printf("    │   │   │   │   │   │   ├──[Note slot=%d]: ptr: %p\n", k, layer->beams[i]->chords[j]->notes[k]);
                        }
                    }
                }
                else
                {
                    printf("    │   │   │   │   │   ├──[Chord slot=\"%d\"]: %p\n", j, layer->beams[i]->chords[j]);
                }
            }
        }
        else
        {
            printf("    │   │   │   │   ├──[Beam slot=\"%d\"]: %p\n", i, layer->beams[i]);
        }
    }
}

static int __LoadXmlNodes(char* file, MENG_MeiXml* xml)
{
    xml->mei_file = xmlReadFile(file, NULL, 0);
    if (xml->mei_file == NULL) return -1;
    xml->root = xmlDocGetRootElement(xml->mei_file);
    if (xml->root == NULL) return -2;
    xml->meiHead = __SearchNode(xml->root, "meiHead");
    if (xml->meiHead == NULL) return -3;
    xml->fileDesc = __SearchNode(xml->meiHead, "fileDesc");
    if (xml->fileDesc == NULL) return -4;
    xml->titleStmt = __SearchNode(xml->fileDesc, "titleStmt");
    if (xml->titleStmt == NULL) return -5;
    xml->title = __SearchNode(xml->titleStmt, "title");
    if (xml->title == NULL) return -6;
    xml->respStmt = __SearchNode(xml->titleStmt, "respStmt");
    if (xml->respStmt == NULL) return -7;
    xml->pubStmt = __SearchNode(xml->fileDesc, "pubStmt");
    if (xml->pubStmt == NULL) return -8;
    xml->publisher = __SearchNode(xml->pubStmt, "publisher");
    if (xml->publisher == NULL) return -9;
    xml->pubPlace = __SearchNode(xml->pubStmt, "pubPlace");
    if (xml->pubPlace == NULL) return -10;
    xml->date = __SearchNode(xml->pubStmt, "date");
    if (xml->date == NULL) return -11;
    xml->availability = __SearchNode(xml->pubStmt, "availability");
    if (xml->availability == NULL) return -12;
    xml->encodingDesc = __SearchNode(xml->meiHead, "encodingDesc");
    if (xml->encodingDesc == NULL) return -13;
    xml->projectDesc = __SearchNode(xml->encodingDesc, "projectDesc");
    if (xml->projectDesc == NULL) return -14;
    xml->projectDesc_p = __SearchNode(xml->projectDesc, "p");
    if (xml->projectDesc_p == NULL) return -15;
    xml->revisionDesc = __SearchNode(xml->meiHead, "revisionDesc");
    if (xml->revisionDesc == NULL) return -16;
    xml->music = __SearchNode(xml->root, "music");
    if (xml->music == NULL) return -17;
    xml->body = __SearchNode(xml->music, "body");
    if (xml->body == NULL) return -18;
    xml->mdiv = __SearchNode(xml->body, "mdiv");
    if (xml->mdiv == NULL) return -19;
    xml->score = __SearchNode(xml->mdiv, "score");
    if (xml->score == NULL) return -20;
    xml->scoreDef = __SearchNode(xml->score, "scoreDef");
    if (xml->scoreDef == NULL) return -21;
    xml->staffGrp = __SearchNode(xml->scoreDef, "staffGrp");
    if (xml->staffGrp == NULL) return -22;
    xml->grpSym = __SearchNode(xml->staffGrp, "grpSym");
    if (xml->grpSym == NULL) return -23;
    xml->label = __SearchNode(xml->staffGrp, "label");
    if (xml->label == NULL) return -24;
    xml->labelAbbr = __SearchNode(xml->staffGrp, "labelAbbr");
    if (xml->labelAbbr == NULL) return -25;
    xml->instrDef = __SearchNode(xml->staffGrp, "instrDef");
    if (xml->instrDef == NULL) return -26;
    xml->section = __SearchNode(xml->score, "section");
    if (xml->section == NULL) return -27;

    bool foundanystaffdef = false;
    for (xmlNode* cur = xml->staffGrp->children; cur != NULL; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE && xmlStrcmp(cur->name, (const xmlChar *)"staffDef") == 0)
        {
            xmlChar* n_str = xmlGetProp(cur, (const xmlChar*)"n");
            if (xmlStrcmp(n_str, (const xmlChar*)"1") == 0)
            {
                xml->staffDef1 = cur;
                foundanystaffdef = true;
            }
            else if (xmlStrcmp(n_str, (const xmlChar*)"2") == 0)
            {
                xml->staffDef2 = cur;
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

    xml->clef1 = __SearchNode(xml->staffDef1, "clef");
    if (xml->clef1 == NULL) return -29;
    xml->keySig1 = __SearchNode(xml->staffDef1, "keySig");
    if (xml->keySig1 == NULL) return -30;
    xml->meterSig1 = __SearchNode(xml->staffDef1, "meterSig");
    if (xml->meterSig1 == NULL) return -31;

    xml->clef2 = __SearchNode(xml->staffDef2, "clef");
    if (xml->clef2 == NULL) return -32;
    xml->keySig2 = __SearchNode(xml->staffDef2, "keySig");
    if (xml->keySig2 == NULL) return -33;
    xml->meterSig2 = __SearchNode(xml->staffDef2, "meterSig");
    if (xml->meterSig2 == NULL) return -34;

    return 0;
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

static char* __TranslateRoleToText(MENG_Role role)
{
    switch (role)
    {
        case MENG_ROLE_AUTHOR:      return "author";
        case MENG_ROLE_COMPOSER:    return "composer";
        case MENG_ROLE_ARRANGER:    return "arranger";
        case MENG_ROLE_EDITOR:      return "editor";
        case MENG_ROLE_CONTRIBUTOR: return "contributor";
        case MENG_ROLE_FUNDER:      return "funder";
        case MENG_ROLE_LYRICIST:    return "lyricist";
        case MENG_ROLE_LIBRETTIST:  return "librettist";
        case MENG_ROLE_SPONSOR:     return "sponsor";
        default:                    return NULL;  // or "unknown"
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

static xmlChar* __uintIntoXmlChar(unsigned int n)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", n);
    return (xmlChar*) strdup(buffer);
}

static MENG_TempoPlacement __TranslateTempoPlacementByText(const char* place_text)
{
    if (place_text == NULL) return -1;

    if (strcmp(place_text, "above") == 0)
        return MENG_TEMPOPLACEMENT_ABOVE;
    else if (strcmp(place_text, "below") == 0)
        return MENG_TEMPOPLACEMENT_BELOW;
    else if (strcmp(place_text, "between") == 0)
        return MENG_TEMPOPLACEMENT_BETWEEN;
    else if (strcmp(place_text, "within") == 0)
        return MENG_TEMPOPLACEMENT_WITHIN;
    else
        return -1;
}

static MENG_Duration __TranslateDurationByText(const char* dur_text)
{
    if (dur_text == NULL) return -1;
    if (strcmp(dur_text, "1") == 0)
        return MENG_DURATION_WHOLE;
    else if (strcmp(dur_text, "2") == 0)
        return MENG_DURATION_HALF;
    else if (strcmp(dur_text, "4") == 0)
        return MENG_DURATION_QUARTER;
    else if (strcmp(dur_text, "8") == 0)
        return MENG_DURATION_EIGHTH;
    else if (strcmp(dur_text, "16") == 0)
        return MENG_DURATION_SIXTEENTH;
    else
        return -1;
}

static MENG_Octave __TranslateOctaveByText(const char* oct_text)
{
    if (oct_text == NULL) return -1;
    if (strcmp(oct_text, "1") == 0)
        return MENG_OCTAVE_1;
    else if (strcmp(oct_text, "2") == 0)
        return MENG_OCTAVE_2;
    else if (strcmp(oct_text, "3") == 0)
        return MENG_OCTAVE_3;
    else if (strcmp(oct_text, "4") == 0)
        return MENG_OCTAVE_4;
    else if (strcmp(oct_text, "5") == 0)
        return MENG_OCTAVE_5;
    else if (strcmp(oct_text, "6") == 0)
        return MENG_OCTAVE_6;
    else if (strcmp(oct_text, "7") == 0)
        return MENG_OCTAVE_7;
    else if (strcmp(oct_text, "8") == 0)
        return MENG_OCTAVE_8;
    else
        return -1;
}

static MENG_Pitch __TranslatePitchByText(const char* pitch_text)
{
    if (pitch_text == NULL) return -1;
    if (strcmp(pitch_text, "c") == 0)
        return MENG_PITCH_C;
    else if (strcmp(pitch_text, "d") == 0)
        return MENG_PITCH_D;
    else if (strcmp(pitch_text, "e") == 0)
        return MENG_PITCH_E;
    else if (strcmp(pitch_text, "f") == 0)
        return MENG_PITCH_F;
    else if (strcmp(pitch_text, "g") == 0)
        return MENG_PITCH_G;
    else if (strcmp(pitch_text, "a") == 0)
        return MENG_PITCH_A;
    else if (strcmp(pitch_text, "b") == 0)
        return MENG_PITCH_B;
    else
        return -1;
}