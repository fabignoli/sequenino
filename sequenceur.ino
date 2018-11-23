
//---------------------------------------------------------------------
// SEQUENCEUR Version 1.6
// Projet de sequenceur 2 pistes 16 pas (ou 1 piste 32 pas)
// F. ABIGNOLI
// 24/04/2013
//---------------------------------------------------------------------
//---------------------------------------------------------------------

// 03/06/2015
// - Modification du mode MIDI+CV
//    - le MIDI dans ce cas ne sort que sur le channel 1 et joue la piste 1
//    - les CV sortent chacun sur leur piste
// - Correction des Note-off midi sur les 2 canaux
//   
//
// 01/06/2015
// - correction bug velocite midi
// - prise en compte 2 canal midi en mode 2 x 16 notes
// - prise en compte bouton FUNC sur edition de note pour + ou - 1 octave (ou velocite)
//
// 28/05/2015
// - Utilisation d'une interruption pour la partie midi
//   afin de supprimer les fonctions MIDI.read() et ne plus utiliser la librairie midi
// - creation d'une fonction midiNoteOn pour jouer une note
// - creation d'une fonction midiNoteOff pour arrêter une note
//
// 17/05/2015
// - optimisation de l'affichage
//   utilisation d'un systeme de flag pour afficher les donnees dans la routine principale
//   du mode edition de preset. Suppression de l'affichage dans les handles midi, cv et interne
// - optimisation de tests redondants
// - prise en compte STOP/PLAY barre de transport dans les menus presets ou setup et si horloge interne
// 05/04/2015
//   passage de la routine Playsequencer en interruption via timer
//   (même timer que celui des encodeurs)

// 03/04/2015
//		correction de bugs
//		modification affichage
// 17/05/2013
// 		entree de donne sequence via clavier
//		reset de la sequence
//		Jouer note en midi
// 10/05/2013
//		bug affichage curseur sequenceur
//		bug	affichage curseur edition
//		Ajout synchro externe analogique
//		Ajout divider horloge x12 et x24
// 08/05/2013
// 		Ajout horloge interne via timer 1
// 30/04/2013
//		sequencement 2 x 16 pas ou 1 x 32 pas en CV via midi-clock
//---------------------------------------------------------------------

#define CURRENT_VERSION "1.6"

// Routine affichage de la RAM libre
#include <MemoryFree.h>

//----------------------------------------------
// Bibliotheque et definitions pour gerer le LCD
//----------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif
LiquidCrystal_I2C lcd(0x27,20,4);  // sLCD adress 0x27, 20 car, 4 lignes
byte rafLCD;                        // Flag rafraichissement LCD
uint8_t symbup[8]  = {0x4,0xe,0xf5,0x4,0x4,0x4,0x4};    	//car fleche up
uint8_t symbdown[8]  = {0x4,0x4,0x4,0x4,0xf5,0xe,0x4};  	//car fleche down
uint8_t symbTriRight[8]  = {0x8,0xc,0xe,0xf,0xe,0xc,0xc8};  //car triangle right
uint8_t symbTriLeft[8]  = {0x2,0x6,0xe,0x1e,0xe,0x6,0x2};  	//car fleche down
uint8_t symbStop[8]  = {0x0,0x1f,0x1f,0x1f,0x1f,0x1f,0x0};  		//car stop
uint8_t symbBarre[8]  = {0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4};  		//car separation

#define CAR_RIGHT 126   // Numero car fleche vers la droite 
#define CAR_LEFT  127   // Numero car fleche vers la gauche
#define CAR_UP    0     // Numero car fleche vers le haut
#define CAR_DOWN  1     // Numero car fleche vers le bas
#define CAR_TRIRIGHT  2     // Numero car Triangle vers la droite
#define CAR_TRILEFT   3     // Numero car Triangle vers la gauche
#define CAR_STOP      4     // Numero car Stop
#define CAR_BARRE     5     // Numero car Barre Verticale



#define CAR_SEP   5   // Trait vertical de séparation
#define LCDON      1     
#define LCDOFF     0    

// --------------------------------
// declaration des pins sur ARDUINO
// --------------------------------
#define MIDIIN  0   // Entree MIDI
#define MIDIOUT 1   // Sortie MIDI
#define CLOCKIN 2   // Pin D2 : clock externe 
#define RESETIN 3   // Pin D3 : reset externe
#define GATE1   A0   // Pin D4 : commande GATE 1
#define GATE2   A1   // Pin D5 : commande GATE 2 

#define DAC1    8   // Pin D7 pour DAC no 1
#define DAC2    9   // Pin D8 pour DAC no 2

