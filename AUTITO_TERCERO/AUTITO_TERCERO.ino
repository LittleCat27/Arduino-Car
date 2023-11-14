/*
 * Estudiantes: 
 * - Joaquin Gregoire
 * - Jose Ignacio Vazquez
 *
 * Proyecto: Auto control Remoto 
 * Caracteristicas: 
 * - Control remoto con Bluetooth
 * - Control Anti-Choque con sensores Ultrasonicos
 * - Dirección con Servomotor
 * - Control de velocidades con Cambios
 * - Sistema de Luces 
 * - LED_TEST para chequear Codigo no bloqueante
 */

#include <DistanceSensor.h>

#define PIN_RX          0

#define PIN_M1_I1       2
#define PIN_M1_I2       4
#define PIN_SPD_M1      3

#define PIN_M2_I1       7
#define PIN_M2_I2       8
#define PIN_SPD_M2      9

#define PIN_TR_1        5
#define PIN_EC_1        6

//Pines sensores Distancia
#define PIN_EC_2        12
#define PIN_TR_2        13

#define PIN_SERVO       10
#define PIN_LUCES       11


//LED_TEST
#define VELOCIDAD_LEDTEST   2000
#define TIEMPO_PRENDIDOTEST 100
#define CAMBIAR_LEDTEST(x)   digitalWrite(A0, x)
#define CONFIGURAR_LEDTEST  pinMode(A0, OUTPUT)


#define CONFIG_PIN_SERV pinMode(PIN_SERVO,OUTPUT)

#define CONFIG_PIN_M1I1 pinMode(PIN_M1_I1,OUTPUT)
#define CONFIG_PIN_M1I2 pinMode(PIN_M1_I2,OUTPUT)
#define CONFIG_PIN_M1SP pinMode(PIN_SPD_M1,OUTPUT)

#define CONFIG_PIN_M2I1 pinMode(PIN_M2_I1,OUTPUT)
#define CONFIG_PIN_M2I2 pinMode(PIN_M2_I2,OUTPUT)
#define CONFIG_PIN_M2SP pinMode(PIN_SPD_M2,OUTPUT)

#define CONFIG_PIN_LUZ  pinMode(PIN_LUCES,OUTPUT)

#define AVANZAR_M2(x,y) digitalWrite(PIN_M2_I1,x); digitalWrite(PIN_M2_I2,!x); analogWrite(PIN_SPD_M2,y)
#define AVANZAR_M1(x,y) digitalWrite(PIN_M1_I1,!x); digitalWrite(PIN_M1_I2,x); analogWrite(PIN_SPD_M1,y)

//#define AVANZAR(x,y)    AVANZAR_M2(x,y); AVANZAR_M1(x,y) //Esto por alguna Razon el codigo no lo tomaba, asi que se convirtio en funcion

#define DOBLAR(x)       analogWrite(PIN_SERVO, map(x,0,180,0,255)) 

#define PRENDER_LUCES(x) digitalWrite(PIN_LUCES,x)

//Configuracion del control de choque
#define TIEMPO_CONTROL  100
#define DISTANCIA_SEGURIDAD 30 //Valor en centimetros de la distancia a la que empieza a frenar el auto

#define ANGULO_MAXIMO 115
#define ANGULO_MINIMO 63

//Declaracion de sensores.
DistanceSensor sensorTracero(PIN_TR_2, PIN_EC_2);
DistanceSensor sensorDelantero(PIN_TR_1, PIN_EC_1);


//Variables globales.
int Cambio = 0;
int DireccionActual;
int AnguloServo = 90;
bool LucesPrendidas;

void setup() 
{
  Serial.begin(9600);
  CONFIG_PIN_M1I1;
  CONFIG_PIN_M1I2;
  
  CONFIG_PIN_M2I1;
  CONFIG_PIN_M2I2;
  
  CONFIG_PIN_M2SP;
  CONFIG_PIN_M1SP;

  CONFIG_PIN_LUZ;
  CONFIGURAR_LEDTEST; 
  

  DOBLAR(AnguloServo);

  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

  LucesPrendidas = false;
  PRENDER_LUCES(LucesPrendidas);
}

void loop() 
{ 
  ControlManual();
  Blink();

}

void AVANZAR(bool x,char y){ //Esto por alguna Razon el codigo no lo tomaba, asi que se convirtio en funcion
  AVANZAR_M2(x,y); 
  AVANZAR_M1(x,y);
}

