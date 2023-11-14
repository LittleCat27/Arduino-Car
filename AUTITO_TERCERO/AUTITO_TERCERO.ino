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

#include "DistanceSensor.h"

#define PIN_RX          0

#define PIN_MOT_DER__I1       2
#define PIN_MOT_DER__I2       4
#define PIN_SPD_MOT_DER      3

#define PIN_MOT_IZQ__I1       7
#define PIN_MOT_IZQ__I2       8
#define PIN_SPD_MOT_IZQ      9

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

#define CONFIG_MOT_DER_I1 pinMode(PIN_MOT_DER__I1,OUTPUT)
#define CONFIG_MOT_DER_I2 pinMode(PIN_MOT_DER__I2,OUTPUT)
#define CONFIG_MOT_DERSP pinMode(PIN_SPD_MOT_DER,OUTPUT)

#define CONFIG_MOT_IZQ_I1 pinMode(PIN_MOT_IZQ__I1,OUTPUT)
#define CONFIG_MOT_IZQ_I2 pinMode(PIN_MOT_IZQ__I2,OUTPUT)
#define CONFIG_MOT_IZQSP pinMode(PIN_SPD_MOT_IZQ,OUTPUT)

#define CONFIG_PIN_LUZ  pinMode(PIN_LUCES,OUTPUT)

#define AVANZAR_MOT_IZQ(x,y) digitalWrite(PIN_MOT_IZQ__I1,x); digitalWrite(PIN_MOT_IZQ__I2,!x); analogWrite(PIN_SPD_MOT_IZQ,y)
#define AVANZAR_MOT_DER(x,y) digitalWrite(PIN_MOT_DER__I1,!x); digitalWrite(PIN_MOT_DER__I2,x); analogWrite(PIN_SPD_MOT_DER,y)

//#define AVANZAR(x,y)    AVANZAR_MOT_IZQ(x,y); AVANZAR_MOT_DER(x,y) //Esto por alguna Razon el codigo no lo tomaba, asi que se convirtio en funcion

#define DOBLAR(x)       analogWrite(PIN_SERVO, map(x,0,180,0,255)) 

#define PRENDER_LUCES(x) digitalWrite(PIN_LUCES,x)

//Configuracion del control de choque
#define TIEMPO_CONTROL  100
#define DISTANCIA_SEGURIDAD 30 //Valor en centimetros de la distancia a la que empieza a frenar el auto

#define ANGULO_MAXIMO 115
#define ANGULO_MINIMO 63


//COMANDOS

#define CMD_IZQUIERDA   'A'
#define CMD_DERECHA     'D'
#define CMD_ARRIBA      'W'
#define CMD_ABAJO       'S'
#define CMD_FRENAR      'F'
#define CMD_SUBE_CAMBIO 'C'
#define CMD_BAJA_CAMBIO 'N'
#define CMD_LUCES       'L'



//Declaracion de sensores.
DistanceSensor sensorTracero(PIN_TR_2, PIN_EC_2);
DistanceSensor sensorDelantero(PIN_TR_1, PIN_EC_1);


//Variables globales.
int Cambio = 0;
int DireccionActual;
int AnguloServo = 90;
bool LucesPrendidas;

int velocidades[] = {0,84, 168, 255 };

void setup() 
{
  Serial.begin(9600);
  CONFIG_MOT_DER_I1;
  CONFIG_MOT_DER_I2;
  
  CONFIG_MOT_IZQ_I1;
  CONFIG_MOT_IZQ_I2;
  
  CONFIG_MOT_IZQSP;
  CONFIG_MOT_DERSP;

  CONFIG_PIN_LUZ;
  CONFIGURAR_LEDTEST; 

  CONFIG_PIN_SERV;
  

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

void AVANZAR(){ //Esto por alguna Razon el codigo no lo tomaba, asi que se convirtio en funcion
  AVANZAR_MOT_IZQ(velocidades[Cambio],DireccionActual); 
  AVANZAR_MOT_DER(velocidades[Cambio],DireccionActual);
}

void ControlManual()
{
  if(CONTROL_DE_CHOQUES()) 
  {
    char comando = Serial.read();
    if(comando == 'P' || comando == ' ') return;
    EJECUTAR_COMANDO(comando);
    
  }else AVANZAR();
}

void EJECUTAR_COMANDO(char comando)
{
  switch(comando)
  {
    case CMD_IZQUIERDA:
      if(AnguloServo < ANGULO_MINIMO) return;
      AnguloServo -= 10;
      DOBLAR(AnguloServo);
      break;
    case CMD_ABAJO:
      DireccionActual = 0;
      AVANZAR();
      break;
    case CMD_DERECHA:
      if(AnguloServo > ANGULO_MAXIMO) return;
      AnguloServo += 10;
      DOBLAR(AnguloServo);
      break;
    case CMD_ARRIBA:
      DireccionActual = 1;
      AVANZAR();
      break;
    case CMD_FRENAR:
      Cambio = 0;
      AVANZAR();
      break;
    case CMD_SUBE_CAMBIO:
      if(Cambio < 3) Cambio++; 
      AVANZAR();
      break;
    case CMD_BAJA_CAMBIO:
      if(Cambio > 0) Cambio--;
      AVANZAR();
      break;
    case CMD_LUCES:
      LucesPrendidas = !LucesPrendidas;
      PRENDER_LUCES(LucesPrendidas);
      break;
    default:
      break;
  }
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

  if(distancia <= 20 && Cambio > 0) {
    Cambio = 0;
    return false;
  }
   
  if(distancia > 20 && distancia <= 30 && Cambio > 1)
  {
    Cambio = 1;
    AVANZAR();
    return true;
  }

  if(distancia > 30 && distancia <= 40 && Cambio > 2)
  {
    Cambio = 2;
    AVANZAR();
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
