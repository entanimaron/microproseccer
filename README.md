# 情報工学実験第2 マイクロプロセッサ実験
## 重要なお知らせ
まずは、「実験補足資料」に含まれる「マイクロプロセッサ実験の概要」をよく読んで欲しい。実験の内容、趣旨、実験に使うFPGAボード、製作するゲーム機の様子、ハードウェア・ソフトウェアの作業内容などが書かれている。 

理想的な実験スケジュール（3週間分）は以下の通り。5限までは延長可。

|週|時限|内容|
|:--|:--|:--|
|1週目|2時限|実験の概要説明。班分け。機材配布。チュートリアル1を実施。|
|     |3時限|チュートリアル2を実施。|
|     |4時限|液晶モニタ（LCD）を使えるようにする（班ごとに演習1、演習2を行う）。開発するゲームの外部仕様および内部仕様を検討。|
|2週目|2時限|先週に引き続き外部仕様および内部仕様を策定。教員とディスカッション。|
|     |3-4時限|FPGAボード上でゲーム開発を開始。LCDを使ったアニメーション、スイッチを使った当たり判定ができた班は解散。|
|3週目|2-3時限|FPGAボード上でゲームを完成させる。必要に応じて拡張ボードを追加。|
|     |4時限|ゲームが完成した班は、学生、教員、TAの前でデモ発表を行う。デモ発表後、レポート作成の指示を聞いたら解散。|

3週目の最後に、完成したゲーム機の発表会を行う。FPGAの実機ボードを用いてゲームを実演してもらい、ゲームの機能や面白さをアピールしてもらう。教員およびTAが審査委員となり、審査委員の合議によって各班の得点が決まる。

## チュートリアル1
製品やツールのひと通りの操作方法を説明したドキュメントのことを「チュートリアル」と呼ぶ。FPGAボード、測定機器、開発ツール、アプリケーションなどには通常のマニュアルに加え、チュートリアルが用意されている場合がある。 実際の開発現場では、まず、チュートリアルをこなして全体の流れを理解したうえで、詳細な使い方については適宜マニュアルを読むという流れが一般的である。

作業にはLinuxマシンを使用する。それでは実際にチュートリアルをやってみよう。

### サンプルコードとコンパイル
チュートリアル用のファイル一式を手元にコピーして解凍する。

~~~sh
> cp ~aa205875/2025/mips-20250719.tar.gz .
> tar zxvf mips-20250719.tar.gz
> cd mips
~~~

上記のaa205875は教員のアカウント名である。同じ班のメンバのアカウント名が分かればディレクトリを覗くことができる。班の中でファイルをやり取りする際に活用しよう。

含まれているファイルは以下の通り。

|ディレクトリ名|ファイル名|内容|
|:--|:--|:--|
|soft/||プロセッサ上で動作するプログラムはsoft/以下で開発する|
|     |Makefile|makeと打つとtest.cをコンパイルし、機械語データを生成する|
|     |test.01.c|プロセッサ上で動作するC言語プログラム（LED、SWITCHのテスト）|
|     |test.02.c|プロセッサ上で動作するC言語プログラム（LED、SWITCH、タイマーのテスト）|
|     |test.03.c|演習2（ソフトウェア）を参照|
|     |ChrFont0.h|液晶モニタ用フォント|
|     |crt0.c|プロセッサの初期化用コード（プログラムをコンパイルする際に必要）|
|     |mips.ld|リンカスクリプト（プログラムをリンクする際に必要）|
|hard/||プロセッサ、メモリ、I/O等のハードウェアモデル（Verilog HDL言語）|
|     |Makefile|makeと打つと論理合成、配置配線を行い、構成データを生成する|
|     |top.v|FPGA上に実装するハードウェア（mipsモジュールを呼び出す）|
|     |mips.v|MIPS R2000互換プロセッサ|
|     |parts.v|MIPS R2000互換プロセッサで使われている部品|
|     |multdiv.v|MIPS R2000互換プロセッサで使われている乗算器、除算器|
|     |fpga.xdc|topモジュールの入出力信号とFPGAの物理ピンとの対応付け|
|     |fpga_z7.xdc|topモジュールの入出力信号とFPGAの物理ピンとの対応付け（**新FPGAボード用**）|
|hard/script/||上記のMakefileの中で使われるスクリプト類|

プロセッサ上で動作するソフトウェアプログラムはsoft/以下で開発する。プロセッサ、メモリ、I/O等のハードウェアモデル（Verilog HDL言語で書かれている）はhard/以下にある。

次にC言語サンプルプログラムsoft/test.01.cの中身を確認して欲しい。
~~~sh
> cd soft
> cat test.01.c
~~~

~~~c
/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
void interrupt_handler(){}

void main() {
        volatile int *sw_ptr = (int *)0xff04;
        volatile int *led_ptr = (int *)0xff08;
        for (;;)
                *led_ptr = *sw_ptr;
}
~~~

先頭3行（正確には2行目と3行目）は消してはいけない。これらについては後述する。

0xff08番地はLEDの番地であり、ここに4-bitの値を書き込むと値の通りに4個のLEDが光る。

0xff04番地はSWITCHの番地であり、4個のスライドスイッチと4個の押しボタンの状態を8-bitの値として取得できる。対応するスイッチは下位ビットから順にスライドスイッチ4個、押しボタン4個である。スイッチがオンならば1、オフならば0が読み出される。押しボタン0と1を同時に押すと強制リセットがかかるようになっている（LEDは光らない）。

上記のプログラムではSWITCHの値8-bitをLEDに表示し続ける。SWITCHのオンオフによってLEDの点灯パターンが変化するので、実機で試してみよう。

このtest.01.cをコンパイルして機械語コードprogram.datを生成したい。このためのMakefileはコンパイル対象のソースファイルをtest.cとしているので、まず、test.01.cをtest.cにコピーする。「test.01.cをtest.cにコピーする」代わりにMakefileのSOURCEの部分を修正しても良い。 

~~~sh
> cp test.01.c test.c
~~~

次にmakeコマンドによってコンパイルする。コンパイルできたらlessコマンドでprogram.datの中身を確認しよう。中間ファイルprogram.dumpも参考になる。

~~~sh
> make
> less program.dat
> less program.dump
~~~

### 実機動作確認
ハードウェアおよびソフトウェアを実際のFPGAボード上で動作させるには、まず、ハードウェア（機械語コードprogram.datも含む）をFPGA向けに論理合成、配置配線し、構成データ（ビットストリーム）を得る。PCとFPGAボードをUSBケーブルでつなぎこの構成データをFPGAに焼き込む。

この操作にはXilinx社のVivadoというツールを使う。機能限定ライセンス（WebPACKライセンス）であればXilinx社のホームページから無料で取得できる。今回使用するZynq Z-7010など比較的小規模なFPGAであればWebPACK版で十分である。

以下のようにしてVivadoを起動する。

~~~sh
> vivado &
~~~

Vivadoを用いた論理合成、配置配線、構成データの焼き込みは最初はGUIベースで行う（CUIベースのやり方はチュートリアル2で扱う）。GUIの操作方法は写真を見たほうが分かりやすいので、「実験補足資料」に含まれる「Xilinx Vivadoの使い方」の通りに作業を進めて欲しい。 

既有のFPGAボードが生産中止になってしまったため、新しく調達したFPGAボードは仕様が若干異なる。お手元のボードに黄色の「新」シールが貼ってあったら「**新FPGAボード**」である。

## チュートリアル2
一通りの操作方法が分かったところで、今度は別のチュートリアルによってハードウェアとソフトウェアの詳細を見ていこう。

まず、サンプルプログラムsoft/test.02.cの中身を確認して欲しい。 

~~~c
/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
        static int cnt = 0;
        volatile int *led_ptr = (int *)0xff08;
        cnt++;
        if (cnt % 10 == 0)
                *led_ptr = cnt / 10;
}

void main() {
        for (;;);
}
~~~

### 割込み
プログラム中のinterrupt_handler()は100msec（0.1秒）に1回呼び出される。このような処理を割込みハンドラと呼ぶ。この割込みハンドラは10回に1回、つまり、1秒毎にLEDの値を1ずつインクリメントするので、実機で試してみよう。

これを実現するために、hard/top.vに以下のようなタイマー回路が実装されている。

~~~verilog
/* Timer module (@62.5MHz) */
timer timer (clk_62p5mhz, reset, irq);
~~~

100msecに1度だけirqという信号の値が1になる。irqはプロセッサに直接つながっていて、irqが1になるとプロセッサは予め決められた番地（本実験環境では0x0100番地）の命令を実行するようになっている。0x0100番地には上述のinterrupt_handler()を呼び出すためのコードが置いてある。詳細はsoft/crt0.cの__vector__:以下のコードを参照。

割込みハンドラの実行中は追加の割込みは禁止である。ユーザからの入力待ちなど時間のかかる処理を割込みハンドラの中で実行してはいけない。

### メモリ空間
上述の通り、0xff08番地はLEDの番地である。SWITCHもメモリ空間上の番地に割り当てられている。まずはメモリ空間について説明する。

FPGA上に実現したMIPS互換プロセッサは65,536 bytes（16,384 words）のメインメモリを内蔵している。メインメモリには、プログラムコードとデータが区別なく格納される。

スタックもメインメモリ上に実現され、C言語のローカル変数の使用または関数コールの度にスタック領域が使われる。スタックは上位番地（0xff00）からスタートして下位番地（0x0000）の方向へ伸びていく。

