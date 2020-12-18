#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "dht11.h"
//#include "include.h"
#include "SysTick.h"
#include "inc/tm4c123gh6pm.h"
#define RELOJ_MICRO 16000000
#define LF  0xA


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
/*Funciones*/
void ESP8266comando(char *pt);
void ESP8266Retorno(char retorno[]);
void printRetorno(char *pt);
void cleanUART1FIFO();
void resetESP();
void connWIFI();
char connServer();
void compruebaEstadoESP();
void sendTemperatura(char _socketID, int temp);
void sendHumedad(char _socketID, int hum);
static char *itoa_simple_helper(char *dest, int i);
char *itoa_simple(char *dest, int i);


//Enviamos un string al uart1
//Para enviar un comando necesariamente debemos finalizarlo con \n, En nuestro caso al querer mandar un comando http
//debemos finalizar el comando con \r\n y ademas agregar el \n final correspondiente al modulo ESP8266
//El puntero vuelve a la primera posicion del string


//Variables globales
char retornoESP8266[100];//String donde guardaremos la respuesta del socket
//int temp, hum, hum_low, temp_low, parity;
int ban = 1;
int32_t retornoConnWifi;
uint8_t retornoConnSocket[4];






uint32_t variable[40];
unsigned long Humidity, HumidityDecimalPart, Temperature, TemperatureDecimalPart, Validation;

/************************ PA3 connect to data pin of DHT11 ************************************/
/************************ Put 10k pull up resistor between data line and Vcc ******************/
/************************ Temperature accuracy between +-2C and +-1C **************************/
/************************ Temperature measurement range 0 - 50C *******************************/
/************************ Humidity accuracy between +-4% and +-5% *****************************/
/************************ Humidity measurement range 20 - 90% *********************************/

unsigned long ConversionBinaryToDecimal(int startBit){
    int i = 0;
    unsigned long convertedNum = 0;
    for(i = startBit; i <= (startBit + 7); i++){
        convertedNum = convertedNum + variable[i] * pow(2, i);
    }
    return convertedNum;
}

void Interrupt_Function_GPIO_PORTF(void){
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, ~GPIO_PIN_3); // low voltage on PA3
    delayMs(19); // 18.12mS delay
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3); // high voltage on PA3
    delayUs(30); // 500 = 30uS
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3); // input pins PF0 and PF4
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  //  pull up resistor , 2mA max
    while(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == 0){};
    while(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == GPIO_PIN_3){};
    int i;
    for(i = 0; i < 40; i++){
        while(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == 0){};
        SysCtlDelay(450); // 45 uS = 750
        if(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == GPIO_PIN_3){
            variable[i] = 1;
            while(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == 8){};
        }else{
            variable[i] = 0;
        }

    }
    //SysCtlDelay(50000000); // 1sec delay
    delayMs(2000);
    Humidity = ConversionBinaryToDecimal(0);
    HumidityDecimalPart = ConversionBinaryToDecimal(8);
    Temperature = ConversionBinaryToDecimal(16);
    TemperatureDecimalPart = ConversionBinaryToDecimal(24);
    Validation = ConversionBinaryToDecimal(32);

}




/*MAIN*/
int
main(void)
{
    // Configuracion de perifericos que serán utilizados, falta agregar los perifericos para el sensor de
    // temperatura
    // Set the clocking to run directly from the crystal.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Enable the peripherals be used
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);



    // Set GPIO A0 and A1 as UART0 pins.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Set GPIO B0 and B1 as UART1 pins.
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);



    // Configure the UART for 115,200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //Enable UartFIFO
    UARTFIFOEnable(UART1_BASE);
    UARTFIFOEnable(UART0_BASE);


    //Sensor DHT11
    //Version3
//    DHT_init();
//    delayMs(2000);
//    th.celsius_temp = 0;
//    th.humidity = 0;
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
//    delayMs(1000);

    //Version1
    //
    //    GPIO_PORTE_Init();//Init PortE
    //    DHT11_Init();//DHT11 Init
    //    delayMs(3000);


    //Version2
//        unsigned int tempo = 500;
//    //    //unsigned char aux;
//    //    unsigned long * ptr;
//    //    int temp, hum;
//       SysTick_Init_ms(tempo);

    //version4
    //initDHT11Hw ();

    //Version 5
