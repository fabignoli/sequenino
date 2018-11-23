//-------------------------------------------------------------
// Routine ecriture d'un octet en eeprom
// Param 1 adresse I2C (defaut 0x50 avec pins 1,2,3 a la masse)
// param 2 adresse memoire a ecrire (de 0 à 32767 pour 24LC256)
// param 3 octet a ecrire
//-------------------------------------------------------------
void eewrite_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
int rdata = data;
  Wire.beginTransmission(deviceaddress);// Selection du device
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.write(rdata);                    // Ecriture
  Wire.endTransmission();               // Fin de transmission
  delay(5);                             // Attente pour flush
}

//-------------------------------------------------------------
// Routine lecture d'un octet en eeprom
// Param 1 adresse I2C (defaut 0x50 avec pins 1,2,3 a la masse)
// param 2 adresse memoire à lire (de 0 à 32767 pour 24LC256)
//-------------------------------------------------------------
byte eeread_byte( int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);      // Selection du device
  Wire.write((int)(eeaddress >> 8));          // MSB
  Wire.write((int)(eeaddress & 0xFF));        // LSB
  Wire.endTransmission();                     // Fin de transmission
  Wire.requestFrom(deviceaddress,1);          // Requete 1 octet
  if (Wire.available()) rdata = Wire.read();  // Lecture tant que possible
    return rdata;                             // Retourner la donnee
}

//---------------------------------------
// Sauvegarde dun preset
// param 1 :  numero de la banque (O a 3)
// param 2 :  numero du preset (0 à 127)
//---------------------------------------
void save_preset(byte mybank,byte mypreset) {

int i;
int myaddress=0;
    // Initialisation de l'adresse du preset
    myaddress=(mybank*BANKSIZE)+(mypreset*PRESETSIZE);
    // Nom du preset
    for (i=0;i<16;i++) { 
      eewrite_byte( 0x50, myaddress++, nomPreset[i]);
    }
	// ecrire les 32 steps de note
    for (i=0;i<32;i++) {
		eewrite_byte( 0x50, myaddress++, SeqNotes[i]);
    }
	// ecrire les 32 steps de gate
    for (i=0;i<32;i++) {
		eewrite_byte( 0x50, myaddress++, SeqGates[i]);
	}
    // nombre de steps
	eewrite_byte( 0x50, myaddress++, setup_Steps);
	// Mode track
	eewrite_byte( 0x50, myaddress++, setup_ModeTrack);
	// BPM
	eewrite_byte( 0x50, myaddress++, setup_Bpm);
    // Type de clock
	eewrite_byte( 0x50, myaddress++, setup_ClockIn);
	// Sens du sequenceur
	eewrite_byte( 0x50, myaddress++, setup_SensSeq);
	// Mode CV
	eewrite_byte( 0x50, myaddress++, setup_ModeCV);
    // Type de sortie
	eewrite_byte( 0x50, myaddress++, setup_ModeOut);
	// Diviseur horloge
	eewrite_byte( 0x50, myaddress++, setup_ClockDivider);
}

//-----------------------------------------------
// Chargement dun preset
// param 1 :  numero de la banque (O a 1)
// param 2 :  numero du preset (0 à 127)
// Charge les donnees dans les variables globales
//-----------------------------------------------
void load_preset(byte mybank,byte mypreset) {
int i;
int myaddress=0;
    // Initialisation de l'adresse du preset
    myaddress=(mybank*BANKSIZE)+(mypreset*PRESETSIZE);
    // Nom du preset
    memset(nomPreset,0,sizeof(nomPreset));
    for (i=0;i<16;i++) {
		nomPreset[i]=eeread_byte( 0x50, myaddress++);
    }
	// lire les 32 steps de note
    for (i=0;i<32;i++) {
		SeqNotes[i]=eeread_byte( 0x50, myaddress++);
    }
	// lire les 32 steps de gate
    for (i=0;i<32;i++) {
		SeqGates[i]=eeread_byte( 0x50, myaddress++);
    }
    // nombre de steps
	setup_Steps=eeread_byte( 0x50, myaddress++);
	// Mode track
	setup_ModeTrack=eeread_byte( 0x50, myaddress++);
	// BPM
	setup_Bpm=eeread_byte( 0x50, myaddress++);
	myfloat=(float)((float)setup_Bpm*(float)24/(float)60);
	myfloat=(float)1/myfloat*250000;
	tcnt1=65535-(int)myfloat;	
    // Type de clock
	setup_ClockIn=eeread_byte( 0x50, myaddress++);
	// Sens du sequenceur
	setup_SensSeq=eeread_byte( 0x50, myaddress++);
	// Mode CV
	setup_ModeCV=eeread_byte( 0x50, myaddress++);
    // Type de sortie
	setup_ModeOut=eeread_byte( 0x50, myaddress++);
	// Diviseur horloge
	setup_ClockDivider=eeread_byte( 0x50, myaddress++);
	CalcPendulum();
	// Envoyer un note off midi si midi actif
	// car si le preset n'est pas en mode midi
	// une note peu rester coincee
	if (setup_ModeOut==0 || setup_ModeOut==2) {
		if (CountNoteOut==1) {
			AllNoteOff();
			CountNoteOut=0;						// remettre le compteur a 0
			// Mettre un delay si necessaire
			// delay(2)
		}
	}
}

