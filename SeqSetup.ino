
//-----------------------------------------------------------
// Positionne le curseur au bon endroit pour afficher la donnee
//-----------------------------------------------------------
void PosCurseurSetup(byte position) {
	switch(position) {
		case 0:
		case 2:
		case 4:
		case 6:
			lcd.setCursor(6,(int)position/2);
			break;
		case 1:
		case 3:
		case 5:
		case 7:
			lcd.setCursor(15,(int)position/2);
			break;
	}
}

//---------------------------------
// Affiche un curseur pour le setup
//---------------------------------
void affCurseurSetup(byte position, byte Curtype) {
	switch(position) {
		case 0:
		case 2:
		case 4:
		case 6:
			lcd.setCursor(5,(int)position/2);
			break;
		case 1:
		case 3:
		case 5:
		case 7:
			lcd.setCursor(14,(int)position/2);
			break;
	}
	switch(Curtype) {
		case 0:
			lcd.print(F(":"));
			break;
		case 1:
			lcd.printByte(CAR_TRIRIGHT);
			break;
	}
}




//--------------------------------
// Raffraichissement ecran setup
//--------------------------------
void aff_menu_LCD_setup(byte parRaff) {
	// Si rafraichissement total
	// Partie statique
	if (parRaff==9) {
		lcd.clear();          // Effacer tout lecran
		lcd.setCursor(0,0);   // Positionner en haut à gauche
		//"STEPS:   |SENS:     "
		affChaineLcdPROGMEM(0);
		//" TRKS:   |MODE:     "
		affChaineLcdPROGMEM(1);
		//"  BPM:   |OUT:      "
		affChaineLcdPROGMEM(2);
		//"CLKIN:   |CLKDIV:   "
		affChaineLcdPROGMEM(3);
		lcd.setCursor(6,0);
		digitsLcd(setup_Steps,2);
		lcd.setCursor(15,0);
		AffSensSeqLCD(setup_SensSeq);
		lcd.setCursor(6,1);
		digitsLcd(setup_Bpm,3);
		lcd.setCursor(15,1);
		AffOutLCD(setup_ModeOut);
		lcd.setCursor(6,2);
		AffModeJeuPiste(setup_ModeTrack);
		lcd.setCursor(15,2);
		AffModeCVLCD(setup_ModeCV);
		lcd.setCursor(6,3);
		AffTypeClockLCD(setup_ClockIn);
		lcd.setCursor(15,3);
		lcd.print(F("x"));
		lcd.print(ListeClockMenu[setup_ClockDivider]);  // Affiche diviseur clock
		affCurseurSetup(PosCurSetup,1);
	}
	// Afficher ici la partie dynamique si besoin
	
}



