/* ========================================
FALTAN LOS 2 PUNTOS RECORDAR
 * ========================================
*/
#include "project.h"
// Inicio de variables
int8 ledV[3]={0,0,0};
uint8 bTime=0,bTime2=0,bTime3=0,cTime=0,bSec=0,csTime=0,sTime=0,ssTemp=0,bTilt=0,bLed=0,Posicion=0,bCont=0,aCont=0,bConf=0,bTimeOut=0,b1Tap=0,cTap=0,aTime=0;
uint8 bandVolver=0,modulo=0;
uint16 cTemp=0, cTime_deconf=0;
//Declaración de interrupciones
CY_ISR_PROTO(time);
CY_ISR_PROTO(contar);
//Declaración de funciones
void tilt(void);
void all_on(void);
void funContar(void);

int main(void)
{
    CyGlobalIntEnable; 
    temp_isr_StartEx(time);
    botonIsr_1_StartEx(contar);
    Counter_Start();
    Led_Start();
    Led_WriteString7Seg(" 000",0);
    for(;;)
    {
        if (bTime){ 
            bTime=0;
            cTime++;
            
            if (cTime==200){ //Cuando el valor sea 200 a pasado 0.3 segundos ya que 1 segundo son 600 conteos
                cTime=0;
                csTime++;
                if(!bandVolver)tilt(); //Se reinicia la bandera y se hace titilar la pantalla de los 7 segmentos
                // Contador extra para 1 segundo el cual funciona de manera similar solo que en este caso cuenta hasta 3, cada vez que se alcancen 200 conteos               
                if (csTime==3){
                    csTime=0;
                    sTime++;                   
                    }
            }
            if (b1Tap)cTap++; //Se comienza el conteo si se presiona el botón una vez
            if (cTap>210){ //Se espera hasta 210 el equivalente a 350 ms y si no se a presionado por segunda vez las banderas vuelven a 0 lo mismo que el contador
            cTap=0;
            b1Tap=0;
            sTime=0; 
            ledV[Posicion]++; //Se aumenta el valor en la posición
            switch(Posicion){ // Se hace un switch con las condiciones pedidas para minutos, decenas de segundos y unidades de segundos
                case 0:
                    if(ledV[Posicion]==3)ledV[Posicion]=0;
                    break;
                case 1:
                    if(ledV[Posicion]==6)ledV[Posicion]=0;
                    break;
                case 2:
                    if(ledV[Posicion]==10)ledV[Posicion]=0;
                    break;
            }
            Led_Write7SegDigitDec(ledV[Posicion],Posicion+1);
            }
        }
        if (!SW1_Read()){
           bTimeOut=0; 
            Led_1_Write(0);
            if (bandVolver||bTilt==3){
                sTime=0;
                bConf=0;
                bandVolver=0;
                Posicion=0;
                for(uint8 i=1;i<4;i++){ledV[i]=0;}
                bTilt=1;
                Led_WriteString7Seg(" 000",0);
                all_on();
            }
            if (aTime){funContar();aTime=0;} //Si se detecta la interrupción se corre la función de contar
             
            
            if (bTilt){
                if (sTime==4){ //cambia de posición cada 4 segundos
                    all_on();
                    sTime=0;
                    Posicion++;             
                }
                if (Posicion==3){ //reinicia las posiciones, se deja afuera del conteo de segundos para cerciorarnos que el cambio de 2Tap no cambie los valores a uno superior
                bConf=1; //Bandera que indica que la configuración fue completada
                Posicion=0;
                }
                           
            } 
                buzzer_Write(0);
                
        }
        else {
            if (!bConf && bTime3){ //cuando no esta configurado pero se pasa a la tarea visualización
                bTime3=0;
                bandVolver=1;
                Led_WriteString7Seg(" 000",0);
                Led_1_Write(0); // apagado cuando no esta config
                bTilt=0;
                cTime_deconf++;  
                Posicion=0;                
                if (cTime_deconf==150){ // 2hz, esta encendido                     
                    cTime_deconf=0;                           
                    tilt();
                }

           }  
            if(bConf){                
                all_on();
                for(uint8 i=0;i<3;i++){                    
                    Led_Write7SegDigitDec(ledV[i],i+1);
                } //Se copia cada valor del vector en el display
                bTilt=3; //Se manda la opcion default para que los display no titilen
                Posicion=0; //Se resetea la posición
                if (bTime2){
                    bTime2=0;
                    if(!SW2_Read()){                   
                        cTemp++;
                    }else{
                        cTemp+=4;
                    } 
                    if (cTemp>=600){ //600 int son 1 seg
                        cTemp=0;
                        ssTemp++;
                    }                 
                }
                if(ssTemp&&!bTimeOut){ 
                    ssTemp=0;
                    ledV[2]--;
                    if(ledV[2]<0){ //Se genera el temporizador de la forma que reste a las unidades de segundos hasta llegar a 0, en ese caso pasara a ser 9 
                        ledV[2]=9;
                        ledV[1]--;
                        if(ledV[1]<0){ //Cuando se alcanza se le resta a las decenas de segundos y estas pasan a ser 5 y se resta a minutos
                        ledV[1]=5;
                        ledV[0]--; 
                        }
                    } 
                }
                if (SW3_Read()&&bConf){
                    modulo = ledV[2]; //calcula el módulo de las unidades de segundo, si es cero se prende el buzzer
                    if(!modulo){
                        buzzer_Write(1);
                    }
                    else {
                        buzzer_Write(0);
                    }
                } else{
                    buzzer_Write(0);
                }
                if(ledV[2]==0&&ledV[1]==0&&ledV[0]==0){
                    Led_1_Write(1);
                    buzzer_Write(0);
                    bTimeOut=1; // bandera que avisa la finalización del conteo
                    bandVolver=1;
                }
            } 
        }
        
        
    }
}
// inicio de funciones de interrupciones
CY_ISR(time){
    bTime=1; // Titileo 1.5Hz y contar segundos a tiempo real
    bTime2=1; // Velocidad
    bTime3=1; // Para titileo 2Hz
}

