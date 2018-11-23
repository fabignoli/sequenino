//------------------------------
// Initialisation de la sequence
//------------------------------
void InitSequence(void) {
byte i;
	// Set pour piste 1
	for (i=0;i<16;i++) {
		SeqNotes[i]=36;		// Note C2
		SeqGates[i]=10;		// Gate de 10
	}
	// Set pour piste 2
	for (i=16;i<32;i++) {
		SeqNotes[i]=36;		// Note C2
		SeqGates[i]=10;		// Gate de 10
	}
}

//-------------------------------------------------
// enregistrement de la sequence via MIDI
// commence au step 0 s'arrete a setup_Steps
// ou s'arrete si bouton REC de nouveau appuye
// ne fonctionne sur si le sequenceur est a l'arret
//-------------------------------------------------
void RecordSequence(void) {
	byte MaxRec;
	// Si le sequenceur tourne
	if (ModePlay==1) {
		return;			// On sort
	}
	ModeRecord==1;
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Record ");
	lcd.print(setup_Steps);
	lcd.print(F(" steps..."));
	PosRec=-1;
	LastPosRec=-1;
	while(PosRec<setup_Steps-1) {
		// lecture midi
		// regarder si le bouton REC est de nouveau appuyer
		// cela marque la fin de l'enregistrement
		valbouton=readButtons(PINTRANSPORT);
		if (valbouton==B_REC) 					// bouton transport appuye
			break;
		// regarder si une note a ete jouee au clavier midi
		// verifier qu'on ne sort pas du cadre de la sequence
		if (LastPosRec<PosRec && PosRec<=setup_Steps-1) {
			LastPosRec=PosRec;
			switch (setup_ModeTrack) {
				// Mode 2 x 16 pas
				case 0:
					// regarder la page edition actuelle
					// Si premiere page
					if (xm==0) {
						SeqNotes[PosRec]=CurrentNote;		// Memorise la note
						SeqGates[PosRec]=10;				// met une longueur de gate de 10
					}
					// Si seconde page
					if (xm==1) {
						SeqNotes[PosRec+16]=CurrentNote;	// Memorise la note
						SeqGates[PosRec+16]=10;				// met une longueur de gate de 10
					}
					break;
					// Mode 1 x 32 pas
				case 1:
					SeqNotes[PosRec]=CurrentNote;			// Memorise la note
					SeqGates[PosRec]=10;					// met une longueur de gate de 10
					break;
					// Mode 1 x 16 pas note et 1 x 16 pas velocite 
				case 2:
					SeqNotes[PosRec]=CurrentNote;			// Memorise la note
					SeqNotes[PosRec+16]=CurrentVelocity;	// Memorise la velocite de cette note
					SeqGates[PosRec]=10;					// met une longueur de gate de 10
					SeqGates[PosRec+16]=10;					// reinitialise l'autre partie 
					break;
				
			}
			// Arrive ici la note a ete memorisee
			// on peut la jouer pour lentendre
			if (setup_ModeOut==0 || setup_ModeOut==2) {
				MidiSendNoteOn(SeqNotes[PosRec],CurrentVelocity,1);
			}
			// Si mode CV out ou MIDI+CV
			// arreter la note jouee en CV
			if (setup_ModeOut==1 || setup_ModeOut==2) {
				PlayNoteCV(PosRec);
			}
			lcd.setCursor(0,2);
			lcd.print(F("Step["));
			digitsLcd(PosRec+1,2);
			lcd.print(F("]"));
			lcd.printByte(CAR_RIGHT);
			affNoteLCD(SeqNotes[PosRec]);
			
		}
	}
	// reset RECORD et PLAY
	ModePlay=0;		// remettre mode lecture a zero si redeclenchement intempestif
	ModeRecord=0;		// fin de lenregistrement
	// reset des differents pointeurs de sequencement
	PosSeq=-1;
	OldCurSeq=-1;
	Tick=-1;
	LastTick=-1;
	ClockInt=0;
	ClockCv=0;
	ClockMidi=0;
	// fermeture des 2 gates au cas ou
	GateOff(GATE1);
	GateOff(GATE2); 	
	// forcer le rafraichissement de lecran
	rafLCD=9;
}



