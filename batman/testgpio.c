#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pigpio.h>
#include <time.h>
#include <unistd.h>

#define SLEEP 1

//static volatile sig_atomic_t keep_running = 1;

// Ctrl+c signal handler
void sigint_handler(int signum) {
    printf(" [!] SIGINT : bye !");
    gpioTerminate();
    exit(signum);
  //  keep_running = 0;
}

// waiting for button to be pressed
int wait_button(int btn_gpio) {
    int level_button;
    while(1){
        level_button=gpioRead(btn_gpio);
        if(level_button == PI_BAD_GPIO){
            printf("ERROR GPIO READ");
            return 1;		
        }  
        if(level_button==0){
            printf("Button pressed\n");
            return 0;
        }
    }
}

// turn led on for 2 seconds
int turnOnLed(int GPIO_led){
    printf("Turning led ON\n");
    if (gpioWrite(GPIO_led, 1) != 0) {
	printf("Error writing 1 on LED");
	return 1;
    }
    sleep(SLEEP);
    printf("Turning led OFF\n");
    if (gpioWrite(GPIO_led, 0) != 0) {
	printf("Error writing 0 on LED");
	return 1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
	int GPIO_button = 15;
	int GPIO_led = 14;
	if (gpioInitialise() < 0){
		printf("Error GPIOInitialise");
		return 1;
 	}

	signal(SIGINT, sigint_handler);

	if(gpioSetMode(GPIO_button, PI_INPUT) != 0){
		printf("ERROR GPIO SET MODE");
		return 1;	
	}
	
	// turn off the led
	if (gpioWrite(GPIO_led, 0) != 0) {
		printf("Error writing 0 on LED");
		return 1;
        }

	while(1){
		if(wait_button(GPIO_button) != 0){
			return 1;		
		}
		if(turnOnLed(GPIO_led) != 0){
			return 1;		
		}
	}	
	return 0;
}
