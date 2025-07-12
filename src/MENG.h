#ifndef MENG_H
#define MENG_H

#include <stdbool.h>

/*Might return 0 on failure, but verovio will output an Error message*/
int MENG_Init(char* verovio_resource_path);
bool MENG_IsInit();
int MENG_Destroy();

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
        typedef struct MENG_ScoreDef MENG_ScoreDef;
            typedef enum MENG_MIDI_Instrument MENG_MIDI_Instrument;
            typedef struct MENG_InstrDef MENG_InstrDef;
            typedef struct MENG_StaffDef MENG_StaffDef;
                typedef struct MENG_Clef MENG_Clef;
                typedef struct MENG_MeterSig MENG_MeterSig;
        typedef struct MENG_Section MENG_Section;
            typedef struct MENG_Measure MENG_Measure;
                typedef struct MENG_Staff MENG_Staff;
                    typedef struct MENG_Layer MENG_Layer;
                        typedef struct MENG_Order MENG_Order;
                        typedef enum MENG_OrderType MENG_OrderType;
                        typedef union MENG_OrderElement MENG_OrderElement;
                        typedef struct MENG_Note MENG_Note;
                        typedef struct MENG_Rest MENG_Rest;
                        typedef struct MENG_Chord MENG_Chord;
                        typedef struct MENG_Beam MENG_Beam;
                        typedef enum MENG_Duration MENG_Duration;
                        typedef enum MENG_Octave MENG_Octave;
                        typedef enum MENG_Pitch MENG_Pitch;
                typedef enum MENG_TempoPlacement MENG_TempoPlacement;
                typedef struct MENG_Tempo MENG_Tempo; 
        typedef struct MENG_Tie MENG_Tie;  


// <------MEI Methods------>
MENG_MEI MENG_LoadMEIFile(char* file);
char* MENG_OutputMeiAsChar(MENG_MEI mei);
void MENG_PrintMEI(MENG_MEI mei);

// <------MEI------>
typedef enum MENG_OrderType
{
    MENG_TYPE_UNDEFINED = 0,
    MENG_TYPE_NOTE,
    MENG_TYPE_REST,
    MENG_TYPE_CHORD,
    MENG_TYPE_BEAM
} MENG_OrderType;

typedef union MENG_OrderElement
{
    MENG_Note* note;
    MENG_Rest* rest;
    MENG_Chord* chord;
    MENG_Beam* beam;
} MENG_OrderElement;

typedef struct MENG_Order
{
    MENG_OrderType type;
    MENG_OrderElement element;
} MENG_Order;

typedef struct MENG_Tie
{
    char* startid; // Ids should contain # at the beginning
    char* endid;
} MENG_Tie;

typedef enum MENG_Duration
{
    MENG_DURATION_WHOLE = 1,
    MENG_DURATION_HALF = 2,
    MENG_DURATION_QUARTER = 4,
    MENG_DURATION_EIGHTH = 8,
    MENG_DURATION_SIXTEENTH = 16
} MENG_Duration;

typedef enum MENG_Octave
{
    MENG_OCTAVE_1 = 1,
    MENG_OCTAVE_2 = 2,
    MENG_OCTAVE_3 = 3,
    MENG_OCTAVE_4 = 4,
    MENG_OCTAVE_5 = 5,
    MENG_OCTAVE_6 = 6,
    MENG_OCTAVE_7 = 7,
    MENG_OCTAVE_8 = 8
} MENG_Octave;

typedef enum MENG_Pitch
{
    MENG_PITCH_C,
    MENG_PITCH_D,
    MENG_PITCH_E,
    MENG_PITCH_F,
    MENG_PITCH_G,
    MENG_PITCH_A,
    MENG_PITCH_B
} MENG_Pitch;

typedef struct MENG_Note
{
    MENG_Duration dur;
    MENG_Octave oct;
    MENG_Pitch pname;
} MENG_Note;

typedef struct MENG_Rest
{
    MENG_Duration dur;
} MENG_Rest;

typedef struct MENG_Chord
{
    MENG_Duration dur;
    MENG_Note** notes;          // Note dur value must = 0, as dur is already defined in chord.dur
    unsigned int notes_count;   // By definition, notes cannot be repeated on the same chord
} MENG_Chord;

typedef struct MENG_Beam
{
    MENG_Note** notes;
    unsigned int notes_count;
    MENG_Rest** rests;
    unsigned int rests_count;
    MENG_Chord** chords;
    unsigned int chords_count;
    MENG_Order** order;
    unsigned int order_count;
} MENG_Beam;