//-----------------------------------
// Raffraichissement ecran sequenceur
//-----------------------------------
void aff_menu_LCD_sequenceur(byte parRaff) {
byte mySteps;
	// Si rafraichissement total
	// Partie statique
	if (parRaff==9) {
		lcd.clear();          // Effacer tout lecran
		lcd.setCursor(0,0);   // Positionner en haut à gauche
		PrintEditCursor();

		//----------------------------------------
		// AFFICHAGE PARTIE STATIQUE
		// uniquement affichee si rafLCD=9
		// Les fonds de pages ne sont pas affiches
		// pour d'autres valeurs de rafLCD
		//----------------------------------------
		switch (ym) {
			// Si menu note
			case 0:
				// Affiche piste, "NOT" ou "VEL", steps
				lcd.setCursor(16,0);
				lcd.printByte(CAR_BARRE);
				if (ModePlay==1) {
					PrintPlay();
				}
				if (ModePlay==0) {
					PrintStop();
				}
				lcd.setCursor(16,1);
				lcd.printByte(CAR_BARRE);
				if (setup_ModeTrack==2 && xm==1) 
					lcd.print(F("VL"));
				else
					lcd.print(F("NT"));
				lcd.print(xm+1);
				lcd.setCursor(16,2);
				lcd.printByte(CAR_BARRE);
				lcd.print(F("S"));
				digitsLcd(setup_Steps,2);
				lcd.setCursor(16,3);
				lcd.printByte(CAR_BARRE);
				AffSensSeqLCD(setup_SensSeq);
				// Afficher les 16 pas en cours
				// note ou ("...") si nbre de steps < 16
				for (i=0;i<16;i++) {
					//positionner le curseur
					lcd.setCursor((i%4)*4+1,(int)i/4);
					// Si note au dela du nombre de step max
					if (i>=setup_Steps && xm==0) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					if (i>=setup_Steps && xm==1 && setup_ModeTrack!=1) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					if (i>=setup_Steps-16 && xm==1 && setup_ModeTrack==1) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					if (setup_ModeTrack==2 && xm==1)	// si mode velocité et piste 2
						digitsLcd(SeqNotes[i+(xm*16)],3);
					else
						affNoteLCD(SeqNotes[i+(xm*16)]);	// Sinon afficher la note
				}
				break;
			// Si affichage gate 
			case 1:
				// Affiche piste, "GAT", step, Sens sequenceur
				lcd.setCursor(16,0);
				lcd.printByte(CAR_BARRE);
				if (ModePlay==1) {
					PrintPlay();
				}
				if (ModePlay==0) {
					PrintStop();
				}
				lcd.setCursor(16,1);
				lcd.printByte(CAR_BARRE);
				lcd.print(F("GA"));
				lcd.print(xm+1);
				lcd.setCursor(16,2);
				lcd.printByte(CAR_BARRE);
				lcd.print(F("S"));
				digitsLcd(setup_Steps,2);
				lcd.setCursor(16,3);
				lcd.printByte(CAR_BARRE);
				AffSensSeqLCD(setup_SensSeq);
				// Afficher les 16 pas en cours
				// gate ou ("...") si nbre de steps < 16
				for (i=0;i<16;i++) {
					//positionner le curseur
					lcd.setCursor((i%4)*4+1,(int)i/4);
					// Si note au dela du nombre de step max
					if (i>=setup_Steps && xm==0) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					if (i>=setup_Steps && xm==1 && setup_ModeTrack!=1) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					if (i>=setup_Steps-16 && xm==1 && setup_ModeTrack==1) {
						lcd.print(F("..."));				// alors afficher "..."
						continue;
					}
					myByte1=SeqGates[i+(xm*16)];
					switch(myByte1) {
						case 0:
							lcd.print(F("Off"));
							break;
						case 127:
							lcd.print(F("On "));
							break;
						default:
							digitsLcd(SeqGates[i+(xm*16)],3);	// Sinon afficher la note
							break;
					}
				}
				break;
		}
	}
	// Afficher ici la partie dynamique si besoin
	
}

