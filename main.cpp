#include <vector>

#include "mbed.h"
#include "mbed_events.h"

#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

InterruptIn sw2(SW2);

EventQueue queue(32 * EVENTS_EVENT_SIZE);

Thread thread;

Ticker log_ticker;
Ticker led_ticker;

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);

int m_addr = FXOS8700CQ_SLAVE_ADDR1;
uint8_t who_am_i, data[2], res[6];
int16_t acc16;
float t[3];

std::vector <float> data_x;
std::vector <float> data_y;
std::vector <float> data_z;
std::vector <int> data_tilt;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);

void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void logger();

void led_blink(){
    led2 = (led2==1)?0:1;
}

void deactivate(){
    log_ticker.detach();
    led_ticker.detach();
    led1 = 1;
    led2 = 1;
    led3 = 1;
    //transmission
    for(int i=0;i<99;i++){
        //pc.printf("%d\nX:%f\nY:%f\nZ:%f\n",i,data_x[i],data_y[i],data_z[i]);
        pc.printf("%f\n%f\n%f\n%d\n",data_x[i],data_y[i],data_z[i],data_tilt[i]);
    }
    data_x.clear();
    data_y.clear();
    data_z.clear();
    data_tilt.clear();
}

void activate(){
    led_ticker.attach(queue.event(&led_blink),1.0f);
    log_ticker.attach(queue.event(&logger),0.1f);
    led1 = 0;
    queue.call_in(10000, deactivate);
}

int evaluate(float x,float y,float z);

int main() {
    led1 = 1;
    led2 = 1;
    led3 = 1;
    
    // 'Trig_led2' will execute in context of thread 't'
    // 'Trig_led2' is directly put into the queue
    // sw3.rise(queue.event(Trig_led2));

    pc.baud(115200);

   // Enable the FXOS8700Q
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);

   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
   //pc.printf("Here is %x\r\n", who_am_i);

    // t is a thread to process tasks in an EventQueue
    // t call queue.dispatch_forever() to start the scheduler of the EventQueue
    thread.start(callback(&queue, &EventQueue::dispatch_forever));
    // 'Trig_led1' will execute in IRQ context
    sw2.rise(activate);
    while(1);
}



void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}

void logger(){
    FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
    acc16 = (res[0] << 6) | (res[1] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[0] = ((float)acc16) / 4096.0f;
    acc16 = (res[2] << 6) | (res[3] >> 2);

    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;

    t[1] = ((float)acc16) / 4096.0f;

    acc16 = (res[4] << 6) | (res[5] >> 2);

    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    
    t[2] = ((float)acc16) / 4096.0f;

    /*printf("FXOS8700Q ACC: X=%1.4f(%x%x) Y=%1.4f(%x%x) Z=%1.4f(%x%x)\r\n",\
        t[0], res[0], res[1],\
        t[1], res[2], res[3],\
        t[2], res[4], res[5]\
    );
    */
   data_x.push_back(t[0]);
   data_y.push_back(t[1]);
   data_z.push_back(t[2]);
   data_tilt.push_back(evaluate(t[0],t[1],t[2]));
   
   //pc.printf("%f\n%f\n%f\n",t[0],t[1],t[2]);
}

int evaluate(float x,float y, float z){
    float total = flaot(sqrt(x*x + y*y + z*z));
    if(z < 0.707 * total){
        return 1;
    }  
    return 0;
}