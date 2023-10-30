/* ========================================
La práctica consiste en diseñar un sistema con acceso restringido por una 
contraseña que sirva para configurar una serie de tareas. La contraseña inicial 
para el administrador será 3207 y deberá mostrarse en los cuatro displays de 7 
segmentos de la tarjeta de desarrollo del PSoC5.

El administrador tendrá la posibilidad de escoger cada una de las tareas 
haciendo uso de un switch SW1:
- Tarea 0: Desplazamiento automático: Esta tarea consta de una secuencia de encendido y apagado de 8 LED. Mientras esto ocurre, en los displays de los 7 segmentos ocurrirá un 
desplazamiento de izquierda a derecha del carácter “A” desde el primer display hasta el cuarto
- Tarea 1: Desvanecimiento: En esta tarea, se va a mostrar los números “4321” en el display de los 7 
segmentos, pero este irá perdiendo intensidad hasta apagarse

*/
#include "project.h"
CY_ISR_PROTO(contar);
CY_ISR_PROTO(entrar);
CY_ISR_PROTO(time);


uint8 Cont[4]={0,0,0,0};
void tilt(uint8 all);
uint8 ClaveIngresada[4];
uint8 CompararClave(const uint8 *Ingresada,const uint8 *Original); 
uint8 ClaveOriginal[4]={3,2,0,7};
uint8 Posicion=0;
uint8 bandClave=0;
uint8 bTime=0,cTime=0, bEntrar=0, bLed=0, sTime=0, bSec=0,bTilt=0,csTime=0;

uint16 cont_250ms=0; // tarea 1
int vector_Leds[8]={0,129,195,231,255,126,60,24}; // vector leds encendidos
int vector_Display[6]={0,1,2,3,2,1}; // vector displays 7seg encendidos
int vector_intensidad[19]={255,224,193,142,120,95,70,45,30,0,30,45,70,95,120,142,193,224,255}; // vector de intensidades (brightness)
uint8 posicion=0; // posicion vector leds (tarea 0)
uint8 posicion2=0; // posicion vector de display (tarea 0)
uint8 posicion3=0;// posicion vector de display de intensidad (tarea 1)
uint8 cTerminado=0;
uint8 bandTms=0;
uint8 intensidad = 0;

int main(void)
{
    CyGlobalIntEnable;
    // CONTRASEÑA
    clock_isr_StartEx(time);
    boton_1_isr_StartEx(contar);
    boton_2_isr_StartEx(entrar);
    Counter_Start();
    Led_Start();
    Led_WriteString7Seg("0000",0);
    

    for(;;)
    {
        if(bTime){
        bTime=0;
        cTime++;
            if (cTime==200){
            cTime=0;
            tilt(bTilt);
            if (bSec){  
                csTime++;
                    if (csTime==3){
                    csTime=0;
                    sTime++;
                    }
                }
            }
        }
        
        if(Posicion==4){           
             bandClave = CompararClave(ClaveIngresada,ClaveOriginal); // Evalua la clave ingresada
             if (bandClave==1){ //Si la clave es ingresada correctamente
                    Led_SetBrightness(0,0);
                    Led_SetBrightness(0,1);
                    Led_SetBrightness(0,2);
                    Led_Write7SegDigitHex(0xC,3);
                    bTilt=2;
                    bSec=1;
                    if (sTime==7){
                        bSec=0;
                        sTime=0;
                        Posicion=0;                    
                        for(uint8 i=0;i<4;i++){Cont[i]=0;}
                        bTilt=0;
                        Led_SetBrightness(255,0);
                        Led_SetBrightness(255,1);
                        Led_SetBrightness(255,2);
                        Led_ClearDisplayAll();
                        //Led_WriteString7Seg("0000",0);
                        cTerminado=1;
                    }
                    
             }
             else{
                    Led_WriteString7Seg("   P",0);
                    bTilt=2;
                    bSec=1;
                    if (sTime==3){
                    sTime=0;
                    bSec=0;
                    Posicion=0;
                    for(uint8 i=0;i<4;i++){Cont[i]=0;}
                    bTilt=0;
                    Led_WriteString7Seg("0000",0);
                    }
             }
            
        }
        if (cTerminado){
            if (SW1_Read() == 0) { // TAREA 0
                Leds_Write(vector_Leds[posicion]);
                Led_ClearDisplayAll();
                Led_PutChar7Seg('A', vector_Display[posicion2]);
                posicion++;
                posicion2++;
                CyDelay(500);
                if (posicion==8){
                    posicion=0;
                }
                if (posicion2==6){
                    posicion2=0;
                }                
            } else{ // TAREA 1
                if (bandTms) {
                    bandTms=0;
                    cont_250ms++;
                    posicion3 = cont_250ms / 120;                     
                    if (posicion3<19) {
                        intensidad = vector_intensidad[posicion3];
                        Led_SetBrightness(intensidad,0);
                        Led_SetBrightness(intensidad,1);
                        Led_SetBrightness(intensidad,2);
                        Led_SetBrightness(intensidad,3);
                        Led_WriteString7Seg("4321",0);
                    } else {
                        cont_250ms = 0;
                        posicion3 = 0;
                    }                    
                }              
            }
        }
        
        
    }
}

CY_ISR(contar){
  if(Posicion!=4){
      Cont[Posicion]++;
      if(Cont[Posicion]==10)Cont[Posicion]=0;
      Led_Write7SegDigitDec(Cont[Posicion],Posicion); 
      bTilt=1;
    Led_SetBrightness(255,0);
    Led_SetBrightness(255,1);
    Led_SetBrightness(255,2);
    Led_SetBrightness(255,3);
}
}


CY_ISR(entrar){
  if(Posicion!=4){
    ClaveIngresada[Posicion]=Cont[Posicion];  
    Posicion++;
    Led_SetBrightness(255,0);
    Led_SetBrightness(255,1);
    Led_SetBrightness(255,2);
    Led_SetBrightness(255,3);
}

}
CY_ISR(time){
    bTime=1;
    bandTms=1;  
}


void tilt(uint8 all){
switch(all){
case 0:
switch (bLed){
            case 1:
                bLed=0;
                Led_SetBrightness(255,0);
                Led_SetBrightness(255,1);
                Led_SetBrightness(255,2);
                Led_SetBrightness(255,3);
                break;
            case 0:
                bLed=1;
                Led_SetBrightness(0,0);
                Led_SetBrightness(0,1);
                Led_SetBrightness(0,2);
                Led_SetBrightness(0,3);     
                break;
            }
break;
case 1:
switch (bLed){
            case 1:
                bLed=0;
                Led_SetBrightness(255,Posicion);
                break;
            case 0:
                bLed=1;
                Led_SetBrightness(0,Posicion);
                break;
            }
break;
case 2:
switch (bLed){
            case 1:
                bLed=0;
                Led_SetBrightness(255,3);
                break;
            case 0:
                bLed=1;
                Led_SetBrightness(0,3);
                break;
            }
break;
}
}


uint8 CompararClave (const uint8 *Ingresada, const uint8 *Original){
    uint8 Aciertos=0;
    uint8 m=0;
    for(m=0;m<4;m++){
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

/* [] END OF FILE */
