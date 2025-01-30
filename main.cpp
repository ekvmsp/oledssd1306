/*
 
  --- Intel  Edison   Pin map ---
00       J17-1: GPIO PWM
01       J17-2:
02       J17-3:
03       J17-4:
04       J17-5: GPIO
05       J17-6:
06       J17-7: GPIO I2C   i2c6_SCL
07       J17-8: GPIO I2C   i2c1_SDA
08       J17-9: GPIO I2C   i2c6_SDA
09      J17-10: GPIO SPI
10      J17-11: GPIO SPI
11      J17-12: GPIO SPI
12      J17-13:
13      J17-14: GPIO
14       J18-1: GPIO PWM
15       J18-2: GPIO
16       J18-3:
17       J18-4:
18       J18-5:
19       J18-6: GPIO I2C   i2c1_SCL
20       J18-7: GPIO PWM
21       J18-8: GPIO PWM
22       J18-9:
23      J18-10: GPIO SPI
24      J18-11: GPIO SPI
25      J18-12: GPIO
26      J18-13: GPIO UART
27      J18-14:
28       J19-1:
29       J19-2:
30       J19-3:            GND
31       J19-4: GPIO
32       J19-5: GPIO
33       J19-6: GPIO
34       J19-7:
35       J19-8: GPIO UART
36       J19-9: GPIO
37      J19-10: GPIO
38      J19-11: GPIO
39      J19-12: GPIO
40      J19-13: GPIO
41      J19-14: GPIO
42       J20-1:
43       J20-2:           3.3v Output
44       J20-3:
45       J20-4: GPIO
46       J20-5: GPIO
47       J20-6: GPIO
48       J20-7: GPIO
49       J20-8: GPIO
50       J20-9: GPIO
51      J20-10: GPIO
52      J20-11: GPIO
53      J20-12: GPIO
54      J20-13: GPIO
55      J20-14: GPIO


*/

#include <stdio.h>
#include <mraa.h>
#include "font_def.h"

/*
   mraa  i2c에서는 한 번에 전송할 수있는 데이터 개수가 정해져 있다.

   I2C_SMBUS_I2C_BLOCK_MAX 32 

   32byte 을 넘으면 안 된다.
*/
#define  MRAA_I2C_MAXBUFFER  32  // mraa_i2c_write() 최대 전송 byte크기
#define  I2C_BUS  6

#define  OLEDI2C_ADDR    0x3C // 0x78 아님
#define  OLEDMODE_CMD    0x00
#define  OLEDMODE_DATA   0x40
#define  LCDH  8
#define  LCDW  128


mraa_i2c_context   _i2c;

uint8_t  _buffer[LCDH * LCDW];

void  ssd1306_send_cmd(uint8_t  cmd)
{
    uint8_t  buf[2];

    buf[0] = OLEDMODE_CMD;
    buf[1] = cmd;
	mraa_i2c_address(_i2c, OLEDI2C_ADDR);
	mraa_i2c_write(_i2c, buf, 2);
}

void  ssd1306_send_data(uint8_t  data)
{
    uint8_t  buf[2];

    buf[0] = OLEDMODE_DATA;
    buf[1] = data;
	mraa_i2c_address(_i2c, OLEDI2C_ADDR);
	mraa_i2c_write(_i2c, buf, 2);
}