//-----------------------------------------------------------
// charge un nom du preset dans la variable globale nomPreset
//-----------------------------------------------------------
void load_preset_name(byte mybank,byte mypreset) {
int i;
int myaddress=0;

    memset(nomPreset,0,sizeof(nomPreset));
    // Initialisation de l'adresse du preset
    myaddress=(mybank*BANKSIZE)+(mypreset*PRESETSIZE);
    // Nom du preset
    for (i=0;i<16;i++) {
      nomPreset[i]=eeread_byte( 0x50, myaddress++);
    }
}

//--------------------------------
// Raffraichissement ecran presets
//--------------------------------
void aff_menu_LCD_presets(byte parRaff) {
	
	// Si rafraichissement total
	// Partie statique
	if (parRaff==9) {
		lcd.clear();          // Effacer tout lecran
		lcd.setCursor(0,0);   // Positionner en haut à gauche
		switch (ym) {
			case 0:
				// "Presets>Load"
				affChaineLcdPROGMEM(4);
				lcd.setCursor(0,2);
				// SEP
				affChaineLcdPROGMEM(10);
				lcd.setCursor(0,3);
				// "P   |B   |     [OK]"
				affChaineLcdPROGMEM(9);
				lcd.setCursor(0,1);
				lcd.print(F(">>"));
				load_preset_name(noBanque,noPreset);
				affChaineLcd(nomPreset);
				lcd.print(F("<<"));
				digitsLcdPos(noPreset+1,1,3,3);
				digitsLcdPos(noBanque,6,3,3);
				break;
			case 1:
				//"Presets>Save"
				affChaineLcdPROGMEM(5);
				lcd.setCursor(0,2);
				// SEP
				affChaineLcdPROGMEM(10);
				lcd.setCursor(0,3);
				// "P   |B   |     [OK]"
				affChaineLcdPROGMEM(9);
				lcd.setCursor(0,1);
				lcd.print(F(">>"));
				load_preset_name(noBanque,noPreset);
				affChaineLcd(nomPreset);
				lcd.print(F("<<"));
				digitsLcdPos(noPreset+1,1,3,3);
				digitsLcdPos(noBanque,6,3,3);
				break;
			case 2:
				//"Presets>Name"
				affChaineLcdPROGMEM(6);
				lcd.setCursor(0,2);
				// SEP
				affChaineLcdPROGMEM(10);
				lcd.setCursor(1,3);
				lcd.printByte(CAR_LEFT);
				lcd.print(F("/"));
				lcd.printByte(CAR_RIGHT);
				lcd.setCursor(12,3);
				lcd.printByte(CAR_UP);
				lcd.print(F("/"));
				lcd.printByte(CAR_DOWN);
				lcd.print(F(" "));
				
				//"(OK)"
				affChaineLcdPROGMEM(8);
				
				 lcd.setCursor(0,1);
				// Afficher le nom de preset en cours
				lcd.print(F(">>"));
				load_preset_name(noBanque,noPreset);
				affChaineLcd(nomPreset);
				//lcd.print(nomPreset);
				lcd.print(F("<<"));
				posCarPreset=2;                 // position x premier caractere
				lcd.setCursor(posCarPreset,1);  // positionner le curseur
				lcd.cursor();                   // Affiche le curseur
				//lcd.blink();                    // Curseur clignotant  
				break;
		}
	}
	// Afficher ici la partie dynamique si besoin
	
}