//------------------------------------------------
// Gestion des encodeurs partie edition sequenceur
//------------------------------------------------
void gestionDonneesSequenceur(void) {
	//lcd.setCursor(0,0);
	switch(valencodeurs) {
		// Positionnement sur note, velocite ou gate suivant
		case ENC1UP:
			ClearEditCursor();
			PosAff++;									// increment position
			if (setup_ModeTrack==1) {
				if (setup_Steps>16 && xm==1) {
					if (PosAff>=setup_Steps-16)					
						PosAff=0;
				}
				if (setup_Steps>16 && xm==0) {
					if (PosAff>=16)					
						PosAff=0;
				}
				if (setup_Steps<=16 && xm==1) {
						PosAff=0;
				}
				if (setup_Steps<=16 && xm==0) {
					if (PosAff>=setup_Steps)					
						PosAff=0;
				}
			}
			else {
				if (PosAff>=setup_Steps)					
					PosAff=0;
			}
			PrintEditCursor();
			break;
		// Positionnement sur note, velocite ou gate precedent
		case ENC1DOWN:
			ClearEditCursor();
			PosAff--;
			
			if (PosAff<0) {
				if (setup_ModeTrack==1) {
					if (setup_Steps>16 && xm==1) {
							PosAff=setup_Steps-1-16;
					}
					if (setup_Steps>16 && xm==0) {
							PosAff=15;
					}
					if (setup_Steps<=16 && xm==1) {
							PosAff=0;
					}
					if (setup_Steps<=16 && xm==0) {
							PosAff=setup_Steps-1;
					}
				}
				else {
					PosAff=setup_Steps-1;
				}
				
			}
			PrintEditCursor();
			break;
		// Valeur note,velocite ou gate suivante a la position du curseur
		case ENC2UP:
			if (setup_ModeTrack==1 && xm==1 && setup_Steps<=16)
				break;
			lcd.setCursor((PosAff%4)*4+1,(int)PosAff/4);	// Trouve position courante a lecran
			switch (ym) {
				// Si note ou velocité
				case 0:	
					// determine si bouton fontion appuye
					// Pour diminuter d'un octave
					valbouton=readButtons(PINTRANSPORT);
					if (valbouton==9 && lastBoutonNav[PINTRANSPORT]==B_FUNC) {
						switch(setup_ModeTrack) {
							// Si 2 x 16
							case 0:
								// Si edition piste 1
								if (xm==0) {
									myByte2=0;
									myByte3=16;
								}
								// sinon edition piste 2
								else {
									myByte2=16;
									myByte3=32;
								}
								break;
							case 1:
								myByte2=0;
								myByte3=32;
								break;
							case 2:
								// Si edition piste 1
								if (xm==0) {
									myByte2=0;
									myByte3=16;
								}
								// sinon edition piste 2
								else {
									myByte2=16;
									myByte3=32;
								}
								break;
						}
						for (myByte1=myByte2; myByte1<myByte3; myByte1++) {
							if (SeqNotes[myByte1]) {
								SeqNotes[myByte1]=SeqNotes[myByte1]+12;
								if (SeqNotes[myByte1]>127)
									SeqNotes[myByte1]=127;
							}
						}
						rafLCD=9;
					}				
					else {
						// recuperer valeur courante
						myByte1=SeqNotes[PosAff+(xm*16)];
						myByte1++;
						if (myByte1>127)
							myByte1=127;
						SeqNotes[PosAff+(xm*16)]=myByte1;	
						// Mode velocite
						// uniquement si modetrack=2 et que nous sommes sur la piste 2
						if (setup_ModeTrack==2 && xm==1) 
							digitsLcd(myByte1,3);
						// Sinon nous somme en mode note
						else 
							affNoteLCD(myByte1);
					}
					break;
				// Si gate
				case 1:
					valbouton=readButtons(PINTRANSPORT);
					if (valbouton==9 && lastBoutonNav[PINTRANSPORT]==B_FUNC) {
						for (myByte1=0; myByte1<32; myByte1++) {
								if (SeqGates[myByte1]) {
									SeqGates[myByte1]=SeqGates[myByte1]+5;
									if (SeqGates[myByte1]>127)
										SeqGates[myByte1]=127;
								}
						}
						rafLCD=9;
					}
					else {
						
						myByte1=SeqGates[PosAff+(xm*16)];
						if (myByte1<127)
							myByte1++;
						SeqGates[PosAff+(xm*16)]=myByte1;	
						if (myByte1==127)
							lcd.print("On ");
						else	
							digitsLcd(myByte1,3);
					}
					break;
			}
			break;
		// Valeur note,velocite ou gate precedente a la position du curseur
		case ENC2DOWN:
			if (setup_ModeTrack==1 && xm==1 && setup_Steps<=16)
				break;
			lcd.setCursor((PosAff%4)*4+1,(int)PosAff/4);	// Trouve position courante a lecran
			switch (ym) {
				// Si note ou velocité
				case 0:
					valbouton=readButtons(PINTRANSPORT);
					// determine si bouton fontion appuye
					// Pour diminuter d'un octave
					if (valbouton==9 && lastBoutonNav[PINTRANSPORT]==B_FUNC) {
						switch(setup_ModeTrack) {
							// Si 2 x 16
							case 0:
								// Si edition piste 1
								if (xm==0) {
									myByte2=0;
									myByte3=16;
								}
								// sinon edition piste 2
								else {
									myByte2=16;
									myByte3=32;
								}
								break;
							case 1:
								myByte2=0;
								myByte3=32;
								break;
							case 2:
								// Si edition piste 1
								if (xm==0) {
									myByte2=0;
									myByte3=16;
								}
								// sinon edition piste 2
								else {
									myByte2=16;
									myByte3=32;
								}
								break;
						}
						for (myByte1=myByte2; myByte1<myByte3; myByte1++) {
							if (SeqNotes[myByte1]>12)
								SeqNotes[myByte1]=SeqNotes[myByte1]-12;
						}
						rafLCD=9;
					}
					else {
						// recuperer valeur courante
						myByte1=SeqNotes[PosAff+(xm*16)];
						// Si mode velocite et velocite actuelle > 0 alors on decremente
						if (setup_ModeTrack==2 && xm==1 && myByte1>0) 
							myByte1--;
						// Sinon, on est en mode note
						else {
							// Decrementer les notes si actuelle superieur a 24
							if (myByte1>24)
								myByte1--;
						}
						SeqNotes[PosAff+(xm*16)]=myByte1;	
						// Mode velocite
						// uniquement si modetrack=2 et que nous sommes sur la piste 2
						if (setup_ModeTrack==2 && xm==1) 
							digitsLcd(myByte1,3);
						// Sinon nous somme en mode note
						else 
							affNoteLCD(myByte1);
					}
					break;
				// Si gate
				case 1:
					valbouton=readButtons(PINTRANSPORT);
					if (valbouton==9 && lastBoutonNav[PINTRANSPORT]==B_FUNC) {
						for (myByte1=0; myByte1<32; myByte1++) {
								if (SeqGates[myByte1]>5)
									SeqGates[myByte1]=SeqGates[myByte1]-5;
						}
						rafLCD=9;
					}
					else {
						myByte1=SeqGates[PosAff+(xm*16)];
						if (myByte1>0)
							myByte1--;
						SeqGates[PosAff+(xm*16)]=myByte1;	
						if (myByte1==0)
							lcd.print("Off");
						else	
							digitsLcd(myByte1,3);
					}
					break;
			}
			break;
			
	}		
			
				
	
}