typedef struct MENG_Layer
{
    MENG_Note** notes;
    unsigned int notes_count;
    MENG_Rest** rests;
    unsigned int rests_count;
    MENG_Chord** chords;
    unsigned int chords_count;
    MENG_Beam** beams;
    unsigned int beams_count;
    MENG_Order** order;
    unsigned int order_count;
} MENG_Layer;

typedef enum MENG_TempoPlacement
{
    MENG_TEMPOPLACEMENT_ABOVE,
    MENG_TEMPOPLACEMENT_BELOW,
    MENG_TEMPOPLACEMENT_BETWEEN,
    MENG_TEMPOPLACEMENT_WITHIN
} MENG_TempoPlacement;

typedef struct MENG_Tempo
{
    unsigned int bpm;
    unsigned int tstamp;
    MENG_TempoPlacement place;
    unsigned char staff;
} MENG_Tempo;

typedef struct MENG_Staff 
{
    MENG_Layer** layers;
    unsigned int layers_count;
} MENG_Staff;

typedef struct MENG_Measure
{
    MENG_Staff staffs[2];
    MENG_Tempo tempo;
} MENG_Measure;

typedef struct MENG_Section
{
    MENG_Measure** measures;
    unsigned int measures_count;
    MENG_Tie** ties;
    unsigned int ties_count;
} MENG_Section;

