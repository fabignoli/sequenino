//------------------------------------
// Affiche symbole separateur vertical
//------------------------------------
void PrintSeparateur(void) {
	lcd.printByte(CAR_BARRE);
}

//---------------------
// Affiche symbole stop
//---------------------
void PrintStop(void) {
	lcd.setCursor(17,0);
	lcd.print(F("("));
	lcd.printByte(CAR_STOP);
	lcd.print(F(")"));
	TogglePlay=0;
}

//---------------------
// Affiche symbole play
//---------------------
void PrintPlay(void) {
	lcd.print(F("("));
	lcd.printByte(CAR_TRIRIGHT);
	lcd.print(F(")"));
}
//---------------------------------
// Affiche nbre step de la sequence
//---------------------------------
void PrintNbSteps(void) {
	lcd.setCursor(17,2);
	lcd.print(F("S"));
	digitsLcd(setup_Steps,2);
}

//-------------------------------
// Efface le curseur de position 
// dans la sequence
//-------------------------------
void ClearEditCursor(void) {
	lcd.setCursor((PosAff%4)*4,(int)PosAff/4);	// Trouve position courante a lecran
	lcd.print(" ");								// Efface ancien pointeur
}

//-------------------------------
// Affiche le curseur de position 
// dans la sequence
//-------------------------------
void PrintEditCursor(void) {
	lcd.setCursor((PosAff%4)*4,(int)PosAff/4);
	lcd.printByte(CAR_TRIRIGHT);
}


//------------------------------
// Affiche letat play
// avec 3 fleches vers la droite
// si dans menu edition
//------------------------------
void AffichePlay(void) {
	if (ModePlay==0) {
		TogglePlay=0;
		return;
	}
	// Verifier si play n'est pas deja afficher
	if (TogglePlay==0) {	// Si ce nest pas le cas
		TogglePlay=1;		// Memoriser que ca va etre fait
		lcd.setCursor(17,0);		// Afficher ->->->
		PrintPlay();
	}
}

//------------------------------------------------------------
// Effacement ancien pointeur de sequenceur ou pointeur actuel
//------------------------------------------------------------
void EffacePointeurSeq(byte Position) {
	if (Position<0 ||Position>32)
		return;
	// Si mode 2 x 16 pas (Note + gate ou Note + Velo)
	if (setup_ModeTrack==0 || setup_ModeTrack==2) {
			lcd.setCursor((Position%4)*4,(int)Position/4);	// Se mettre a la bonne position
			lcd.print(F(" "));								// Afficher le pointeur de sequence
			return;
	}
	// Si mode 32 pas, position <16 et page 0
	if (setup_ModeTrack==1 && Position<16 && xm==0) {
			lcd.setCursor((Position%4)*4,(int)Position/4);	// Se mettre a la bonne position
			lcd.print(F(" "));								// Afficher le pointeur de sequence
			return;
	}
	// Si mode 32 pas, position >16 et page 1
	if (setup_ModeTrack==1 && Position>=16 && xm==1) {
			lcd.setCursor(((Position-16)%4)*4,(int)(Position-16)/4);	// Se mettre a la bonne position
			lcd.print(F(" "));							// Afficher le pointeur de sequence
			return;
	}

	if (ModePlay==0) {
		// Effacement curseur si PosSeq >16 et mode 32 pas
		if (xm==1 && PosSeq>=16 && modeMenu==EDITSEQ) {
			lcd.setCursor(((OldCurSeq-15)%4)*4,(int)(OldCurSeq-15)/4);	// Se mettre a la bonne position
			lcd.print(F(" "));								// Afficher le pointeur de sequence	
		}
	}
}