#define _CLOCKMIDI	0
#define _CLOCKCV	1
#define	_CLOCKINT	2
#define MIDI_CHANNEL_1	1
#define MIDI_CHANNEL_2	2

//------------------------------------------------
// DECLARATION POUR HANDLER MIDI PAR INTERRUPTIONS
//------------------------------------------------
enum { 	MIDI_OTHER, 
		MIDI_NOTE_OFF, 
		MIDI_NOTE_ON, 
		MIDI_CONTROL_CHANGE,
		MIDI_PROGRAM_CHANGE,
		MIDI_PITCH_BEND
};
// volatile uint8_t usemidi = 0;	// enable midi or manual mode
// volatile uint8_t noteon = 0;	// Etat de la sortie son
uint8_t midimode = MIDI_OTHER;
int8_t midibytesleft = 0;
// uint8_t midilastnote = 0xFF;
uint8_t midibuffer[2];



//------------------------------------------------
// Bibliotheque et declarations pour gerer les DAC
//------------------------------------------------
#include "SPI.h"      // SPI library
//int del=0;            // used for various delays
word outputValue = 0; // a word is a 16-bit number
byte data = 0;        // and a byte is an 8-bit number
int vdac1 = 0;         // valeur a ecrire dans les DAC
int vdac2 = 0;         // valeur a ecrire dans les DAC
// int lastvdac = 0;     // Stockage ancienne valeur vdac

// Tableaux de conversion midi -> CV
//----------------------------------
#include "sequenceur.h"    // Valeur DAC suivant note midi
byte CountNote;     		// Nombre de note appuyees sur le clavier a un instant "t"
int	CountNoteOut;			// Nombre de note midi en out
byte LastCountNote; 		// Derniere valeur de midi note pour voir la difference                      
int lastnote=0;     		// Valeur du dac de la derniere note
byte CurrentNote;			// Derniere note jouee en midi
byte CurrentVelocity;		// Velocite de la derniere note jouee
byte LastClock;     		// deniere valeur de clock

int PosSeq;         		// Position en cours dans la sequence
int LastPosSeq;     		// Position en cours dans la sequence
byte LastNoteOn1,LastNoteOn2;	// Derniere notes midi jouees
byte ClockMidi;     		//  Horloge maitre midi en IN
byte ClockCv;     			//  Horloge maitre analogique
volatile byte ClockInt;     //  Horloge maitre interne

int Tick;       			// tick horloge 
int LastTick;   			// last tick horloge


long StartGate;     		// Time du gate on
long EndGate;				// Time du gate off
#define	_UNITGATE	3		// Unite de gate en ms
#define	_UNITNOTEOFF	3	// Unite de note-off en ms

byte NbNoteSeq;     		// Nombre de note appuyee et stockee dans NoteArp
char BuffAff[24];     		// Buffer pour transferer donnees en FLASH
int myreset,oldreset;
byte ModeRecord=0;			// Mode enregistrement (0 rien, 1 enregistrement)
byte TogglePlay=0;			// toggle affichage play
int PosRec;					// Pointeur position enregistrement
int LastPosRec;				// Ancien pointeur position enregistremetn
byte ModePlay=0;			// Mode lecture (0 a l'arret, 1 en cours de jeu)
#define DELAY_RETRIG  5   	// Delai du retrig en ms
volatile byte ListeClockMenu[]={1,2,3,4,6,8,12,24};

#define ITEMS_CLOCK 5
// donnees pour positionnement mode colonne
byte ModeColonne[]={4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,1,2,3,0};
// tableau calcule dynamiquement pour le mode de lecture pendulum
byte ModePendulum[32];
byte PosPendulum=0;


// Gestion des menus
//------------------
byte xm=0;                  // Position x dans la chaine de menu
byte ym=0;                  // Position y dans la chaine de menu
byte lxm=0;                 // last position x menu
byte lym=0;                 // last position y menu
#define EDITSEQ 0       	// Mode edition Sequenceur
#define EDITPRESETS 1   	// Mode edition Name PRESET
#define MENUPRESETS 5 	
#define EDITSETUP 2     	// Mode edition setup
#define EDITEUCLIDIAN 3    	// Mode edition EUCLIDIAN

byte modeMenu=EDITSEQ;  	// Mode par defaut au demarrage

// ENCODEURS ET BOUTONS
//---------------------
int pinA[] = {5,7};               	// Pins A des encodeurs
int pinB[] = {4,6};               	// Pins B des encodeurs
unsigned int tcnt2;               	// Counter encoder 
unsigned int tcnt1;               	// Counter BPM 