//--------------------------
// Gestion partie sequenceur
//--------------------------
void gestion_menu_sequenceur(void) {
	xm=0;	// track 1
	ym=0;	// edition note 
	aff_menu_LCD_sequenceur(9);	// rafraichir ecran
	rafLCD=0;
	flagAffLcd1=0;
	// Boucler dans le menu sequenceur
	// sauf si changement de contexte
	while(1) {
		// Tester si on doit changer de contexte
		// ou si changement piste ou note <-> gate
		//----------------------------------------
		valbouton=readButtons(PINEDIT);
			
		// Si un bouton appuye
		if (valbouton) {
		
			if (valbouton==B_PRESETS) {		// Test si bouton gestion preset appuye
				modeMenu=EDITPRESETS;		// si oui, setter le contexte PRESET
				
				return;						// et revenir gestion menu principal
			}
		
		
		
			if (valbouton==B_SETUP) {		// Test si bouton preset appuye
				modeMenu=EDITSETUP;			// si oui, setter le contexte SETUP
				return;						// et revenir gestion menu principal
			}
		
			// Si bouton encodeur 1 (TRACK 1)
			if (valbouton==B_ENC1) {
				if (xm==0) {	// Si deja dans menu track 1
					if (ym==0)	// Si edition de note 1
					   ym=1;	// alors on edite les gates 1
					else 
						ym=0;	// sinon on edite les notes
				}
				else {			// Sinon on est sur piste 2 on revient sur la 1
					PosAff=0;
					xm=0;		// affiche track 1
					ym=0;		// edition note
				}
				rafLCD=9;
			}
			// si bouton encodeur 2 (TRACK 2)
			if (valbouton==B_ENC2) {
				if (xm==1) {	// Si deja dans menu track 2
					if (ym==0 && setup_ModeTrack!=2)	// Si edition de note 2 et pas mode 1+V
					   ym=1;	// alors on edite les gates 2
					else 
						ym=0;	// sinon on edite les notes
				}
				else {			// Sinon on est sur piste 1 on va sur la 2
					PosAff=0;
					xm=1;		// affiche track 2
					ym=0;		// edition note
				}
				rafLCD=9;		// On force le rafraichissement d'ecran
				
			}
		}
			// Ici on reste dans le contexte dedition
			//---------------------------------------
			
			 // Lire la valeur des boutons de transport
			valbouton=readButtons(PINTRANSPORT);
			if (valbouton!=0) 					// bouton transport appuye
				gestionBoutonsTransport();		// gerer les boutons de transport
	  
			// Lire ensuite les encodeurs
			// on est en mode note ou gate
			// sur la piste 1 ou 2
			valencodeurs=readEncodeurs();
			if (valencodeurs) {					// Si un encodeur a bouge
				gestionDonneesSequenceur();		// gerer les encodeurs pour editer la sequence
			}
		
		// arrive ici on a lu les boutons de contexte et les boutons de transports
		// les valeurs des encodeurs ont eventuellement changé les données
		// Si changement de donnees par utilisateur
		// rafraichir LCD
		if (rafLCD) {         				// Si flag affichage
			aff_menu_LCD_sequenceur(rafLCD);  			// Rafraichir lecran si besoin
			rafLCD=0;
		}
		
		// Gestion affichage curseur position et pointeur de sequence
		// en fonction du contexte.
		// le contexte est géré par des flags
		
		// Le flag F_IntCVReset a ete positionne dans l'interrupt externe 
		// en relation avec l'entree RESET CV le sequenceur est donc arrete
		if (F_IntCVReset==1)
			MyHandleStopSeqCv2();
		
		// Flag affichage positionne par la routine PlaySequencer
		// le sequenceur est en fonction peut importe l'horloge de synchro
		if (flagAffLcd1) {
			flagAffLcd1=0;
			AffichePlay();								// Affiche mode play ">>>"
			EffacePointeurSeq(OldCurSeq);				// Efface le pointeur "*" de sequence
			AffichePointeurSeq(PosSeq);					// Affiche "*" pointeur sequence
			AffichePointeurEdition();					// Affiche ">" curseur edition
		}
		
		// Flag affichage positionne par une des routines d'arret du sequenceur
		// (MyHandleStopSeqCv2, MyHandleStopSeqInt ou MyHandleStopSeqMidi)
		// MAJ pointeur sequence, pointeur edition et nombre de steps
		if (flagAffLcd2) {
			// On retranche 1 pour retrouver la position de sequence correcte
			// parce que flagAffLcd2 fait office de flag de rafraichissement
			// et mais contient aussi la valeur PosSeq+1 (car PosSeq peut etre egal à 0)
			flagAffLcd2--;
			EffacePointeurSeq(flagAffLcd2);				// Efface pointeur sequence
			PrintEditCursor();							// Positionne pointeur edition
			PrintNbSteps();								// Affiche nbre de steps en cours
			PrintStop();								// Affiche STOP
			flagAffLcd2=0;
		}
		if (flagAffLcd3) {
			lcd.setCursor(0,0);
			lcd.print(cn1);
			lcd.print("  ");
			flagAffLcd3=0;
		}
		
	}
}

