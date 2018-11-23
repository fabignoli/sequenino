//---------------------------------------
// Procedures MIDI et CV
// Pour le sequenceur
//---------------------------------------
#define F_CPU 16000000UL
#define BAUD 31250
//#define BAUD 31250
#include <util/setbaud.h>
void midi_init() {
// set baud rate
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~_BV(U2X0);
#endif
	// enable rx, enable tx, turn on interrupt
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0B |= _BV(RXCIE0);
	// 8 data bits, no parity, 1 stop bit
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}


//---------------------
// INTERRUPTION MIDI IN
//---------------------
ISR(USART_RX_vect) {
	uint8_t byte = UDR0;		// recuperation de l'octet midi
	
	//------------
	// STATUS BYTE
	//------------
	if (byte >= 0x80) {			// Byte de status si >127
		switch (byte) {
			// MIDI NOTE OFF
			case 0x80:
				midimode = MIDI_NOTE_OFF;
				midibytesleft = 2;
				break;
			// MIDI NOTE ON
			case 0x90:
				midimode = MIDI_NOTE_ON;
				midibytesleft = 2;
				break;
			// MIDI CONTROL CHANGE
			case 0xB0:
				midimode = MIDI_CONTROL_CHANGE;
				midibytesleft = 2;
				break;
			// MIDI PROGRAM CHANGE
			case 0xC0:
				midimode = MIDI_PROGRAM_CHANGE;
				midibytesleft = 2;
				break;
			// MIDI PITCH-BEND
			case 0xE0:
				midimode = MIDI_PITCH_BEND;
				midibytesleft = 2;
				break;
			// MIDI CLOCK
			case 0xF8:  
				MyHandleClockSeqMidi();
				break;
			// MIDI START
			case 0xFA:  
				MyHandleStartSeqMidi();
				break;
			// MIDI CONTINUE
			case 0xFB:  
				MyHandleContinueSeqMidi();
				break;
			// MIDI STOP
			case 0xFC:  
				MyHandleStopSeqMidi();
				break;
			case 0xFE: // active sense
				// ** HANDLER ACTIVE SENSING
				break;
			case 0xFF: // reset
				// ** HANDLER RESET
				break;
			default:
				midimode = MIDI_OTHER;
				midibytesleft = 0;
				break;
		}
	}
	//------------------------
	// SINON OCTETS DE DONNEES
	//------------------------
	else {
		// Stocker les octets a lire si besoin
		if (midibytesleft > 0) {			// Reste il des octets a lire ?
			midibuffer[0] = midibuffer[1];	// Oui, swap des octets
			midibuffer[1] = byte;			// stockage octet courant
			midibytesleft--;				// un octet de moins a lire
		}
		// Traitement du message
		if (midibytesleft <= 0) {
			uint8_t note = midibuffer[0];
			switch (midimode) {
				case MIDI_NOTE_OFF:
					MyHandleNoteOffSeqMidi(1,midibuffer[0],midibuffer[1]);
					// ** HANDLER NOTE OFF
					midibytesleft = 2; // receive more note off events
					break;
				case MIDI_NOTE_ON:
					// turn note off if velocity is zero
					if (midibuffer[1] == 0) {
						MyHandleNoteOffSeqMidi(1,midibuffer[0],midibuffer[1]);		
					}
					else if (midibuffer[1] != 0) {
					// ** HANDLER NOTE ON
						MyHandleNoteOnSeqMidi(1,midibuffer[0],midibuffer[1]);
					}
					midibytesleft = 2; // receive more note on events
					break;
				default:
					break;
			}
		}
	}
}

//-------------------
// Envoi d'un note-on
//-------------------
void MidiSendNoteOn(byte pitch, byte velocity, byte channel) {
uint8_t nstatus = channel-1+0x90;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = nstatus;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = pitch;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = velocity;
}

//--------------------
// Envoi d'un note-off
//--------------------
void MidiSendNoteOff(byte pitch, byte velocity, byte channel) {
uint8_t nstatus = channel-1+0x80;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = nstatus;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = pitch;
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = velocity;
}
//------------------------
// Envoi horloge midiclock
//------------------------
void MidiSendClock() {
	 loop_until_bit_is_set(UCSR0A, UDRE0); 
     UDR0 = 0xF8;

}