//    /**************************** CLOCK and ENABLE PERIPHERALS *********************************/
////        SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_4); // clock set 50MHz
//        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // enable portF
////        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // enable portA
//    /*******************************************************************************************/

    /**************************** GPIO CONFIG **************************************************/
        GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
        GPIO_PORTF_CR_R = 0x01;
        GPIO_PORTA_LOCK_R = GPIO_LOCK_KEY;
        GPIO_PORTA_CR_R = 0x08;

        GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0); // input pins PF0
        GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  //  pull up resistor , 2mA max

        GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);  // output pin PA3
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3); // high voltage on PA3
    /*******************************************************************************************/

    /**************************** GPIO INTERRUPT CONFIG ****************************************/
        IntMasterEnable();
        IntEnable(INT_GPIOF);
        IntPrioritySet(INT_GPIOF, 4);
        GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
        GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
        IntRegister(INT_GPIOF, Interrupt_Function_GPIO_PORTF);
    /*******************************************************************************************/
       // Interrupt_Function_GPIO_PORTF();
    printRetorno("Perifericos habilidados para el uso\r\n");


    //Comprobamos el estado del modulo WIFI
    compruebaEstadoESP();
    int temp, hum;

    temp = TemperatureDecimalPart;
    hum = HumidityDecimalPart;
    temp = 41;
    hum = 34;
    //Bucle infinito
    while(1)
    {
        //Reiniciamos el modulo para garantizar su funcionamiento
        resetESP();
        delayMs(3000);
        temp++;
        hum++;

        /*Obtenemos los datos del Sensor DHT11*/

        printRetorno("\r\n\nObteniendo datos de temperatura y humedad\r\n");

/////////////////////////////////////////////////////VERSION2/////////////////////////////////////////////////////////////////////////
//
//
//        DHT11_In((unsigned long *) &sensor);
//        temp = sensor.temperatura;
//        hum = sensor.umidade;
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////VERSION1/////////////////////////////////////////////////////////////////////////
//
//        DHT11_Master_Request();//Set PE1 to dht11
//        //Verificamos Si el sensor se encuentra encendido
//
//        if (DHT11_Slave_Response()){
//            printRetorno("El sensor se encuentra encendido\r\n");
//            hum = DHT11_Rx_8_Data();
//            hum_low = DHT11_Rx_8_Data();
//            temp = DHT11_Rx_8_Data();
//            temp_low = DHT11_Rx_8_Data();
//            parity = DHT11_Rx_8_Data();
//            if (DHT11_Verify_Parity(parity)){
//                //code
//            }
//            if(hum >= 100){
//                hum = hum - 50;
//            }
//            if(temp >= 100){
//                temp = temp - 50;
//            }
//        }else{
//            printRetorno("El sensor se encuentra apagado\r\n");
//            hum = 30;
//            temp = 24;
//        }
//
//
//        int i = 0;
//        for (i = 0; i < 30; i++){
//            delayMs(100);
//        }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION3/////////////////////////////////////////////////////////////////////////

//        dht_readTH(&th);
//
//        temp = th.celsius_temp;
//        hum = th.humidity;
//
//
//




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION4/////////////////////////////////////////////////////////////////////////
//
//        getReading ();
//        temp = temperature;
//        hum = humidity;





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION5/////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////














/////////////////////////////////////////////////////Enviamos los datos de temperatura////////////////////////////////////////////////////
        //Realizamos la conexion con la red WIFI
        connWIFI();
        delayMs(1000);

        //Nos conectamos al servidor de thingspeack
        char socketId;
        socketId = connServer();
        delayMs(2000);
        printRetorno("\r\nSocket = ");
        UARTCharPut(UART0_BASE, socketId);
        printRetorno("\r\n");
        cleanUART1FIFO();



        //Con el valor del socket asignado procedemos a enviar el paquete al servidor
        if(socketId == '0' || socketId == '1' || socketId == '2' || socketId == '3'){
            printRetorno("Se establecio la conexion TCP/IP a traves del socket= ");
            UARTCharPut(UART0_BASE, socketId);
            if(socketId == '0'){
                sendTemperatura(socketId, temp);
            }
            if(socketId == '1'){
                sendTemperatura(socketId, temp);
            }
            if(socketId == '2'){
                sendTemperatura(socketId, temp);
            }
            if(socketId == '3'){
                sendTemperatura(socketId, temp);
            }
        }
        else{
            printRetorno("No se establecio la conexion");
            printRetorno("\r\n");
        }




        //Esperamos 10 segundos antes de mandar los datos de humedad
        delayMs(10000);
/////////////////////////////////////////////////////Enviamos los datos de humedad////////////////////////////////////////////////////
        //Realizamos la conexion con la red WIFI
        connWIFI();
        delayMs(1000);

        //Nos conectamos al servidor de thingspeack
        socketId = connServer();
        delayMs(2000);
        printRetorno("\r\nSocket = ");
        UARTCharPut(UART0_BASE, socketId);
        printRetorno("\r\n");
        cleanUART1FIFO();



        //Con el valor del socket asignado procedemos a enviar el paquete al servidor
        if(socketId == '0' || socketId == '1' || socketId == '2' || socketId == '3'){
            printRetorno("Se establecio la conexion TCP/IP a traves del socket= ");
            UARTCharPut(UART0_BASE, socketId);
            if(socketId == '0'){
                sendHumedad(socketId, hum);
            }
            if(socketId == '1'){
                sendHumedad(socketId, hum);
            }
            if(socketId == '2'){
                sendHumedad(socketId, hum);
            }
            if(socketId == '3'){
                sendHumedad(socketId, hum);
            }
        }
        else{
            printRetorno("No se establecio la conexion");
            printRetorno("\r\n");
        }



    }
}