機械語コードprogram.datは、以下の通り、hard/top.vにてRAMに読み込まれる。RAMのサイズは16,384 wordsである。 

~~~verilog
/* Specify your program image file (e.g., program.dat) */
initial $readmemh("program.dat", RAM, 0, 16383);
~~~

実験で使うFPGAボードにはSWITCHやLEDなどのI/Oが接続されており、プロセッサのメモリ空間にマッピングされている。つまり、プロセッサからSWITCHの番地の値をロード（lw命令）すればSWITCHの状態を取得できる。プロセッサからLEDの番地にデータをストア（sw命令）すればその値の通りにLEDが点灯する。現時点ではまだ使えないが、決められた手順でLCDの番地にデータをストアすれば液晶モニタに文字や画像を表示できる。このようなI/O方式をメモリマップトI/Oと呼ぶ。

メモリ空間とその使途を以下にまとめる。メモリ番地は16進数で書かれている。実験では32-bitプロセッサを使っているため、32-bitのメモリ空間（4,294,967,296 bytes、つまり4 Gbytes）を扱えるが、本実験環境のメモリサイズは64 kbytesである。 

|番地|極性|内容|
|:--|:--|:--|
|0x00000000-0x0000ff00|Read/Write|メインメモリ（プログラムコード、データ、スタック領域）。サイズは16,384 words弱。|
|0x0000ff04|Read Only|SWITCHの番地。この番地の値をロードすると押しボタンおよびスライドスイッチの値を取得できる。下位8-bitの値は、0-bit目から順にスライドスイッチ4個、押しボタン4個に対応しており、スイッチがオンならば1、オフならば0。上位24-bitは常に0を返す。押しボタン0と1を同時に押すと強制リセットがかかるようになっている。|
|0x0000ff08|Write Only|LEDの番地。この番地に4-bitの値をストアすると、その2進数表記の通りに4個のLEDが点灯する。上位28-bitの値は無視される。|
|0x0000ff0c|Write Only|液晶モニタ（LCD）の番地。画面は96×64ピクセル。この番地に10-bitの値をストアすると各ピクセルに「色」が表示される。各ビットの意味は「LCD表示の基礎知識」で述べる。上位22-bitは無視される。|
|0x00010000-0xffffffff|N/A|未使用|

### VivadoのCUIモード
メモリ空間を理解したところ、さっそくtest.02.cをFPGAボード上で動かしてみたい。

チュートリアル2ではCUI（コマンドライン）ベースの開発方法を覚えよう。チュートリアル1で使用したGUIベースのツールは操作が直感的で分かりやすいという利点があるものの、ツールに慣れてくるとマウス操作を煩わしく感じることもある。操作に慣れたらCUIベースのツールが便利である。

まず、test.02.cをコンパイルして新しいprogram.datを作る。makeの前に前回のコンパイル結果を削除するためにmake cleanしておく。 

~~~sh
> cd soft
> cp test.02.c test.c
> make clean
> make
~~~

次に論理合成、配置配線を行って、構成データを生成する。

**新FPGAボード**の場合はhard/Makefileを以下のように書き換える。つまり、bitgen.tclの代わりにbitgen_z7.tclが呼ばれるように修正する。makeやvivadoコマンドの前の空白はスペースではなく、タブなので注意。

~~~
all:
	make clean
	#vivado -mode batch -source script/bitgen.tcl
	vivado -mode batch -source script/bitgen_z7.tcl
~~~

ここもmakeの前にmake cleanしておく。実行内容の詳細はhard/Makefileおよびhard/script/bitgen.tcl（**新FPGAボード**の場合はbitgen_z7.tcl）を参照。 

~~~sh
> cd hard
> make clean
> make
~~~

最後にPCとFPGAボードをUSBケーブルでつなぎ、FPGAボードの電源をオンにしたうえでこの構成データをFPGAに焼き込む。実行内容の詳細はhard/Makefileおよびhard/script/program.tclを参照。 

~~~sh
> make program
~~~

プログラムを開始するために、押しボタン0と1を同時に押してプロセッサをリセットしよう。

**注意: test.cを書き換える度にsoft/以下でmake clean、makeしてprogram.datを作り直し、そのあとhard/以下でmake clean、make、make programする必要がある。**

## 拡張I/Oボード
FPGAボード上にはSWITCHやLEDなどのI/Oが載っている。この後、液晶モニタ（LCD）も接続する。これだけでゲーム機を作ることもできるが、さらにI/Oを追加するとゲーム機っぽくなる。モグラ叩きの例ならば、モグラを叩いたらブザーが鳴る、ライフ（残基）数を追加のLEDに表示する、回転スイッチ（ロータリーエンコーダ）で速度を調整するなど。以下のPmodボードを貸与する。

* Pmod OLEDrgb: カラー液晶モニタ
* Pmod KYPD: 16ボタンのキーパッド
* Pmod BB: ブレッドボード（ブザーを実装済）
* Pmod 8LD: 8個の追加LED
* Pmod SWT: 4個の追加スライドスイッチ
* Pmod BTN: 4個の追加押しボタン
* Pmod ENC: ロータリーエンコーダ（スライドスイッチ、押しボタン付き）
* Pmod SSD: 2桁7セグメントLED

## ソフトウェア部分の基礎知識
本実験環境におけるソフトウェアの動作の仕組みを紹介する。各自、目を通しておいて欲しい。

### プログラム動作の仕組み
本実験環境では、メインメモリの0番地目に格納されている命令からプログラムの実行が開始される。C言語のmain()関数からではないことに注意されたい。

サンプルプログラム（test.01.cなど）では先頭でcrt0.cをインクルードしていたことを思い出して欲しい。crt0.cにはmain()が呼び出される前のスタートアップ処理がインラインアセンブラで記述されている。下記はcrt0.cを簡略化したもの。

~~~nasm
__start__:
        lui $sp, 0
        ori $sp, 0xff00
        li $gp, 0
        li $k0, 0x02000101
        mtc0 $k0, $12
	/* Initialize .data */
	/* Initialize .bss */
        j main
~~~

上記のluiやoriなどはMIPSプロセッサの命令である。詳細はWikipediaの[MIPSアーキテクチャ](https://ja.wikipedia.org/wiki/MIPSアーキテクチャ)を参照されたい。命令は32-bit長なので、0番地目、4番地目、8番地目、12番地目、...の順に命令が格納される。

まず、スタックポインタ（spレジスタ）の初期化（命令1と2）、gpレジスタの初期化（命令3）、CPUの動作モード設定（命令4と5）を行う。CPUの動作モード設定では割込みを許可している。

具体的なコードは省略しているが、その後、初期値を持つデータ領域や初期値を持たないデータ領域を初期化している（Initialize .data/.bssの部分）。最後に、j命令によってC言語のmain()関数へジャンプしている。

このようなmainの前に呼ばれる初期化ルーチンは「C Run Time start up file」（別名crt0）と呼ばれる。組込みシステムでは、一般的に、1) スタックポインタ、キャッシュ、メモリなど各種ハードウェアの初期化、2) 初期値を持つデータ領域の定数値での初期化、3) 初期値を持たないデータ領域の0初期化、4) main関数の呼び出しなどを行う。詳細はcrt0.cを見て欲しい。

### メモリアロケーション
本実験環境ではメインメモリは16,384 words分実装されている。ただし、実際にメモリとして利用可能な領域は0x0000番地から0xff00番地までなので16,384 wordsより若干少ない。メインメモリ上に以下の領域が確保される。 
* text領域: 機械語命令。
* data領域: 初期値を持つglobal変数、static変数。
* rodata領域: read-onlyなデータ。つまり定数。
* bss領域: 初期値を持たないglobal変数、static変数（プログラム起動時に0に初期化される）。
* stack領域: local変数。関数コールの度にメモリ空間末尾から先頭方向へ伸びる。

プログラムのリンク時に、リンカスクリプトで指定された通りにメモリアロケーションが行われる。下記はsoft/mips.ldを簡略化したもの（実物はもう少し複雑）。0x0000番地目からtext領域が始まり、その直後にrodata、data、bss領域が置かれる。スタックポインタはプログラム冒頭のインラインアセンブラでメインメモリの末尾に設定済みである。

~~~c
ENTRY(__start__)
SECTIONS {
        .text   : { *(.text); }
        .rodata : { *(.rodata); . = ALIGN(4); }
        .data   : { *(.data); . = ALIGN(4); }
        .bss    : { *(.*bss); . = ALIGN(4); }
}
~~~

### クロス開発ツールの使い方
C言語プログラム（test.c）がどのようにして機械語コード（program.dat）に変換されるか説明する。ここではsoft/Makefileに沿って説明していく。なお、本soft/MakefileではSOURCE=test.cとしている。ファイル名を変更したければSOURCEの値を書き換えれば良い。

まず、C言語プログラム（test.c）をコンパイルして、アセンブリ言語プログラム（program.asm）に変換する。コンパイルオプションはCFLAGSで定義している。最適化レベルは-O0（オー・ゼロ）としているので一切の最適化は行わない。

~~~make
        $(CC) $(CFLAGS) -c -S $(SOURCE) -o program.asm
~~~

次に、アセンブリ言語プログラム（program.asm）をオブジェクファイル（program.o）に変換する。オブジェクファイルは機械語で書かれた中間ファイルである。

~~~make
        $(AS) program.asm -o program.o
~~~