//----------------------
// Envoi midiclock start
//----------------------
void MidiSendClockStart() {
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = 0xFA;
}
//---------------------
// Envoi midiclock stop
//---------------------
void MidiSendClockStop() {
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = 0xFC;
}
//-------------------------
// Envoi midiclock continue
//-------------------------
void MidiSendClockContinue() {
	loop_until_bit_is_set(UCSR0A, UDRE0); 
        UDR0 = 0xFB;
		
}


//--------------------------------------
// Arrete toutes les notes en cours
// Envoie midinoteoff et remets GATE a 0
//--------------------------------------
void AllNoteOff(void) {
	GateOff(GATE1);					// Clear Gate 1
	GateOff(GATE2); 				// Clear Gate 2
	// Si sortie MIDI ou MIDI+CV
	if (setup_ModeOut==0 || setup_ModeOut==2) {
		if (CountNoteOut==1) {
			// Arret note premier canal midi
			MidiSendNoteOff(LastNoteOn1,0,MIDI_CHANNEL_1);
			// Si mode 2 pistes
			// et sortie MIDI Uniquement (pas MIDI+CV)
			if (setup_ModeTrack==0 && setup_ModeOut==0)
				// Arret note du deuxieme canal midi
				MidiSendNoteOff(LastNoteOn2,0,MIDI_CHANNEL_2);
			CountNoteOut=0;						// remettre le compteur a 0
		}
	}
}

//---------------------------------------------	
// HANDLER CONTINUE MIDI
// Gestion du message continue pour arpegiateur
//---------------------------------------------	
void MyHandleContinueSeqMidi(void) {
	if (setup_ClockIn!=_CLOCKMIDI)
			return;
}

//------------------------------------------
// HANDLER START sequenceur MIDI
//------------------------------------------	
void MyHandleStartSeqMidi(void) {

	if (setup_ClockIn!=_CLOCKMIDI)
		return;
	ModePlay=0;			// Reset du mode Play
	ClockMidi=0;		// reset clock midi
	PosSeq=-1;			// position de sequence indeterminee
	GateOff(GATE1);		// Couper GATE 1
	GateOff(GATE2);		// Couper Gate 2
	Tick=-1;			// Tick indetermine
	LastTick=-1;		// LastTick indetermine
}

//-----------------------------------------
// HANDLER STOP sequenceur MIDI
// Gestion du message clock midi stop
//-----------------------------------------
void MyHandleStopSeqMidi(void) {
	if (setup_ClockIn!=_CLOCKMIDI)
		return;
	ModePlay=0;						// arret sequence
	if (modeMenu==EDITSEQ) {
		flagAffLcd2=PosSeq+1; 			// Mettre +1 dans flag pour le valider
										// On enleve 1 au moment de l'affichage pour retrouver PosSeq
	}
	ClockMidi=0;			
	PosSeq=-1;
	AllNoteOff();
	Tick=-1;
	LastTick=-1;
}

//----------------------------------------------------- 
// HANDLER CLOCK MIDI
// Procedure appellee a chaque tick de l'horloge midi
// 1 tick= 1/24eme de noire
//----------------------------------------------------- 
void  MyHandleClockSeqMidi(void) {
	if (setup_ClockIn!=_CLOCKMIDI)
		return;
	ModePlay=1;								// Mode play on
	ClockMidi++;							// Incrementation horloge midi
	if (ClockMidi==1)        				// Si debut de division
			Tick++;							// on change tick principal
	// On verifie si la clockMidi doit être remise a zero
	if (ClockMidi==24/ListeClockMenu[setup_ClockDivider]) {	// Si valeur atteinte
		ClockMidi=0;     									// remet a zero et ca rejouera au prochain tick midi
	}
}

//-----------------------------------------
// HANDLER STOP sequenceur clock externe
// geree par des attach interrupt
//-----------------------------------------
void MyHandleStopSeqCv(void) {
	
	// Si horloge active n'est pas analogique
	if (setup_ClockIn!=_CLOCKCV) 
		return;						// On sort
	// Si deja arrete on sort
	if (ModePlay==0)
		return;
	F_IntCVReset=1;
}