//----------------------------------------------
// Affiche le pointeur de position du Sequenceur
//----------------------------------------------
void AffichePointeurSeq(byte Position) {
	if (Position <0 || Position >32)
		return;
	lcd.setCursor(17,2);
	lcd.print(F("P"));
	if (Position <9)
		lcd.print("0");
	lcd.print(Position+1);
	// Si mode 2 x 16 pas (Note + gate ou Note + Velo)
	if (setup_ModeTrack==0 || setup_ModeTrack==2) {
		lcd.setCursor((Position%4)*4,(int)Position/4);	// Se mettre a la bonne position
		lcd.print(F("*"));								// Afficher le pointeur de sequence
		return;
	}
	// Si mode 32 pas, position <16 et page 0
	if (setup_ModeTrack==1 && Position<16 && xm==0) {
		lcd.setCursor((Position%4)*4,(int)Position/4);	// Se mettre a la bonne position
		lcd.print(F("*"));								// Afficher le pointeur de sequence
		return;
	}
	// Si mode 32 pas, position >16 et page 1
	if (setup_ModeTrack==1 && Position>=16 && xm==1) {
		lcd.setCursor(((Position-16)%4)*4,(int)(Position-16)/4);	// Se mettre a la bonne position
		lcd.print(F("*"));								// Afficher le pointeur de sequence
		return;
	}
}


//-----------------------------------
// Affiche le pointeur edition">"
// A la bonne position
// permet de rafraichir si besoin est
//-----------------------------------
void AffichePointeurEdition(void) {
		// Remet le curseur edition ">" si besoin
		// Mode 2 x 16 pas
		if (setup_ModeTrack==0 || setup_ModeTrack==2 ){
			if (PosAff!=PosSeq) {
				PrintEditCursor();
			}
		}
		// Remet le curseur edition ">" si besoin
		// Mode 32 pas page 0
		if (setup_ModeTrack==1 && PosSeq<16 && xm==0){
			if (PosAff!=PosSeq) {
				PrintEditCursor();
			}
		}
		if (setup_ModeTrack==1 && PosSeq>=16 && xm==0){
			PrintEditCursor();
		}
		
		// Remet le curseur edition ">" si besoin
		// Mode 32 pas page 1
		if (setup_ModeTrack==1 && PosSeq>=16 && xm==1){
			if (PosAff!=PosSeq-16) {
				PrintEditCursor();
			}
		}
		// Remet le curseur edition ">" si besoin
		// Mode 32 pas page 1
		if (setup_ModeTrack==1 && PosSeq<16 && xm==1){
			PrintEditCursor();
		}
}
//----------------------------------------
// Affiche une note sous forme americaine
// Exemple C#3
// Le parametre est le numero de note midi
//----------------------------------------
void affNoteLCD(byte pitch) {
   if (pitch>=24) {
    //lcd.print(Note[ (pitch % 12 ) ]);
    strcpy_P(BuffAff, (char*)pgm_read_word(&(Note[(pitch % 12 )])));
    lcd.print(BuffAff); 
    lcd.print((pitch/12)-2,DEC);
    if (strlen(BuffAff)<2) {
       lcd.print(" ");
	}
  }
  else {
    lcd.print(F("---"));
  }
}
//----------------------------------------
// Affiche un octet en binaire
// Sert pour du debug
// A enlever une fois le programme termine
//----------------------------------------
void aff_binary(byte oct) {
byte i;
    lcd.print(F("B"));
    for (i=8; i>0; i--) {
        if (bitRead(oct,i-1))
            lcd.print(F("1"));
        else
            lcd.print(F("0"));
    }
}

//---------------------------------
// Affichage d'une sequence generee
// par lalgorythme euclidien
//---------------------------------
void aff_seq_binary(byte oct) {
byte i;
    for (i=8; i>0; i--) {
        if (bitRead(oct,i-1))
            lcd.print(F("X"));
        else
            lcd.print(F("."));
    }
}


//--------------------------------------------------------
// Affiche une donne numerique avec des 0 non significatif
// nbdigits = longueur totale du nombre  entre 1 et 3 chiffres
// le tout à la position x,y
//--------------------------------------------------------
void digitsLcdPos(byte valeur, byte x, byte y, byte nbdigits) {
  lcd.setCursor(x,y);
  if (nbdigits==3 && valeur <100)
      lcd.print(F("0"));
  if (valeur<10)
      lcd.print(F("0"));
  lcd.print(valeur,DEC);
}

//--------------------------------------------------------
// Affiche une donne numerique avec des 0 non significatif
// nbdigits = longueur totale du nombre  entre 1 et 3 chiffres
//--------------------------------------------------------
void digitsLcd(byte valeur, byte nbdigits) {
  if (nbdigits==3 && valeur <100)
      lcd.print(F("0"));
  if (valeur<10)
      lcd.print(F("0"));
  lcd.print(valeur,DEC);
}

