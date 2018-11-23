//-----------------------------------------
// Initialise les parametres necessaire
// pour execution d'une ISR par le timer 2
// toutes les 2ms, afin de lire la position
// des 4 encodeurs rotatifs
//-----------------------------------------
void encodeursSetup(void) {
byte i;
  //-------------------------------------
  // Initialisation des pin des encodeurs
  // Mode input
  // Resistance pull-up activee
  //-------------------------------------
  for(i=0;i<2;i++) {
    pinMode(pinA[i], INPUT);
    pinMode(pinB[i], INPUT);
    digitalWrite(pinA[i],HIGH);
    digitalWrite(pinB[i],HIGH);
  }

  //----------------------------------------
  // Configuration du timer 2
  // Pour la lecture des encodeurs
  // Execution de la routine d'interruption
  // toutes les 2ms (par overflow)
  //----------------------------------------
  TIMSK2 &= ~(1<<TOIE2);  
  // Timer 2 en mode normal
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));  
  TCCR2B &= ~(1<<WGM22);  
  // Horloge interne
  ASSR &= ~(1<<AS2);  
  // Mode overflow
  TIMSK2 &= ~(1<<OCIE2A);  
  // Prescaler a 256
  TCCR2B |= (1<<CS22)  | (1<<CS21) ; // Set bits  
  TCCR2B &= ~(1<<CS20);              // Clear bit  
  // Sauvegarde de la valeur pou reloader dans l'ISR
  tcnt2 = 131;   
  // Chargement et mise en route du timer
  TCNT2 = tcnt2;  
  TIMSK2 |= (1<<TOIE2);
}


//----------------------------------------
// Routine executee tous les 2ms
// par overflow sur le compteur du timer 2
//----------------------------------------
ISR(TIMER2_OVF_vect) {  
int MSB;
int LSB;
int encoded;
int sum;
byte i;
  TCNT2 = tcnt2;      // Reload du timer
  // Lecture des 2 encodeurs
  for (i=0;i<2;i++) {  
    MSB = digitalRead(pinA[i]); // MSB = most significant bit
    LSB = digitalRead(pinB[i]); // LSB = least significant bit
    encoded = (MSB << 1) |LSB; // conversion
    sum  = (lastEncoded[i] << 2) | encoded; // Ajout a la valeur precedent
    // Test si on avance
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) 
      counter[i]++;
    // Test si on recule  
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)    
      counter[i]--;                                                       
    lastEncoded[i] = encoded;   // On memorise la valeur pour la prochaine fois
    bcounter[i]=counter[i]>>2;  // Compteur utilisateur divise par 4
  }
PlaySequencer();
}


//--------------------------------------------
// Routine principale de lecture des encodeurs
//--------------------------------------------
byte readEncodeurs(void) {
byte i,val;
  val=0;
  // Lire les 2 encodeurs
  for (i=0;i<2;i++) {
    // si la valeur a variee pour un des encodeurs
    if (lcounter[i]!=bcounter[i]) {
      // Tester le sens
      // Si UP valeur de 11 a 14
      if (bcounter[i]>lcounter[i])
        val=i+11;
      // Si down valeur de 1 a 4
      else
        val=i+1;
      lcounter[i]=bcounter[i];
      return val;
    }
  }
  return 0;
}

//-------------------------------------------
// Lit les boutons et renvoie la valeurs lues
// Ne permet qu'une saisie quand un bouton
// est appuyé
//-------------------------------------------
byte readButtons(int pin) {
int b,c = 0;
  if (pin==0)
	c=analogRead(A2);      // Lit lentree analogique 
  if (pin==1)
	c=analogRead(A3);      // Lit lentree analogique 
  
  if (c>950) {
      lastBoutonNav[pin]=0;  // Remettre flag etat a zero
      return 0;           // Pas de bouton appuyé sortir
  }
  // Arrive ici normalement un bouton a ete appuye
  for (int i=1; i<3; i++) {
	delay(1);              // Attente pour deboucong
  }
   if (pin==0)
	c=analogRead(A2);      // Lit lentree analogique 
  if (pin==1)
	c=analogRead(A3);      // Lit lentree analogique  
  //c=analogRead(pin);      // Relire pour debouncing 
  if (c>950) {
      lastBoutonNav[pin]=0;  // Remettre flag etat a zero
      return 0;           // Toujours pas de bouton appuye
  }
 
  if (lastBoutonNav[pin]>0)   // Si le dernier bouton n'a pas ete relache
      return 9;   // renvoit code+10 (touche 1 = 11, 2=12, etc)
  // Arrivé ici on est sur que le bouton a ete appuye
  if (c>=0 && c<40) {
      lastBoutonNav[pin]=1;
      return 1;           // bouton 1 appuye
  } 
  if (c>70 && c<250) {
      lastBoutonNav[pin]=2;
      return 2;           // bouton 2 appuye
  } 
  if (c>250 && c<400) {
      lastBoutonNav[pin]=3;
      return 3;           // bouton 3 appuye
  }                       
  if (c>400 && c<600) {
        lastBoutonNav[pin]=4;
      return 4;           // bouton 4 appuye
  }
  return 0;
}





//---------------------------------------------------
// Gestion des boutons transport sequenceur
// enregistrement sequence en midi si en mode EDITSEQ
// arret/relance du sequenceur si horloge interne
//---------------------------------------------------
void gestionBoutonsTransport(void) {
	switch(valbouton) {
		case B_REC:
			if (modeMenu==EDITSEQ)
				RecordSequence();
			break;
		case B_FUNC:
			break;
		case B_STOP:
			// Si horlorge interne
			// On utilise le transport STOP
			if (setup_ClockIn==_CLOCKINT) {
				MyHandleStopSeqInt();
				MidiSendClockStop();
			}
			break;
		case B_PLAY:
			// Si horlorge interne
			// On utilise le transport PLAY
			if (setup_ClockIn==_CLOCKINT ) {
				// Si sequenceur actuellement a l'arret
				if (ModePlay==0) {
					ClockInt=0;		// On remet l'horloge int a 0
					ModePlay=1;		// On lance le mode de lecture
					MidiSendClockStart();
					return;
				}
				// Si on jouait deja
				// on reinitialise au debut de la sequence
				// et on continue de jouer
				if (ModePlay==1) {
					flagAffLcd2=PosSeq+1;
					PosSeq=-1;					// RAZ pointeur de sequence
					OldCurSeq=-1;
					Tick=-1;					// RAZ du tick global
					LastTick=-1;				// RAZ ancien tick
					ClockInt=0;					// Clock int a 0
					AllNoteOff();
					MidiSendClockStop();
					MidiSendClockStart();
				}
			}
			break;
	}
}