//----------------------------------------------------- 
// HANDLER CLOCK CV
// Geree par des attach interrupt
//----------------------------------------------------- 
void  MyHandleClockSeqCv(void) {
	// Si horloge active n'est pas analogique
	if (setup_ClockIn!=_CLOCKCV)
		return;						// On sort
	ModePlay=1;						// Mode play on
	ClockCv++;						// Incrementation horloge midi
	if (ClockCv==1)        			// Si debut de division
			Tick++;					// on change tick principal
	// On verifie si la clockMidi doit être remise a zero
	if (ClockCv==24/ListeClockMenu[setup_ClockDivider]) {	// Si valeur atteinte
		ClockCv=0;     										// remet a zero et ca rejouera au prochain tick midi
	}
	F_IntCVReset=0;					// RAZ flag Reset externe
	
}

//--------------------------------------------------
// remets les curseurs en place en mode interruption
// Fini arret sequenceur en dehors de linterruption
//--------------------------------------------------
void MyHandleStopSeqCv2(void) {
	F_IntCVReset=0;
	if (modeMenu==EDITSEQ) {
		flagAffLcd2=PosSeq+1;
	}
	ModePlay=0;
	ClockCv=0;
	PosSeq=-1;
	Tick=-1;
	LastTick=-1;
	F_IntCVReset=0;
	AllNoteOff();
}


//-----------------------------------------
// HANDLER STOP sequenceur horloge interne
//-----------------------------------------
void MyHandleStopSeqInt(void) {
	// Si horloge active n'est pas interne
	if (setup_ClockIn!=_CLOCKINT) 
		return;						// On sort

	// Si sequenceur deja arrete
	// et qu'on rappuie sur stop
	// alors on reinitialise au debut de la sequence
	if (ModePlay==0) {
		PosSeq=-1;
		OldCurSeq=-1;
		Tick=-1;
		LastTick=-1;
		return;
	}	// Sinon, on se contente d'arreter le sequenceur
		// Et le pointeur de sequence reste ou il est
	
	// Dans tous les cas, on arrete de sequencer
	// On remets les horloges a zero
	// on ferme les gates
	ModePlay=0;
	if (modeMenu==EDITSEQ) {	
		// On memorise la position de sequence dans le flag
		// Et on ajoute 1 pour que flagAffLcd2 soit toujours >=1
		flagAffLcd2=PosSeq+1;
	}
	ClockInt=0;
	AllNoteOff();
}


