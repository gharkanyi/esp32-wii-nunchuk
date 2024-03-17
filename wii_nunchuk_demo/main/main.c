#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wii_i2c.h"

#define PIN_SDA  5
#define PIN_SCL  4
#define WII_I2C_PORT 0

//sleep
#define SLEEP(ms) vTaskDelay(pdMS_TO_TICKS(ms))

unsigned int controller_type = 0;

void show_nunchuk(const unsigned char *data)
{
  struct wii_i2c_nunchuk_state state;
  wii_i2c_decode_nunchuk(data, (struct wii_i2c_nunchuk_state *)&state);

  printf("a = (%5d,%5d,%5d)\n", state.acc_x, state.acc_y, state.acc_z);
  printf("d = (%5d,%5d)\n", state.x, state.y);
  printf("c=%d, z=%d\n", state.c, state.z);
}

void show_classic(const unsigned char *data)
{
  struct wii_i2c_classic_state state;
  wii_i2c_decode_classic(data, (struct wii_i2c_classic_state*)&state);

  printf("lx,ly = (%3d,%3d)\n", state.lx, state.ly);
  printf("rx,ry = (%3d,%3d)\n", state.rx, state.ry);
  printf("a lt,rt = (%3d,%3d)\n", state.a_lt, state.a_rt);
  printf("d lt,rt = (%d,%d)\n", state.d_lt, state.d_rt);
  printf("a,b,x,y = (%d,%d,%d,%d)\n", state.a, state.b, state.x, state.y);
  printf("up, down, left, right = (%d,%d,%d,%d)\n", state.up, state.down, state.left, state.right);
  printf("home, plus, minus = (%d,%d,%d)\n", state.home, state.plus, state.minus);
  printf("zl, zr = (%d,%d)\n", state.zl, state.zr);
}

void setup()
{
  printf("Starting...\n");

  if (wii_i2c_init(WII_I2C_PORT, PIN_SDA, PIN_SCL) != 0) {
    printf("ERROR initializing wii i2c controller\n");
    return;
  }
  const unsigned char *ident = wii_i2c_read_ident();
  if (! ident) {
    printf("no ident :(\n");
    return;
  }

  controller_type = wii_i2c_decode_ident(ident);
  switch (controller_type) {
  case WII_I2C_IDENT_NUNCHUK: printf("-> nunchuk detected\n"); break;
  case WII_I2C_IDENT_CLASSIC: printf("-> classic controller detected\n"); break;
  default:                    printf("-> unknown controller detected: 0x%06x\n", controller_type); break;
  }
  wii_i2c_request_state();
}

void app_main(void)
{
    setup();

    for (;;) {
	    const unsigned char *data = wii_i2c_read_state();
	    wii_i2c_request_state();
	    if (data) {
	        switch (controller_type) {
	            case WII_I2C_IDENT_NUNCHUK: show_nunchuk(data); break;
	            case WII_I2C_IDENT_CLASSIC: show_classic(data); break;
	            default:
		            printf("data: %02x %02x %02x %02x %02x %02x\n",
                    data[0], data[1], data[2], data[3], data[4], data[5]);
    		    break;
            }
	    } else {
	        printf("no data :(\n");
	    }

	    SLEEP(250);
    }
}