上述のリンカスクリプト（mips.ld）を用いてオブジェクファイル（program.o）をリンクし、実行ファイル（program.bin）を生成する。 

~~~make
        $(LD) -T mips.ld program.o -o program.bin
~~~

通常、実行ファイル（program.bin）はプロセッサ上でそのまま実行できる。ただし、今回はこの実行ファイルをVerilog HDLで書かれたmipsモジュールに組み込むという追加ステップが必要である。そこで、soft/Makefileでは実行ファイル（program.bin）を逆アセンブルして、text、data、rodata、bssなどの領域を16進数で書かれたテキストファイル（program.dump）に保存している。

このために逆アセンブル（objdump）や簡単なテキスト処理（grep、tr、awk）を行っている。逆アセンブルの部分を以下に示すが、残りのテキスト処理の部分は少々込み入っているので、興味がある人はsoft/Makefileを確認されたい。 

~~~make
        $(DUMP) -D --disassemble-zeroes program.bin > program.dump
~~~

ここまで読んだらtest.c、program.asm、program.dump、program.datをよく確認し、機械語コード生成の過程を理解して欲しい。

## ハードウェア部分の基礎知識
本実験環境において各種I/Oを制御するための周辺回路の仕組みを紹介する。各自、目を通しておいて欲しい。

### SWITCHとLEDの制御回路の解説
FPGAチップからSWITCH、LED、LCDなどのI/Oを制御するには、FPGAチップの物理ピン番号と各種I/Oの対応付けが必要である。この対応付けを管理するのが制約ファイル（Xilinx Design Constraints File、略してXDC）である。hard/fpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）を確認してみよう。 

~~~sh
> cd hard
> less fpga.xdc
~~~

