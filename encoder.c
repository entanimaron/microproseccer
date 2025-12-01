/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void show_dot(int y);
void lcd_init();
void lcd_sync_vbuf();
void lcd_clear_vbuf();

#define INIT	0
#define PLAY	1

int state = INIT;

/* ===== エンコーダ ===== */
volatile int *rot_enc = (int *)0xff14;

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
	static int prev_val = 0;
	static int y = 3;
	int val;
	int diff;

	if (state == PLAY) {
		/* エンコーダ値取得 */
		val = (*rot_enc >> 2) & 0xff;

		/* 変化量だけ取得 */
		diff = val - prev_val;
		prev_val = val;

		/* 回した方向で1マスずつ移動 */
		if (diff > 0) y++;   // 右回し
		if (diff < 0) y--;   // 左回し

		/* 画面上下制限 */
		if (y < 0) y = 0;
		if (y > 7) y = 7;

		show_dot(y);
	}

	lcd_sync_vbuf();
}


void main() {
	while (1) {
		if (state == INIT) {
			lcd_init();
			state = PLAY;
		}
	}
}

/* ===== 左端にドット表示 ===== */
void show_dot(int y) {
	lcd_clear_vbuf();
	lcd_putc(y, 0, 'O');   // 左端(x=0)、上下(y)
}

/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];

void lcd_wait(int n) {
	for (int i = 0; i < n; i++);
}

void lcd_cmd(unsigned char cmd) {
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = cmd;
	lcd_wait(1000);
}

void lcd_data(unsigned char data) {
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = 0x100 | data;
	lcd_wait(200);
}

void lcd_pwr_on() {
	volatile int *lcd_ptr = (int *)0xff0c;
	*lcd_ptr = 0x200;
	lcd_wait(700000);
}

void lcd_init() {
	lcd_pwr_on();
	lcd_cmd(0xa0);
	lcd_cmd(0x20);
	lcd_cmd(0x15);
	lcd_cmd(0);
	lcd_cmd(95);
	lcd_cmd(0x75);
	lcd_cmd(0);
	lcd_cmd(63);
	lcd_cmd(0xaf);
}

void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
	r >>= 5; g >>= 5; b >>= 6;
	lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}

void lcd_clear_vbuf() {
	for (int row = 0; row < 64; row++)
		for (int col = 0; col < 96; col++)
			lcd_vbuf[row][col] = 0;
}

void lcd_sync_vbuf() {
	for (int row = 0; row < 64; row++)
		for (int col = 0; col < 96; col++)
			lcd_data(lcd_vbuf[row][col]);
}

void lcd_putc(int y, int x, int c) {
	for (int v = 0; v < 8; v++)
		for (int h = 0; h < 8; h++)
			if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
				lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 0);
}
