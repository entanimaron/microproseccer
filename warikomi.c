void interrupt_handler() {

    lcd_clear_vbuf();  // 毎フレーム画面をクリア

    if (state == INIT) {

        // 初期化中の描画(optional)

    } else if (state == OPENING) {
        // タイトル画面などを描画（例）
        lcd_puts(3, 2, "GAME START");

    } else if (state == PLAY) {
        // プレイヤー
        if (player.state) {
            lcd_putc(player.posy, player.posx, player.image);
        }

        // 障害物
        for (int i = 0; i < OBSTACLE_MAX; i++) {
            if (obstacles[i].state) {
                lcd_putc(obstacles[i].posy, obstacles[i].posx, obstacles[i].image);
            }
        }

        // 数字
        for (int i = 0; i < DIGIT_MAX; i++) {
            if (digits[i].state) {
                lcd_putc(digits[i].posy, digits[i].posx, digits[i].image);
            }
        }

        // 弾
        for (int i = 0; i < BULLET_MAX; i++) {
            if (bullets[i].state) {
                lcd_putc(bullets[i].posy, bullets[i].posx, bullets[i].image);
            }
        }

        // ボス
        if (boss.state) {
            lcd_putc(boss.posy, boss.posx, boss.image);
        }

    } else if (state == ENDING) {
        lcd_puts(3, 3, "GAME OVER"); //任意の画面(仮)
    }

    // 最後に LCD に反映
    lcd_sync_vbuf();
}