typedef enum MENG_MIDI_Instrument
{
    // Piano
    MENG_MIDI_INSTRUMENT_ACOUSTIC_GRAND_PIANO = 0,
    MENG_MIDI_INSTRUMENT_BRIGHT_ACOUSTIC_PIANO = 1,
    MENG_MIDI_INSTRUMENT_ELECTRIC_GRAND_PIANO = 2,
    MENG_MIDI_INSTRUMENT_HONKY_TONK_PIANO = 3,
    MENG_MIDI_INSTRUMENT_ELECTRIC_PIANO_1 = 4,
    MENG_MIDI_INSTRUMENT_ELECTRIC_PIANO_2 = 5,
    MENG_MIDI_INSTRUMENT_HARPSICHORD = 6,
    MENG_MIDI_INSTRUMENT_CLAVINET = 7,

    // Chromatic Percussion
    MENG_MIDI_INSTRUMENT_CELESTA = 8,
    MENG_MIDI_INSTRUMENT_GLOCKENSPIEL = 9,
    MENG_MIDI_INSTRUMENT_MUSIC_BOX = 10,
    MENG_MIDI_INSTRUMENT_VIBRAPHONE = 11,
    MENG_MIDI_INSTRUMENT_MARIMBA = 12,
    MENG_MIDI_INSTRUMENT_XYLOPHONE = 13,
    MENG_MIDI_INSTRUMENT_TUBULAR_BELLS = 14,
    MENG_MIDI_INSTRUMENT_DULCIMER = 15,

    // Organ
    MENG_MIDI_INSTRUMENT_DRAWBAR_ORGAN = 16,
    MENG_MIDI_INSTRUMENT_PERCUSSIVE_ORGAN = 17,
    MENG_MIDI_INSTRUMENT_ROCK_ORGAN = 18,
    MENG_MIDI_INSTRUMENT_CHURCH_ORGAN = 19,
    MENG_MIDI_INSTRUMENT_REED_ORGAN = 20,
    MENG_MIDI_INSTRUMENT_ACCORDION = 21,
    MENG_MIDI_INSTRUMENT_HARMONICA = 22,
    MENG_MIDI_INSTRUMENT_BANDONEON = 23,

    // Guitar
    MENG_MIDI_INSTRUMENT_ACOUSTIC_GUITAR_NYLON = 24,
    MENG_MIDI_INSTRUMENT_ACOUSTIC_GUITAR_STEEL = 25,
    MENG_MIDI_INSTRUMENT_ELECTRIC_GUITAR_JAZZ = 26,
    MENG_MIDI_INSTRUMENT_ELECTRIC_GUITAR_CLEAN = 27,
    MENG_MIDI_INSTRUMENT_ELECTRIC_GUITAR_MUTED = 28,
    MENG_MIDI_INSTRUMENT_OVERDRIVEN_GUITAR = 29,
    MENG_MIDI_INSTRUMENT_DISTORTION_GUITAR = 30,
    MENG_MIDI_INSTRUMENT_GUITAR_HARMONICS = 31,

    // Bass
    MENG_MIDI_INSTRUMENT_ACOUSTIC_BASS = 32,
    MENG_MIDI_INSTRUMENT_ELECTRIC_BASS_FINGER = 33,
    MENG_MIDI_INSTRUMENT_ELECTRIC_BASS_PICKED = 34,
    MENG_MIDI_INSTRUMENT_FRETLESS_BASS = 35,
    MENG_MIDI_INSTRUMENT_SLAP_BASS_1 = 36,
    MENG_MIDI_INSTRUMENT_SLAP_BASS_2 = 37,
    MENG_MIDI_INSTRUMENT_SYNTH_BASS_1 = 38,
    MENG_MIDI_INSTRUMENT_SYNTH_BASS_2 = 39,

    // Strings
    MENG_MIDI_INSTRUMENT_VIOLIN = 40,
    MENG_MIDI_INSTRUMENT_VIOLA = 41,
    MENG_MIDI_INSTRUMENT_CELLO = 42,
    MENG_MIDI_INSTRUMENT_CONTRABASS = 43,
    MENG_MIDI_INSTRUMENT_TREMOLO_STRINGS = 44,
    MENG_MIDI_INSTRUMENT_PIZZICATO_STRINGS = 45,
    MENG_MIDI_INSTRUMENT_ORCHESTRAL_HARP = 46,
    MENG_MIDI_INSTRUMENT_TIMPANI = 47,

    // Ensemble
    MENG_MIDI_INSTRUMENT_STRING_ENSEMBLE_1 = 48,
    MENG_MIDI_INSTRUMENT_STRING_ENSEMBLE_2 = 49,
    MENG_MIDI_INSTRUMENT_SYNTHSTRINGS_1 = 50,
    MENG_MIDI_INSTRUMENT_SYNTHSTRINGS_2 = 51,
    MENG_MIDI_INSTRUMENT_CHOIR_AAHS = 52,
    MENG_MIDI_INSTRUMENT_VOICE_OOHS = 53,
    MENG_MIDI_INSTRUMENT_SYNTH_VOICE = 54,
    MENG_MIDI_INSTRUMENT_ORCHESTRA_HIT = 55,

    // Brass
    MENG_MIDI_INSTRUMENT_TRUMPET = 56,
    MENG_MIDI_INSTRUMENT_TROMBONE = 57,
    MENG_MIDI_INSTRUMENT_TUBA = 58,
    MENG_MIDI_INSTRUMENT_MUTED_TRUMPET = 59,
    MENG_MIDI_INSTRUMENT_FRENCH_HORN = 60,
    MENG_MIDI_INSTRUMENT_BRASS_SECTION = 61,
    MENG_MIDI_INSTRUMENT_SYNTH_BRASS_1 = 62,
    MENG_MIDI_INSTRUMENT_SYNTH_BRASS_2 = 63,

    // Reed
    MENG_MIDI_INSTRUMENT_SOPRANO_SAX = 64,
    MENG_MIDI_INSTRUMENT_ALTO_SAX = 65,
    MENG_MIDI_INSTRUMENT_TENOR_SAX = 66,
    MENG_MIDI_INSTRUMENT_BARITONE_SAX = 67,
    MENG_MIDI_INSTRUMENT_OBOE = 68,
    MENG_MIDI_INSTRUMENT_ENGLISH_HORN = 69,
    MENG_MIDI_INSTRUMENT_BASSOON = 70,
    MENG_MIDI_INSTRUMENT_CLARINET = 71,

    // Pipe
    MENG_MIDI_INSTRUMENT_PICCOLO = 72,
    MENG_MIDI_INSTRUMENT_FLUTE = 73,
    MENG_MIDI_INSTRUMENT_RECORDER = 74,
    MENG_MIDI_INSTRUMENT_PAN_FLUTE = 75,
    MENG_MIDI_INSTRUMENT_BLOWN_BOTTLE = 76,
    MENG_MIDI_INSTRUMENT_SHAKUHACHI = 77,
    MENG_MIDI_INSTRUMENT_WHISTLE = 78,
    MENG_MIDI_INSTRUMENT_OCARINA = 79,

    // Synth Lead
    MENG_MIDI_INSTRUMENT_LEAD_1_SQUARE = 80,
    MENG_MIDI_INSTRUMENT_LEAD_2_SAWTOOTH = 81,
    MENG_MIDI_INSTRUMENT_LEAD_3_CALLIOPE = 82,
    MENG_MIDI_INSTRUMENT_LEAD_4_CHIFF = 83,
    MENG_MIDI_INSTRUMENT_LEAD_5_CHARANG = 84,
    MENG_MIDI_INSTRUMENT_LEAD_6_VOICE = 85,
    MENG_MIDI_INSTRUMENT_LEAD_7_FIFTHS = 86,
    MENG_MIDI_INSTRUMENT_LEAD_8_BASS_LEAD = 87,

    // Synth Pad
    MENG_MIDI_INSTRUMENT_PAD_1_NEW_AGE = 88,
    MENG_MIDI_INSTRUMENT_PAD_2_WARM = 89,
    MENG_MIDI_INSTRUMENT_PAD_3_POLYSYNTH = 90,
    MENG_MIDI_INSTRUMENT_PAD_4_CHOIR = 91,
    MENG_MIDI_INSTRUMENT_PAD_5_BOWED = 92,
    MENG_MIDI_INSTRUMENT_PAD_6_METALLIC = 93,
    MENG_MIDI_INSTRUMENT_PAD_7_HALO = 94,
    MENG_MIDI_INSTRUMENT_PAD_8_SWEEP = 95,

    // Synth Effects
    MENG_MIDI_INSTRUMENT_FX_1_RAIN = 96,
    MENG_MIDI_INSTRUMENT_FX_2_SOUNDTRACK = 97,
    MENG_MIDI_INSTRUMENT_FX_3_CRYSTAL = 98,
    MENG_MIDI_INSTRUMENT_FX_4_ATMOSPHERE = 99,
    MENG_MIDI_INSTRUMENT_FX_5_BRIGHTNESS = 100,
    MENG_MIDI_INSTRUMENT_FX_6_GOBLINS = 101,
    MENG_MIDI_INSTRUMENT_FX_7_ECHOES = 102,
    MENG_MIDI_INSTRUMENT_FX_8_SCI_FI = 103,

    // Ethnic
    MENG_MIDI_INSTRUMENT_SITAR = 104,
    MENG_MIDI_INSTRUMENT_BANJO = 105,
    MENG_MIDI_INSTRUMENT_SHAMISEN = 106,
    MENG_MIDI_INSTRUMENT_KOTO = 107,
    MENG_MIDI_INSTRUMENT_KALIMBA = 108,
    MENG_MIDI_INSTRUMENT_BAG_PIPE = 109,
    MENG_MIDI_INSTRUMENT_FIDDLE = 110,
    MENG_MIDI_INSTRUMENT_SHANAI = 111,

    // Percussive
    MENG_MIDI_INSTRUMENT_TINKLE_BELL = 112,
    MENG_MIDI_INSTRUMENT_AGOGO = 113,
    MENG_MIDI_INSTRUMENT_STEEL_DRUMS = 114,
    MENG_MIDI_INSTRUMENT_WOODBLOCK = 115,
    MENG_MIDI_INSTRUMENT_TAIKO_DRUM = 116,
    MENG_MIDI_INSTRUMENT_MELODIC_TOM = 117,
    MENG_MIDI_INSTRUMENT_SYNTH_DRUM = 118,
    MENG_MIDI_INSTRUMENT_REVERSE_CYMBAL = 119,

    // Sound Effects
    MENG_MIDI_INSTRUMENT_GUITAR_FRET_NOISE = 120,
    MENG_MIDI_INSTRUMENT_BREATH_NOISE = 121,
    MENG_MIDI_INSTRUMENT_SEASHORE = 122,
    MENG_MIDI_INSTRUMENT_BIRD_TWEET = 123,
    MENG_MIDI_INSTRUMENT_TELEPHONE_RING = 124,
    MENG_MIDI_INSTRUMENT_HELICOPTER = 125,
    MENG_MIDI_INSTRUMENT_APPLAUSE = 126,
    MENG_MIDI_INSTRUMENT_GUNSHOT = 127
} MENG_MIDI_Instrument;

typedef struct MENG_InstrDef
{
    unsigned char midi_channel; // Range [0, 15]
    MENG_MIDI_Instrument midi_instrnum; // Range [0, 127]
    unsigned char midi_volume; // Range [0, 100]
} MENG_InstrDef;

typedef struct MENG_Clef
{
    char shape; // Example: "G", "F"...
    unsigned char line;
} MENG_Clef;

typedef struct MENG_MeterSig
{
    unsigned char count;
    unsigned char unit;
} MENG_MeterSig;

typedef struct MENG_StaffDef
{
    unsigned int n; 
    unsigned char lines;
    MENG_Clef clef;
    char* keySig; // Example: "1f, 2s, 3f"
    MENG_MeterSig meterSig;
} MENG_StaffDef;

typedef struct MENG_ScoreDef
{
    char* grpSym;
    bool barthru;
    char* label;
    char* labelAbbr;
    MENG_InstrDef instrDef;
    MENG_StaffDef staffDef1;
    MENG_StaffDef staffDef2;
} MENG_ScoreDef;

typedef struct MENG_Music
{
    MENG_ScoreDef scoreDef;
    MENG_Section section;
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
    MENG_PubStmt pubStmt;
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