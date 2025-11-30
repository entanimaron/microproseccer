#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//---------------------------------------------------------
//  ▼▼ ハードウェアAPI（実験環境で実装） ▼▼
//---------------------------------------------------------
int readKnobADC();
bool readFireButton();
void drawPlayer(int x, int y);
void drawBullet(int x, int y);
void drawBoss(int x, int y, int hp);
void drawObstacle(int x, int y);
void drawNumber(int x, int y, int v);
void drawLifeItem(int x, int y);
void vibrateMotor(int ms);
int  getRandom(int min, int max);

//---------------------------------------------------------
//  ▼▼ Aさんのデータ構造 ▼▼
//---------------------------------------------------------

typedef struct {
    int x, y;
    int hp;
    bool alive;
} Boss;

typedef struct {
    int x, y;
    bool active;
} Obstacle;

typedef struct {
    int x, y;
    int value;
    bool active;
} NumberObj;

typedef struct {
    int x, y;
    bool active;
} LifeItem;

#define MAX_OBSTACLES 10
#define MAX_ITEMS 5
#define MAX_SAVED_NUMBERS 30

Boss boss;
Obstacle obstacles[MAX_OBSTACLES];
NumberObj numbers[MAX_OBSTACLES];
LifeItem items[MAX_ITEMS];
int savedNumbers[MAX_SAVED_NUMBERS];
int savedIndex = 0;

//---------------------------------------------------------
//  ▼▼ Bさんのデータ ▼▼
//---------------------------------------------------------
int playerLife = 5;
int maxLife = 5;

//---------------------------------------------------------
//  ▼▼ Cさんのデータ（プレイヤー＋弾キュー） ▼▼
//---------------------------------------------------------
typedef struct {
    int x, y;
    bool active;
} Bullet;

#define MAX_BULLETS 20

Bullet bulletQueue[MAX_BULLETS];
int q_front = 0;
int q_rear = 0;

int playerX = 10;
int playerY = SCREEN_HEIGHT / 2;


//=========================================================
//  ▼▼ Aさんの担当処理 ▼▼
//=========================================================

void initA() {
    boss.x = SCREEN_WIDTH - 20;
    boss.y = SCREEN_HEIGHT / 2;
    boss.hp = 20;
    boss.alive = true;

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
        numbers[i].active = false;
    }
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].active = false;
    }
}

void saveNumber(int num) {
    if (savedIndex < MAX_SAVED_NUMBERS)
        savedNumbers[savedIndex++] = num;
}

// 障害物生成
void spawnObstacle() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].active = true;
            obstacles[i].x = SCREEN_WIDTH - 1;
            obstacles[i].y = getRandom(0, SCREEN_HEIGHT - 1);

            // 数字付きにする
            if (getRandom(0, 10) > 6) {
                numbers[i].active = true;
                numbers[i].value = getRandom(2, 99);
                numbers[i].x = obstacles[i].x;
                numbers[i].y = obstacles[i].y;

                saveNumber(numbers[i].value); // Bさん用に保存
            }
            return;
        }
    }
}

// ライフアイテム生成
void spawnLifeItem() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) {
            items[i].active = true;
            items[i].x = SCREEN_WIDTH - 1;
            items[i].y = getRandom(0, SCREEN_HEIGHT - 1);
            return;
        }
    }
}

// 更新（障害物・数字）
void updateObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            obstacles[i].x -= 1;
            if (obstacles[i].x < 0)
                obstacles[i].active = false;
            else
                drawObstacle(obstacles[i].x, obstacles[i].y);
        }

        if (numbers[i].active) {
            numbers[i].x -= 1;
            if (numbers[i].x < 0)
                numbers[i].active = false;
            else
                drawNumber(numbers[i].x, numbers[i].y, numbers[i].value);
        }
    }
}

void updateItems() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].active) {
            items[i].x -= 1;
            if (items[i].x < 0)
                items[i].active = false;
            else
                drawLifeItem(items[i].x, items[i].y);
        }
    }
}

void updateBoss() {
    if (boss.alive)
        drawBoss(boss.x, boss.y, boss.hp);
}