//------------------------------------------------
// Gestion des encodeurs partie edition presets
//------------------------------------------------
void gestionDonneesPresets(void) {
      switch(ym) {
        case 0:
          gestionEncPresetsLoad();
          break;
        case 1:
          gestionEncPresetsSave();
          break;
        case 2:
          gestionEncPresetsName();
          break;
      }
}

//----------------------------------------------
// Gestion chargement des presets
// Selection de la banque et du numero de preset
//----------------------------------------------
void gestionEncPresetsLoad(void) {
boolean affpreset;
  affpreset=false;
  switch(valencodeurs) {
      // Preset down
      case ENC1DOWN:
        if (noPreset) {
          noPreset--;
          affpreset=true;
        }  
        break;
      // Preset UP
      case ENC1UP:
        if (noPreset<127) {
          noPreset++;
          affpreset=true;
        }                   
        break;
      // Banque Down
      case ENC2DOWN:
        if (noBanque) {
            noBanque--;
            affpreset=true;
        }                   
        break;
      // Banque UP
      case ENC2UP:
        if (noBanque<MAXBANK-1) {
             noBanque++;
             affpreset=true;
        }                   
        break;
  }
  if (affpreset) { 
      lcd.setCursor(0,1);
      lcd.print(F(">>"));
      load_preset_name(noBanque,noPreset);
      affChaineLcd(nomPreset);
      //lcd.print(nomPreset);
      lcd.print(F("<<"));
      digitsLcdPos(noPreset+1,1,3,3);
      digitsLcdPos(noBanque,6,3,3);
  }
}

//-------------------------------------
// Gestion sauvegarde du preset
// Selection du preset et de la banque
// et sauvegarde des variables globales
// dans ce preset
//-------------------------------------
void gestionEncPresetsSave(void) {
boolean affpreset;
  affpreset=false;
  switch(valencodeurs) {
      // Preset down
      case ENC1DOWN:
        if (noPreset) {
          noPreset--;
          affpreset=true;
        }  
        break;
      // Preset UP
      case ENC1UP:
        if (noPreset<127) {
          noPreset++;
          affpreset=true;
        }                   
        break;
      // Banque Down
      case ENC2DOWN:
        if (noBanque) {
            noBanque--;
            affpreset=true;
        }                   
        break;
      // Banque UP
      case ENC2UP:
        if (noBanque<MAXBANK-1) {
             noBanque++;
             affpreset=true;
        }                   
        break;
  }
  if (affpreset) {
      lcd.setCursor(0,1);
      lcd.print(F(">>"));
      load_preset_name(noBanque,noPreset);
      affChaineLcd(nomPreset);
      //lcd.print(nomPreset);
      lcd.print(F("<<"));
      digitsLcdPos(noPreset+1,1,3,3);
      digitsLcdPos(noBanque,6,3,3);
  }
}

//----------------------------------
// Gestion edition dun nom de preset
// avant la sauvegarde
// Jeu de caractere utilisee :
// [Space]()*+-./0123456789
// ABCDEFGHIJKLMNOPQRSTUVWXYZ
// abcdefghijklmnopqrstuvwxyz
//----------------------------------
void gestionEncPresetsName(void) {
boolean affchar;
char mychar;
  affchar=false;
  switch(valencodeurs) {
        
      // caractere vers la gauche
      case ENC1DOWN:
        if (posCarPreset>2) {     // x=2 sur le premier caractere
          posCarPreset--;         // decrementer la position en x
          affchar=true;
        }
        break;
      // caractere vers la droite
      case ENC1UP:
        if (posCarPreset<17) {    // x=18 sur le dernier caractere
          posCarPreset++;         // Incrementer la position en x
          affchar=true;
        }
        break;
      // Changement du caractere courant
      // prendre le precedent dans le jeu
      // fait le tour complet
      case ENC2DOWN:
        mychar=nomPreset[posCarPreset-2];   // Recuperer le caractere courant
        switch(mychar) {
          case ' ':
            mychar='z';
            break;
          case 'a':
            mychar='Z';
            break;
          case 'A':
            mychar='9'; 
            break;
          case '-':
            mychar='+'; 
            break;
          case '(':
            mychar=' '; 
            break;
          default:
            mychar--;
            break;
        }                   
        nomPreset[posCarPreset-2]=mychar;     // Rafraichir nouveau caractere
        affchar=true;                         // Forcer l'affichage
        break;
      // Changement du caractere courant
      // prendre le suivant dans le jeu
      // fait le tour complet
      case ENC2UP:
        mychar=nomPreset[posCarPreset-2];   // Recuperer le caractere courant
        switch(mychar) {
          case ' ':
            mychar='(';
            break;
          case '+':
            mychar='-'; 
            break;
          case '9':
            mychar='A'; 
            break;
          case 'Z':
            mychar='a'; 
            break;
          case 'z':
            mychar=' ';
            break;
          default:
            mychar++;
            break;
        }                   
        nomPreset[posCarPreset-2]=mychar;     // Rafraichir nouveau caractere
        affchar=true;                         // Forcer l'affichage
        break;
  }
  if (affchar) {
      lcd.noCursor(); 
      lcd.setCursor(posCarPreset,1);        // Positionner le curseur
      lcd.print(nomPreset[posCarPreset-2]); // posCarPreset - 2 pour indicer
      lcd.setCursor(posCarPreset,1);        // repositionner le curseur
      lcd.cursor();
  }
}

