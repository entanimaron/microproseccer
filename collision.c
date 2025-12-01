/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void lcd_init();
void lcd_sync_vbuf();
void lcd_clear_vbuf();
void lcd_putc(int y, int x, int c);

/* ===== 状態 ===== */
#define INIT 0
#define PLAY 1
int state = INIT;

/* ===== デバイス ===== */
volatile int *rot_enc = (int *)0xff14;
volatile int *led_ptr = (int *)0xff08;

/* ===== プレイヤー & 障害物 ===== */
int player_y = 3;
int obstacle_x = 11;
int obstacle_y = 3;

/* ===== 割り込み (100msごと) ===== */
void interrupt_handler() {
	static int prev_val = 0;
	int val, diff;

	if (state == PLAY) {

		/* ===== プレイヤー移動（エンコーダ） ===== */
		val = (*rot_enc >> 2) & 0xff;
		diff = val - prev_val;
		prev_val = val;

		if (diff > 0) player_y++;
		if (diff < 0) player_y--;

		if (player_y < 0) player_y = 0;
		if (player_y > 7) player_y = 7;

		/* ===== 障害物流れる（右 → 左） ===== */
		obstacle_x--;

		if (obstacle_x < 0) {
			obstacle_x = 11;
			obstacle_y = (player_y + 3) % 8;  // ランダム風
		}

		/* ===== 衝突判定 ===== */
		if (obstacle_x == 0 && obstacle_y == player_y) {
			*led_ptr = 0xF;   // 衝突でLED点灯
		} else {
			*led_ptr = 0x0;
		}

		/* ===== 描画 ===== */
		lcd_clear_vbuf();
		lcd_putc(player_y, 0, 'O');         // プレイヤー
		lcd_putc(obstacle_y, obstacle_x, 'X'); // 障害物
	}

	lcd_sync_vbuf();
}

/* ===== メイン ===== */
void main() {
	while (1) {
		if (state == INIT) {
			lcd_init();
			state = PLAY;
		}
	}
}

/*
 * ================= LCD関数群（変更不要） =================
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