//--------------------------------------------------------
// Affiche "On" si valeur >0 sinon Off
// le tout a la position x,y
//--------------------------------------------------------
void OnOffLcd(byte valeur, byte x, byte y) {
  lcd.setCursor(x,y);      
  if (valeur)
      lcd.print(F("On "));
  else
      lcd.print(F("Off"));
}


//---------------------------------------------------
// Affiche des chaines statiques sur le LCD
// on aurait pu utiliser lcd.print simplement
// mais cette procedure permet de lire le MIDI
// entre chaque caractere affiche, cela permet
// pendant un affichage important de reduire
// la latence.
// Les chaines sont en PROGMEM pour liberer de la RAM
//---------------------------------------------------
void affChaineLcdPROGMEM(byte item) {
byte pos;
  memset(BuffAff,0,sizeof(BuffAff));
  // recopie item de la flash vers une chaine en RAM
  strcpy_P(BuffAff, (char*)pgm_read_word(&(TextMenu[item])));
  for (pos=0;pos<strlen(BuffAff);pos++) {
       lcd.print(BuffAff[pos]);
  }
}

//-----------------------------------------
// Affiche le mode CV actif
//-----------------------------------------
void AffModeCVLCD(byte valeur) {
  switch(valeur) {
    case 0:
		lcd.print(F("V/Oct"));	// Volt par octave
      break;
    case 1:
		lcd.print(F("Hz/V "));	// Hertz par volt
      break;
    case 2:
		lcd.print(F("SX150"));	// SX150
      break;
  }
}

//---------------------------
// Affiche le type de sortie
// MIDI, CV ou MIDI + CV
//---------------------------
void AffOutLCD(byte valeur) {
  switch(valeur) {
    case 0:
		lcd.print(F("MIDI "));     // MIDI
      break;
    case 1:
		lcd.print(F("CV   "));   // CV
      break;
    case 2:
		lcd.print(F("MidCV"));    // MID+CV
      break;
  }
}

//---------------------------
// Affiche le type de sortie
// MIDI, CV ou MIDI + CV
//---------------------------
void AffTypeClockLCD(byte valeur) {
  switch(valeur) {
    case 0:
		lcd.print(F("Mid"));     // MIDI Clock
      break;
    case 1:
		lcd.print(F("Cv "));   // CV (clock analogique)
      break;
    case 2:
		lcd.print(F("Int"));    // Internal
      break;
  }
}

//-----------------------------------
// Affiche le mode de jeu des pistes
// 2 pistes de 16 pas en //
// 1+2= 1 piste de 32 pas
// 1+V 1 piste note, 1 piste velocité
//-----------------------------------
void AffModeJeuPiste(byte valeur) {
  switch(valeur) {
    case 0:
		lcd.print(F("// "));     // MIDI Clock
      break;
    case 1:
		lcd.print(F("1+2"));   // CV (clock analogique)
      break;
    case 2:
		lcd.print(F("1+V"));    // Internal
      break;
  }
}



//-----------------------------------------
// Affiche le sens du sequenceur
//-----------------------------------------
void AffSensSeqLCD(byte valeur) {
  switch(valeur) {
    case 0:
		lcd.print(F("Up "));   // Up
      break;
    case 1:
		lcd.print(F("Dow"));   // Down
      break;
    case 2:
		lcd.print(F("Col"));   // Colonne
      break;
    case 3:
		lcd.print(F("Pen"));   // Pendulaire
      break;
    case 4:
		lcd.print(F("Rnd"));	// Random
      break;
  }
}

//---------------------------------------------------
// Affiche une chaine passee en parametre
// avec lecture du midi entre chaque caractere
// Identique au fonctionnement de affChaineLcdPROGMEM
// mais utilise la RAM
//---------------------------------------------------
void affChaineLcd(char *mychaine) {
int i;
byte mychar;
char *p;
  p=mychaine;
  for (i=0;i<strlen(mychaine);i++,p++) {
     lcd.printByte(*p);
  }
}