volatile int lastEncoded[] = {0,0}; // Dernieres valeur lue encodeurs
volatile int counter[] = {0,0};     // Valeur courante des encodeurs
int bcounter[] = {0,0};             // valeurs encodeurs divisees par 4
int lcounter[] = {0,0};             // valeurs précédentes des encodeurs
byte valbouton;                     // Numero du bouton appuye
byte lastBoutonNav[] = {0,0};       // Stockage du dernier etat dun bouton
byte valencodeurs;                  // numero encodeur et sens
int enc1=0;
int enc2=0;

#define PINTRANSPORT 1
#define PINEDIT 0
#define ENC1DOWN  1     // Encodeur 1 down
#define ENC2DOWN  2     // Encodeur 2 down
#define ENC1UP    11    // Encodeur 1 up
#define ENC2UP    12    // Encodeur 2 up
#define B_REC     1     // Numero bouton Record
#define B_FUNC    2     // Numero bouton fonction diverse
#define B_STOP    3     // Numero bouton Stop
#define B_PLAY    4     // Numero bouton Play
#define B_PRESETS 1     // Numero bouton menu PRESETS
#define B_ENC1    2     // Numero bouton encodeur 1
#define B_ENC2    3     // Numero bouton encodeur 2
#define B_SETUP   4     // Numero bouton menu SETUP
#define B_MAINT   9     // Derniere touche maintenue appuyee

// Donnes stockees dans les presets
//---------------------------------
byte noPreset=0;                        // Numero du preset courant
byte noBanque=0;                        // Numero de banque courante
char nomPreset[18];                     // Nom du preset courant
byte SeqNotes[32];						// Sequence note des 2 pistes
byte SeqGates[32];						// Sequence gate (ou velocite) des 2 pistes
byte setup_Steps=16;					// Nombre de steps (<=16 si 2 pistes, <=32 si une piste)
byte setup_ModeTrack=0;					// Mode track : 0->2 x 16, 1->1 x  32, 2->1 piste 16 + velo	
byte setup_Bpm=120;						// BPM si horloge locale
volatile byte setup_ClockIn=0;			// Type clock: 0->MIDI, 1:external analogique, 2:internal
byte setup_SensSeq=0;					// Sens de la sequence (up, down, etc...)
#define VOCT    0         				// Mode Volt par octave
#define HVOLT   1         				// Mode Hertz par volt
#define SX150   2         				// Mode SX150
byte setup_ModeCV=0;     	     		// Mode CV actif (Octave, Hertz, SX150...)
byte setup_ModeOut=1;					// Type de sortie (0:MIDI, 1:CV, 2:MIDI+CV)
byte setup_ClockDivider=0;  			// Diviseur de l'horloge maitre( midi ou CV)
#define MAXPROG 128
#define MAXBANK 2
#define PRESETSIZE  128
#define BANKSIZE  16384
byte posCarPreset=2;                    // Position x edition nom preset
int OldCurSeq;							// Sauvegarde position du curseur
//int CurSeq;
int MaxPosSeq;
volatile byte F_IntCVReset=0;			// Flag reset CV externe
// DIVERS
//-------
int i,j;
byte myByte1,myByte2,myByte3;           // Variable de travail
word myWord1,myWord2;                   // Variable de travail
int myInt;                              // Variable de travail
float myfloat;
int PosAff=0;							// Position affichage pour edition note ou gate
int PosCurSetup=0;						// Position du curseur pour fonction de setup
unsigned int EucliSeq[2];
byte flagAffLcd1=0;						// Flag reaffichage LCD
byte flagAffLcd2=0;						// Flag reaffichage LCD
volatile byte flagAffLcd3=0;						// Flag reaffichage LCD
byte flagAffLcd4=0;						// Flag reaffichage LCD
byte flagAffLcd5=0;						// Flag reaffichage LCD
volatile byte cn1=0;
//----------------------------------------------------- 
// HANDLER CLOCK INTERNE
// Procedure geree par timer 2
//----------------------------------------------------- 
ISR(TIMER1_OVF_vect) {  
	TCNT1 = tcnt1;      // Reload du timer
	// Si horloge active n'est pas interne
	if (setup_ClockIn!=_CLOCKINT)
		return;						// On sort
	if (ModePlay!=1)
		return;
	ClockInt++;									// Incrementation horloge midi
	if (ClockInt==1) {       					// Si debut de division
			Tick++;								// on change tick principal
	}    
	MidiSendClock();
	// On verifie si la clockMidi doit être remise a zero
	if (ClockInt==24/ListeClockMenu[setup_ClockDivider]) {	// Si valeur atteinte
		ClockInt=0;     										// remet a zero et ca rejouera au prochain tick midi
	}
}


