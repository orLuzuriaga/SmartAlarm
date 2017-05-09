#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
//#include <Password.h> //Incluimos la libreria Password
#include <Keypad.h> //Incluimos la libreria Keypad
#include <EEPROM.h>


SoftwareSerial SIM900(7, 8); //Inicializamos el modulo GPRS
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5); // Inicializamos el lcd


//Password password = Password( "1234" );

char* password = "1234"; // Establecemos el password
char* master = "123456";
int position = 0;  
long tiempo;
byte sensorpir = 10;
byte buzzer = 13;
byte led = 9;
byte num_click = 0;
byte estado_alarma = 0;
byte intentos = 0;
char key; 
boolean passOK = false; 
boolean passWrong = false; 

int tamanyo_password = 4;


/****Inicializamos el teclado*****/
const byte ROWS = 4; // Cuatro Filas
const byte COLS = 3; // Tres Columnas

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = { 12,11,6,5 };
byte colPins[COLS] = { 4,3,2};

// Creamos el Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );




void setup(){
      lcd.begin(16, 2);
      lcd.print("INICIALIZANDO..."); // Enviar el mensaje
      
      pinMode(sensorpir,INPUT);
      pinMode(buzzer, OUTPUT); 
      //pinMode(led, OUTPUT); 
     
      
      /*--------------encendemos el módulo GPRS ---------------------------*/
    
      SIM900.begin(19200);  //Configura velocidad del puerto serie para el SIM900
      Serial.begin(19200);  //Configura velocidad del puerto serie del Arduino
      Serial.println("Módulo encendido");
      delay (1000);
      SIM900.println("AT + CPIN = \"XXXX\"");  //Comando AT para introducir el PIN de la tarjeta
      delay(25000);  //Tiempo para que encuentre una RED
      Serial.println("PIN OK");
     
      
        
  }
      
      
     