~~~tcl
##LEDs          M14 M15 G14 D18
set_property PACKAGE_PIN M14 [get_ports {led[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[0]}]
set_property PACKAGE_PIN M15 [get_ports {led[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[1]}]
set_property PACKAGE_PIN G14 [get_ports {led[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[2]}]
set_property PACKAGE_PIN D18 [get_ports {led[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[3]}]
...
~~~

上記は4個のLEDとFPGAの物理ピンの対応付けを定義している。FPGAの各物理ピンは「M14」のように、アルファベットと数字を組み合わせた名前が付いている。例えば、FPGAのG14ピンはLEDの3つ目（led[2]）につながっている。FPGAの各物理ピンの先に何がつながっているかはFPGAボードのマニュアルに書いてある。

同様に、4個のスライドスイッチ（sw[3:0]）、4個の押しボタン（btn[3:0]）、LCD（lcd[7:0]）、クロック（clk_125mhz）についてもfpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）を確認しよう。他にも、8-bit汎用入出力ポートA、B（ioa[7:0]、iob[7:0]）も実装されているが、これらについては後述する。

次に、fpga_topモジュールの入出力ポートを確認しよう。fpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）に書かれていた信号名（ledなど）がfpga_topの入出力ポートとして宣言されている。 

~~~sh
> less top.v
~~~

~~~verilog
module fpga_top (
        input                   clk_125mhz,
        input           [3:0]   sw,
        input           [3:0]   btn,
        output  reg     [3:0]   led,
        output          [7:0]   lcd,
        output  reg     [7:0]   ioa,
        output  reg     [7:0]   iob
);
~~~

SWITCHとLEDの制御回路を紹介する前に、まず、mipsプロセッサの入出力ポートを説明する。hard/top.vの続きを見て欲しい。 

~~~verilog
/* CPU module (@62.5MHz) */
mips mips (clk_62p5mhz, reset, pc, instr, {7'b0000000, irq}, memwrite,
        memtoregM, swc, byteen, dataadr, writedata, readdata, 1'b1, 1'b1);
~~~

主要な入出力ポートの使途を以下にまとめる。 

* clk_62p5mhz: 入力ポート（1-bit）。FPGAボードから供給される125MHzクロック（clk_125mhz）を半分の周波数に分周してプロセッサに与える。
* pc: 出力ポート（32-bit）。プログラムカウンタ。命令を読み出すためのメモリ番地を出力。
* instr: 入力ポート（32-bit）。pcで指定したメモリ番地から読み出したMIPS R2000命令。
* irq: 入力ポート（1-bit）。タイマー回路からの割込み信号。100msecに1回だけ1になる。
* memwrite: 出力ポート（1-bit）。メインメモリもしくはI/Oに値をストアする際は1を出力。それ以外は0を出力。
* dataadr: 出力ポート（32-bit）。ストア対象のメモリ番地を出力。
* writedata: 出力ポート（32-bit）。ストアする書き込みデータ。
* readdata: 入力ポート（32-bit）。メインメモリもしくはI/Oから読み出したデータの値。

メモリマップトI/Oでは、dataadrを基にロード/ストア対象がメインメモリなのか、SWITCHなのか、LEDなのか、LCDなのか判別する必要がある。そのために、ここでは、ロード/ストア対象がメインメモリの場合はcs0信号を1に、SWITCHの場合はcs1信号を1に、LEDの場合はcs2信号を1に、LCDの場合はcs3信号を1にしようと思う。このような判定を行う回路をアドレスデコーダと呼ぶ。以下にアドレスデコーダの例を示す。 

~~~verilog
/* Memory(cs0), Switch(cs1), LED(cs2), LCD(cs3), and more ... */
assign  cs0     = dataadr <  32'hff00;
assign  cs1     = dataadr == 32'hff04;
assign  cs2     = dataadr == 32'hff08;
...
~~~

SWITCH（cs1）の値をロードするための回路は以下の通りである。8-bitのSWITCHの値（swとbtn）がreaddata1に継続代入されている（readata1の上位24-bitは0に固定）。cs1が1のとき、readdata1の値がプロセッサに与えられる。なお、readdata0はメインメモリから読み出した値であり、cs0が1のときはreaddata0の値がプロセッサに与えられる。 

~~~verilog
assign  readdata        = cs0 ? readdata0 : cs1 ? readdata1 : 0;
...
/* cs1 */
assign  readdata1       = {24'h0, btn, sw};
~~~

LED（cs2）に値をストアするための回路は以下の通りである。プロセッサからの書き込み信号（memwrite）が1、かつ、cs2が1のとき、プロセッサからの書き込みデータの下位4-bit（writedata[3:0]）をledに書き込む。 

~~~verilog
/* cs2 */
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      led     <= 0;
        else if (cs2 && memwrite)       led     <= writedata[3:0];
~~~

## LCD表示の基礎知識
本実験環境においてLCDに文字等を表示するための方法を紹介する。各自、目を通しておいて欲しい。

### LCDの制御方法の概要
チュートリアル2で示した通り、LCDのメモリ番地は0xff0cである。この番地は書き込み専用であり、この番地に10-bitの値をストアすると各ピクセルに「色」が表示される。LCDに送る値は0から7-bit目までの8-bitで指定。8-bit目と9-bit目は内部的な制御に使用。上位22-bitは無視される。

各ビットの意味を以下にまとめる。 

|信号|説明|
|:--|:--|
|lcd[7:0]|LCDに送る8-bitの値。|
|lcd[8]|内部的な制御信号。lcd[8]が0ならlcd[7:0]はコマンドとして扱われ、1ならデータとして扱われる。|
|lcd[9]|内部的な制御信号。lcd[9]が1ならLCDの電源をオンにする。通常は0にしておく。|

LCDの利用手順は下記の通り。起動時にLCDの初期化を行わなければならない。 

1. LCDの初期化。まず、LCDの電源をオンにする。次にLCDの色数（階調）や表示範囲の設定などを行う。色数は256（8-bit）としている。
2. ビデオバッファへの書き込み。画面は96×64ピクセルあり、1ピクセルあたり8-bitの色情報を格納する。つまり、ビデオバッファは6,144 bytes。
3. LCDへの書き込み。6,144 bytes分のビデオバッファを8-bitずつLCDに送る。

LCDの初期化は1度のみ実行。初期化後は、ビデオバッファへの書き込みとLCDへの書き込みを繰り返す。

### LCDの低レベル関数
LCDを制御するためのC言語関数は下記の通り。

- lcd_wait(int n): 待ちを入れる。
- lcd_cmd(unsigned char cmd): LCDに8-bitのコマンドを送る。
- lcd_data(unsigned char data): LCDに8-bitのデータを送る。
- lcd_pwr_on(): LCDの電源をオンにする。
- lcd_init(): LCDの初期化を行う。

~~~c
void lcd_wait(int n) {
        for (int i = 0; i < n; i++);
}
~~~

lcd_wait()では「空のforループ」を使って時間を稼いでいる。プロセッサの動作周波数は62.5MHzなので1命令当たり16nsecかかる。lcd_wait()中のforループは、ループ1回あたり9命令の処理（lwからnopまで）に翻訳されるようなので、forループ1回あたり144nsecかかる。この特徴を利用して「待ち」を作っている。

~~~c
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
~~~

lcd[9:0]の各ビットの意味のところで述べたとおり、lcd[8]が0ならlcd[7:0]はコマンドとして扱われ、1ならデータとして扱われる。lcd_data()において、dataと0x100の論理和を取ってからLCDの番地に書き込んでいるのはこのためである。

同様に、lcd[9]はLCDの電源をオンにするためのビットである。lcd_pwr_on()ではLCDの番地に0x200を書き込んでいる。その後、約100msecの待ちを入れている。

~~~c
void lcd_init() {
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
~~~

まず、lcd_pwr_on()を呼び出している。

次の2行ではコマンド0xa0の後、0x20を書き込んでいる。具体的には色数（階調）を指定している。色数は256（8-bit）や65k（16-bit）が選べるが、ここでは256にしている。256色の場合、Rは3-bit、Gは3-bit、Bは2-bitである。

次の3行ではコマンド0x15の後、0と95を書き込んでいる。これは横方向の表示範囲として0から95を指定している。次の3行ではコマンド0x75の後、0と63を書き込んでいる。これは縦方向の表示範囲として0から63を指定している。

最後にコマンド0xafを書き込んでいる。これでLCDの表示がオンになる。

### ビデオバッファ用関数
ビデオバッファを操作するためのC言語関数は下記の通り。

- lcd_set_vbuf_pixel(int row, int col, int r, int g, int b): rowとcolで指定したピクセルの色（RGB）を設定する。
- lcd_clear_vbuf(): ビデオバッファの値をすべて0にする。
- lcd_sync_vbuf(): ビデオバッファの値によって画面を更新する。
- lcd_putc(int y, int x, int c): ビデオバッファの指定した番地に、cで指定した文字コードのドットパターンを設定する。
- lcd_puts(int y, int x, char *str): lcd_putc()の文字列バージョン。

~~~c
unsigned char lcd_vbuf[64][96];
void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
        r >>= 5; g >>= 5; b >>= 6;
        lcd_vbuf[row][col] = ((r << 5) | (g << 2) | (b << 0)) & 0xff;
}
void lcd_clear_vbuf() {
        for (int row = 0; row < 64; row++)
                for (int col = 0; col < 96; col++)
                        lcd_vbuf[row][col] = 0;
}
~~~

lcd_set_vbuf_pixel()ではRGBはそれぞれ8-bitで与えるが、関数内でRは3-bit、Gは3-bit、Bは2-bitに変換している。
一方、lcd_clear_vbuf()ではRGBをすべて0にしている。つまり、黒。

~~~c
void lcd_sync_vbuf() {
        for (int row = 0; row < 64; row++)
                for (int col = 0; col < 96; col++)
                        lcd_data(lcd_vbuf[row][col]);
}
~~~

lcd_sync_vbuf()ではビデオバッファの値をすべてLCDに転送している。これによって画面が更新される（これを呼び出さないと画面は更新されない）。

~~~c
void lcd_putc(int y, int x, int c) {
        for (int v = 0; v < 8; v++)
                for (int h = 0; h < 8; h++)
                        if ((font8x8[(c - 0x20) * 8 + h] >> v) & 0x01)
                                lcd_set_vbuf_pixel(y * 8 + v, x * 8 + h, 0, 255, 0);
}
void lcd_puts(int y, int x, char *str) {
        for (int i = x; i < 12; i++)
                if (str[i] < 0x20 || str[i] > 0x7f)
                        break;
                else
                        lcd_putc(y, i, str[i]);
}
~~~

lcd_putc()ではビデオバッファを12×8マスとみなし、y（行）とx（列）で指定したマスに、cで指定した文字コードのドットパターンを設定している。
文字色は緑を指定している。文字色を変えたければRGBの値を変更する。

lcd_puts()では指定したマスにstrで指定した文字列のドットパターンを設定している。
画面に表示した文字列を消すときはlcd_clear_vbuf()とlcd_sync_vbuf()を実行する。

## 演習1（ハードウェア）
「ハードウェア部分の基礎知識」で紹介したSWITCHおよびLED回路を参考に、LCD制御回路を作ろう。

LCDを制御するための物理ピンは8本（8-bit）である。内部的に使用しているビットも含めると10-bitになる（各ビットの意味は「LCD表示の基礎知識」を参照）。

LCDを制御するための物理ピン番号は、すでにhard/fpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）に書かれているので修正は不要である。参考までに該当箇所を以下に示す。

~~~tcl
##Pmod Header JA (XADC) N15 L14 K16 K14 N16 L15 J16 J14
set_property PACKAGE_PIN N15 [get_ports {lcd[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[0]}]
set_property PACKAGE_PIN L14 [get_ports {lcd[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[1]}]
set_property PACKAGE_PIN K16 [get_ports {lcd[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[2]}]
set_property PACKAGE_PIN K14 [get_ports {lcd[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[3]}]
set_property PACKAGE_PIN N16 [get_ports {lcd[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[4]}]
set_property PACKAGE_PIN L15 [get_ports {lcd[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[5]}]
set_property PACKAGE_PIN J16 [get_ports {lcd[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[6]}]
set_property PACKAGE_PIN J14 [get_ports {lcd[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {lcd[7]}]
~~~

演習1（ハードウェア）ではhard/top.vを修正する。まず、fpga_topモジュールの中にcs3のためのアドレスデコーダを実装する。具体的には以下を追記する。
~~~verilog
assign  cs3     = dataadr == 32'hff0c;
~~~

LCDへのデータ転送はSPI（Serial Peripheral Interface）という規格に沿って行う。SPIの仕様はここでは説明しないが、SPIでデータ転送を行うための専用回路がhard/top.v中に実装してあるので、それを使う。
具体的には、fpga_topの中に以下を追記する。
~~~verilog
/* SPI module (@62.5MHz) */
spi spi (clk_62p5mhz, reset, cs3 && memwrite, writedata[9:0], lcd);
~~~
これによって、cs3とmemwriteが有効であるとき、10-bitのwritedataの値を、SPIという規格に沿ってlcdに出力できるようになる。

拡張ボードは8-bit汎用入出力ポートA、B、C、Dに挿入して使う。fpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）についてはioa[7:0]、iob[7:0]、ioc[7:0]、iod[7:0]はすでに定義されているので修正は不要である。一方、hard/top.vについては適宜修正する必要がある。サンプルコードではioa[7:0]とiob[7:0]は出力ポート扱いとなっているので、これらを入力ポートとして使いたければ「output reg」を「input」に変更する。必要に応じて、入力もしくは出力ポートとしてioc[7:0]やiod[7:0]も追加する。アドレスデコーダとしてはcs4、cs5、...を割り当てると良い。入力ポートとして使用する場合はcs1、出力ポートとして使用する場合はcs2の回路を参考にすると良い。以下に参考例を示す。

~~~verilog
module fpga_top (
        input                   clk_125mhz,
        input           [3:0]   sw,
        input           [3:0]   btn,
        output  reg     [3:0]   led,
        output          [7:0]   lcd,
        //output reg    [7:0]   ioa,
        input           [7:0]   ioa,
        output  reg     [7:0]   iob
);
wire    [31:0]  pc, instr, readdata, readdata0, readdata1, readdata4, writedata, dataadr;
...
assign  readdata        = cs0 ? readdata0 : cs1 ? readdata1 : cs4 ? readdata4 : 0;
...
/* cs4 */
//always @ (posedge clk_62p5mhz or posedge reset)
//      if (reset)                      ioa     <= 0;
//      else if (cs4 && memwrite)       ioa     <= writedata[7:0];
assign  readdata4       = {24'h0, ioa};
~~~

## 演習2（ソフトウェア）
押しボタン、LED、LCDを使うゲームプログラムを実機上で動かしてみよう。ソースコードを読みながらこれはどのようなゲームなのか考えてみよう。

LCD関連の関数は空欄になっているが、ここは「LCD表示の基礎知識」で解説したコードを適宜コピーしてこよう。
実機で上記のコードを試すには、演習1（ハードウェア）の通りhard/top.vを修正したうえで、Xilinx Vivadoを用いた論理合成からやり直す必要がある。

実機に焼き込んだ後は押しボタン0と1を同時に押してプロセッサをリセットしよう。実機で動作を確認できたら、自分たちが作るゲームの構想を練って欲しい。

~~~c
/* Do not remove the following line. Do not remove interrupt_handler(). */
#include "crt0.c"
#include "ChrFont0.h"

void show_ball(int pos);
void play();
int  btn_check_0();
int  btn_check_1();
int  btn_check_3();
void led_set(int data);
void led_blink();
void lcd_init();
void lcd_putc(int y, int x, int c);
void lcd_sync_vbuf();
void lcd_clear_vbuf();

#define INIT    0
#define OPENING 1
#define PLAY    2
#define ENDING  3

int state = INIT, pos = 0;

/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
        static int cnt;
        if (state == INIT) {
        } else if (state == OPENING) {
                cnt = 0;
        } else if (state == PLAY) {
                /* Display a ball */
                pos = (cnt < 12) ? cnt : 23 - cnt;
                show_ball(pos);
                if (++cnt >= 24) {
                        cnt = 0;
                }
        } else if (state == ENDING) {
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
                } else if (state == PLAY) {
                        play();
                        state = ENDING;
                } else if (state == ENDING) {
                        state = OPENING;
                }
        }
}
void play() {
        while (1) {
                /* Button0 is pushed when the ball is in the left edge */
                if (pos == 0 && btn_check_0()) {
                        led_blink();    /* Blink LEDs when hit */
                /* Button3 is pushed when the ball is in the right edge */
                } else if (pos == 11 && btn_check_3()) {
                        led_blink();    /* Blink LEDs when hit */
                } else if (btn_check_1()) {
                        break;          /* Stop the game */
                }
        }
}
void show_ball(int pos) {
        lcd_clear_vbuf();
        lcd_putc(3, pos, '*');
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
        led_set(0xf);                           /* Turn on */
        for (int i = 0; i < 300000; i++);       /* Wait */
        led_set(0x0);                           /* Turn off */
        for (int i = 0; i < 300000; i++);       /* Wait */
        led_set(0xf);                           /* Turn on */
        for (int i = 0; i < 300000; i++);       /* Wait */
        led_set(0x0);                           /* Turn off */
}
/*
 * LCD functions
 */
unsigned char lcd_vbuf[64][96];
void lcd_wait(int n) {
	/* Not implemented yet */
}
void lcd_cmd(unsigned char cmd) {
	/* Not implemented yet */
}
void lcd_data(unsigned char data) {
	/* Not implemented yet */
}
void lcd_pwr_on() {
	/* Not implemented yet */
}
void lcd_init() {
	/* Not implemented yet */
}
void lcd_set_vbuf_pixel(int row, int col, int r, int g, int b) {
	/* Not implemented yet */
}
void lcd_clear_vbuf() {
	/* Not implemented yet */
}
void lcd_sync_vbuf() {
	/* Not implemented yet */
}
void lcd_putc(int y, int x, int c) {
	/* Not implemented yet */
}
void lcd_puts(int y, int x, char *str) {
	/* Not implemented yet */
}
~~~

## ディスカッション
教員とのディスカッションは2週目の午前中に行う。班の中でゲーム機の構想を練ったうえで、適当な紙に外部仕様と内部仕様をまとめる。ディスカッションでは、まず外部仕様と内部仕様が書かれた紙を提出し、計画について口頭でプレゼンする。

### 内部仕様と外部仕様
外部仕様: ユーザ向けの仕様。ゲームのルールや開発する製品がどのような操作に対してどのように動くかを詳細に決める。設計者の間で動き方の理解に違いがあるとバグの原因になる。例えば、「〇〇ボタンを押すとゲームがスタートする」「〇〇ボタンを押すとモグラを叩く」「LCDの上段にモグラを表示する」「LEDに残機数を表示する」「残機が0になるとゲームオーバー」「ヒットすると振動する」等々。箇条書きで良い。

内部仕様: どのように実装するか、中身の仕様。データ構造やプログラムの流れなどソフトウェアの仕様を書く。データ構造については、どのようなC言語変数（例えば、スコア、残機数、速度）を使うか決める。プログラムの流れについては、下記のリンクを参考に疑似コード、もしくは、フローチャート（流れ図）を描いて説明する。例えば、「スタートボタンの入力待ち」→「モグラを表示」→「叩くボタンの入力待ち」→「ヒットしたらスコアをインクリメント、ミスしたら残機をデクリメント」→「残機が0ならばゲームオーバー、そうでなければモグラ表示に戻る」等々。計画した機能が実装できるかどうかの自己確認にもなるので、プログラムの流れ図は可能な限り詳細に描くこと。 

### 理解度を確認するための質問
実験で使っているシステムにはOSもスレッドライブラリもない。このようなシステムにおいて、例えばテニスゲームならば、「画面のボールを動かしつつ、ユーザからの入力を待ち、もし入力があればラケットを振る」という同時処理が必要である。

これを実現するにはどのようなプログラムを書けば良いだろうか？自分の言葉で説明して欲しい。

### ヒント
以下のプログラムを考えて欲しい。

~~~c
/* interrupt_handler() is called every 100msec */
void interrupt_handler() {
        ボールの位置を動かす処理;
        画面を更新する処理;
}
void main() {
        初期化;
        while (1) {
                if (ボタン0が押された) {
                        ボタンが0が押されたときの処理;
                } else if (ボタン1が押された) {
                        ボタンが1が押されたときの処理;
                }
        }
}
~~~

画面として96×64ピクセルのLCDを使い、例えばlcd_putc(y, x, '*')によって画面にボールを表示する。

一定時間間隔ごとに、「ボールの位置を動かす処理」としてビデオバッファの値を更新する。
例えば、ある時刻にy=3、x=5のマスにボールが表示されているとする。100msec後にlcd_clear_vbuf()したうえで、y=3、x=6のマスにlcd_putc(y, x, '*')すれば、ボールが右に1マス移動したことになる。
そのうえで「画面を更新する処理」としてlcd_sync_vbuf()を呼び出す。

「ボタン0が押されたときの処理」はボールの位置（y、x）に応じてラケットにボールが当たったかどうかの当たり判定を行う。「ボタン1が押されたときの処理」も大体一緒である。 

必ずしもこのように作る必要はないが、このようにすることで「画面のボールを動かしつつ、ユーザからの入力を待ち、もし入力があればラケットを振る」という同時処理を実現できる。詳細は「演習2（ソフトウェア）」を読んで欲しい。

## 最終レポートに関する注意
最終レポートの内容は内部仕様と外部仕様である。2週目のディスカッションのときよりも詳細に書くこと。分量の目安はA4サイズ4ページ以上。内容が少ないとプロジェクトへの貢献が少ないと判断され、不当に低い点数になってしまうので注意。自分の貢献を積極的にアピールして欲しい。

外部仕様はA4サイズ1ページ以上。ゲームのルール（スコアの加点条件、ゲームの終了条件等）、画面の見方（スコア、残機数等）、スイッチ/ボタンの意味（スタートボタン、叩くボタン等）など。画面の見方やスイッチ/ボタンの意味は図や実機の写真を使って説明すると良い。内容は班の中で共通化して良い。

内部仕様の1ページ目には詳細なフローチャートを入れる。プログラム中の主要な関数は網羅して欲しい（貢献として主張するほどでもない細かい関数は含めなくて良い）。フローチャート自体は班の中で共通化して良いが、自分が貢献した部分についてはフローチャートに色付けするなどして強調する。フローチャートが大雑把過ぎて各自の貢献が判断できない場合は点数が付かない。なお、色付けする箇所が班の中で被っても良い。複数人で協力して1つのモジュールを実装することはよくある事である。

内部仕様の2ページ目以降は、自分が貢献した部分（フローチャートに色付けした箇所）を詳細に解説して欲しい（A4サイズ2ページ以上）。ソースコードもしくは疑似コードを適宜含めて、自分の貢献を積極的にアピールして欲しい。ハードウェア実装を頑張った場合はVerilog HDLコードを含めて説明すると良い。拡張ボードの実装に力を注いだ場合は拡張ボードの写真もレポートに含めると良い。繰り返しになるが、内容が少ないとプロジェクトへの貢献が少ないと判断され、不当に低い点数になってしまうので注意。

締切までにレポートを提出すること。遅れると減点もしくは受け取ってもらえない。実験アンケートには必ず回答すること。

## 付録A: ハードウェア記述言語Verilog HDL
ハードウェア記述言語Verilog HDLは、近藤先生の計算機構成同演習（B2秋学期）、コンピュータアーキテクチャ（B3春学期）で扱っている。

実験中にVerilog HDLの文法を確認したくなったら、授業のホームページにある「Verilog HDL入門」を適宜参照して欲しい。Verilog HDLに触れるのは今回が初めてという人は一通り読んでおいたほうが良い。

Verilog HDLシミュレーションにはIcarus Verilogを使用する。テストベンチのファイル名をadd_test.v、回路のファイル名をadd.vとするとき、以下のコマンドによってVerilog HDLソースがコンパイルされる。Verilog HDLファイルが複数個ある場合はすべて指定すること。テストベンチはファイルリストの最初に指定する。

~~~sh
> iverilog test_add.v add.v
~~~

実際のシミュレーションは以下のように実行する。 

~~~sh
> vvp a.out
~~~

iverilogコマンドのオプションは以下の通り。 

* -h : ヘルプを表示。
* -o <ファイル名> : 出力ファイル名を指定。デフォルトではa.out。
* -I <パス名> : インクルードパスに追加するディレクトリを指定。
* -D <マクロ名>(=<値>) : defineを追加。値はなくても良い。

テストベンチに以下のような記述を入れておくと、シミュレーション中の信号変化を波形として生成できる。例えば、sim/test.vなどを参照。この場合、test.fpgaというインスタンスの信号変化を、dump.vcdという名前のファイルに記録する。

~~~verilog
initial begin
       $dumpfile("dump.vcd");
       $dumpvars(0, test.fpga);
end
~~~

波形ビューワ（GTKWave）を起動するには以下のように実行する。

~~~sh
> gtkwave dump.vcd &
~~~

## 付録B: プログラミング言語C

演算子とその優先順位を以下にまとめる。文法については、市販の教科書もしくはウェブ等を参照。 

|優先順位|演算子|
|:--|:--|
|1|( )　　[ ]　　間接メンバアクセス ->　　直接メンバアクセス .　　後置型インクリメント ++　　後置型デクリメント --|
|2|論理否定 !　　ビット否定 ~　　前置型インクリメント ++　　前置型デクリメント --　　ポインタ演算子 *　　ポインタ演算子 &　　sizeof|
|3|キャスト (型)|
|4|乗算 *　　除算 /　　剰余 %|
|5|足し算 +　　引き算 -|
|6|左シフト <<　　右シフト >>|
|7|関係演算子 <　　<=　　>　　>=|
|8|等値演算子 ==　　!=|
|9|ビット積 &|
|10|ビット排他的論理和 ^|
|11|ビット和 \| |
|12|論理積 &&|
|13|論理和 \|\| |
|14|条件演算子 ?:|
|15|代入演算子 =　　+=　　-=　　\*=　　/=　　%=　　&=　　|=　　^=　　<<=　　>>=|
|16|カンマ演算子 ,|

## 付録C: 便利なサンプルコード集 
以下のサンプルコードでは省略しているが、プログラムの先頭にcrt0.cやChrFont0.hのインクルード、及び、interrupt_handler()が必要。

### 数値（スコア等）をLCDに表示したい
以下のlcd_digit3()は引数で与えた3桁の数字をLCDに表示する関数である。 

~~~c
void lcd_digit3(int y, int x, unsigned int val) {
       int digit3, digit2, digit1;
       digit3 = (val < 100) ? ' ' : ((val % 1000) / 100) + '0';
       digit2 = (val <  10) ? ' ' : ((val %  100) /  10) + '0';
       digit1 = (val %  10) + '0';
       lcd_putc(y, x + 0, digit3);
       lcd_putc(y, x + 1, digit2);
       lcd_putc(y, x + 2, digit1);
}
void main() {
       lcd_init();
       lcd_clear_vbuf();
       lcd_digit3(0, 0, 9876);
       lcd_digit3(1, 0, 432);
       lcd_digit3(2, 0, 65);
       lcd_digit3(3, 0, 7);
       lcd_sync_vbuf();
}
~~~

### ブザーで音階を作りたい（ソフトウェア版）
8音から成る音階をソフトウェアで作ってみた。出来はあまり良くないので各自で調整して欲しい。

ここではブザーは汎用入出力ポートBにつながっているものとする。入出力ポートBの番地は0xff14としているが、班によっては別の用途で既に使われている可能性がある。必要に応じて修正して欲しい。 

~~~c
void beep(int mode) {
        int len;
        volatile int *iob_ptr = (int *)0xff14;
        switch (mode) {
        case 1: len = 13304; break;
        case 2: len = 11851; break;
        case 3: len = 10554; break;
        case 4: len =  9949; break;
        case 5: len =  8880; break;
        case 6: len =  7891; break;
        case 7: len =  7029; break;
        case 8: len =  6639; break;
        }
        *iob_ptr = 1;
        lcd_wait(len);
        *iob_ptr = 0;
        lcd_wait(len);
}
void main() {
        while (1) {
                for (int i = 0; i < 250; i++) beep(1);
                for (int i = 0; i < 250; i++) beep(2);
                for (int i = 0; i < 250; i++) beep(3);
                for (int i = 0; i < 250; i++) beep(4);
                for (int i = 0; i < 250; i++) beep(5);
                for (int i = 0; i < 250; i++) beep(6);
                for (int i = 0; i < 250; i++) beep(7);
                for (int i = 0; i < 250; i++) beep(8);
        }
}
~~~

しかし、このソフトウェア版には音を鳴らしている最中に他の処理ができないという致命的な欠点がある。例えば、音を鳴らしている最中にユーザからのボタン入力待ちをすることはできない。この問題は次のハードウェア版で解決する。

### ブザーで音階を作りたい（ハードウェア版）
13音から成る音階のハードウェア版である。top.vに修正が必要であるが、先にプログラムがどのようになるかを示しておく。汎用入出力ポートBの番地（ここでは0xff14）に音階を書き込んだ後は別の処理ができる。 

~~~c
void main() {
        volatile int *iob_ptr = (int *)0xff14;
        *iob_ptr = 1; lcd_wait(7000000);
        *iob_ptr = 2; lcd_wait(7000000);
        *iob_ptr = 3; lcd_wait(7000000);
        *iob_ptr = 4; lcd_wait(7000000);
        *iob_ptr = 5; lcd_wait(7000000);
        *iob_ptr = 6; lcd_wait(7000000);
        *iob_ptr = 7; lcd_wait(7000000);
        *iob_ptr = 8; lcd_wait(7000000);
        *iob_ptr = 9; lcd_wait(7000000);
        *iob_ptr = 10; lcd_wait(7000000);
        *iob_ptr = 11; lcd_wait(7000000);
        *iob_ptr = 12; lcd_wait(7000000);
        *iob_ptr = 13; lcd_wait(7000000);	
}
~~~

このためにhard/top.vにブザーを制御するためのbeepモジュールを追加する。beepモジュールはfpga_topモジュールのendmoduleの後に追加すると良い。assign文中の定数は、TAさんが何度もブザー音を聞きながらドレミファソラシドに近づけたものである。

~~~verilog
module beep (
       input clk_62p5mhz,
       input reset,
       input [7:0] mode,
       output buzz
);
reg  [31:0] count;
wire [31:0] interval;
assign interval =      (mode ==  1) ? 14931 * 2: /* C  */
                       (mode ==  2) ? 14093 * 2: /* C# */
                       (mode ==  3) ? 13302 * 2: /* D  */
                       (mode ==  4) ? 12555 * 2: /* D# */
                       (mode ==  5) ? 11850 * 2: /* E  */
                       (mode ==  6) ? 11185 * 2: /* F  */
                       (mode ==  7) ? 10558 * 2: /* F# */
                       (mode ==  8) ?  9965 * 2: /* G  */
                       (mode ==  9) ?  9406 * 2: /* G# */
                       (mode == 10) ?  8878 * 2: /* A  */
                       (mode == 11) ?  8380 * 2: /* A# */
                       (mode == 12) ?  7909 * 2: /* B  */
                       (mode == 13) ?  7465 * 2: /* C  */
		       0;
assign buzz = (mode > 0) && (count < interval / 2) ? 1 : 0;
always @ (posedge clk_62p5mhz or posedge reset)
       if (reset)
               count   <= 0;
       else if (mode > 0)
               if (count < interval)
                       count   <= count + 1;
               else
                       count   <= 0;
endmodule
~~~

fpga_topモジュール中からこのbeepモジュールにアクセスしたい。そのためにfpga_topモジュール中にbeepモジュールのインスタンスを追加する。さらにbeepモジュールが使用するregやwireも追加する。 

~~~verilog
reg     [7:0]   mode;
wire            buzz;
beep beep (clk_62p5mhz, reset, mode, buzz);
~~~

汎用入出力ポートBがcs5空間に割り当てられている場合、modeおよびiobの制御は次のようになる。 

~~~verilog
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      mode    <= 0;
        else if (cs5 && memwrite)       mode    <= writedata[7:0];
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      iob     <= 0;
        else                            iob[0]  <= buzz;
~~~

### キーパッドを使いたい
キーパッドには4行4列の16個のボタンがある。1行目は左から1、2、3、A。2行目は左から4、5、6、B。3行目は左から7、8、9、C。4行目は左から0、F、E、Dに対応している。

キーパッドの使い方はやや特殊である。8-bitピンのうち、下位4-bitを出力、上位4-bitを入力ポートとして使う。
ここでは、汎用入出力ポートBにキーパッドがつながっていると仮定し、その下位をiob_lo、上位をiob_hiとする（汎用入出力ポートAを使う場合はioa_lo、ioa_hiになる）。

まず、ボタンが押されているか知りたい列を0、そうでない列を1として、4-bitの値を作る。例えば、ボタン1、4、7、0のどれかが押されているか知りたい場合は0111となる。ボタン3、6、9、Eのどれかがが押されているか知りたい場合は1101となる。これをiob_loに出力し、数サイクル待つ。

次に、iob_hiの値を読み取る。読み取った4-bitの値は「指定した列」の行の値に対応している。例えば、値が0111ならば1行目、値が1101ならば3行目が押されているということになる。

以下にサンプルコードを示す。kypd_scan()は押されているボタンの数字に対応した値が戻り値として返される。

~~~c
int kypd_scan() {
        volatile int *iob_ptr = (int *)0xff14;
        *iob_ptr = 0x07;                /* 0111 */
        for (int i = 0; i < 1; i++);    /* Wait */
        if ((*iob_ptr & 0x80) == 0)
                return 0x1;
        if ((*iob_ptr & 0x40) == 0)
                return 0x4;
        if ((*iob_ptr & 0x20) == 0)
                return 0x7;
        if ((*iob_ptr & 0x10) == 0)
                return 0x0;
        *iob_ptr = 0x0b;                /* 1011 */
        for (int i = 0; i < 1; i++);    /* Wait */
        if ((*iob_ptr & 0x80) == 0)
                return 0x2;
        if ((*iob_ptr & 0x40) == 0)
                return 0x5;
        if ((*iob_ptr & 0x20) == 0)
                return 0x8;
        if ((*iob_ptr & 0x10) == 0)
                return 0xf;
        *iob_ptr = 0x0d;                /* 1101 */
        for (int i = 0; i < 1; i++);    /* Wait */
        if ((*iob_ptr & 0x80) == 0)
                return 0x3;
        if ((*iob_ptr & 0x40) == 0)
                return 0x6;
        if ((*iob_ptr & 0x20) == 0)
                return 0x9;
        if ((*iob_ptr & 0x10) == 0)
                return 0xe;
        *iob_ptr = 0x0e;                /* 1110 */
        for (int i = 0; i < 1; i++);    /* Wait */
        if ((*iob_ptr & 0x80) == 0)
                return 0xa;
        if ((*iob_ptr & 0x40) == 0)
                return 0xb;
        if ((*iob_ptr & 0x20) == 0)
                return 0xc;
        if ((*iob_ptr & 0x10) == 0)
                return 0xd;
        return 0;
}
void main() {
        volatile int *led_ptr = (int *)0xff08;
        for (;;)
		*led_ptr = kypd_scan();
}
~~~

これを実現するには、汎用入出力ポートiob[7:0]をiob_lo[3:0]とiob_hi[3:0]に分ける必要がある。これにはhard/fpga.xdc（**新FPGAボード**の場合はfpga_z7.xdc）の該当箇所を以下のように修正する。

~~~tcl
##Pmod Header JC                V15 W15 T11 T10 W14 Y14 T12 U12
set_property PACKAGE_PIN V15 [get_ports {iob_lo[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_lo[0]}]
set_property PACKAGE_PIN W15 [get_ports {iob_lo[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_lo[1]}]
set_property PACKAGE_PIN T11 [get_ports {iob_lo[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_lo[2]}]
set_property PACKAGE_PIN T10 [get_ports {iob_lo[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_lo[3]}]

set_property PACKAGE_PIN W14 [get_ports {iob_hi[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_hi[0]}]
set_property PACKAGE_PIN Y14 [get_ports {iob_hi[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_hi[1]}]
set_property PACKAGE_PIN T12 [get_ports {iob_hi[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_hi[2]}]
set_property PACKAGE_PIN U12 [get_ports {iob_hi[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {iob_hi[3]}]
~~~

hard/top.vの入出力ポート宣言は下記のように修正する。

~~~verilog
module fpga_top (
        input                   clk_125mhz,
＜中略＞
        output  reg     [7:0]   ioa,
        //output reg    [7:0]   iob
        output  reg     [3:0]   iob_lo,
        input           [3:0]   iob_hi
);
~~~

汎用入出力ポートBがcs5空間に割り当てられている場合、iob_loとiob_hiの制御は下記の通り。これに加え、cs5とreaddata5によるreaddataの制御も必要になるので忘れずに。

~~~verilog
wire	[31:0]	readdata5;
~~~

~~~verilog
/* cs5 */
assign  readdata5       = {24'h0, iob_hi, iob_lo};
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      iob_lo  <= 0;
        else if (cs5 && memwrite)       iob_lo  <= writedata[3:0];
~~~

### ロータリーエンコーダを使いたい
ロータリーエンコーダには回転スイッチとスライドスイッチが実装されていて、回転スイッチのシャフトは押しボタンにもなっている。回転スイッチのシャフトは時計回り、反時計回りに回る。

ロータリーエンコーダ自体の出力は4-bitで、ここでは汎用入出力ポートBの上段4-bitにロータリーエンコーダがつながっていると仮定する。
4-bitのうち、最初の2-bit（AとB）の位相差によって回転スイッチのシャフトが時計回りか反時計回りかを判定する。次の2-bitはシャフトの押しボタンとスライドスイッチがつながっている。

シャフトの回転方向を判定する部分は専用回路として作る。
hard/top.vにロータリーエンコーダを制御するためのrotary_encモジュールを追加する。次のようなrotary_encモジュールをfpga_topモジュールのendmoduleの後に追加すると良い。

~~~verilog
module rotary_enc (
	input clk_62p5mhz,
	input reset,
	input [3:0] rte_in,
	output [9:0] rte_out
);
reg	[7:0]	count;
wire		A, B;
reg		prevA, prevB;
assign	{B, A} = rte_in[1:0];
assign	rte_out	= {count, rte_in[3:2]};
always @ (posedge clk_62p5mhz or posedge reset)
	if (reset) begin
		count	<= 128;
		prevA	<= 0;
		prevB	<= 0;
	end else
		case ({prevA, A, prevB, B})
		4'b0100: begin
			count <= count + 1;
			prevA <= A;
		end
		4'b1101: begin
			count <= count + 1;
			prevB <= B;
		end
		4'b1011: begin
			count <= count + 1;
			prevA <= A;
		end
		4'b0010: begin
			count <= count + 1;
			prevB <= B;
		end
		4'b0001: begin
			count <= count - 1;
			prevB <= B;
		end
		4'b0111: begin
			count <= count - 1;
			prevA <= A;
		end
		4'b1110: begin
			count <= count - 1;
			prevB <= B;
		end
		4'b1000: begin
			count <= count - 1;
			prevA <= A;
		end
		endcase
endmodule
~~~

rotary_encモジュールの入力（rte_in）はロータリーエンコーダ本体の4-bitで、rotary_encモジュールの出力（rte_out）は10-bitである。

rte_out[9:0]のうち最初の2-bitは、シャフトの押しボタン（rte_in[3]）とスライドスイッチ（rte_in[2]）がそのままつながっている。
次の8-bitはカウンタ出力になっていて、初期値が128で、シャフトが反時計回りに回されるとカウンタの値が増え、時計回りに回されるとカウンタの値が減る。
なお、カウンタは8-bitなので、カウンタの値が255のときに反時計回りに回すとカウンタの値は0に戻り、カウンタの値が0のときに時計回りに回すとカウンタの値は255になってしまう点に注意。

fpga_topモジュール中からこのrotary_encモジュールにアクセスしたい。
汎用入出力ポートBにロータリーエンコーダがつながっていると仮定するので、hard/top.vの入出力ポート宣言は下記のように修正する。
iobは「output reg」ではなくて「input」にする。

~~~verilog
module fpga_top (
	input			clk_125mhz,
＜中略＞
	//output　reg	[7:0]	iob
	input		[7:0]	iob
);
~~~

次にfpga_topモジュール中にrotary_encモジュールのインスタンスを追加する。さらにrotary_encモジュールが使用するwireも追加する。 
ここでは汎用入出力ポートBがcs5空間に割り当てられているものとする。

~~~verilog
wire	[31:0]	readdata5;
wire	[9:0]	rte;
rotary_enc rotary_enc (clk_62p5mhz, reset, iob, rte);
~~~

rotary_encモジュールの出力（rte_out）は10-bitなので、これをreaddata5につなぐ。これに加え、cs5とreaddata5によるreaddataの制御も必要になるので忘れずに。

~~~verilog
/* cs5 */
assign	readdata5	= {22'h0, rte};
~~~

以下にソフトウェア側のサンプルコードを示す。
ここでは入出力ポートBの番地は0xff14としているが、必要に応じて修正して欲しい。 

~~~c
void main() {
	volatile int *led_ptr = (int *)0xff08;
	volatile int *rte_ptr  = (int *)0xff14;
	lcd_init();
	for (;;) {
		*led_ptr = (*rte_ptr) & 0x3;
                lcd_clear_vbuf();
		lcd_digit3(0, 0, (*rte_ptr) >> 2);
                lcd_sync_vbuf();
	}
}
~~~

シャフトの押しボタンとスライドスイッチのオンオフ状態をLEDの下位2-bitに出している。
rotary_encモジュールの8-bitカウンタの値はLCDに表示している。値の表示には上述のlcd_digit3()関数を使っている。

### 7セグメントLEDを使いたい
2桁の7セグメントLEDによって、00～99までの数字を表示できる。

2桁7セグメントLEDの出力は4-bitが2つ横に並んでいるため、ここでは汎用入出力ポートAの上段4-bit、および、汎用入出力ポートBの上段4-bitを使うものとする。

ここでは0～99までの数字（7-bit必要）を入力として受け取り、4-bitの制御信号を2つ出力する専用回路を作る。
hard/top.vに2桁7セグメントLEDを制御するためのseg7ledモジュールを追加する。次のようなseg7ledモジュールをfpga_topモジュールのendmoduleの後に追加すると良い。

~~~verilog
module seg7led (
        input clk_62p5mhz,
        input reset,
        input [6:0] num,        /* 0 ... 99 */
        output [3:0] dout1,
        output [3:0] dout2
);
reg             digit;
reg     [19:0]  counter;
assign  dout1 = seg7out1(digit ? digit1(num) : digit10(num));
assign  dout2 = {digit, seg7out2(digit ? digit1(num) : digit10(num))};
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset) begin
                counter <= 0;
                digit   <= 0;
        end else if (counter < 20'd625000)
                counter <= counter + 1;
        else begin
                counter <= 0;
                digit   <= ~digit;
        end
function [3:0] digit1 (input [6:0] num);
        if (num < 10)           digit1  = num;
        else if (num < 20)      digit1  = num - 10;
        else if (num < 30)      digit1  = num - 20;
        else if (num < 40)      digit1  = num - 30;
        else if (num < 50)      digit1  = num - 40;
        else if (num < 60)      digit1  = num - 50;
        else if (num < 70)      digit1  = num - 60;
        else if (num < 80)      digit1  = num - 70;
        else if (num < 90)      digit1  = num - 80;
        else                    digit1  = num - 90;
endfunction
function [3:0] digit10 (input [6:0] num);
        if (num < 10)           digit10 = 0;
        else if (num < 20)      digit10 = 1;
        else if (num < 30)      digit10 = 2;
        else if (num < 40)      digit10 = 3;
        else if (num < 50)      digit10 = 4;
        else if (num < 60)      digit10 = 5;
        else if (num < 70)      digit10 = 6;
        else if (num < 80)      digit10 = 7;
        else if (num < 90)      digit10 = 8;
        else                    digit10 = 9;
endfunction
function [3:0] seg7out1 (input [3:0] din);
        case (din)
        4'd0: seg7out1 = 4'b1111;
        4'd1: seg7out1 = 4'b0000;
        4'd2: seg7out1 = 4'b1011;
        4'd3: seg7out1 = 4'b1001;
        4'd4: seg7out1 = 4'b0100;
        4'd5: seg7out1 = 4'b1101;
        4'd6: seg7out1 = 4'b1111;
        4'd7: seg7out1 = 4'b1100;
        4'd8: seg7out1 = 4'b1111;
        4'd9: seg7out1 = 4'b1100;
        default: seg7out1 = 4'b0000;
        endcase
endfunction
function [2:0] seg7out2 (input [3:0] din);
        case (din)
        4'd0: seg7out2 = 3'b011;
        4'd1: seg7out2 = 3'b011;
        4'd2: seg7out2 = 3'b101;
        4'd3: seg7out2 = 3'b111;
        4'd4: seg7out2 = 3'b111;
        4'd5: seg7out2 = 3'b110;
        4'd6: seg7out2 = 3'b110;
        4'd7: seg7out2 = 3'b011;
        4'd8: seg7out2 = 3'b111;
        4'd9: seg7out2 = 3'b111;
        default: seg7out2 = 3'b000;
        endcase
endfunction
endmodule
~~~

seg7ledモジュールの入力（num）は0から99までの整数で、seg7ledモジュールの出力（dout1、dout2）はそれぞれ4-bitである。
numに99より大きな数字を入れないように注意。

fpga_topモジュール中からこのseg7ledモジュールにアクセスしたい。
汎用入出力ポートAとBに2桁7セグメントLEDがつながっていると仮定するので、hard/top.vの入出力ポート宣言は下記のように修正する。
ここではioaとiobはwire型を仮定しているので、ioaとiobは「output reg」ではなくて「output」にする。

~~~verilog
module fpga_top (
	input			clk_125mhz,
＜中略＞
	//output　reg	[7:0]	ioa,
	//output　reg	[7:0]	iob
	output		[7:0]	ioa,
	output		[7:0]	iob
);
~~~

次にfpga_topモジュール中にseg7ledモジュールのインスタンスを追加する。seg7ledモジュールに与える数字（num）を記憶するためのregも追加する。 
ここでnumはcs5空間に割り当てられているものとする。

~~~verilog
reg     [6:0]   num;
seg7led seg7led (clk_62p5mhz, reset, num, ioa, iob);
~~~

cs5でnumに値を書き込めるようにする。

~~~verilog
/* cs5 */
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      num     <= 0;
        else if (cs5 && memwrite)       num     <= writedata[6:0];
~~~

以下にソフトウェア側のサンプルコードを示す。
ここではseg7ledモジュールに与えるnumの番地は0xff14としているが、必要に応じて修正して欲しい。 

~~~c
void interrupt_handler() {
        static int cnt = 0;
        volatile int *seg7_ptr = (int *)0xff14;
        cnt++;
        if (cnt % 10 == 0)
                *seg7_ptr = cnt / 10;
}
void main() {
        for (;;);
}
~~~

### タイマー割込みの頻度を変えたい
hard/top.v中のtimerモジュールのカウンタの最大値を変更すれば良い。
動作周波数は62.5MHz（1クロックは16nsec）である。
デフォルトでは100msec間隔でirqを1にするため、カウンタの最大値は6250000になっている。

~~~verilog
module timer (
        input                   clk, reset,
        output                  irq
);
reg     [22:0]  counter;

assign  irq = (counter == 23'd6250000);

always @ (posedge clk or posedge reset)
        if (reset)                      counter <= 0;
        else if (counter < 23'd6250000) counter <= counter + 1;
        else                            counter <= 0;
endmodule
~~~

### RGB LEDを使いたい（新FPGAボード限定）
**新FPGAボード**のひとは他と若干の違いがあり恐縮だが、良い点としてはボード上にRGB LEDが2個実装されている。

ポート名はled5とled6。それぞれ3-bit幅で、下位から順にR、G、Bに対応する。3'b000は消灯なので計8種類の色で光らせることができる。

hard/top.vの入出力ポート宣言は下記のように修正する。

~~~verilog
module fpga_top (
        input                   clk_125mhz,
        input           [3:0]   sw,
        input           [3:0]   btn,
        output  reg     [3:0]   led,
        output  reg     [2:0]   led5,
        output  reg     [2:0]   led6,
＜中略＞
);
~~~

### 浮動小数点数を使いたい
float型やdouble型変数のこと。結論から言うと現状では使えない。

本実験で使用しているプロセッサは教育用の簡易版であり、浮動小数点演算ユニットを装備していない。ソフトウェアによる浮動小数点ライブラリをリンクするという代案もあるが、現状ではまだ移植できていない。

## 付録D: 実験環境の基礎知識
本実験ではFPGAボードとしてDigilent社のZybo Zynq-7000 ARM/FPGA SoCトレーナーボードを使用している。以下は、このボードの公式マニュアルである。

* https://digilent.com/reference/programmable-logic/zybo/reference-manual

本実験で使用するLCDの高度な使い方に興味がある場合は以下のマニュアルを読むと良い。

* https://digilent.com/reference/pmod/pmodoledrgb/reference-manual

16ボタンキーパッドのマニュアルは以下にある。付録C「便利なサンプルコード集」のkypd_scan()関数はこれを見て実装した。

* https://digilent.com/reference/pmod/pmodkypd/reference-manual

ロータリーエンコーダのマニュアルは以下にある。付録C「便利なサンプルコード集」のrotary_encモジュールはこれを見て実装した。

* https://digilent.com/reference/pmod/pmodenc/reference-manual

MIPS R2000互換プロセッサの命令セットについては下記のリンクが参考になる。アセンブリ言語コードを見ながらのデバッグで、もし知らない命令が出てきたら適宜参照しよう。「7 MIPS アセンブリ言語」および「8 コンパイラのレジスタ使用規則」は一度目を通しておくと良い。 

* https://ja.wikipedia.org/wiki/MIPSアーキテクチャ

本実験ではXilinx社のZynq Z-7010というFPGAを使っている。Zynq 7000シリーズについては下記のリンクをたどると良い。

* https://japan.xilinx.com/products/silicon-devices/soc/zynq-7000.html

## 付録E: 環境構築
本実験で使用するツールは、MIPS用クロス開発ツール（gcc、binutils）、FPGA用合成・配置配線ツール（Xilinx Vivado）などである。シミュレーションを行うにはVerilog HDLシミュレータ（Icarus Verilog）、波形ビューワ（GTKWave）も必要になる。

これらはすでにITCで利用可能であるため、新しくインストールする必要はない。参考までにクロス開発環境のインストール方法を以下に書き記す。

### クロス開発環境の構築
MIPS I命令セットアーキテクチャ用のCコンパイラ（gcc）、バイナリツール（as、ld、objdump）をインストールする。バイトオーダはbig endianとする。
この例では、インストール先は/home/md401/aa205875/usr/としている。ここの部分は各自の環境に合わせて適宜修正すること。

まずは、バイナリツール（binutils）をインストールする。 

~~~sh
> wget https://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.gz
> tar zxvf binutils-2.30.tar.gz
> cd binutils-2.30
> mkdir build_mips && cd build_mips
> ../configure --target=mips --prefix=/home/md401/aa205875/usr
> make
> make install
~~~

次に任意精度数演算ライブラリ（gmp）をインストールする。これはCコンパイラ（gcc）をコンパイルするために必要である。 

~~~sh
> wget https://ftp.gnu.org/gnu/gmp/gmp-4.3.2.tar.gz
> tar zxvf gmp-4.3.2.tar.gz
> cd gmp-4.3.2
> ./configure --prefix=/home/md401/aa205875/usr
> make
> make install
~~~

高品質多倍長浮動小数点ライブラリ（mpfr）をインストールする。こちらもgccをコンパイルするために必要である。 

~~~sh
> wget https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.2.tar.gz
> tar zxvf mpfr-3.1.2.tar.gz
> ./configure --prefix=/home/md401/aa205875/usr \
	--with-gmp=/home/md401/aa205875/usr
> make
> make install
~~~

複素数ライブラリ（mpc）をインストールする。こちらもgccをコンパイルするために必要である。 

~~~sh
> wget https://ftp.gnu.org/gnu/mpc/mpc-1.0.1.tar.gz
> tar zxvf mpc-1.0.1.tar.gz
> ./configure --prefix=/home/md401/aa205875/usr \
	--with-gmp=/home/md401/aa205875/usr \
	--with-mpfr=/home/md401/aa205875/usr
> make
> make install
~~~

ここで、やっとCコンパイラ（gcc）のインストールに入る。configureスクリプトの引数としてgmp、mpfr、mpcライブラリのパスを指定する必要がある。マシンによってはコンパイルには1時間以上かかる。 

~~~sh
> wget https://ftp.gnu.org/gnu/gcc/gcc-7.4.0/gcc-7.4.0.tar.gz
> tar zxvf gcc-7.4.0.tar.gz
> cd gcc-7.4.0
> mkdir build_mips && cd build_mips
> ../configure --target=mips --without-fp --enable-languages="c" \
       --disable-libssp --prefix=/home/md401/aa205875/usr \
	--with-gmp=/home/md401/aa205875/usr \
	--with-mpfr=/home/md401/aa205875/usr \
	--with-mpc=/home/md401/aa205875/usr
> make -j4
> make install
~~~

### 環境変数の設定
開発ツールを使えるようにするために、シェルの設定ファイル（.bashrc）を修正し、環境変数PATHを設定する。 

~~~sh
> emacs .bashrc
＜末尾に以下を追加＞
export PATH=/home/md401/aa205875/usr/bin/:${PATH}
> source .bashrc
~~~