//-------------------
// Choix du sous-menu
//-------------------
void gestion_menu_principal(void) {
	switch(modeMenu) {
		case EDITSEQ:
			//lcd.cursor();
			PosAff=0;
			gestion_menu_sequenceur();
			//lcd.noCursor();
			break; 
		case EDITPRESETS:
			gestion_menu_presets();
			break; 
		case EDITSETUP:
			gestion_menu_setup();
			break; 
			
	}
}

//----------------------------
// INITIALISATION DU PROGRAMME
//----------------------------
void setup() {

//pinMode(0,INPUT);
  // Initialisation du LCD
  //----------------------
  lcd.init();                     // init du lcd 
  lcd.backlight();                // Retroeclairage
  lcd.createChar(CAR_UP, symbup);      			// Creation fleche UP 
  lcd.createChar(CAR_DOWN, symbdown);    		// Creation fleche DOWN
  lcd.createChar(CAR_TRIRIGHT,symbTriRight);	// Creation Triangle vers la droite
  lcd.createChar(CAR_TRILEFT,symbTriLeft);		// Creation Triangle vers la gauche
  lcd.createChar(CAR_STOP,symbStop);			// Creation carre pour stop
  lcd.createChar(CAR_BARRE,symbBarre);			// Creation barre verticale
  
  
  lcd.home();                     // curseur vers home
  lcd.setCursor(0,0);
  lcd.print(F("********************"));
  lcd.setCursor(0,1);
  lcd.print(F("*    SEQUENINO     *"));   // Affichage titre
  lcd.setCursor(0,2);
  lcd.print(F("*   Version "));   // Affichage titre
  lcd.print(F(CURRENT_VERSION));
  lcd.print(F("    *"));   // Affichage titre
  
  lcd.setCursor(0,3);
  lcd.print(F("********************"));
  
  delay(2000);          
  
  lcd.clear();                      // Effacement ecran
  lcd.print(F("RAM Free : "));
  lcd.print(freeMemory(),DEC);
  lcd.print(F(" Oct."));
  delay(2000);
  
  lcd.clear();                      // Effacement ecran
  rafLCD=0;                         // flag affichage LCD
  vdac1=0;
  vdac2=0;
  // Initialisation du MIDI
  //-----------------------
  CountNote=0;                      // Compteur de note actives
  LastCountNote=0;                  // Dernier compteur de note
 
  // Initialisation des pins
  //------------------------  
  pinMode(2,INPUT);         // Clock externe IN
  pinMode(3,INPUT);         // Reset externe IN
  pinMode(10, OUTPUT);      // Pin prog DAC
  pinMode(DAC1,OUTPUT);     // Selection 
  pinMode(DAC2,OUTPUT);     // Selection DAC2
  pinMode(GATE1,OUTPUT);    // Gate 1
  pinMode(GATE2,OUTPUT);    // Gate 2
  digitalWrite(GATE1,LOW);  // Reset Gate 1
  digitalWrite(GATE2,LOW);  // Reset Gate 2
  
  // Mise en route des encodeurs
  //----------------------------
  encodeursSetup();
  // Mise en route du timer 16 bits pour le BPM clock interne
  //---------------------------------------------------------
  BpmTimerSetup();
  // Gestion de la SPI pour les 2 DACS
  //----------------------------------
  SPI.begin();                  // Initialisation SPI
  SPI.setBitOrder(MSBFIRST);    // Octet de poid fort en premier
  writeDAC(DAC1,0);             // RAZ DAC1
  writeDAC(DAC2,0);             // RAZ DAC2
  
  // Definition des interruptions externe
  //-------------------------------------
  attachInterrupt(0, MyHandleClockSeqCv, RISING);	// Interruption sur entree clockt
  attachInterrupt(1, MyHandleStopSeqCv, RISING);	// Interruption sur entree reset
  
  // Initialisation variables diverses
  //----------------------------------
  setup_ClockDivider=0;
  memset(nomPreset,0,sizeof(nomPreset));
  ModePlay=0;
  ModeRecord=0;
  ClockMidi=0;
  ClockInt=0;
  ClockCv=0;
  PosSeq=-1;
  PosSeq=-1;
  OldCurSeq=-1;
  InitSequence();
  load_preset(0,0);
  CountNoteOut=0;
  midi_init();			// Initialisation du midi par interruption
}

//--------------------
// PROGRAMME PRINCIPAL
//--------------------
void loop() {
byte byt1,byt2;
	gestion_menu_principal();
}