/*Funcion para envio de comandos
 * Parametros: Ninguno
 * Retorno: Ninguno
 * */
void ESP8266comando(char *pt){
    char *aux;
    aux = pt;
    while(*pt){
        UARTCharPut(UART1_BASE, *pt);
        pt++;
    }
    pt = aux;
}


/*El final de cada respuesta del modulo Esp8266 es el \n por ende el salto de linea nos indica
 * donde termina la respuesta del modulo*/
void ESP8266Retorno(char retorno[]){
    //Recibimos el retorno del ESP8266 a traves del Uart1
    int bandera = 1;
    int32_t caracter;
    int count = 0;
    caracter = UARTCharGet(UART1_BASE);
    while(bandera){
        if(caracter == '\n'){
            bandera = 0;
        }else{
            retorno[count] = caracter;
            caracter = UARTCharGet(UART1_BASE);
            count++;
        }
    }
}


/*Funcion printRetorno
 * Imprimimos la cadena en la terminal
 * Parametros: Ninguno
 * Retorno: Ninguno
 * */
void printRetorno(char *pt){
    char *aux;
    aux = pt;
    while(*pt){
        UARTCharPut(UART0_BASE, *pt);
        pt++;
    }
    pt = aux;
}


/*Funcion ClearFIFO
 * Limpia el buffer del UART1
 * Parametros: Ninguno
 * Retorno: Ninguno
 * */
void cleanUART1FIFO(){
    int32_t clear;
    clear = UARTCharGetNonBlocking(UART1_BASE);
    while (clear != -1 ){
        clear = UARTCharGetNonBlocking(UART1_BASE);
    }
}


/*Funcion de reseteo ESP8266
 * Parametros: Ninguno
 * Retorno: Ninguno
 * */
void resetESP(){
    int32_t reset;
    //Limpiamos previamente el buffer Rx del UART1
    //ESP8266comando("\n");
    //cleanUART1FIFO();
    ESP8266comando("MRS\n");//Enviamos el comando de reseteo
    //delayMs(5000);//Esperamos 5 segundos
    int ban = 1;
    while(ban){
        //delayMs(1000);
        reset = UARTCharGetNonBlocking(UART1_BASE);
        //UARTCharPut(UART0_BASE, reset);
        if(reset != -1 && reset != 'R'){
            UARTCharPut(UART0_BASE, reset);
        }
        if(reset == 'R'){
            printRetorno("\r\n\nEl modulo se a reiniciado correctamente\r\n");
            //cleanUART1FIFO();
            UARTCharPut(UART0_BASE, reset);
            ban = 0;
        }

        if(reset == '\n'){
            UARTCharPut(UART0_BASE, '\r');
        }
    }
    cleanUART1FIFO();
}


/*Funcion para conectar a la red WIFI
 * Parametros: Ninguno
 * Retorno: Ninguno
 * */
