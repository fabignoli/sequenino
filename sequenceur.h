
// Definition des valeurs DAC
// pour conversion midi vers CV
// Standard Volt/Octave
// (61 VALEURS)
static prog_uint16_t VOct[] PROGMEM = {
0,68,137,205,273,341,410,478,546,614,683,751,
819,887,956,1024,1092,1160,1229,1297,1365,1433,1502,1570,
1638,1706,1775,1843,1911,1979,2048,2116,2184,2252,2321,2389,
2457,2525,2594,2662,2730,2798,2867,2935,3003,3071,3140,3208,
3276,3344,3413,3481,3549,3617,3686,3754,3822,3890,3959,4027,
4095,
};

// Definition des valeurs DAC
// pour conversion midi vers CV
// Standard Volt/Octave
// Juste les notes blanches
// (36 VALEURS)
static prog_uint16_t VOct_blanches[] PROGMEM = {
0, 137, 273,341, 478, 614, 751,
819, 956, 1092,1160, 1297, 1433, 1570,
1638, 1775, 1911,1979, 2116, 2252, 2389,
2457, 2594, 2730,2798, 2935, 3071, 3208,
3276, 3413, 3549,3617, 3754, 3890, 4027,
4095,
};



// Definition des valeurs DAC
// pour conversion midi vers CV
// Standard Hertz/Volt
// (52 VALEURS)
static prog_uint16_t HVolt[] PROGMEM = {
205,217,230,244,258,273,290,307,325,344,365,387,
410,434,460,487,516,547,579,614,650,689,730,773,
819,868,919,974,1032,1093,1158,1227,1300,1377,1459,1546,
1638,1736,1839,1948,2064,2187,2317,2454,2600,2755,2919,3092,
3276,3471,3677,3896,
};

// Definition des valeurs DAC
// pour conversion midi vers CV
// Standard Hertz/Volt
// Juste les notes blanches
// (30 VALEURS)
static prog_uint16_t HVolt_blanches[] PROGMEM = {
205, 230, 258,273, 307, 344, 387,
410, 460, 516,547, 614, 689, 773,
819, 919, 1032,1093, 1227, 1377, 1546,
1638, 1839, 2064,2187, 2454, 2755, 3092,
3276, 3677,
};





// Liste des textes des menus
//---------------------------
//
// Partie Setup
prog_char menu_0[] PROGMEM = "Steps:   |Sens:     ";
prog_char menu_1[] PROGMEM = " Trks:   |Mode:     ";
prog_char menu_2[] PROGMEM = "  Bpm:   | Out:     ";
prog_char menu_3[] PROGMEM = "Clock:   |Cdiv:     ";
prog_char menu_4[] PROGMEM = "Presets>Load";
prog_char menu_5[] PROGMEM = "Presets>Save";
prog_char menu_6[] PROGMEM = "Presets>Name";
prog_char menu_7[] PROGMEM = "Select";
prog_char menu_8[] PROGMEM = "[OK]";
prog_char menu_9[] PROGMEM = "P   |B   |    [OK]";
prog_char menu_10[] PROGMEM = "____________________";


// Tableau de pointeur qui pointe vers les items
PROGMEM const char *TextMenu[] =
{   
  menu_0,
  menu_1,
  menu_2,
  menu_3,
  menu_4,
  menu_5,
  menu_6,
  menu_7,
  menu_8,
  menu_9,
  menu_10
};
#define ITEMS_MENU_PRINCIPAL  10




// Nom des notes
prog_char Note_1[] PROGMEM = "C";
prog_char Note_2[] PROGMEM = "C#";
prog_char Note_3[] PROGMEM = "D";
prog_char Note_4[] PROGMEM = "D#";
prog_char Note_5[] PROGMEM = "E";
prog_char Note_6[] PROGMEM = "F";
prog_char Note_7[] PROGMEM = "F#";
prog_char Note_8[] PROGMEM = "G";
prog_char Note_9[] PROGMEM = "G#";
prog_char Note_10[] PROGMEM = "A";
prog_char Note_11[] PROGMEM = "A#";
prog_char Note_12[] PROGMEM = "B";
PROGMEM const char *Note[] =
{
  Note_1,
  Note_2,
  Note_3,
  Note_4,
  Note_5,
  Note_6,
  Note_7,
  Note_8,
  Note_9,
  Note_10,
  Note_11,
  Note_12
};

// Nom des notes blanches uniquement
prog_char NoteB_1[] PROGMEM = "C";
prog_char NoteB_2[] PROGMEM = "D";
prog_char NoteB_3[] PROGMEM = "E";
prog_char NoteB_4[] PROGMEM = "F";
prog_char NoteB_5[] PROGMEM = "G";
prog_char NoteB_6[] PROGMEM = "A";
prog_char NoteB_7[] PROGMEM = "B";
PROGMEM const char *NoteB[] =
{
  NoteB_1,
  NoteB_2,
  NoteB_3,
  NoteB_4,
  NoteB_5,
  NoteB_6,
  NoteB_7
};



