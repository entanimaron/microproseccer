/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"
#define BULLET_MAX 100
#define ENE_MAX 100
#define WIDTH 96
#define HEIGHT 64
#define RTE_ADDR 0xff12
#define MAX_Y 7
#define ENCODER_BASE 0.1
char boss_img = 'B';
int playGame();
void initGame();
void initVariable();
void movePlayer();
void setBullet();
void moveBullet();
int setEnemy(int x, int y, int vx, int vy, char img, int life, int ptn, int wid, int hei);
void moveEnemy();
void drawImg(int x, int y, char img);
void hitCheck();
int  btn_check_0();
int  btn_check_1();
int  btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_sync_vbuf();
void lcd_clear_vbuf();
enum { ENE_BULLET, ENE_BOSS};

enum { INIT, OPENING, PLAY, CLEAR, OVER};
int state = INIT, pos = 0;
int rte_prev = 128;

struct OBJECT
{
	int x;
	int y;
	int vx;
	int vy;
	char img;
	int  wid;
	int hei;
	int life;
	int state;
	int ptn;
};

struct OBJECT player;
struct OBJECT bullet[BULLET_MAX];
struct OBJECT enemy[ENE_MAX];
struct OBJECT boss;
/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
	lcd_clear_vbuf();
	static int cnt;
	if (state == INIT) {
	} else if (state == OPENING) {
		cnt = 0;
		//描画
	} else if (state == PLAY) {
		//描画
		drawImg(player.x, player.y, player.img);
		for (int i = 0; i < BULLET_MAX; i++) {
			if (bullet[i].state == 0) continue;
			drawImg(bullet[i].x, bullet[i].y, '-');
		}
		for (int i = 0; i < ENE_MAX; i++) {
            if (enemy[i].state == 0) continue;
            drawImg(enemy[i].x, enemy[i].y, enemy[i].img);
        }
		if (boss.state == 1) { 
            for (int r = 0; r < 3; r++) { // 行 (Y)
                for (int c = 0; c < 3; c++) { // 列 (X)
                    drawImg(boss.x + c * 8, boss.y + r * 8, boss.img); 
                }
            }
        }
		
	    if (cnt % 10 == 0) {
			setEnemy(90, (cnt / 10 % 6) * 8, 3, 0, '*', 1, ENE_BULLET, 8, 8);
			setEnemy(86, (cnt / 10 % 6) * 8, 3, 0, '*', 1, ENE_BULLET, 8, 8);
		}
		cnt++;
		lcd_putc(7, 0, 'L');
        lcd_putc(7, 1, 'I');
        lcd_putc(7, 2, 'F');
        lcd_putc(7, 3, 'E');
        lcd_putc(7, 4, '0' + player.life);
        
	} else if (state == CLEAR) {
		//描画
		lcd_puts(0, 4, "GAME CLEAR!");
	} else if (state == OVER) {
		//描画
		lcd_puts(0, 4, "GAME OVER!");
	}
	lcd_sync_vbuf();
}
void main() {
	while (1) {
		if (state == INIT) {
			lcd_init();
			state = OPENING; 
		} else if (state == OPENING) {
			state = PLAY;
			initVariable();
		} else if (state == PLAY) {
			int s = playGame();
			if (s == 0) state = CLEAR;
			if (s == 1) state = OVER;
		} else if (state == CLEAR) {
			state = OPENING;
		} else if (state == OVER) {
			state = OPENING;
		}
	}
}
int playGame() {

	if (player.life < 1) return 1;
	if (boss.life < 1) return 0;
	if (boss.state == 0 && player.life > 0) {
		boss.x = WIDTH - 24; 
        boss.y = HEIGHT / 2 - 12; 
        boss.wid = 24;      
        boss.hei = 24;      
        boss.life = 30;
        boss.state = 1;
        boss.img = boss_img;
	}
	
	moveBullet();
	movePlayer();
	moveEnemy();
	hitCheck();
	return -1;
}

void drawImg(int x, int y, char img) {
	int cnt = 0;
	int x_char = x / 8;
	int y_char = y / 8;
	if (x_char >= 0 && x_char < WIDTH / 8 && y_char >= 0 && y_char < HEIGHT / 8) {
		lcd_putc(y_char, x_char, img);
	}

}



void initVariable()
{
	player.x = WIDTH / 4;
	player.y = HEIGHT / 2;
	player.vx = 5;
	player.vy = 5;
	player.img = '>';
	player.wid = 8;
	player.hei = 8;
	player.life = 3;
}

void movePlayer()
{
	volatile int *rte_ptr = (int*)RTE_ADDR;
	int rte_current = (*rte_ptr) >> 2;
	int diff = rte_current - rte_prev;
	rte_prev = rte_current;
	player.y += diff * ENCODER_BASE * player.vy;
	if (player.y < 0) player.y = 0;
	if (player.y > HEIGHT - player.hei) player.y = HEIGHT - player.hei;
	if (btn_check_0()) setBullet(); 
}

void setBullet()
{
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullet[i].state == 0) {
			bullet[i].x = player.x + player.wid;
			bullet[i].y = player.y + player.hei / 2;
			bullet[i].vx = 20;
			bullet[i].vy = 0;
			bullet[i].state = 1;
			bullet[i].wid = 8;
			bullet[i].hei = 8;
			break;
		}
		
	}
}

