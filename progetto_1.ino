#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>

const int but=10; // pin del bottone
int pos = 0; // posizione iniziale del servo
// variabili utilizzate nelle funzioni di controllo e inserimento
// usate principalmente per i cicli for
int i=0;
int t, y, s;
// variabili booleane utilizzate nelle funzioni di controllo
// e inserimento
bool prova;
bool control;
bool ctrl;
bool pieno;
// variabili per l'inizializzazione del sensore di temperatura
// e il controllo della temperatura
int val_adc= 0;
float temp;
float tempStudente;
Servo servo13; // variabile per il microservo
int posti= 2; // posti totali disponibili nell'aula
LiquidCrystal lcd(12, 11, A0, A1, A2, A3); 
// inizializzazione del tastierino numerico 4x4
byte colonnePin[4]= {5, 4, 3, 2};
byte righePin[4]= {9, 8, 7, 6};
char Keys[4][4] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};                   
Keypad keyp = Keypad(makeKeymap(Keys), righePin, colonnePin, 4, 4);

char key;
char keyInserita[5]; // array dei numeri inseriti da input dagli
					 // studenti che tentano di accedere in aula
char inAula [30][5]; // matrice delle matricole degli studenti
					 // che sono dentro l'aula
char matrPrenotati [5][5]= // matrice (già nota) delle matricole
{						   // degli studenti già prenotati
{'1','2','3','4'},
{'5','6','7','8'},
{'9','0','1','2'},
{'3','4','5','6'},
{'7','8','9','0'}
};
                 