void  ssd1306_setup(void)
{
	//...............................
	ssd1306_send_cmd(0xAE); // Set display OFF		
	ssd1306_send_cmd(0xD4); // Set Display Clock Divide Ratio / OSC Frequency
	ssd1306_send_cmd(0x80); // Display Clock Divide Ratio / OSC Frequency 
	ssd1306_send_cmd(0xA8); // Set Multiplex Ratio
	ssd1306_send_cmd(0x3F); // Multiplex Ratio for 128x64 (64-1)
	ssd1306_send_cmd(0xD3); // Set Display Offset
	ssd1306_send_cmd(0x00); // Display Offset
	ssd1306_send_cmd(0x40); // Set Display Start Line
	ssd1306_send_cmd(0x8D); // Set Charge Pump
	ssd1306_send_cmd(0x14); // Charge Pump (0x10 External, 0x14 Internal DC/DC)
	ssd1306_send_cmd(0xA1); // Set Segment Re-Map
	ssd1306_send_cmd(0xC8); // Set Com Output Scan Direction
	ssd1306_send_cmd(0xDA); // Set COM Hardware Configuration
	ssd1306_send_cmd(0x12); // COM Hardware Configuration
	ssd1306_send_cmd(0x81); // Set Contrast
	ssd1306_send_cmd(0xCF); // Contrast
	ssd1306_send_cmd(0xD9); // Set Pre-Charge Period
	ssd1306_send_cmd(0xF1); // Set Pre-Charge Period (0x22 External, 0xF1 Internal)
	ssd1306_send_cmd(0xDB); // Set VCOMH Deselect Level
	ssd1306_send_cmd(0x40); // VCOMH Deselect Level
	ssd1306_send_cmd(0xA4); // Set all pixels OFF
	ssd1306_send_cmd(0xA6); // Set display not inverted
	ssd1306_send_cmd(0xAF); // Set display On

	//ssd1306_send_cmd(0x22);// Addressing Mode 
}

void  draw_buffer(void)
{
	int  blocksize = LCDW / 8; // MRAA_I2C_MAXBUFFER 보다 작아야 한다.
	uint8_t  temp[MRAA_I2C_MAXBUFFER];
	temp[0] = OLEDMODE_DATA;

	ssd1306_send_cmd(0x10);
	ssd1306_send_cmd(0x00);

	for (int y = 0; y < LCDH; y++)
	{
		    ssd1306_send_cmd(0xB0 | y);
		for (unsigned int x = 0; x < LCDW; x += blocksize)
		{
			for (int n = 0; n < blocksize; n++) temp[n + 1] = _buffer[LCDW * y + x + n];
			mraa_i2c_write(_i2c, temp, blocksize+1);
		}
	}
}

void  draw_pixel(uint8_t x, uint8_t y)
{
	uint8_t page = y / 8;
	uint8_t sh = y % 8;

	_buffer[(LCDW * page) + x] |= 0x01 << sh;
}


void  draw_clear(uint8_t v)
{
	for (int n = 0; n < (LCDW * LCDH); n++) _buffer[n] = v;
}

void  draw_text(uint8_t sx, uint8_t sy, uint8_t  t)
{
	uint8_t  ch;
	uint8_t page;
	uint8_t sh;

	for(unsigned int y=0; y < FONT_H; y++)
	{
		ch = ~_fontbits[t][y];
		for(unsigned int x=0; x < FONT_W; x++)
		{
			if ((ch & (0x80 >> x)) != 0)
			{
				page = (sy + y) / 8;
				sh = (sy + y) % 8;
				_buffer[(LCDW * page) + sx + x] |= 0x01 << sh;
			}
		}
	}
}

void  draw_string(uint8_t sx, uint8_t sy, char* pstr)
{
	for (int n = 0; *(pstr + n) != 0; n++)
	{
		draw_text(sx + FONT_W * n, sy, *(pstr + n));
	}
}

int main()
{
	char  buf[20];
    mraa_result_t   result;
    mraa_init();

    printf("MRAA  version  %s\n", mraa_get_version());
    printf("Platfrom   %s\n", mraa_get_platform_name());

    _i2c =  mraa_i2c_init(I2C_BUS);
   if (_i2c == NULL) {
       printf("Failed to initialize I2C\n");
       mraa_deinit();
       return 0;
   }
   
   for (int n = 0; n < 40; n++)
   {
	   sprintf(buf, "%d Edison", n);


	   draw_clear(0x00);
	       draw_string(15, n, buf);
	   draw_buffer();
   }

    mraa_i2c_stop(_i2c);
    mraa_deinit();

	printf("exit\n");

    return 0;
}