//-----------------------------------
// Gestion des encodeurs partie setup
//-----------------------------------
void gestionDonneesSetup(void) {
byte affCurseur;
	affCurseur=false;
	//lcd.setCursor(0,0);
	switch(valencodeurs) {
		case ENC1UP:
			if (PosCurSetup<7) {
				affCurseurSetup(PosCurSetup,0);
				PosCurSetup++;
				affCurseurSetup(PosCurSetup,1);
			}
			break;
		case ENC1DOWN:
			if (PosCurSetup>0) {
				affCurseurSetup(PosCurSetup,0);
				PosCurSetup--;
				affCurseurSetup(PosCurSetup,1);
			}
			break;
		case ENC2UP:
			switch (PosCurSetup) {
				// Gestion nombre de steps UP
				case 0:
					// Si mode 1 piste 32 pas
					if (setup_ModeTrack==1 ) {
						if (setup_Steps<32) {
							setup_Steps++;
							PosCurseurSetup(PosCurSetup);
							digitsLcd(setup_Steps,2);
							CalcPendulum();
						}
					}
					// Sinon mode 16 pas
					else {
						if (setup_Steps<16) {
							setup_Steps++;
							PosCurseurSetup(PosCurSetup);
							digitsLcd(setup_Steps,2);
							CalcPendulum();
						}
					}
					break;
				// Gestion du sens de la sequence
				case 1:
					if ( setup_SensSeq <4 ) {
						setup_SensSeq++;
						PosCurseurSetup(PosCurSetup);
						AffSensSeqLCD(setup_SensSeq);
					}
					break;
				
				// BPM
				case 2:
					if ( setup_Bpm <180 ) {
						setup_Bpm++;
						PosCurseurSetup(PosCurSetup);
						digitsLcd(setup_Bpm,3);
						myfloat=(float)((float)setup_Bpm*(float)24/(float)60);
						myfloat=(float)1/myfloat*250000;
						tcnt1=65535-(int)myfloat;
					}
					break;
				// Out (midi, CV, midi + CV)
				case 3:
					if ( setup_ModeOut < 2 ) {
						AllNoteOff();
						setup_ModeOut++;
						PosCurseurSetup(PosCurSetup);
						AffOutLCD(setup_ModeOut);
						
					}
					break;
				// Gestion mode lecture de piste (//, 1+2, 1+Vel)
				case 4:
					if ( setup_ModeTrack<2) {
						setup_ModeTrack++;
						PosCurseurSetup(PosCurSetup);
						AffModeJeuPiste(setup_ModeTrack);
						if (setup_ModeTrack!=1) { 			// Si mode de jeu sur 16 pas
							if (setup_Steps >16) {			// et nombre de pas actuel > 16
								setup_Steps=16;				// remettre steps a 16
								lcd.setCursor(6,0);			// rafraichir donnees steps
								digitsLcd(setup_Steps,2);
							}
						}
					}
					break;


					// Gestion mode CV
				case 5:
					if ( setup_ModeCV < 2) {
						setup_ModeCV++;
						PosCurseurSetup(PosCurSetup);
						AffModeCVLCD(setup_ModeCV);
					}
					break;
				// Gestion Clock (MIDI, analogique externe, interne)
				case 6:
					if ( setup_ClockIn < 2) {
						setup_ClockIn++;
						PosCurseurSetup(PosCurSetup);
						AffTypeClockLCD(setup_ClockIn);
						ModePlay=0;
					}
					break;
				// Division de la clock
				case 7:
					if ( setup_ClockDivider <7) {
						setup_ClockDivider++;
						PosCurseurSetup(PosCurSetup);
						lcd.print(F("x"));
						lcd.print(ListeClockMenu[setup_ClockDivider]);
						lcd.print(F(" "));
						ClockMidi=0;
						ClockInt=0;
						ClockCv=0;
					}
					break;
			}
			break;
		case ENC2DOWN:
			switch (PosCurSetup) {
				// Gestion nombre de steps DOWN
				case 0:
					// Si mode 1 piste 32 pas
					if (setup_ModeTrack == 1 ) {
						if (setup_Steps > 1) {
							setup_Steps--;
							PosCurseurSetup(PosCurSetup);
							digitsLcd(setup_Steps,2);
							CalcPendulum();
						}
					}
					// Sinon mode 16 pas
					else {
						if (setup_Steps > 1) {
							setup_Steps--;
							PosCurseurSetup(PosCurSetup);
							digitsLcd(setup_Steps,2);
							CalcPendulum();
						}
					}
					break;
				// Gestion du sens de la sequence
				case 1:
					if ( setup_SensSeq > 0 ) {
						setup_SensSeq--;
						PosCurseurSetup(PosCurSetup);
						AffSensSeqLCD(setup_SensSeq);
					}
					break;
				// BPM
				case 2:
					if ( setup_Bpm >20 ) {
						setup_Bpm--;
						PosCurseurSetup(PosCurSetup);
						digitsLcd(setup_Bpm,3);
						myfloat=(float)((float)setup_Bpm*(float)24/(float)60);
						myfloat=(float)1/myfloat*250000;
						tcnt1=65535-(int)myfloat;
					}
					break;
				// Out (midi, CV, midi + CV)
				case 3:
					if ( setup_ModeOut > 0 ) {
						AllNoteOff();
						setup_ModeOut--;
						PosCurseurSetup(PosCurSetup);
						AffOutLCD(setup_ModeOut);
						
					}
					break;



					// Gestion mode lecture de piste (//, 1+2, 1+Vel)
				case 4:
					if ( setup_ModeTrack> 0 ) {
						setup_ModeTrack--;
						PosCurseurSetup(PosCurSetup);
						AffModeJeuPiste(setup_ModeTrack);
						if (setup_ModeTrack!=1) { 			// Si mode de jeu sur 16 pas
							if (setup_Steps >16) {			// et nombre de pas actuel > 16
								setup_Steps=16;				// remettre steps a 16
								lcd.setCursor(6,0);			// rafraichir donnees steps
								digitsLcd(setup_Steps,2);
							}
						}
					}
					
					break;
				// Gestion mode CV
				case 5:
					if ( setup_ModeCV > 0 ) {
						setup_ModeCV--;
						PosCurseurSetup(PosCurSetup);
						AffModeCVLCD(setup_ModeCV);
					}
					break;
				// Gestion Clock (MIDI, analogique externe, interne)
				case 6:
					if ( setup_ClockIn > 0 ) {
						setup_ClockIn--;
						PosCurseurSetup(PosCurSetup);
						AffTypeClockLCD(setup_ClockIn);
						ModePlay=0;
					}
					break;
				// Division de la clock
				case 7:
					if ( setup_ClockDivider > 0 ) {
						setup_ClockDivider--;
						PosCurseurSetup(PosCurSetup);
						lcd.print(F("x"));
						lcd.print(ListeClockMenu[setup_ClockDivider]); 
						lcd.print(F(" "));
//						StartGate=millis();
						ClockMidi=0;
						ClockInt=0;
						ClockCv=0;
					}
				
					break;
			}
			break;
	}	
}