//=========================================================
//  ▼▼ Bさんの担当部分 ▼▼
//=========================================================

// 素因数分解 → 弾数戻す
int getBulletFromPrimeFactor(int n) {
    int bullet = 0;

    for (int i = 2; i * i <= n; i++) {
        while (n % i == 0) {
            bullet++;
            n /= i;
        }
    }
    if (n > 1) bullet++;
    return bullet;
}

// 数字一致判定
bool checkNumberMatch(int input) {
    for (int i = 0; i < savedIndex; i++) {
        if (savedNumbers[i] == input) return true;
    }
    return false;
}

void increaseLife(int amount) {
    playerLife += amount;
    if (playerLife > maxLife) playerLife = maxLife;
}

void decreaseLife(int amount) {
    playerLife -= amount;
    if (playerLife < 0) playerLife = 0;
}



//=========================================================
//  ▼▼ Cさんの担当部分 ▼▼
//=========================================================

// キュー
bool queueFull() { return ((q_rear + 1) % MAX_BULLETS) == q_front; }
bool queueEmpty() { return (q_front == q_rear); }

void enqueueBullet(int x, int y) {
    if (!queueFull()) {
        bulletQueue[q_rear].x = x;
        bulletQueue[q_rear].y = y;
        bulletQueue[q_rear].active = true;
        q_rear = (q_rear + 1) % MAX_BULLETS;
    }
}

// プレイヤー移動
void updatePlayer() {
    int adc = readKnobADC();  // 0〜1023と想定
    playerY = (adc * SCREEN_HEIGHT) / 1023;
    drawPlayer(playerX, playerY);
}

// 発射処理
void handleShoot() {
    if (readFireButton()) {
        enqueueBullet(playerX + 5, playerY);
    }
}

// 弾更新
bool bulletHitsBoss(int bx, int by) {
    return (bx >= boss.x && bx <= boss.x + 10 &&
            by >= boss.y && by <= boss.y + 10);
}

void updateBullets() {
    for (int i = q_front; i != q_rear; i = (i + 1) % MAX_BULLETS) {
        Bullet *b = &bulletQueue[i];
        if (b->active) {
            b->x += 3;

            if (b->x > SCREEN_WIDTH) {
                b->active = false;
            } else if (bulletHitsBoss(b->x, b->y)) {
                boss.hp--;
                b->active = false;
            }

            if (b->active)
                drawBullet(b->x, b->y);
        }
    }
}



//=========================================================
//  ▼▼ ABC統合：メインゲームループ ▼▼
//=========================================================

void gameLoop() {

    initA();

    while (1) {

        //-------------------------------------------------
        // A：生成（ランダム）
        //-------------------------------------------------
        if (getRandom(0, 100) > 97) spawnObstacle();
        if (getRandom(0, 200) > 198) spawnLifeItem();

        //-------------------------------------------------
        // C：プレイヤー
        //-------------------------------------------------
        updatePlayer();
        handleShoot();

        //-------------------------------------------------
        // C：弾の更新（→ボスのHP減少）
        //-------------------------------------------------
        updateBullets();

        //-------------------------------------------------
        // A：障害物・アイテム・ボス更新
        //-------------------------------------------------
        updateObstacles();
        updateItems();
        updateBoss();

        //-------------------------------------------------
        // B：プレイヤーの被弾 & ライフ処理
        //-------------------------------------------------
        // 障害物によるダメージ
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (obstacles[i].active &&
                obstacles[i].x == playerX &&
                obstacles[i].y == playerY) {

                decreaseLife(1);
                vibrateMotor(200);
                obstacles[i].active = false;
                numbers[i].active = false;
            }
        }

        // アイテム取得
        for (int i = 0; i < MAX_ITEMS; i++) {
            if (items[i].active &&
                items[i].x == playerX &&
                items[i].y == playerY) {

                increaseLife(1);
                items[i].active = false;
            }
        }

        //-------------------------------------------------
        // ゲーム終了条件
        //-------------------------------------------------
        if (boss.hp <= 0) {
            // ボス撃破 → クリア
            break;
        }
        if (playerLife <= 0) {
            // プレイヤー死亡 → ゲームオーバー
            break;
        }
    }
}

