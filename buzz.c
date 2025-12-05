// アドレス定義
#define ADDR_BUZZ ((volatile int *)0xff14)
void main() {
    // ド (C) の音を鳴らす
    *ADDR_BUZZ = 1; 
    // しばらく待つ (音が鳴り続ける)
    for(int i=0; i<1000000; i++);
    // レ (D) の音に変える (mode = 3)
    *ADDR_BUZZ = 3;
    for(int i=0; i<1000000; i++);
    // 音を止める
    *ADDR_BUZZ = 0;
}