//-----------------
// Gestion du setup
//-----------------
void gestion_menu_setup(void) {
	xm=0;
	ym=0;	// edition note 
	aff_menu_LCD_setup(9);	// rafraichir ecran
	rafLCD=0;
	
	// Boucler dans le menu presets
	// sauf si changement de contexte
	while(1) {
		// Tester si on doit changer de contexte
		// ou si changement piste ou note <-> gate
		//----------------------------------------
		valbouton=readButtons(PINEDIT);
			
		// Si un bouton appuye
		if (valbouton) {
		
			if (valbouton==B_PRESETS) {		// Test si bouton gestion preset appuye
				modeMenu=EDITPRESETS;		// si oui, setter le contexte Sequenceur
				xm=0;
				ym=0;
				lcd.noCursor(); 
				return;						// et revenir gestion menu principal
			}
		
		
		
			if (valbouton==B_SETUP) {		// Test si bouton preset appuye
//				modeMenu=EDITEUCLIDIAN;			// si oui, setter le contexte SETUP
				modeMenu=EDITSEQ;			// si oui, setter le contexte SETUP
				return;						// et revenir gestion menu principal
			}
		
			// Si bouton encodeur 1 	
			if (valbouton==B_ENC1) {
				
			}
			// si bouton encodeur 2 
			if (valbouton==B_ENC2) {
			}
		}
		// Tester les boutons de transport
		// pour arreter ou demarrer le sequenceur si mode horloge interne
		valbouton=readButtons(PINTRANSPORT);
		if (valbouton!=0) 					// bouton transport appuye
			gestionBoutonsTransport();		// gerer les boutons de transport
		
		// Ici on reste dans le contexte dedition
		//---------------------------------------
			
		// Lire ensuite les encodeurs
		valencodeurs=readEncodeurs();
		if (valencodeurs) {					// Si un encodeur a bouge
			gestionDonneesSetup();		// gerer les encodeurs pour mode preset
		}
		
		// arrive ici on a lu les boutons de contexte et les boutons de transports
		// les valeurs des encodeurs ont eventuellement changé les données
		// Si changement de donnees par utilisateur
		// rafraichir LCD
		if (rafLCD) {         				// Si flag affichage
			aff_menu_LCD_setup(rafLCD);  			// Rafraichir lecran si besoin
			rafLCD=0;
		}
		// gere la synchronisation midi
		// Le flag F_IntCVReset a ete positionne dans l'interrupt externe 
		// en relation avec l'entree RESET CV le sequenceur est donc arrete
		if (F_IntCVReset==1)
			MyHandleStopSeqCv2();
	}
}
