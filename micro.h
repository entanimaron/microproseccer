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
    int timer;
};
int playGame();  //ゲームをする関数
void initGame();  //ゲームを初期化する関数
void initVariable();  //変数を初期化する関数
void movePlayer();  //playerの動きを定義する関数
void setBullet();  //弾をセットする関数
void moveBullet();  //弾の動きを決める関数
int setEnemy(int x, int y, int vx, int vy, char img, int life, int ptn, int wid, int hei);  //敵をセットする関数
void moveEnemy();  //敵(今は弾)の動きを決める関数
void setItem();  //アイテムをセットする感ん数
void moveItem();  //アイテムの動きを決める関数、ゲットした時の動作も決める
void drawImg(int x, int y, char img);  //imgを描画
void hitCheck();  //当たり判定を確認する関数
int myRand();  //rand関数
int createNum();  //合成数生成