void loop(){



   /*
    * Estado 0 = alarma inactiva
    * Ponemos en nivel bajo las salidad de buzzer, led y mostramos mensaje en el LCD
    * Comprobamos si hay una tecla pulsada con la funcion eventoTeclado()
    * Verifica si la clave pulsada es correcta o incorrecta, actualiza las variables passOK = false  
    * y estado_alarma = 1 si el password es correcto, pasando al estado 1
     */
while(estado_alarma == 0)
{

      digitalWrite(buzzer, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Alarm disarmed:");
      lcd.setCursor(0, 1);
      lcd.print("Enter key:...");
    
    eventoTeclado();
    if(passOK)
      {
        estado_alarma = 1;
        Serial.print("1");
        passOK = false;
      }
    else if(passWrong)
        {
            estado_alarma = 5;
            Serial.print("5");
            passWrong = false;
            lcd.clear();
        }
}


/*
 * Estado 1 = alarma activada pero la salida del sensorpir esta en low
 * en este estado se estable en un retardo de 20 segundos para activar
 * la alarma donde se llama al metodo retardo(), se envia por el LCD 
 * el mensaje de activacion en 20 segundo y por el buzzer se emite una señal 
 * sonora, posteriormete nuestro sistema pasa al estado 2
 *
*/
while(estado_alarma == 1)
{
   
    lcd.setCursor(0, 0);
    lcd.print("Alarm ON,20sc ");
    lcd.setCursor(0, 1);
    lcd.print("to activate ");
    retardo();
    estado_alarma = 2;
    Serial.print("2");
}




/*
 * Estado 2 = el sistema esta activo y comprueba si las salida del sensopir esta en HIGH 
 * se indica por pantalla que la salida esta activa y es necesario pulsar la contraseña para desactivarla
 * si llegara a captar un intruso pasamos al estado 3
*/

while(estado_alarma == 2)
{
    
    lcd.setCursor(0, 0);
    lcd.print("<<System armed>> ");
    lcd.setCursor(0, 1);
    lcd.print("press key:OFF..");
     eventoTeclado();
//Si el password es correcto pasamos al estado 0 si es incorrecto al estado 6  
if(passOK){
  estado_alarma = 0;
  Serial.print("0");
  passOK = false;
  digitalWrite(buzzer, LOW);
  Serial.print("alarma desconectada");
}else if(passWrong)
{
    estado_alarma = 6;
    Serial.print("6");
    passWrong = false;
}
    if(digitalRead (sensorpir) == HIGH)
      {
      estado_alarma = 3;
      Serial.print("3");
      delay(1000);
      lcd.clear();
      }
}



/*
* Estado 3= Se acaba de captar un intruso y se pondrá en funcionamiento el buzzer que indicara el tiempo de retardo que tiene 
* el usuario para desconectarla, se muestra un mensaje por pantalla indicando el tiempo de retardo que tiene para desconectarla
* el sistema
*/
while(estado_alarma == 3)
{

tiempo = millis() + 20000;

do
{
  //digitalWrite(leds, HIGH);

  digitalWrite(buzzer, HIGH);  
  delay(400);            
  digitalWrite(buzzer, LOW);  
  delay(100); 
  lcd.setCursor(0, 0);
  lcd.print("Disarmet");
  lcd.setCursor(0, 1);
  lcd.print("in 20s");
     
  eventoTeclado();
  Serial.println(keypad.getKey());
  if(passOK)
    {
    estado_alarma= 0;
    Serial.print("estoy en el estado 0, alarma apagada");
    passOK = false;
     break;
     
    }
}while(tiempo > millis());


  if (estado_alarma!= 0){
  estado_alarma = 4;
  mensaje_sms();
  Serial.print("4");
  lcd.clear();
  }
}



/*
 * Estado 4 = El sistema esta activo, se confirma que hay un intruso en la instalacion del usuario para ello se utiliza el buzzer
 * para notificar la intrusion y se enviara un mensaje de texto a través de la funcion mensaje_sms(), tambien indicaremos  en el 
 */
 

while(estado_alarma == 4){
  
  digitalWrite(buzzer, HIGH);
 

  lcd.setCursor(0, 0);
  lcd.print(" <<  ALERTA >>  ");
  lcd.setCursor(0, 1);
  lcd.print("*** Intruso  ***");
  eventoTeclado(); 
if(passOK){
  estado_alarma = 0;
  Serial.print("0");
  passOK = false;
  digitalWrite(buzzer, LOW);
  
  
}else if(passWrong)
{
    estado_alarma = 6;
    Serial.print("6");
    passWrong = false;
    llamar();
}
}




/*
 * Estado 5 = El password esta bloqueado, nos pide el codigo master para poder conectar el sistema nos ayudamos de la funcion claveMaster
 * que comprobara si el codigo es correcto
*/
while(estado_alarma == 5)
{

//digitalWrite(leds, LOW);
digitalWrite(buzzer, LOW);
//digitalWrite(altavoz, LOW);
lcd.setCursor(0, 0);
lcd.print("Password locked ");
lcd.setCursor(0, 1);
lcd.print("Enter Master ");
claveMaster();
if(passOK)
{
    estado_alarma = 1;
    Serial.print("1");
    passOK = false;
}
}




/*
 * Estado 6 se utiliza para desconectar el sistema con el codigo master para desactivar el sistema
 * se indica un mensaje por el display pidiendo el codigo master para desconectar el sistema
*/
while(estado_alarma == 6)
{

//digitalWrite(leds, HIGH);
digitalWrite(buzzer, HIGH);
//digitalWrite(altavoz, LOW);
lcd.setCursor(0, 0);
lcd.print("ON, enter Master ");
lcd.setCursor(0, 1);
lcd.print("to desactivate ");
claveMaster();
if(passOK)
{
estado_alarma = 0;
Serial.print("0");
passOK= false;
}
}


}







/*
 * Emite un pitido de 20 segundo de retardo
 */
void retardo(){

   tiempo = millis() + 20000;
    do
      {
        
        digitalWrite(buzzer, HIGH);  
        delay(400);            
        digitalWrite(buzzer, LOW);  
        delay(100);      
       }while(tiempo > millis());
 
}
   
  
void llamar()
   {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Realizando llamada...");
      Serial.println("Llamando...");
      SIM900.println("ATDXXXXXXXXX;");  //Comando AT para realizar una llamada
      delay(30000);  // Espera 30 segundos mientras realiza la llamada
      SIM900.println("ATH");  // Cuelga la llamada
      delay(1000);
      Serial.println("Llamada finalizada");
   }




/*
 * Envia un mensaje al modulo si se produce un salto de alarma
 */

void mensaje_sms()
  {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enviando SMS...");
      Serial.println("Enviando SMS...");
      SIM900.print("AT+CMGF=1\r");  //Configura el modo texto para enviar o recibir mensajes
      delay(1000);
      SIM900.println("AT + CMGS=\"XXXXXXXXX\"");  //Numero al que vamos a enviar el mensaje
      delay(1000);
      SIM900.println("ALARMA, HAY INTRUSOS EN SU HOGAR");  // Texto del SMS
      delay(100);
      SIM900.println((char)26); //Comando de finalización ^Z
      delay(100);
      SIM900.println();
      delay(5000);  // Esperamos un tiempo para que envíe el SMS
      Serial.println("SMS enviado");
   }


/*
 * Almacena un tecla pulsada, emite un sonido y la oculata por el caracter "*"
 */

void eventoTeclado(){

key = keypad.getKey();
    if (key == '*' || key == '#')
    {
      position = 0;
      num_click = 0;
      Serial.print("reiniciado");
      delay(100);
    }
    // Si es correcta la contraseña se incrementa
   if (key == password[position]){
 
        position ++;
        delay(200);
    }
   if (key >= '0')
    {
      num_click ++;
      delay(200);
    }


if (num_click == 4)
{
      
    if (position == 4)
    {
      passwordCorrecto(); 
        passOK = true;
        num_click = 0;
        intentos = 0;
        position = 0;
    }else{
     passwordIncorrecto();
       intentos++;
       num_click = 0;
       position = 0;
    }
}

 if (intentos == 3){
  
      intentos = 0;
      passWrong = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("pass bloqueado ");
  }

  
}




void claveMaster()
{
 key = keypad.getKey();
if (key == '*' || key == '#')
{
position = 0;
num_click = 0;
delay(100);
}
if (key == master[position])
{
position ++;
delay(100);
}
if (key >= '0')
{
num_click ++;
delay(100);
}
if (num_click == 6)
{
if (position == 6)
{
    passOK = true;
    num_click = 0;
    position = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Correct key ");
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(500);
}
else
{
  num_click = 0;
  position = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong key ");
  digitalWrite(buzzer, HIGH);
  delay(2000);
  }
}
}



/*
 * Emite una señal sonora indicando la conexion de la alarma e imprime un mensaje por el lcd
 */
void passwordCorrecto(){
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Correct key ");
        delay(1000);
        lcd.clear();
     
  }

/*
 * Emite una señal sonora de desconexion e imprime un mensaje de confirmacion por el lcd
 */
void passwordIncorrecto(){
  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error Password ");
    delay(1000);
    lcd.clear();

         
  }  
  
 