//-------------------
// Joue une note midi
//-------------------
void PlayNoteMidi(byte PosNote) {
	// Si le gate actuelle est au maxi
	// et qu'une note est en cours de jeu on sort
	// cela va permettre de continuer de la jouer
	if (SeqGates[PosNote]==127 && CountNoteOut==1) 
		return;
	
	//---------------------------------------------------------
	// Arrive ici, on peu avoir une note en cours de jeu ou pas
	// En tout cas, on a :
	// 	- Soit pas de note jouee et n'importe quelle valeur de gate
	// 	- Soit une note en cours de jeu mais un gate <127
	//---------------------------------------------------------

	
	// Si la derniere note joue encore (CountNoteOut==1)
	// et que l'ancienne n'est pas arretee
	//		- soit par erreur de prog, 
	//		- soit saturation midi perte de NoteOff
	// 		- soit parce que le temps de NoteOff n'est pas arrive a expiration 
	//		  et que le BPM est plus rapide que le temps de relachement
	// Il faut envoyer d'abord un note off pour arreter la note en cours
	if (CountNoteOut==1) {
		// envoyer note-off sur premier canal midi
		MidiSendNoteOff(LastNoteOn1,0,MIDI_CHANNEL_1);	// Envoyer le note-off
		// Si mode 2 pistes
		// et sortie MIDI uniquement (pas MIDI+CV)
		if (setup_ModeTrack==0 && setup_ModeOut==0) 
			// Arret note du deuxieme canal midi
			MidiSendNoteOff(LastNoteOn2,0,MIDI_CHANNEL_2);
		CountNoteOut=0;						// remettre le compteur a 0
		// Mettre un delay si necessaire
		// delay(2)
	}

	// Arrive ici
	// 	- soit on avait rien en cours
	//	- soit une note jouait et on l'a arretee
	// On regarde la nouvelle note qui se presente
	// Si on a une gate >0 et que le compteur de note est a 0 (normalement il est forcement a zero)
	// On joue la note
	// si le Gate est a Off (gate=0) on passera cette portion de programme
	// Si CountNoteOut est toujours a 1 c'est une erreur de programme
	// et on ne rejoue pas de note.
	if (SeqGates[PosNote]>0 && CountNoteOut==0) {
		CountNoteOut=1;								// compteur de note active
		// Si mode 1+V
		if (setup_ModeTrack==2) {
			// SeqNotes[PosNote] contient la note
			// SeqNotes[PosNote+16] contient la velocite
			MidiSendNoteOn(SeqNotes[PosNote],SeqNotes[PosNote+16],MIDI_CHANNEL_1);	// On joue la note
		}
		// Sinon mode 2 x 16 ou 1 x 32 (forcer la velocite a fond)
		else {
			MidiSendNoteOn(SeqNotes[PosNote],127,MIDI_CHANNEL_1);	// On joue la note
		}
		LastNoteOn1=SeqNotes[PosNote];				// On memorise la note en cours
		// Si mode 2 x 16 pistes
		// et si sortie uniquement midi (pas MIDI+CV)
		// (Dans le cas ou MIDI+CV seul le canal 1, piste 1 est joué)
		// On joue le deuxieme canal midi, velocite a fond
		if (setup_ModeTrack==0 && setup_ModeOut==0) {
			MidiSendNoteOn(SeqNotes[PosNote+16],127,MIDI_CHANNEL_2);	// On joue la note
			LastNoteOn2=SeqNotes[PosNote+16];			// On memorise la note en cours
		}
		StartGate=millis();							// On memorise le timer de depart de jeu
		return;										// On sort
	}
	// Si on arrive ici
	// cest que soit CountNoteOn etait=1 (donc deniere note pas annulee)
	// Soit on a pas de note du tout a jouer gate=0
}