void connWIFI(){
    cleanUART1FIFO();
    ESP8266comando("WFC,HUAWEIP20Marcelo,majubafe29797\n");
    delayMs(10000);
    retornoConnWifi = UARTCharGet(UART1_BASE);
    cleanUART1FIFO();
    UARTCharPut(UART0_BASE, retornoConnWifi);
    while(retornoConnWifi != '0'){
        //UARTCharPut(UART1_BASE, '\n');
        cleanUART1FIFO();
        ESP8266comando("WFC,HUAWEIP20Marcelo,majubafe29797\n");
        delayMs(10000);
        retornoConnWifi = UARTCharGet(UART1_BASE);
        cleanUART1FIFO();
        UARTCharPut(UART0_BASE, '\n');
        UARTCharPut(UART0_BASE, retornoConnWifi);
    }

    printRetorno("\r\nSe a conectado correctamente con la red wifi\r\n");
    UARTCharPut(UART0_BASE, retornoConnWifi);
    delayMs(10);
}


/*Funcion para conectar al servidor
 * RETORNO
 * - socket -> Valor del socket asignado*/
char connServer(){

    char socketId;
    //UARTCharPut(UART1_BASE, '\n');
    cleanUART1FIFO();
    ESP8266comando("CCS,TCP,api.thingspeak.com,80\n");
    delayMs(6000);
    retornoConnSocket[0] = UARTCharGet(UART1_BASE);//Recibimos el primer valor de retorno, debe estar entre 0 y 5
    delayMs(1000);
    retornoConnSocket[1] = UARTCharGet(UART1_BASE);//Recibimos una coma
    delayMs(1000);
    retornoConnSocket[2] = UARTCharGet(UART1_BASE);//Recibimos el identificador del socket
    delayMs(1000);
    if(retornoConnSocket[0] == '0'){
        printRetorno("\r\nEl socket a sido creado correctamente");
    }else{
        printRetorno("\r\nNo se a logrado establecer la conexion\r\nCodigo de error = ");
        UARTCharPut(UART0_BASE, retornoConnSocket[0]);

    }
    cleanUART1FIFO();//Limpiamos lo que sobre
    while(retornoConnSocket[0] != '0'){
        //UARTCharPut(UART1_BASE, '\n');
        cleanUART1FIFO();
        ESP8266comando("CCS,TCP,api.thingspeak.com,80\n");
        delayMs(6000);
        //ESP8266Retorno(retornoESP8266);
        retornoConnSocket[0] = UARTCharGet(UART1_BASE);//Recibimos el primer valor de retorno, debe estar entre 0 y 5
        delayMs(1000);
        retornoConnSocket[1] = UARTCharGet(UART1_BASE);//Recibimos una coma
        delayMs(1000);
        retornoConnSocket[2] = UARTCharGet(UART1_BASE);//Recibimos el identificador del socket
        cleanUART1FIFO();
        if(retornoConnSocket[0] == '0'){
            printRetorno("\r\nEl socket a sido creado correctamente");
        }else{
            printRetorno("\r\nNo se a logrado establecer la conexion\r\nCodigo de error = ");
            UARTCharPut(UART0_BASE, retornoConnSocket[0]);
        }
    }
    if (retornoConnSocket[2] == '0' || retornoConnSocket[2] == '1' || retornoConnSocket[2] == '2' || retornoConnSocket[2] == '3'){
        printRetorno("\r\nEl socket creado tiene id=");
        socketId = retornoConnSocket[2]; //cargamos el valor del identificador de socket en otra variable que es entera
        UARTCharPut(UART0_BASE, retornoConnSocket[2]);
    }
    cleanUART1FIFO();
    return socketId;
}


/*Funcion que comprueba el estado del modulo ESP0-1
 * Parametros: ninguno
 * RETORNO: ninguno
 * */
void compruebaEstadoESP(){
    int32_t retornoMIS;
    printRetorno("Verificando estado del modulo ESP-01\r\n");
    //UARTCharPut(UART1_BASE, '\n');
    cleanUART1FIFO();
    ESP8266comando("MIS\n");
    //    retornoMIS = UARTCharGet(UART1_BASE);
    //    cleanUART1FIFO();
    while(ban){
       if((retornoMIS = UARTCharGet(UART1_BASE))){
             if(retornoMIS == '0'){
                 printRetorno("Modulo ESP01 listo para recibir comandos\r\n");
                 ban = 0;
             }
       }
    }
    cleanUART1FIFO();//Finalizado el envio limpiamos el buffer rx
    UARTCharPut(UART0_BASE, retornoMIS);
}

