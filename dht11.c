/////////////////////////////////////////////////////VERSION1/////////////////////////////////////////////////////////////////////////
//#include <stdint.h>
//#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/systick.h"
//#include "dht11.h"
//
////extern int hum, hum_low, temp, temp_low, parity;
//void delayMs(uint32_t ui32Ms) {
//
//    // 1 clock cycle = 1 / SysCtlClockGet() second
//    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
//    // 1 second = SysCtlClockGet() / 3
//    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000
//
//    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
//}
//
//void delayUs(uint32_t ui32Us) {
//    SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
//}
//
//void GPIO_PORTE_Init(void){
//    SYSCTL_RCGCGPIO_R |= (0x10);
//    while(!((SYSCTL_PRGPIO_R)&(0x10)));//Esperamos hasta que se habilite el puerto E
//    GPIO_PORTE_PCTL_R &= (0xFFFFFF0F);//Asignamos el pin 1 del puerto E
//    GPIO_PORTE_DEN_R |= (0x02); //PE1 digital enable
//}
//
//void DHT11_Init(void){
//    int i;
//    for(i=0;i<10;i++){ //2segundos de delay
//        delayMs(200);
//    }
//}
//
//void DHT11_Master_Request(void){
//    GPIO_PORTE_DIR_R |= (0x02);//PE1 to set output Direction
//    GPIO_PORTE_DATA_R &= ~(0x02); //Clear PE1 bit as low
//    delayMs(20);
//    GPIO_PORTE_DATA_R |= (0x02);//Set PE1 bit as high
//}
//
//int DHT11_Slave_Response(void){
//    volatile int counter = 0;
//    GPIO_PORTE_DIR_R &= ~(0x02); //PE1 to set input direction
//    while ((GPIO_PORTE_DATA_R&0x02) != 0){
//        counter++;
//        if (counter == 200)
//            return 0;//device offline
//    }
//    while((GPIO_PORTE_DATA_R&0x02)==0){;}
//    while((GPIO_PORTE_DATA_R&0x02)!=0){;}//Mientras PE1 este en alto espera
//    return 1; //Slave exists
//}
//
//
//char DHT11_Rx_8_Data(void){
//    char data = 0;
//    int i;
//    for (i = 0; i < 8; i++) {
//        while((GPIO_PORTE_DATA_R & 0x02)){;}//While is low, wait
//        delayUs(30);
//        if((GPIO_PORTE_DATA_R & 0x02) == 0){//PE1 is low
//            data <<= 1;//Bit 0
//        }else{
//            data <<= 1;//Bit 1
//            data |= 0x1;
//            while((GPIO_PORTE_DATA_R & 0x02) != 0);
//        }
//    }
//    return data;
//}
//
//int DHT11_Verify_Parity(int parity){
//    int hum, hum_low, temp, temp_low;
//    int result;
//    result = hum + hum_low + temp + temp_low; //check sum
//    if(result == parity){
//        return 1;
//    }else{
//        return 0;
//    }
//}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////VERSION2/////////////////////////////////////////////////////////////////////////
#include "dht11.h"
#include "SysTick.h"