//-------------------------------------------------------
// Joue une note en mode CV
// Avec la conversion en fonction du mod CV selectionne
// PosNote represente l'index dans la sequence
// Si 32 pas on joue la note
// si 2 x 16 on joue index de la note et index+16 ensuite
//-------------------------------------------------------
void PlayNoteCV(byte PosNote) {                                      
	vdac1=0;              
	vdac2=0;

	//------------------------
	// Si mode // 2 x 16 pas
	//------------------------
	if (setup_ModeTrack==0) {
		switch(setup_ModeCV) {
			// Volt par octave
			case VOCT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=96) {
					vdac1= pgm_read_word_near( VOct + SeqNotes[PosNote] - 36);
				}
				if (SeqNotes[PosNote+16]>=36 and SeqNotes[PosNote+16] <=96) {
					vdac2= pgm_read_word_near( VOct + SeqNotes[PosNote+16] - 36);
				}
				break;
				// Hertz par volt
			case HVOLT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=87) {
					vdac1= pgm_read_word_near( HVolt + SeqNotes[PosNote] - 36);
				} 
				if (SeqNotes[PosNote+16]>=36 and SeqNotes[PosNote+16] <=87) {
					vdac2= pgm_read_word_near( HVolt + SeqNotes[PosNote+16] - 36);
				} 
				break;
				// SX150
			case SX150 :
				break;
		};
	}
	//------------------------
	// Si mode 32 pas
	//------------------------
	if (setup_ModeTrack==1) {
		switch(setup_ModeCV) {
			// Volt par octave
			case VOCT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=96) {
					vdac1= pgm_read_word_near( VOct + SeqNotes[PosNote] - 36);
				}
				break;
				// Hertz par volt
			case HVOLT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=87) {
					vdac1= pgm_read_word_near( HVolt + SeqNotes[PosNote] - 36);
				} 
				break;
				// SX150
			case SX150 :
				break;
		};
	}
	//---------------------------
	// Si mode 16 PAS + Velocite
	//---------------------------
	if (setup_ModeTrack==2) {
		switch(setup_ModeCV) {
			// Volt par octave
			case VOCT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=96) {
					vdac1= pgm_read_word_near( VOct + SeqNotes[PosNote] - 36);
				}
				vdac2=SeqNotes[PosNote+16]*32;
				if (vdac2>4095)
					vdac2=4095;
				break;
				// Hertz par volt
			case HVOLT :
				if (SeqNotes[PosNote]>=36 and SeqNotes[PosNote] <=87) {
					vdac1= pgm_read_word_near( HVolt + SeqNotes[PosNote] - 36);
				} 
				vdac2=SeqNotes[PosNote+16]*32;
				if (vdac2>4095)
					vdac2=4095;
				break;
				// SX150
			case SX150 :
				break;
		};
	}
	
	// ecriture de la valeur en tension dans le dac
	writeDAC(DAC1,vdac1);
	if (setup_ModeTrack!=1) 	// Si mode 2 x 16 pas ou 16 pas + velocite
		writeDAC(DAC2,vdac2);

	// Si mode 2 x 16 pas
	if (setup_ModeTrack==0) {
		if (SeqGates[PosNote] > 0)
			GateOn(GATE1);			// Ouverture gate 1 si pas Off
		if (SeqGates[PosNote+16] > 0)
			GateOn(GATE2);			// Ouverture gate 2 si pas Off
	}
	// Si mode 32 pas
	if (setup_ModeTrack==1) {			
		if (SeqGates[PosNote] > 0)
			GateOn(GATE1);			// Ouverture gate 1 si pas Off
	}
	// Si mode 16 pas + velocite
	if (setup_ModeTrack==2) {			
		if (SeqGates[PosNote] > 0)
			GateOn(GATE1);			// Ouverture gate 1 si pas Off
	}
	
	// memorisation de l'ouverture des 2 GATES
	StartGate=millis();
}


//---------------------------- 
// HANDLER NOTE OFF MIDI -> CV
//---------------------------- 
void  MyHandleNoteOffSeqMidi(byte channel, byte pitch, byte velocity) {
    CountNote--;              // Decrement du nombre de note
    if (CountNote ==255)         // Test si negatif (impossible normalement)
        CountNote=0;          // Si c'est le cas RAZ
	// Si mode Midi out ou MIDI+CV
	// arreter la note jouee en midi
	if (setup_ModeOut==0 || setup_ModeOut==2)
		MidiSendNoteOn(pitch,0,1);
	// Si mode CV out ou MIDI+CV
	// arreter la note jouee en CV
	if (setup_ModeOut==1 || setup_ModeOut==2) {
		GateOff(GATE1);
		GateOff(GATE2);
	}
}

//--------------------------- 
// HANDLER NOTE ON MIDI -> CV
//--------------------------- 
void MyHandleNoteOnSeqMidi(byte channel, byte pitch, byte velocity) {
int cv2;
	// Si velocity == 0 alors on a recu un note off
	//---------------------------------------------
	if (velocity==0) {
		MyHandleNoteOffSeqMidi(channel,pitch,velocity);
		// gestion du note off et sortie du handler
		//MyHandleNoteOffCV(channel, pitch, velocity);
		return;
	}

	CountNote++;    					// Incrementation du nombre de note actives
	CurrentNote=pitch;					// Memorise le numero de la note jouee
	CurrentVelocity=velocity;			// Memorise la velocite de la note jouee
	PosRec++;							// incrementation position dans la sequence a enregistrer
	
}

//---------------
// Ouvre une gate
//---------------
void GateOn(byte nogate) {
  if (nogate==GATE1)
      digitalWrite(GATE1,HIGH);
  if (nogate==GATE2)
      digitalWrite(GATE2,HIGH);
}

//---------------
// Ferme une gate
//---------------
void GateOff(byte nogate) {
  if (nogate==GATE1)
      digitalWrite(GATE1,LOW);
  if (nogate==GATE2)
      digitalWrite(GATE2,LOW);
}


