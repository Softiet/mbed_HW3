#include "mbed.h"
#include "mbed_events.h"


DigitalOut led1(LED1);

DigitalOut led2(LED2);

InterruptIn sw2(SW2);

EventQueue queue(32 * EVENTS_EVENT_SIZE);

Thread t;

Ticker log_ticker;
Ticker led_ticker;

void logger(){
    led2 = (led2==1)?0:1;
}

void deactivate(){
    log_ticker.detach();
    led_ticker.detach();
    led1 = 1;
    led2 = 1;
}

void activate(){
    led_ticker.attach(queue.event(&logger),1.0f);
    
    led1 = 0;
    queue.call_in(10000, deactivate);
}





int main() {
    led1 = 1;
    led2 = 1;
    // t is a thread to process tasks in an EventQueue

    // t call queue.dispatch_forever() to start the scheduler of the EventQueue

    t.start(callback(&queue, &EventQueue::dispatch_forever));

    // 'Trig_led1' will execute in IRQ context

    sw2.rise(activate);

    // 'Trig_led2' will execute in context of thread 't'

    // 'Trig_led2' is directly put into the queue

    // sw3.rise(queue.event(Trig_led2));
}

