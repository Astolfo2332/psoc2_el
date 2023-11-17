/* ========================================
 * PRUEBA MODULO
 *
 * ========================================
*/
#include "./Generated_Source/PSoC5/project.h"
#include <stdio.h>
#include <stdlib.h>
//Interrupciones
CY_ISR_PROTO(teclado_interrupt);
CY_ISR_PROTO(count);
//Fucniones
void contra(void);
void resetContra(void);
void menu(void);
void menu_pot(void);
void menu_teclado(void);
void time(void);
void clear_angle(void);
void salir(void);
//Banderas y variables
uint8 ClaveIngresada[4];
uint8 CompararClave(const uint8 *Ingresada,const uint8 *Original); 
uint8 ClaveOriginal[4]={'1','2','2','3'};
uint8 contClave=0;
float comp,Duty,vPot,vPWM,vAng,vDur;
struct band {
    uint32 bConf:2,pBand:1,bPrint:1,bCont:1,cCont:10,bHcont:1,bInicio:1,bClaveIng:1,bClaveCorr:1,bSeg:1,bcontSeg:2,bContar:1,bC:1,bAng:3,bSalir:1,aSalir:1,bCC:1;
};
struct band bands ={0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0}; // Estructura para las banderas y contadores
uint8 bMenu=0, bContra=0, bAngulo=0 ; //Se deja solo porque este al poder ser una letra ocupara siempre 8 bits
uint16 cSalir=0;
uint16 angulo=0;
char angu[20], duty[20],adc[20],pot[20],ang[20];

int main(void)
{
    Teclado_Start();
    LCD_Start();
    PWM_Start(); 
    ADC_Start();
    Counter_Start();
    isr_KBI_StartEx(teclado_interrupt);
    isr_cont_StartEx(count);

    CyGlobalIntEnable; 
    
    for(;;)
    {
        time();
        switch (bands.bConf) {
            case 0: // Caso contraseña
                contra();
                break;
            case 1: // Caso menu
                menu(); 
                break;
            case 2: // Caso salir del sistema
                salir();
                break;
        }
    }
}

//Interrupciones
CY_ISR(teclado_interrupt){
    bands.pBand = 1;
}
CY_ISR(count){
    bands.bCont=1;
}

//Funciones

void contra(void){
    time();
    if(bands.bInicio){ // PAGINA DE INICIO
        LCD_Position(0,5);
        LCD_PrintString("BIENVENIDO");
        LCD_Position(2,8);
        LCD_PrintString("****");
    }
    if (bands.pBand) { // entra cuando se activa interrupcion de letra
        bands.bInicio=0;
        bands.pBand=0; 
        bContra=Teclado_AsignarTecla();
        
        if(bContra=='#'){ // se borran los numeros ingresados de conytraseña
            resetContra();
        }
        else if(bContra=='*' && bands.bClaveIng){ // verifica que la contraseña ya este ingresada para y se presione * para entrar al sistema
            bands.bClaveCorr = CompararClave(ClaveIngresada,ClaveOriginal); // Evalua la clave ingresada para ver si coincide con la original
            LCD_ClearDisplay();
            if (bands.bClaveCorr){  // Si la clave es correcta entra al sistema con bandera bConf
                  bands.bConf=1;  
            }else{ // Entra aqui si la contraseña es incorrecta 
                LCD_Position(1,2);
                LCD_PrintString("Clave incorrecta");
                bands.bContar=1;  
                bands.bcontSeg=0;
                bands.cCont=0;
            }
        }
        else if(bContra!='*'){     // entra cuando se presiona caracter y lo pone en el LCD reemplazando los *
            ClaveIngresada[contClave]=bContra;     
            LCD_Position(2,8+contClave);
            contClave++;   // contador para cambiar posicion donde se pone el caracter
            LCD_PutChar(bContra);
            if (contClave==4){ // resetea cuando la posicion es 4
                bands.bClaveIng=1; // bandera que dice que ya se ingresaron 4 numeros para permitir enviar contraseña arriba con *
                contClave=0;
            }
        }
        
    }
    if(bands.bSeg){ // LIMPIA luego de 2 seg de contraseña incorrecta
        bands.bSeg=0;
        LCD_ClearDisplay();
        resetContra();
    }
}