CY_ISR(contar){
    aTime=1; //Se avisa cada vez que se presiona para estar atento a la segunda presión en menos de 350 ms
}
//Funciones
void funContar(void){
    sTime=0;
    b1Tap=1;
    if (b1Tap&&cTap){ //En este caso si el contador de pulsación sigue activo significa que se presiono el botón en menos del tiempo establecido así que aumenta la posición
      Posicion++;
      b1Tap=0;
      cTap=0;
    }
      bTilt=1;
      all_on();


}


void all_on(void){ //prende todos los displays en la mayor intensidad
for(uint8 i=1;i<4;i++){Led_SetBrightness(255,i);}
}   

void tilt(){ 
    switch(bTilt){ //Función de titilar la cual funciona con varios switch cuya función es manejar los 3 estados del programa
        case 0: //Primer caso todos los display del 7 segmentos titilan
            switch (bLed){ //Se logra mediante un cambio constante de la variable bLed la cual alterna entre 1 y 0 a medida de que se llame la función en el intervalo de 0.3 seg
                case 1:
                    bLed=0; 
                    all_on();
                    break;
                case 0:
                    bLed=1;
                    for(uint8 i=1;i<4;i++){Led_SetBrightness(0,i);}
                    break;
                }
        break;
        case 1:
            switch (bLed){ //Caso 1 donde depende de la posición para solo titilar, nótese que se toma la misma variable de la instancia de la contraseña para que solo este titile mientras los otros mantien encendidos
                case 1:
                    bLed=0;
                    Led_SetBrightness(255,Posicion+1);
                    break;
                case 0:
                    bLed=1;
                    Led_SetBrightness(0,Posicion+1);
                    break;
                }
        break;
        case 2:
            switch (bLed){ //Se logra mediante un cambio constante de la variable bLed la cual alterna entre 1 y 0 a medida de que se llame la función en el intervalo de 0.3 seg
                case 1:
                    bLed=0; 
                    all_on();
                    break;
                case 0:
                    bLed=1;
                    for(uint8 i=1;i<4;i++){Led_SetBrightness(0,i);}
                    break;
                }
        break;          
        default:
            all_on();
    }
}


/* [] END OF FILE */