struct dht11_T sensor;
unsigned long DHT11_In( unsigned long * ptrValor )
{
    unsigned char crc;
    unsigned char checkSum;
    unsigned char DHT11_STATE = 0;
    unsigned char DHT11_cBits = 0;

    do
    {
      switch( DHT11_STATE )
      {
            case 0: DHT11_STATE = 1;
                            GPIO_PORTD_DIR_R |= 0x00000008;             // PD3 <- OUT
                            SysTick_Init_us( 1 );
                            DHT11_PIN = 0x00;
                            crc = 0;
                            checkSum = 0;
                            DHT11_cBits = 0;
                            *ptrValor = 0;
                            break;
            case 1:   DHT11_STATE = 2;
                            SysTickWaitBusy( 20000 );
                            break;
            case 2:   DHT11_STATE = 3;
                            DHT11_PIN = 0xFF;
                            GPIO_PORTD_DIR_R &= 0xFFFFFFF7;             // PD3 <- IN
                            break;
            case 3:                                                                           // wait 0 - 20..40us - Inicio de recepção
                            if( !DHT11_PIN )
                                DHT11_STATE = 4;
                            break;
            case 4:                                                                           // wait 1 - 80us
                            if( DHT11_PIN )
                                DHT11_STATE = 5;
                            break;
            case 5:                                                                               // wait 0 - 80us
                            if( !DHT11_PIN )
                                DHT11_STATE = 6;
                            break;
            case 6:                                                                               // wait 1 - 50us
                            if( DHT11_PIN )
                            {
                                SysTickWaitBusy( 30 );                          // wait 30us
                                DHT11_STATE = 7;
                            }
                            break;
            case 7:   DHT11_STATE = 8;                                            // Read Bit
                            if( ++DHT11_cBits <= 32 )
                            {
                                *ptrValor <<= 1;
                                if( DHT11_PIN )
                                    ++*ptrValor;
                            }
                            else
                            {
                                crc <<= 1;
                                if( DHT11_PIN )
                                    ++crc;
                            }
                            break;
            case 8:                                                                               // wait 0
                            if( DHT11_cBits >= 40 )
                                DHT11_STATE = 9;
                            else
                                if( !DHT11_PIN )
                                    DHT11_STATE = 6;
                            break;
            case 9: DHT11_STATE = 0;
                            checkSum  = *(((unsigned char *)ptrValor)+3);
                            checkSum += *(((unsigned char *)ptrValor)+2);
                            checkSum += *(((unsigned char *)ptrValor)+1);
                            checkSum += *(((unsigned char *)ptrValor)+0);
                            SysTick_Init_ms( 1 );
                            SysTickWaitBusy( 100 );
                            break;
          }
    }
  while( DHT11_STATE );
  return(crc==checkSum);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION3/////////////////////////////////////////////////////////////////////////
//#include "include.h"
//#include "dht11.h"
//
void delayMs(uint32_t ui32Ms) {

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

void delayUs(uint32_t ui32Us) {
    SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

//
//
//
//// Private methods
//
//float dht_readTemperature(void);
//float dht_readHumidity(void);
//uint8_t dht_read(void);
//void DHTIntHandler(void);
//
//
//// Private data
//uint8_t data_buffer[6];
//uint32_t dht_timing;
//bool quit_timing = false;
//
//void DHT_init(void){
//    //Provide clock for Port F
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
//    TimerConfigure(TIMER5_BASE, TIMER_CFG_ONE_SHOT_UP);//Full-with
//    TimerLoadSet(TIMER5_BASE, TIMER_A, DHT_TIMEOUT+1000);
//
//    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
//    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES); // config interrupt both edges
//    GPIOIntRegister(GPIO_PORTF_BASE, DHTIntHandler);
//
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
//}
//
//// RETURN:
//// 1 = success reading from sensor;
//// 0 = error on reading from sensor;
//// also return T and H
//
//uint8_t dht_readTH(DHT_TypeDef *values) {
//    if (dht_read() == 1) {
//        // temperature
//        values->celsius_temp = dht_readTemperature();
//        // humidity
//        values->humidity = dht_readHumidity();
//        return 1;
//    }
//    return 0;
//}
//
//float dht_readTemperature() {
//    float t;
//    t = data_buffer[2];
//
//    t = data_buffer[2] & 0x7F;
//    t *= 256;
//    t += data_buffer[3];
//    t /= 10.0;
//
//    if (data_buffer[2] & 0x80)
//        t *= -1.0;
//
//    if (t > 80.0) {
//        t = 80.0;
//    }
//    if (t < -40.0) {
//        t = -40.0;
//    }
//
//    return t;
//  }
//
//float dht_readHumidity() {
//    float h;
//    h = data_buffer[0];
//    h = data_buffer[0];
//    h *= 256;
//    h += data_buffer[1];
//    h /= 10.0;
//
//    if (h > 100.0) {
//        h = 100.0;
//    }
//    if (h < 0.0) {
//        h = 0.0;
//    }
//    return h;
//}
//
//uint8_t dht_read(void) {
//    uint8_t ii, byteIdx, bitCounter;
//
//     // clear data
//    for (ii=0; ii< 5; ii++)
//    *(data_buffer + ii) = 0;
//
//     // send start signal low and wait 1-18 ms
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);
//    SysCtlDelay(DHT_WAIT_18ms);
//
//    // send pull up signal high and wait 20-40 us
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
//
//    // wait at least 20us
//    SysCtlDelay(DHT_WAIT_20us);
//
//    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
//    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4);
//
//    // acknowledge ~80us low
//    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0x00);
//    // acknowledge ~80us high
//    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4);
//
//    // gets data 5 bytes, 40 bits
//    // every bit start with low level for 50us then high level last
//    // 0 = 26-28us
//    // 1 = 70us
//    byteIdx = 0;
//    bitCounter = 7;
//    for (ii=0; ii<40; ii++) {
//        quit_timing = false;
//
//        // do elaboration
//        while(!quit_timing);
//
//        // check bit (timing is 1us) 26-28 -> 0 - 70us -> 1
//        if (dht_timing > DHT_TIME_BIT_1) {    // >70us
//            data_buffer[byteIdx] |= (1 << bitCounter);
//          }
//
//         if (bitCounter == 0) {
//             bitCounter = 7;
//             byteIdx++;
//         }
//         else bitCounter--;
//      }
//      return 1;
//  }
//
//void DHTIntHandler(){
//    volatile int32_t start_timer;
//
//    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear interrupt flag
//    if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4) { // check low to high
//    TimerEnable(TIMER5_BASE, TIMER_A);
//    start_timer = TimerValueGet(TIMER5_BASE, TIMER_A);
//    quit_timing = false;
//    }
//    else { // check high to low, finish
//    TimerDisable(TIMER5_BASE, TIMER_A);
//    dht_timing = TimerValueGet(TIMER5_BASE, TIMER_A);
//    dht_timing -= start_timer;
//    quit_timing = true;
//    HWREG(TIMER5_BASE + TIMER_O_TAV) = 0;//Loads value 0 into the timer
//    }
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION4/////////////////////////////////////////////////////////////////////////
//
//#include <stdint.h>
//#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/systick.h"
//#include "dht11.h"
//
//
//
//void delayMs(uint32_t ui32Ms) {
//
//    // 1 clock cycle = 1 / SysCtlClockGet() second
//    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
//    // 1 second = SysCtlClockGet() / 3
//    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000
//
//    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
//}
//
//void delayUs(uint32_t ui32Us) {
//    SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
//}
//
//
//uint8_t i = 0;
//uint16_t time[DATABITS + SETUPBITS];
//
////float temperature;
////float humidity;
//
//
//bool UpdateRdy = 0;
//
//
//#ifndef test1
////#define test1
//#endif
//
//void
//initDHT11Hw ()
//{
////   Configure System clock as 40Mhz
////  SYSCTL_RCC_R =
////    SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4
////                                         <<
////                                         SYSCTL_RCC_SYSDIV_S);
//
//  // Enable GPIO port E
//  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
//
//  //SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
////    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
////    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
////    TimerConfigure(TIMER5_BASE, TIMER_CFG_ONE_SHOT_UP);//Full-with
////    TimerLoadSet(TIMER5_BASE, TIMER_A, DHT_TIMEOUT+1000);
////
////    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
////    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES); // config interrupt both edges
////    GPIOIntRegister(GPIO_PORTF_BASE, DHTIntHandler);
////
////    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
////    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
//}
//
//
//void
//initWTimer1A ()
//{
//  SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;  // turn-on timer
//  WTIMER1_CTL_R &= ~TIMER_CTL_TAEN; // turn-off counter before reconfiguring
//  WTIMER1_CFG_R = 4;        // configure as 32-bit counter (A only)
//  WTIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;  // Oneshot up counter
//  WTIMER1_TAILR_R = 0x2625A00; // Timeout
//}
//
//
//
//void
//initSampler ()
//{
//  GPIO_PORTE_IEV_R |= (1 << 3); // Rising Edge detection
//
//  /* Enable Interrupt */
//
//  GPIO_PORTE_IM_R |= (1 << 3);
//  NVIC_EN0_R |= (1 << (INT_GPIOE - 16));
//
//}
//
//
//void
//readSensordata ()
//{
//
//  // PE3 as Digital O/P Pin
//
//  GPIO_PORTE_DIR_R |= (1 << 3);
//  GPIO_PORTE_DEN_R |= (1 << 3);
//  DATA = 1;
//  delayMs(1000);    // 1 second
//  DATA = 0;
//  delayUs(25000);// ~25ms
//  DATA = 1;
//
//  // PE3 as Digital I/P Pin
//
//  GPIO_PORTE_DIR_R &= ~(1 << 3);
//  while (DATA);         // Wait for response
//  initWTimer1A ();
//  initSampler ();
//
//}
//
//
//uint8_t
//getReading ()
//{
//  uint8_t j;
//  uint8_t base;
//  uint8_t data[DATABITS];   // 2 byte each data (temperature and humidity) + 1 byte parity
//
//  uint8_t inthumid = 0;
//  uint8_t inttemp = 0;
//  uint8_t dechumid = 0;
//  uint8_t dectemp = 0;
//  uint8_t parity = 0;
//
//
//
//  readSensordata ();
//
//  while (!UpdateRdy);
//  UpdateRdy = 0;
//  i = 0;
//
//
//  for (j = 0; j < DATABITS; j++)
//    {
//      if (time[j + SETUPBITS] > LOW)
//    data[j] = 1;
//      else
//    data[j] = 0;
//    }
//
//  memset(time,0,sizeof(time));
//
//
//  for (j = INTHMDSTRT, base = ((RESOLUTION / 2) - 1); j <= INTHMDEND; j++)
//    {
//      inthumid += data[j] << base;
//      base--;
//    }
//
//  for (j = DECHMDSTRT, base = ((RESOLUTION / 2) - 1); j <= DECHMDEND; j++)
//    {
//      dechumid += data[j] << base;
//      base--;
//    }
//  for (j = DECTEMPSTRT, base = ((RESOLUTION / 2) - 1); j <= DECTEMPEND; j++)
//    {
//      dectemp += data[j] << base;
//      base--;
//    }
//  for (j = INTTEMPSTRT, base = ((RESOLUTION / 2) - 1); j <= INTTEMPEND; j++)
//    {
//      inttemp += data[j] << base;
//      base--;
//    }
//
//  for (j = PARITYSTRT, base = ((RESOLUTION / 2) - 1); j <= PARITYEND; j++)
//  {
//      parity += data[j] << base;
//      base--;
//  }
//
//  inthumid += ((float) dechumid / 10);  //float math can be avoided by multiplying integral part by 10 then adding fractional part
//  inttemp += ((float) dectemp / 10);  //TODO : Need to improve code to avoid decimal data beyond the first digit
//
//  if(parity == inthumid + dechumid + dectemp + inttemp)
//  {
//    temperature = inttemp;
//    humidity = inthumid;
//    return 0;
//  }
//
//  else
//  {
//      return 1;
//  }
//
//
//}
//
//
//
//void
//gpioEISR ()
//{
//  if ((GPIO_PORTE_MIS_R & 0xFF) & (1 << 3))
//    {
//      GPIO_PORTE_ICR_R |= (1 << 3);
//
//      time[i++] = WTIMER1_TAV_R;
//
//      WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;
//
//      WTIMER1_TAV_R = 0;
//
//      WTIMER1_CTL_R |= TIMER_CTL_TAEN;
//
//      if (i == (DATABITS + SETUPBITS))
//      {
//          UpdateRdy = 1;
//      }
//    }
//}
//
//
//#ifdef test1
//  OUTPUT = 1;
//  OUTPUT = 0;
//#endif
//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