//-------------------------
// Setup interruption
// pour horloge BPM interne
//-------------------------
void BpmTimerSetup(void) {
  //----------------------------------------
  // Configuration du timer 1
  // pour generer une horloge BPM
  //----------------------------------------
  TIMSK1 &= ~(1<<TOIE1);  
  // Timer 2 en mode normal
  TCCR1A &= ~((1<<WGM11) | (1<<WGM10));  
  TCCR1B &= ~(1<<WGM12);  
  // Mode overflow
  TIMSK1 &= ~(1<<OCIE1A);  
  // Prescaler a 64
  TCCR1B |= (1<<CS10)  | (1<<CS11) ; // Set bits  
  TCCR1B &= ~(1<<CS12);              // Clear bit  
  // Sauvegarde de la valeur pou reloader dans l'ISR
  tcnt1 = 60327;	// Pour 120BPM par defaut   
  // Chargement et mise en route du timer
  TCNT1 = tcnt1;  
  TIMSK1 |= (1<<TOIE1);

}

//----------------------------------
// Calcul dynamique du mode pendulum
//----------------------------------
void CalcPendulum(void) {
byte MaxPend,MinPend;
	//return;
	MaxPend=setup_Steps-1;
	MinPend=0;
	// reinitialiser le tableau
	for (myInt=0; myInt<32 ; myInt++)
		ModePendulum[myInt]=0;
	// recalculer le tableau
	// en fonction du nombre de step actif
	for (myInt=0; myInt<setup_Steps-1 ; myInt++) {
		if ( (myInt%2)==0 ) {
			ModePendulum[myInt]=MaxPend;
			MaxPend--;
		}
		else {
			ModePendulum[myInt]=MinPend;
			MinPend++;
		}
	}
}