void setup() {
  pinMode(but,INPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Posti disponibili:");
  servo13.attach(13);
  servo13.write(pos);
  lcd.setCursor(0, 1);
  lcd.print(posti);
}

void loop() {

int ON=digitalRead(but); // variabile che determina lo stato
  						 // del bottone (acceso, spento)
// se i posti sono esauriti, tutte le funzioni sono bloccate
// e non è possibile digitare nulla sul tastierino
if (posti==0){ 
  if (pieno == true) {
    servo13.write(pos);
    lcd.clear();
    lcd.print("Aula piena!");
    pieno=false;
    delay(1000);
  }
  // se si preme il bottone quando l'aula è piena, si procederà
  // a svuotare l'aula dagli studenti e quindi il numero dei posti
  // verrà resettato al suo massimo, ovvero 30
  if (ON==1){
    delay(100);
    posti=30;
    for(t=0;t<30;t++){
      for(i=0;i<5;i++){
        inAula[t][i]='\0';
      }
    }
    visualizzaPosti();
  }
  // tutto ciò che avviene in questo else può accadere solo nel
  // caso in cui ci sia ALMENO un posto disponibile in aula
  } else {
  key = keyp.getKey();
  delay(50);
  // quando si inizia a premere un pulsante, si entra in questa
  // funzione, che salverà ogni numero premuto nell' array
  // keyInserita (nota bene, visto che le matricole sono composte
  // da 4 cifre non è possibile digitare più di 5 numeri)
  if (key != NO_KEY && i<5) {
    if (i == 0) {
      lcd.clear();   
    }
    keyInserita[i]=key;
    i++;
    if (key != '#') {
      lcd.print(key);
    }
  }
  // il pulsante # funge da invio e dà inizio a tutte le funzioni
  // di controllo (il funzionamento di queste funzioni sarà 
  // descritto dopo il loop)
  if (key == '#') {
      ctrl= controlloMatricola(keyInserita);
      prova= true;
      tempStudente= controlloTemperatura();
      control= controlloInAula();
  }
  // se tutti i controlli vanno a buon fine: la porta si apre,
  // la matricola dello studente che è appena entrato viene
  // inserita nella matrice inAula e il numero di posti disponib.
  // dell'aula viene decrementato
  if (ctrl==true && tempStudente < 37 && control== false) {
    servo13.write(90);
    lcd.setCursor(0, 1);
    lcd.print("Ingresso OK");
    inserimentoMatricola();
    posti--;
    aulaPiena();
    delay(5000);
    visualizzaPosti();
  } else if(ctrl==false && prova == true) {
    // se lo studente non si è prenotato (ctrl==false) ma tutti
    // gli studenti prenotati sono già dentro l'aula, allora
    // può comunque effettuare l'ingresso, ripetendo gli stessi
    // passi spiegati sopra
    if(inAula[4][0]!= '\0'){
      servo13.write(90);
      lcd.setCursor(0, 1);
      lcd.print("Ingresso OK");
      inserimentoMatricola();
      posti--;
      aulaPiena();
      delay(5000);
      visualizzaPosti();
    } else {
    // se non tutti i prenotati sono dentro l'aula, allora lo
    // studente la cui matricola non risulta nella matrice dei
    // prenotati non potrà entrare e sarà costretto ad aspettare
    lcd.setCursor(0, 1);
    lcd.print("Non prenotato");
    delay(3000);
    visualizzaPosti();
      }
  } else if (tempStudente > 37) {
    // se la temperatura dello studente supera la soglia massima,
    // nessun studente, prenotato o no, può accedere in aula
    lcd.setCursor(0, 1);
    lcd.print("Temp alta");
    delay(3000);
    visualizzaPosti();
  }  
  // il pulsante C consente, nel caso di errori di digitazione,
  // a cancellare tutti i numeri digitati e ripetere l'operazione
  if (key == 'C') {
    i=0;
    lcd.clear();
  }
 }  
}

// questa è la funzione di default che viene invocata subito dopo
// qualsiasi operazione (sia in caso di successo che di fallimento)
// consisente semplicemente nel resettare tutte le variabili
// utilizzate nel loop() e visualizzare quanti posti rimangono
void visualizzaPosti() {
  i=0;
  prova=false;
  ctrl=false;
  tempStudente= 0;
  servo13.write(pos);
  lcd.clear();
  lcd.print("Posti disponibili:");
  lcd.setCursor(0, 1);
  lcd.print(posti);
}

float controlloTemperatura() {
   val_adc = analogRead(A4);
   temp = ((val_adc * 0.00488) - 0.5) / 0.01;
   return temp;
}

// questa funzione ha lo scopo di verificare se la matricola
// digitata è già entrata in aula o no; questo perchè sappiamo
// che la matricola è univoca, motivo per cui non è possibile
// utilizzare due o più volte una stessa matricola per poter
// accedere in aula
bool controlloInAula() {
int var=0;
  for(t=0; t<30; t++) {
    for (y=0; y<5; y++) {
    	if (keyInserita[y] == inAula[t][y]) {
      		var++;
        }else {
        	var=0;
            break;
        }if(var==4) break;        
    }if(var==4) break;
  }
  if (var==4) {
    lcd.setCursor(0, 1);
    lcd.write("Gia' dentro");
    delay(5000);
    visualizzaPosti();
    return true;
  }
  else {
    return false;
  }
}

// questa funzione inserisce la matricola degli studenti che
// effettuano l'ingresso in aula nella matrice inAula, così
// da avere una lista completa di tutti gli studenti attualmente
// presenti all'interno dell'aula
void inserimentoMatricola() {
int conta=0;
  for (t=0; t<30; t++) {
    if (inAula[t][0] == '\0') {
    	for (y=0; y<5; y++) {
            if (conta==4) break;
        	inAula[t][y]= keyInserita[y];
            conta++;      
      	}      
      if (conta==4) break;
    }
  }
}

// questa funzione controlla se la matricola inserita dallo
// studente risulta tra i prenotati o no
bool controlloMatricola(char* mat){
	delay(1000);
	int x=0;
    int j;
	for(j=0;j<5;j++){
		for(s=0; s<4; s++){
			if(mat[s]== matrPrenotati[j][s]){
				x++;
				if(x==4) break;
			}else break;
		if(x==4)break;
		}
	}
	if(x==4) return true;
	else return false;
} 

void aulaPiena() {
  if(posti==0) {
    pieno=true;
  }
}