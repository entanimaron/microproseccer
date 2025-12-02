/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"
#include "micro.h"
#define BULLET_MAX 100
#define ENE_MAX 100
#define WIDTH 96
#define HEIGHT 64
#define RTE_ADDR 0xff12
#define MAX_Y 7
#define ENCODER_BASE 0.1
char boss_img = 'B';
char item_img = 'I';

enum { ENE_BULLET, ENE_BOSS , NUM };  //敵の種類
enum { NORMAL, SHOTGUN, BEAM, HEART };  //アイテム？とか
enum { INIT, OPENING, PLAY, CLEAR, OVER };  //状態
int state = INIT, pos = 0;
int rte_prev = 128;
int shotType = NORMAL;
static int seed = 23;
static int timer = 0;
int startPowerUp = 0;
static int product = 1;
int input_len = 0;
char input_str[16] = {0};


struct OBJECT player;  //プレイヤー
struct OBJECT bullet[BULLET_MAX];  //自分が打つ弾
struct OBJECT enemy[ENE_MAX];  //敵の弾or数字
struct OBJECT boss;  //ボス
struct OBJECT item;  //アイテム

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
		for (int i = 0; i < BULLET_MAX; i++) {  //自分の撃った弾の表示
			if (bullet[i].state == 0) continue;
			drawImg(bullet[i].x, bullet[i].y, '-');
		}
		for (int i = 0; i < ENE_MAX; i++) {  //敵の表示
            if (enemy[i].state == 0) continue;
            if (enemy[i].ptn == ENE_BULLET) {
                drawImg(enemy[i].x, enemy[i].y, enemy[i].img);
            } else if (enemy[i].ptn == NUM) {  //数字の表示
                int val = enemy[i].life;
                if (val >= 10) {
                    drawImg(enemy[i].x, enemy[i].y, '0' + val / 10);
                    drawImg(enemy[i].x + 8, enemy[i].y, '0' + val % 10);
                } else {
                    drawImg(enemy[i].x, enemy[i].y, '0' + val);
                }
                
            }
            
        }
		if (boss.state == 1) { 
            for (int r = 0; r < 3; r++) { // 行 (Y)
                for (int c = 0; c < 3; c++) { // 列 (X)
                    drawImg(boss.x + c * 8, boss.y + r * 8, boss.img); 
                }
            }
        }

        if (item.state == 1) {
            drawImg(item.x, item.y, item_img);
        }
		
	    if (cnt % 10 == 0) {  //敵のセット　playGame()の方がいい?
			setEnemy(90, (cnt / 10 % 6) * 8, 3, 0, '*', 1, ENE_BULLET, 8, 8);
			setEnemy(86, (cnt / 10 % 6) * 8, 3, 0, '*', 1, ENE_BULLET, 8, 8);
		} else if (cnt % 30 == 0 && cnt > 1) {  //数字のセット
            setEnemy(80, (cnt / 10 % 6) * 8, 3, 0, 'n', createNum(), NUM, 16, 8);
        }
		cnt++;
        draw_input_formula();
        //ライフの表示 LEDにしたい
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
            timer++;
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
    if (btn_check_0()) setBullet();
	if (item.state == 0 && timer % 600 == 1) setItem();

    if (shotType != NORMAL && timer - startPowerUp >= 100) shotType = NORMAL;
	moveBullet();
	movePlayer();
	moveEnemy();
    moveItem();
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
    timer = 0;
    startPowerUp = 0;
    shotType = NORMAL;
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
	 
}