//--------------------
// Gestion des presets
//--------------------
void gestion_menu_presets(void) {
	xm=0;
	ym=0;	// edition note 
	aff_menu_LCD_presets(9);	// rafraichir ecran
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
				modeMenu=EDITSEQ;		// si oui, setter le contexte Sequenceur
				xm=0;
				ym=0;
				lcd.noCursor(); 
				return;						// et revenir gestion menu principal
			}
		
		
		
			if (valbouton==B_SETUP) {		// Test si bouton preset appuye
				modeMenu=EDITSETUP;			// si oui, setter le contexte SETUP
				return;						// et revenir gestion menu principal
			}
		
			// Si bouton encodeur 1 	
			if (valbouton==B_ENC1) {
				switch(ym) {
					// Si menu LOAD
					case 0:
						ym=1;			// Aller dans menu SAVE
						rafLCD=9;
						break;
					// Si menu SAVE		// Aller dans menu LOAD
					case 1:			
						ym=0;
						rafLCD=9;
						break;
					// Si menu NAME
					// Effacer le caractere courant et deplacer le curseur
					case 2:
						nomPreset[posCarPreset-2]=' ';
						lcd.noCursor();							 
						lcd.setCursor(posCarPreset,1);        // Positionner le curseur
						lcd.print(nomPreset[posCarPreset-2]); // posCarPreset - 2 pour indicer
						if (posCarPreset<17)     				// x=18 sur le dernier caractere
							posCarPreset++;  
						lcd.setCursor(posCarPreset,1);        // repositionner le curseur
						lcd.cursor();					
						break;
						
				}
			}
			// si bouton encodeur 2 
			if (valbouton==B_ENC2) {
				switch(ym) {
					// Si dans menu LOAD
					case 0: 
					
						// Charger le preset
						load_preset(noBanque,noPreset);
						break;
					// Si dans menu SAVE
					case 1:
						ym=2;
						// Aller dans le menu edition de nom
						break;
					// SI dans menu edition de nom
					case 2:
						// Sauvegarder le preset
						// revenir sur le sequenceur
						save_preset(noBanque,noPreset);
						modeMenu=EDITSEQ;
						xm=0;
						ym=0;
						lcd.noCursor();
						return;
						break;
				}
				rafLCD=9;		// On force le rafraichissement d'ecran
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
			gestionDonneesPresets();		// gerer les encodeurs pour mode preset
		}
		
		// arrive ici on a lu les boutons de contexte et les boutons de transports
		// les valeurs des encodeurs ont eventuellement changé les données
		// Si changement de donnees par utilisateur
		// rafraichir LCD
		if (rafLCD) {         				// Si flag affichage
			aff_menu_LCD_presets(rafLCD);  			// Rafraichir lecran si besoin
			rafLCD=0;
		}
		// Le flag F_IntCVReset a ete positionne dans l'interrupt externe 
		// en relation avec l'entree RESET CV le sequenceur est donc arrete
		if (F_IntCVReset==1)
			MyHandleStopSeqCv2();
	}
}


