void interrupt_handler(){
  game_tick();
  lcd_sync_vbuf();
}

void game_tick(){
  lcd_clear_vbuf(); //　画面クリア

// player更新
  int enc = encoder_read();

  if (enc == 1){
    if(player.posy > 0)
      player.posy -= player.vy;
  }else if(enc == -1){
    if(player.posy < LCD_ROWS - player.hie)
      player.posy += player.vy;
  }
  lcd_putc(player.posy, player.posx, player.image);

// 障害物の更新
  obstacle_timer++;
  if (obstacle_timer >= obstacle_speed) {
    obstacle_timer = 0;

    for(int i = 0; i < OBSTACLE_MAX; i++){
      if(!obstacle[i].state) continue;

      obstacle[i].posx += obstacles[i].vx;

      if(obstacles[i].posx < 0){
        obstacles[i].state = 0;
      }
    }
  }

  for(int i = 0; i < OBSTACLE_MAX; i++) {
    if (obstacles[i].state)
      lcd_putc(obstacles[i].posy, obstacles[i].posx, obstacles[i].image);
  }

// 数字
  digit_timer++;
  if(digit_timer >= digit_speed){
    digit_timer = 0;

    for(int i = 0; i < DIGIT_MAX; i++){
      if(!digits[i].state) continue;

      digits[i].posx += digits[i].vx;

      if(digits[i].posx < 0)
        digits[i].state = 0;
    }
  }

  for(int i = 0; i < DIGIT_MAX; i++) {
    if (digits[i].state)
      lcd_putc(digits[i].posy, digits[i].posx, digits[i].image);
  }
    
// 弾の更新
  for(int i = 0; i < BULLET_MAX; i++){
    if(!bullets[i].state) continue;

    bullets[i].posx += bullets[i].vx * bullet_speed;
    bullets[i].posy += bullets[i].vy * bullet_speed;

    //　弾が画面外にでたら消す
    if(bullets[i].posx < 0 || bullets[i].posx >= LCD_COLS || bullets[i].posy < 0 || bullets[i].posy >= LCD_ROWS){
      bullets[i].state = 0;
      continue;
    }

    lcd_putc(bullets[i].posy, bullets[i].posx, bullets[i].image);
  }

// ボス更新
  if(boss.state){
    lcd_putc(boss.posy, boss.posx, boss.image);
  }

// 衝突判定
  for(int i = 0; i < OBSTACLE_MAX; i++){
    if(!obstacles[i].state) continue;

    if(chechk_collision(&player, &obstacles[i])){
      player.life--;
      obstacles[i].state = 0;

      write_iob(1); //振動
    }
  }

  write_iob(0);
}