void moveBullet()
{
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullet[i].state == 0) continue;
		bullet[i].x += bullet[i].vx;
		bullet[i].y += bullet[i].vy;
		
		if (bullet[i].x > WIDTH || bullet[i].x < -bullet[i].wid) bullet[i].state = 0;
	}
}

int setEnemy(int x, int y, int vx, int vy, char img, int life,int ptn, int wid, int hei)
{
	
	for (int i = 0; i < ENE_MAX; i++) {
		if (enemy[i].state == 0) {
			enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].vx = vx;
			enemy[i].vy = vy;
			enemy[i].life = life;
			enemy[i].img = img;
			enemy[i].ptn = ptn;
			enemy[i].wid = wid;
			enemy[i].hei = hei;
			enemy[i].state = 1;
			return i;
		}
	}
	return -1;
}

void moveEnemy()
{
	for (int i = 0; i < ENE_MAX; i++) {
		if (enemy[i].state == 0) continue;
		if (enemy[i].ptn == ENE_BULLET) {
			enemy[i].x -= enemy[i].vx;
		}
		if (enemy[i].x < 0) enemy[i].state = 0;
	}
}

void hitCheck()
{
	//ボスの弾と自分のあたり判定
	for (int i = 0;i < ENE_MAX; i++) {
		if (enemy[i].state == 0) continue;
		int dx = enemy[i].x - player.x;
		int dy = enemy[i].y - player.y;
		if (dx < 0) dx *= -1;
		if (dy < 0) dy *= -1;
		if (dx < 8 && dy < 8) {
			player.life--;
			enemy[i].state = 0;
            break;
		}
	}
	//ボスと自分の弾のあたり判定
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullet[i].state == 0) continue;
		int dx = boss.x + boss.wid / 2 - (bullet[i].x + bullet[i].wid / 2);
		int dy = boss.y + boss.hei / 2 - (bullet[i].y + bullet[i].hei / 2);
		if (dx < 0) dx *= -1;
		if (dy < 0) dy *= -1;
		if (dx <  (boss.wid + bullet[i].wid) / 2 && dy < (boss.hei + bullet[i].hei) / 2) {
            boss.life--;
            bullet[i].state = 0;
            break;
        }
	}
}

 /*
 * Switch functions
 */
int btn_check_0() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x10) ? 1 : 0;
}
int btn_check_1() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x20) ? 1 : 0;
}
int btn_check_3() {
	volatile int *sw_ptr = (int *)0xff04;;
	return (*sw_ptr & 0x80) ? 1 : 0;
}
/*
 * LED functions
 */
void led_set(int data) {
	volatile int *led_ptr = (int *)0xff08;
	*led_ptr = data;
}
void led_blink() {
	led_set(0xf);				/* Turn on */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0x0);				/* Turn off */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0xf);				/* Turn on */
	for (int i = 0; i < 300000; i++);	/* Wait */
	led_set(0x0);				/* Turn off */
}
/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];
void lcd_wait(int n) {
	/* Not implemented yet */
	for (int i = 0; i < n; i++);
}
void lcd_cmd(unsigned char cmd) {
	/* Not implemented yet */
	volatile int *lcd_ptr = (int *)0xff0c;
        *lcd_ptr = cmd;
        lcd_wait(1000);
}
void lcd_data(unsigned char data) {
	/* Not implemented yet */
	volatile int *lcd_ptr = (int *)0xff0c;
    *lcd_ptr = 0x100 | data;
    lcd_wait(200);
}
void lcd_pwr_on() {
	/* Not implemented yet */
	volatile int *lcd_ptr = (int *)0xff0c;
        *lcd_ptr = 0x200;
        lcd_wait(700000);
}
void lcd_init() {
	/* Not implemented yet */
	lcd_pwr_on();   /* Display power ON */
    lcd_cmd(0xa0);  /* Remap & color depth */
    lcd_cmd(0x20);
    lcd_cmd(0x15);  /* Set column address */
	lcd_cmd(0);
    lcd_cmd(95);
    lcd_cmd(0x75);  /* Set row address */
    lcd_cmd(0);
    lcd_cmd(63);
    lcd_cmd(0xaf);  /* Display ON */
}
void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
	/* Not implemented yet */
	r >>= 5; g >>= 5; b >>= 6;
    lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}
void lcd_clear_vbuf() {
	/* Not implemented yet */
    for (int row = 0; row < 64; row++)
            for (int col = 0; col < 96; col++)
                    lcd_vbuf[row][col] = 0;
}
void lcd_sync_vbuf() {
	/* Not implemented yet */
	for (int row = 0; row < 64; row++)
        	for (int col = 0; col < 96; col++)
            		lcd_data(lcd_vbuf[row][col]);
}
void lcd_putc(int y, int x, int c) {
	/* Not implemented yet */
	for (int v = 0; v < 8; v++)
                for (int h = 0; h < 8; h++)
                        if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 0);
}
void lcd_puts(int y, int x, char *str) {
	/* Not implemented yet */
	for (int i = x; i < 12; i++)
                if (str[i] < 0x20 || str[i] > 0x7f)
                        break;
                else
                        lcd_putc(y, i, str[i]);
}