void ControlManual()
{
  if(CONTROL_DE_CHOQUES()) 
  {
    char comando = Serial.read();
    if(comando == 'P' || comando == ' ') return;
    EJECUTAR_COMANDO(comando);
    
  }else AVANZAR(true,0);
}

void EJECUTAR_COMANDO(char comando)
{
  switch(comando)
  {
      case 'A':// "A" -> IZQUIERDA
        IR_IZQUIERDA();
        break;
      case 'S':// "S" -> ABAJO
        DireccionActual = 0;
        IR_ATRAS();
        break;
      case 'D':// "D" -> DERECHA
        IR_DERECHA();
        break;
      case 'W':// "W" -> ARRIBA
        DireccionActual = 1;
        IR_ADELANTE();
        break;
      case 'F':// "F" -> FRENAR 
        FRENAR();
        break;
      case 'C':// "C" -> Cambio++
        METER_CAMBIO();
        break;
      case 'N':// "N" -> Cambio--
        BAJAR_CAMBIO();
        break;
      case 'L':// "L" -> Luces
        LucesPrendidas = !LucesPrendidas;
        PRENDER_LUCES(LucesPrendidas);
        break;
      /*case 'E':
        ESTACIONAR();
      break;*/
      default:
        break;
  }
}

void IR_ADELANTE()
{
  switch(Cambio)
  { 
    case 1:
      AVANZAR(true,84);
      break;

    case 2:
      AVANZAR(true,168);
      break;

    case 3:
      AVANZAR(true,255);
      break;

    default:
      AVANZAR(true,0);
      break;
  }
  //AVANZAR(true,255);
}

void IR_ATRAS()
{
  switch(Cambio)
  { 
    case 1:
        AVANZAR(false,84);
    break;

    case 2:
        AVANZAR(false,168);
    break;

    case 3:
        AVANZAR(false,255);
    break;

    default:
        AVANZAR(false,0);
    break;
  }
}

void IR_DERECHA()
{
  if(AnguloServo > ANGULO_MAXIMO) return;
  AnguloServo += 10;
  DOBLAR(AnguloServo);
}

void IR_IZQUIERDA()
{
  if(AnguloServo < ANGULO_MINIMO) return;
  AnguloServo -= 10;
  DOBLAR(AnguloServo);
}

void FRENAR()
{
   AVANZAR(false,0);
   Cambio = 0;
}

void METER_CAMBIO()
{
  if(Cambio < 3) Cambio++; 
  
  if(DireccionActual) IR_ADELANTE();
  else IR_ATRAS();
}

void BAJAR_CAMBIO()
{
  if(Cambio > 0) Cambio--;

  if(DireccionActual) IR_ADELANTE();
  else IR_ATRAS();
}

bool CONTROL_DE_CHOQUES()
{
  static unsigned long tiempoAnterior = 0;

  if((millis() - tiempoAnterior) < TIEMPO_CONTROL) return true;
  tiempoAnterior = millis();

  return(ControlDistancia(DireccionActual ? sensorDelantero.getCM() : sensorTracero.getCM()));
}

bool ControlDistancia(int distancia)
{

  if(distancia <= 20 && Cambio > 0) return false;
   
  if(distancia > 20 && distancia <= 30 && Cambio > 1)
  {
    Cambio = 1;
    if (DireccionActual) IR_ADELANTE();
    else IR_ATRAS();
    return true;
  }

  if(distancia > 30 && distancia <= 40 && Cambio > 2)
  {
    Cambio = 2;
    if (DireccionActual) IR_ADELANTE();
    else IR_ATRAS();
    return true;
  }

  return true;
}

void Blink(void)
  {
  static bool estado=0;
  static unsigned long tpo_ini=0, tpo_espera;
 
  if(millis() - tpo_ini < tpo_espera) return;
 
  tpo_ini=millis();
  //encender o apagar el led
  estado=!estado;
  if(estado==true)
    {
      CAMBIAR_LEDTEST(HIGH);
      tpo_espera=TIEMPO_PRENDIDOTEST;
    }
  else            
    {
      CAMBIAR_LEDTEST(LOW);
      tpo_espera=VELOCIDAD_LEDTEST - TIEMPO_PRENDIDOTEST;
    }
  }
