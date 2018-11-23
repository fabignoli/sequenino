//---------------------------------------
// Fonction ecriture dans un DAC
// Param 1 : Numero du dac (DAC1 ou DAC2)
// Param 2 : donnee
//---------------------------------------
void writeDAC(uint8_t numdac, uint16_t datain) {
    
    outputValue = datain;           // recuperation de la donnee
    digitalWrite(numdac, LOW);      // Mettre a l'ecoute le bon dac
    data = highByte(outputValue);   // Extraire la partie MSB des donnees
    data = 0b00001111 & data;       // Nutiliser que les 4 bits de poids faibles
    data = 0b00110000 | data;       // Mettre les bits 4 et 5 à 1
    SPI.transfer(data);             // transferer la permiere partie
    data = lowByte(outputValue);    // extraire la partie LSB des donnees
    SPI.transfer(data);             // transferer les donnees
    digitalWrite(numdac, HIGH);     // rentre le DAC disable en entree
} 
