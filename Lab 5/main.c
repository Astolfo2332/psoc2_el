/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "./Generated_Source/PSoC5/project.h"
#include <stdio.h>
#include <stdlib.h>
//Funciones
void tilt();
void control(uint8 bvalue );
void funcontar(void);
void control7(void);
void datos(float peso,float temp); //Función para imprimir los datos de una vez en el lcd

// Interrupciones
CY_ISR_PROTO(time);
CY_ISR_PROTO(contar);
CY_ISR_PROTO(entrar);

//Variables
uint8 Cont[4]={0,0,0,0};
uint8 ClaveIngresada[4];
uint8 CompararClave(const uint8 *Ingresada,const uint8 *Original); 
uint8 ClaveOriginal[4]={2,4,9,1};
struct band {
	uint16 bTime:1, bEntrar:1, bLed:1, aPos:1, bTap:1,bTilt:2,bSec:1,fConf:1,bAlarma:1,Posicion:2,bandClave:1;
}; 
struct band es ={0,0,0,0,0,0,0,0,0,0,0};
struct cont {
	uint32 cTime:8,csTime:8,cClave:3,bCont:1,cSec:10;
};
struct cont con ={0,0,0,0,0};
int main(void)
{
    CyGlobalIntEnable; 
    temp_isr_StartEx(time);
    Counter_Start();
    LCD_Start();
    LED_Start();   
    LED_WriteString7Seg("0000",0);
    botonIsr_1_StartEx(contar);
    botonIsr_2_StartEx(entrar);
    for(;;)
    {
        //Contador
        if(es.bTime){
        es.bTime=0;
        con.cTime++;
        if (con.cTime==200){
        con.cTime=0;
        if(!es.bEntrar&&!es.fConf){tilt();LCD_ClearDisplay();} 
        }
        if(es.bTap)con.csTime++;
        if(con.csTime>228){
            con.csTime=0;
            es.bTap = 0;
            Cont[es.Posicion]++;
            if (Cont[es.Posicion] == 10)
                Cont[es.Posicion] = 0;
            LED_Write7SegDigitDec(Cont[es.Posicion], es.Posicion);
            control(255);
        }
        }
        //Funcion para contar
        if (con.bCont&& !es.bEntrar){
            funcontar(); con.bCont = 0;
        }
        //Función para validar la clave
        if (es.bEntrar&& !es.fConf){
            for(uint8 i=0;i<4;i++) {ClaveIngresada[i]=Cont[i];}
            es.bandClave = CompararClave(ClaveIngresada,ClaveOriginal); // Evalua la clave ingresada
            control7(); 
        }
        if (es.fConf){
            control(0);
            //Aquí entra después de confirmar y hacer lo de los patrones así que ;-) 
        }
    }
}
//Interrupciones
CY_ISR(time){
    es.bTime=1;
    es.bSec=1;
}
CY_ISR(entrar){
    es.bEntrar=1;
}
CY_ISR(contar){
    con.bCont=1;
}
// Funciones
void funcontar(void){
    es.bTilt = 1;
    es.bTap=1;
    control(255);
    if (es.bTap&&con.csTime){
        es.Posicion++;
        es.bTap=0;
        con.csTime=0;
		if (es.Posicion==3)es.aPos=1;	
        if(es.Posicion==4)es.Posicion=0;
    }
}
void control(uint8 bvalue){ //prende todos los displays en la mayor intensidad
for(uint8 i=0;i<4;i++){LED_SetBrightness(bvalue,i);}
}
void tilt(){ 
switch(es.bTilt){ //Funcion de titilar la cual funciona con varios switch cuya función es manejar los 3 estados del programa
case 0: //Primer caso todos los display del 7 segmentos titilan
switch (es.bLed){ //Se logra mediante un cambio constante de la variable bLed la cual alterna entre 1 y 0 a medida de que se llame la función en el intervalo de 0.3 seg
            case 1:
                es.bLed=0; 
				control(255);
                break;
            case 0:
                es.bLed=1;
				control(0);
               break;
            }
break;
case 1:
switch (es.bLed){ //Caso 1 donde depende de la posición para solo titilar, notese que se toma la misma variable de la instancia de la contraseña para que solo este titile mientras los otros mantien encendidos
            case 1:
                es.bLed=0;
                LED_SetBrightness(255,es.Posicion);
                break;
            case 0:
                es.bLed=1;
                LED_SetBrightness(0,es.Posicion);
                break;
            } break;
}
}
uint8 CompararClave (const uint8 *Ingresada, const uint8 *Original){
    uint8 Aciertos=0; //cuenta el número de aciertos que tiene la respona
    uint8 m=0;
    for(m=0;m<4;m++){ //m va de 0 a 4 para poder ir comparando las posiciones de los vectores ClaveOriginal y ClaveIngresada con sus respectivos apuntadores
        if(*Ingresada==*Original){ //Si lo apuntado por Ingresada es igual a lo apuntado por Original
            Aciertos++; //Cuente el número de aciertos
        }
        Ingresada++; //Apunte a la siguiente posición
        Original++;// Apunte a la siguiente posición
    }
    if (Aciertos==4){ //4 aciertos significa que la clave es correcta
        return 1;
    }
    else{
        return 0;
    }
}

void control7(void){
        if(es.bSec){
            if (es.bLed==1){
                control(255);
                es.bLed=0;
                }
            if(!es.bandClave&&!es.aPos)control(0);
            es.bSec=0;
            con.cSec++;
            if(con.cSec==600){
                con.cSec=0;
                if(!es.bandClave){
					if (es.aPos) {
					for(uint8 i=0;i<4;i++){
						 LED_Write7SegDigitDec(Cont[i],i+con.cClave);
					}
					if(con.cClave>0)LED_SetBrightness(0,con.cClave-1);
					}
					LCD_ClearDisplay();
					LCD_Position(1,7);
					LCD_PrintString("Error");
					LCD_Position(2,2);
					LCD_PrintString("Intente de nuevo");
				   }
                else {
				LCD_ClearDisplay();
                LCD_Position(1,4);
                LCD_PrintString("Bienvenido al");
                LCD_Position(2,1);
                LCD_PrintString("Sistema de medicion");
				}
                con.cClave++;
                if(con.cClave==5){
                if (es.bandClave) es.fConf=1;
                    con.cClave=0;
                    es.bandClave=0;
                    es.bEntrar=0;
					es.Posicion=0;
                    control(255);
                    for(uint8 i=0;i<4;i++){
                        LED_Write7SegDigitDec(0,i);
                        Cont[i]=0;
                        }
                }
                }
            }
}
void datos(float peso ,float temp){
    char tem[50];
    char pes[50];
    LCD_ClearDisplay();
    LCD_Position(0,0);
    switch (es.bAlarma)
    {
    case 1:
    LCD_PrintString("Sistema de medicion");
        break;
    default:
    LCD_PrintString("Advertencia!");
        break;
    }
    LCD_Position(0,2);
    sprintf(tem,"Temp= %.1f   F",temp);
    LCD_PrintString(tem);
    LCD_Position(12,2);
    LCD_PutChar(LCD_CUSTOM_0); // en este caso así se imprime el simbolo personalizado de grados
    LCD_Position(0,1);
    sprintf(pes,"Peso= %.2f",peso);
    LCD_PrintString(pes);
} 
/* [] END OF FILE */