uint8 CompararClave (const uint8 *Ingresada, const uint8 *Original){
    uint8 Aciertos=0; //cuenta el número de aciertos que tiene la respona
    uint8 m=0;
    for(m=0;m<4;m++){ //m va de 0 a 4 para poder ir comparando las posiciones de los vectores ClaveOriginal y ClaveIngresada con sus respectivos apuntadores
        if(*Ingresada==*Original){ //Si lo apuntado por Ingresada es igual a lo apuntado por Original
            Aciertos++; //Cuente el número de aciertos
        }
        else{
            return 0;
            break;
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

void resetContra(void){ // como la contraseña se resetea tanto cuando se presiona # y cuando se ingresa la clave incorrecta se hace como funcion
    for(uint8 i=0;i<4;i++) {
        ClaveIngresada[i]=0; // resetea la clave ingresada y las banderas
    };
    bands.bInicio=1; 
    contClave=0;
    bands.bClaveIng=0;
    bands.bContar=0;
}

void menu(void){ // funcion del menu
    time();
    if (!bMenu) { // menu inicial
        if (!bands.bPrint) {
            bands.bPrint=1;
            LCD_ClearDisplay();
            LCD_Position(0,0);
            LCD_PrintString(" CONTROL SERVOMOTOR ");
            LCD_Position(2,0);
            LCD_PrintString("A-Potenciometro");
            LCD_Position(3,0);
            LCD_PrintString("B-Teclado");
        }
    if (bands.pBand) { // entra cuando se presiona una tecla y se guarda caracter en bMenu
        bands.pBand=0; 
        bMenu=Teclado_AsignarTecla();
    }
    }
    switch (bMenu) { // segun letra que se presionó se ingresa a menu para modificar servo con potenciómetro o con teclado
        case 'A':
           menu_pot();
           break;
        case 'B':
           menu_teclado();        
           break;
        default:
           bMenu=0;
           break;
   }
}

// POTENCIÓMETRO
void menu_pot(void){
    ADC_StartConvert(); // Se inicia conversion adc
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT); // espera a que termine
    vPot=0;
    vPot=ADC_GetResult16(); // se guarda resultado
    ADC_StopConvert();
    vPWM=499+(180*(vPot/4095))*11; // formula para convertir el valor ADC en compare del PWM para mover el servomotor
    PWM_WriteCompare(vPWM); // configura el PWM con el compare obtenido
    if (bands.pBand) { // ingresa cuando se presiona una tecla
            bands.pBand=0;
            bAngulo=Teclado_AsignarTecla();
            if (bAngulo=='#') { // Si es # se devuelve en el menu
                bMenu=0;
                bands.bPrint=0;
                bAngulo=0;
            }
        }
    if (bands.bHcont) { // Imprime informacion y refresca cada medio segundo
        bands.bHcont=0;
        LCD_ClearDisplay();
        vAng=180*(vPot/4095); // obtiene valor del angulo
        vDur=(vPWM/PWM_ReadPeriod())*100; // obtiene valor de la dureza o duty
        LCD_Position(0,0);
        LCD_PrintString("  A-Potenciometro  ");
        LCD_Position(1,11);
        LCD_PutChar(LCD_CUSTOM_0);
        LCD_Position(1,0);
        sprintf(ang,"Angulo: %3d",(int)vAng);  // muestra valor de angulo en LCD
        LCD_PrintString(ang);
        LCD_Position(2,0);
        sprintf(pot,"Dureza: %.2f%%",vDur); //Muestra duty. Para que esto funciona toco activar que se compilara las funciones de float en el build del poryecto en las sección de linker y aumentar el tamaño del heapsize a 200
        LCD_PrintString(pot);
        LCD_Position(3,0);
        sprintf(adc,"ADC: %d",(int)vPot); // muestra valor del ADC
        LCD_PrintString(adc);
        
    }
    }

// TECLADO
void menu_teclado(void){
    if(bands.bAng!=2){ 
        bands.bAng=1;
    }
    if (bands.bAng==1){
        LCD_ClearDisplay(); // limpia informacion de menu anterior
        bands.bAng=2;
    }
    LCD_Position(0,5);
    LCD_PrintString("B-Teclado");
    LCD_Position(1,0);
    LCD_PrintString("Angulo: ");
    if (bands.pBand) {  // Entra cuando se presiona tecla
        bands.pBand=0; 
        bAngulo=Teclado_AsignarTecla();
        if(bAngulo == 'C'){   // cuando se presiona C se confirma valor de angulo ingresado
            if (angulo==0 || angulo>180){ // entra cuando angulo no está en el rango correcto
                bands.bC=1;
                bands.bcontSeg=0;
                bands.bContar=1;
                bands.cCont=0;
                LCD_Position(1,8);
                LCD_PrintString("*Error*"); // mensaje de error
            } else { // cuando angulo esta dentro del rango:
                bands.bCC=1;
            comp=angulo*(11.1)+499; // formula para convertir angulo en compare para PWM
            PWM_WriteCompare(comp); // define compare del PWM por software
            Duty=(comp/PWM_ReadPeriod())*100; // calcula el duty segun angulo ingresado                       
            sprintf(duty,"Dureza: %.2f%%",Duty); 
            LCD_Position(2,0);
            LCD_PrintString(duty);}
            
        }
        else if (bAngulo != 'A' && bAngulo != 'B' && bAngulo != 'D'  && bAngulo != '*' && !bands.bCC){ // Aca solo puede ingresar numeros
            LCD_Position(1,8);
            angulo = angulo * 10 + (bAngulo - '0'); // acumula valor del angulo dado que se ingresa digito por digito
            sprintf(angu, "%d", (int)angulo);
            LCD_PrintString(angu);

        }   
        if (bAngulo=='#') {  // se vuelve a menu anterior con #
            bMenu=0;
            bands.bPrint=0;
            clear_angle();
        }
    }
    if (bands.bSeg && bands.bC){ // resetea para volver a menu inicial cuando se ingresa angulo fuera de rango o no se ingresa ningun valor
        clear_angle();
    }
      
}
void clear_angle(void){ // resetea lo relacionado al angulo ingresado cuando se pone fuera de rango
    LCD_ClearDisplay();
    bMenu=0,bAngulo=0,angulo=0,comp=0,Duty=0;
    struct band resetBands ={1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};
    bands = resetBands;
}
void time(void){ // función para manejar todo lo relacionado a tiempos y contadores
    if (bands.bCont) { // entra cada ms
        bands.bCont=0;
        bands.cCont++;
        if (bands.cCont==500) { // cuenta medio segundo (500 ms)
            bands.bHcont=1;
        }
        if (bands.cCont==1000){ // cuenta un segundo (1000 ms)
            bands.bCont=0;
            if(bands.bContar){
                bands.bcontSeg++;   
            }
        }
        if(bands.bcontSeg==2){ // cuenta 2 segundos
            bands.bcontSeg=0;
            bands.bSeg=1;
        }
        if (bands.bConf==1){
            switch (dSbuton_Read()) { // el caso se determina según el valor del boton de salir, si está presionado o no
                case 1:
                    bands.bSalir=0;
                    cSalir++;
                    if (cSalir==3000) { // conteo de 3 segundos para verificar que se presiono el boton de salida por más de este tiempo
                        bands.aSalir=1;
                        cSalir=0;
                    }
                    break;
                case 0:
                    if (bands.aSalir) { 
                        bands.aSalir=0;
                        bands.bConf=2; // entra a caso de salir del sistema en el for infinito
                    }
                    cSalir=0;
                    break;
            }
        }
    }
}
void salir(void){ // Función para salir del sistema
    bands.bContar=1;
    if (bands.bHcont) { // refrescamiento cada medio segundo
        bands.bHcont=0;
        LCD_ClearDisplay();
        LCD_Position(1,0);
        LCD_PrintString("Saliendo del sistema");
    }
    if (bands.bSeg) { // se resetean todas las variables luego de 2 segundos de mostrar el aviso de saliendo del sistema
    bMenu=0, bContra=0, bAngulo=0, angulo=0, contClave=0,comp=0,Duty=0,vPot=0,vPWM=0,vAng=0,vDur=0;
    struct band resetBands ={0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};
    bands = resetBands;    
    resetContra();
    LCD_ClearDisplay();
    }
}
/* [] END OF FILE */