void setBullet()
{
    int base_x = player.x + player.wid;
    int base_y = player.y + player.hei / 2;
    if (shotType == NORMAL) {  //通常弾
        for (int i = 0; i < BULLET_MAX; i++) {
		    if (bullet[i].state == 0) {
			    bullet[i].x = base_x;
			    bullet[i].y = base_y;
			    bullet[i].vx = 20;
			    bullet[i].vy = 0;
			    bullet[i].state = 1;
			    bullet[i].wid = 8;
			    bullet[i].hei = 4;
			    break;
		    }
	    }
    } else if (shotType == SHOTGUN) {  //ショットガン
        int bulletAngleNum = 3;
        for (int k = 0; k < bulletAngleNum; k++) {
            for (int i = 0; i < BULLET_MAX; i++) {
		        if (bullet[i].state == 0) {
			        bullet[i].x = base_x;
			        bullet[i].y = base_y;
			        bullet[i].vx = 15;
			        bullet[i].vy = (k - 1) * (-5);
			        bullet[i].state = 1;
			        bullet[i].wid = 8;
			        bullet[i].hei = 4;
			        break;
		        }
	        }
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
		if (enemy[i].state == 0 && ptn == ENE_BULLET) {  //敵の球をセット
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
		} else if (enemy[i].state == 0 && ptn == NUM) {  //数字を撃ち出す
            enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].vx = vx;
			enemy[i].vy = vy;
			enemy[i].life = life;  //面倒なのでlifeを打ち出す数とする
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
		if (enemy[i].ptn == ENE_BULLET || enemy[i].ptn == NUM) {
			enemy[i].x -= enemy[i].vx;  //とりあえずx軸のみ移動
		}
		if (enemy[i].x + enemy[i].wid < 0) enemy[i].state = 0;
	}
}

void setItem()
{
    item.x = WIDTH * 3 / 4;
    item.y = (myRand() % 8) * 8 + 4;
    item.vx = 2;
    item.vy = 0;  //今のところ0
    item.state = 1;
    item.wid = 8;
    item.hei = 8;
    item.timer = 0;
}

void moveItem()
{
    if (item.state == 0) return;
    item.x -= item.vx;
    item.y -= item.vy;  //今のところ0
}

void hitCheck()
{
	//ボスの弾と自分のあたり判定(数も含む)
	for (int i = 0;i < ENE_MAX; i++) {
		if (enemy[i].state == 0) continue;
		int dx = (enemy[i].x + enemy[i].wid / 2) - (player.x + player.wid / 2);
		int dy = (enemy[i].y + enemy[i].hei / 2) - (player.y + player.hei / 2);
		if (dx < 0) dx *= -1;
		if (dy < 0) dy *= -1;
		if (dx <= (player.wid + enemy[i].wid) / 2 && dy <= (player.hei + enemy[i].hei) / 2) {
			player.life--;
			enemy[i].state = 0;
            break;
		}
	}
	//ボスと自分の弾のあたり判定
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullet[i].state == 0) continue;
		int dx = (boss.x + boss.wid / 2) - (bullet[i].x + bullet[i].wid / 2);
		int dy = (boss.y + boss.hei / 2) - (bullet[i].y + bullet[i].hei / 2);
		if (dx < 0) dx *= -1;
		if (dy < 0) dy *= -1;
		if (dx <=  (boss.wid + bullet[i].wid) / 2 && dy <= (boss.hei + bullet[i].hei) / 2) {
            boss.life--;
            bullet[i].state = 0;
            break;
        }
	}

    if (item.x + item.wid < 0) item.state = 0;
    if (dx < 0) dx *= -1;
    if (dy < 0) dy *= -1;
    if (dx <= (item.wid + player.wid) / 2 && dy <= (item.hei + player.hei) / 2) {
        item.ptn = 1;  //本来ならtimerでshotgun,beamのどちらか抽選
        startPowerUp = timer;
        if (item.ptn == SHOTGUN) {
            shotType = SHOTGUN;
        } else if (item.ptn == HEART) {
            player.life++;
        }
        item.state = 0;
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

int key_pad_scan()
{
    const int key_map[4][4] = {
        { 13, 14, 15, 16},
        { 12,  9,  8,  7},
        { 11,  6,  5,  4},
        { 10,  3,  2,  1}
    };

    int scan_pattern[4] = {0xE, 0xD, 0xB, 0x7}; 
    volatile int *keypad_addr = (int *)0xff18; // ADDR_KEYPAD の代わりにアドレスを直接使用
    
    for (int col = 0; col < 4; col++) {
        *keypad_addr = scan_pattern[col]; // ① 列線にスキャンパターンを出力

        for(volatile int w=0; w<100; w++); // ② 安定化のための短いウェイト

        int input_data = *keypad_addr;
        int rows = input_data & 0xF; // ③ 下位4ビット（行入力）を取得

        // ④ 行線をチェックして押されたキーを特定
        if ((rows & 0x1) == 0) return key_map[col][0];
        if ((rows & 0x2) == 0) return key_map[col][1];
        if ((rows & 0x4) == 0) return key_map[col][2];
        if ((rows & 0x8) == 0) return key_map[col][3];
    }
    return -1; // どのキーも押されていない場合
}

void handle_key_input() {
    static int prev_key = -1;
    int key = key_pad_scan();

    if (key == prev_key) return;  // 押しっぱなし防止
    prev_key = key;

    if (key == -1) return;
    
    // ターゲットがアクティブでなければ判定を行わない
    if (state != PLAY) return;

    /* ===== 数字キー（素因数）===== */
    if (key >= 2 && key <= 9) { 
        // 配列境界チェック 
        if (input_len + 2 < 16) { 
            product *= key;

            input_str[input_len++] = '0' + key; 
            input_str[input_len++] = 'x'; 
            input_str[input_len]   = 0;
        }
    }

    /* ===== Aキー (テンキーで10) → 判定 / ライフ増加 ===== */
    if (key == 10) {   
        // 最後の 'x' を削除
        if (input_len > 0 && input_str[input_len-1] == 'x') {
             input_len--;
        }
        
        // 素因数分解の成功判定を実行
        check_factor_solution(); 

        // 入力リセット
        product = 1;
        input_len = 0;
        input_str[0] = 0;
    }

    /* ===== Bキー → 全消去 (テンキーで11) ===== */
    if (key == 11) {
        product = 1;
        input_len = 0;
        input_str[0] = 0;
    }
}

void check_factor_solution() {
    // ターゲットがアクティブでなければ判定を行わない
    if (state != PLAY) return;
    
    // --- 素因数分解判定ロジック ---
    for (int i = 0; i < ENE_MAX; i++) {
        if (enemy[i].state == 1 && enemy[i].ptn == NUM) {
             
             // 判定: 入力積が敵の数字と一致するかチェック
             if (product == enemy[i].life) {
                
                //好きな動作にする現在はlife++
                player.life = (player.life < 9) ? player.life + 1 : 9;
                enemy[i].state = 0;  //ターゲット消滅
                return; 
             }
        }
    }
}

void drawFormula() {
    // 画面下部のY=7行目に表示
    for(int i = 0; i < input_len; i++) {
        lcd_putc(7, i, input_str[i]);
    }
}