//-------------------------------------
// Joue la sequence
// determine si le jeu est actif ou non
//-------------------------------------
void PlaySequencer(void) {

	// Si mode de jeu a larret
	if (ModePlay==0)
		return;						

	EndGate=millis();	// On memorise le timer en cours

	//----------------------------------------------
	// TEST DE FERMETURE DES GATES POUR LA PARTIE CV
	// (si mode CV ou mode MIDI+CV)
	//----------------------------------------------
	if (setup_ModeOut==1 || setup_ModeOut==2) {
		// Quelque soit le mode de jeu des pistes
		// Verifier si on doit fermer la GATE 1
		// Si le timer en cours est > temps de depart + (delay * unite de temps)
		// alors on ferme le GATE 1
		if (EndGate > StartGate + (SeqGates[PosSeq]*_UNITGATE))
			GateOff(GATE1);
			
		// Si mode 2 x 16 pas (// ou 1 + vel)
		// Verifier si on doit fermer la GATE 2
		if (setup_ModeTrack!=1) {
			// Verifier gate piste 2
			// Meme principe que gate 1
			// Si timer en cours > temps de depart + (delay * unite de temps)
			// On ferme le GATE 2
			if (EndGate > StartGate + (SeqGates[PosSeq+16]*_UNITGATE))
				GateOff(GATE2);
		}
	}
	
	//------------------------------
	// TEST DU NOTE-OFF POUR LE MIDI
	// (Si mode MIDI ou mode MIDI+CV
	//------------------------------
	if (setup_ModeOut==0 || setup_ModeOut==2) {
		// Verifier si on doit envoyer un note off
		// Seulement si une note joue en ce moment
		if (CountNoteOut>0) {
			// Si la longueur de note n'est pas infinie (=127)
			// dans ce cas tester la longueur de note
			if (SeqGates[PosSeq]!=127) {
				// Si le temps de gate est depasse
				if (EndGate > StartGate + (SeqGates[PosSeq]*_UNITNOTEOFF)) {
					MidiSendNoteOff(LastNoteOn1,0,MIDI_CHANNEL_1);	// Envoyer un note-off
					// Si mode 2 pistes
					// et sortie MIDI uniquement (pas MIDI+CV)
					if (setup_ModeTrack==0 && setup_ModeOut==0) {
						MidiSendNoteOff(LastNoteOn2,0,MIDI_CHANNEL_2);	// Envoyer un note-off
					}
					CountNoteOut=0;									// remettre le compteur de note a zero
				}
			}
		}
	}
	// Si le dernier tick n'a pas change
	// on a deja joue la note
	if (LastTick==Tick)
			return;		// On sort
			
	// arrive ici, le tick s'est incremente
	// Il va falloir jouer une note
	
	// Sauvegarde du nouveau tick
	LastTick=Tick;
	OldCurSeq=PosSeq;
	// On determine si cest le premier pas
	// si premier pas, alors tick=0
	if (Tick==0) {
		// Calcul de la premier position de PosSeq
		switch (setup_SensSeq) {
			// UP
			case 0:
				PosSeq=0;
				break;
			// Down
			case 1:
				PosSeq=setup_Steps-1;
				break;
			// Col
			case 2:
				PosSeq=0;
				break;
			// Pendulaire
			case 3:
				CalcPendulum();
				PosPendulum=0;
				PosSeq=ModePendulum[PosPendulum];
				break;
			// Random
			case 4:
				PosSeq=random(setup_Steps);
				break;
		}
	}
	// Sinon ce n'est pas la premiere note
	// il faut recalculer PosSeq
	else {
		// Calcul de la premier position de PosSeq
		switch (setup_SensSeq) {
			// UP
			case 0:
				if (PosSeq>=setup_Steps-1)
					PosSeq=0;
				else
					PosSeq++;
				break;
			// Down
			case 1:
				if (PosSeq>0)
					PosSeq--;
				else
					PosSeq=setup_Steps-1;
				break;
			// Col
			case 2:
				// Lire les donnees colonne
				// en boucle infinie
				while (1) {
					// lire la valeur suivante
					if (PosSeq>=0) {
						PosSeq=ModeColonne[PosSeq];
						// Si elle est < au nombre de step
						if (PosSeq<setup_Steps)
							break;					// alors on sort
					}
					else
						break;
				}
			
				break;
			// Pendulaire
			case 3:
				if (PosPendulum>=setup_Steps-1)
					PosPendulum=0;
				else 
					PosPendulum++;
				PosSeq=ModePendulum[PosPendulum];
			break;
			// Random
			case 4:
				// calcul une position random
				// entre 0 et steps maxi
				PosSeq=random(setup_Steps);
				break;
		}
	
	}
	
	
	// Si mode sortie CV, ou (CV et midi)
	// jouer la note en CV
	//-----------------------------------
	if (setup_ModeOut==1 || setup_ModeOut==2) {
		PlayNoteCV(PosSeq);
	}
	// Si mode sortie MIDI, ou (CV et midi)
	// Jouer la note en MIDI
	//-------------------------------------
	if (setup_ModeOut==0 || setup_ModeOut==2) {
		// jouer la note en MIDI
		PlayNoteMidi(PosSeq);
	}
	
	flagAffLcd1=1;
}