/*Funcion donde enviamos los datos de temperatura atraves del socket0
 * Paramentros: _socketID
 * Retorno: ninguno*/
void sendTemperatura(char _socketID, int temp){

    char outstring[2];
    uint8_t retornotemp;
    //outstring = itoa_simple(outstring,temp);
    //snprintf(outstring, 3, "%d", temp);
    //printf(outstring,"%i",temp);//Transformamos a string
    cleanUART1FIFO();
    delayMs(1000);
    if(_socketID == '0'){
        ESP8266comando("SOW,0,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field1=");
    }
    if(_socketID == '1'){
        ESP8266comando("SOW,1,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field1=");
    }
    if(_socketID == '2'){
        ESP8266comando("SOW,2,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field1=");
    }
    if(_socketID == '3'){
        ESP8266comando("SOW,3,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field1=");
    }
    itoa_simple(outstring,temp);

    ESP8266comando(outstring);
    ESP8266comando("\r\n\n");
    delayMs(3000);


    retornotemp = UARTCharGet(UART1_BASE);
    if(retornotemp == '0'){
        printRetorno("\r\nSe ha enviado correctamente los datos de temperatura\r\n");
        cleanUART1FIFO();
    }else{
        printRetorno("\r\nInconvenientes para enviar los datos.\r\nCodigo de error = ");
        UARTCharPut(UART0_BASE, retornotemp);
        ESP8266Retorno(retornoESP8266);
        delayMs(3000);
        printRetorno(retornoESP8266);
    }
    //UARTCharPut(UART1_BASE, '\n');
    cleanUART1FIFO();
//    //Cerramos el socket
//    delayMs(5000);
//    ESP8266comando("SOC,0\n");
//    delayMs(3000);
//    ESP8266Retorno(retornoESP8266);
//    delayMs(3000);
//    printRetorno(retornoESP8266);
//    printRetorno("\r\n");
//    cleanUART1FIFO();

}


/*Funcion envia datos de humedad atravez del socket0
 * Parametros: ninguno
 * Retorno: ninguno*/
void sendHumedad(char _socketID, int hum){
    uint8_t retornohum;
    char outstring[2];
    //outstring = itoa_simple(outstring,temp);
    //sprintf(outstring,"%i",hum);//Transformamos a string
    cleanUART1FIFO();
    delayMs(1000);
    if(_socketID == '0'){
        ESP8266comando("SOW,0,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field2=");
    }
    if(_socketID == '1'){
        ESP8266comando("SOW,1,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field2=");
    }
    if(_socketID == '2'){
        ESP8266comando("SOW,2,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field2=");
    }
    if(_socketID == '3'){
        ESP8266comando("SOW,3,74,GET https://api.thingspeak.com/update?api_key=WHFDS5MPBM95JAU5&field2=");
    }
    itoa_simple(outstring, hum);
    ESP8266comando(outstring);
    //UARTCharPut(UART1_BASE, "20");
    ESP8266comando("\r\n\n");
    delayMs(3000);


   retornohum = UARTCharGet(UART1_BASE);
   if(retornohum == '0'){
       printRetorno("\r\nSe ha enviado correctamente los datos de humedad\r\n");
       cleanUART1FIFO();
   }else{
       printRetorno("\r\nInconvenientes para enviar los datos.\r\nCodigo de error = ");
       UARTCharPut(UART0_BASE, retornohum);
       ESP8266Retorno(retornoESP8266);
       delayMs(3000);
       printRetorno(retornoESP8266);
   }
}





//Funciones necesarias para parsear un valor entero a un string

static char *itoa_simple_helper(char *dest, int i) {
  if (i <= -10) {
    dest = itoa_simple_helper(dest, i/10);
  }
  *dest++ = '0' - i%10;
  return dest;
}

char *itoa_simple(char *dest, int i) {
  char *s = dest;
  if (i < 0) {
    *s++ = '-';
  } else {
    i = -i;
  }
  *itoa_simple_helper(s, i) = '\0';
  return dest;
}
