import re

with open('arduino.ino', 'r', encoding='utf-8') as f:
    ino = f.read()

new_funcs = """// ==========================================
// TELA PET (principal)
// ==========================================
void desenharTelaPet(unsigned long agora) {
  int respY = (int)(sin(agora / 800.0) * 1.0);
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  
  int x = (27 - w) / 2;
  int y = (48 - h) / 2 + respY;

  if (fome < 30 || felicidade < 30) {
    telaVirtual.setTextSize(1);
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setCursor(4 * 5, (21 + respY) * 5);
    telaVirtual.print("(o_O)");
  } else {
    desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
  }
}

// ==========================================
// TELA DO MENU (tela cheia com opções)
// ==========================================
void desenharMenu() {
  for (int i = 0; i < MENU_TOTAL; i++) {
    int y = 9 + i * 9;
    if (i == menuCursor) {
      telaVirtual.fillRect(2 * 5, (y - 1) * 5, 23 * 5, 6 * 5, TFT_WHITE);
      telaVirtual.setTextColor(TFT_BLACK);
    } else {
      telaVirtual.setTextColor(TFT_WHITE);
    }
    telaVirtual.setTextSize(1);
    telaVirtual.setCursor(4 * 5, y * 5);
    telaVirtual.print(menuOpcoes[i]);
  }
}

// ==========================================
// TELA DE STATUS
// ==========================================
void desenharTelaStatus() {
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(5 * 5, 2 * 5);
  telaVirtual.print("STAT");

  telaVirtual.fillRect(2 * 5, 6 * 5, 23 * 5, 1 * 5, TFT_WHITE);

  telaVirtual.setTextColor(TFT_WHITE);
  int minutos = (idade * 5) / 60;
  telaVirtual.setCursor(2 * 5, 9 * 5);
  telaVirtual.print("Id:");
  telaVirtual.print(minutos);
  telaVirtual.print("m");

  telaVirtual.setCursor(2 * 5, 16 * 5);
  telaVirtual.print("Fome");

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(17 * 5, 16 * 5);
  telaVirtual.print(fome);
  telaVirtual.print("%");

  telaVirtual.fillRect(2 * 5, 20 * 5, 23 * 5, 3 * 5, TFT_WHITE);
  int fomeW = map(fome, 0, 100, 0, 21);
  if (fomeW > 0) telaVirtual.fillRect(3 * 5, 21 * 5, fomeW * 5, 1 * 5, TFT_WHITE);

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(2 * 5, 27 * 5);
  telaVirtual.print("Felz");

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(17 * 5, 27 * 5);
  telaVirtual.print(felicidade);
  telaVirtual.print("%");

  telaVirtual.fillRect(2 * 5, 31 * 5, 23 * 5, 3 * 5, TFT_WHITE);
  int felW = map(felicidade, 0, 100, 0, 21);
  if (felW > 0) telaVirtual.fillRect(3 * 5, 32 * 5, felW * 5, 1 * 5, TFT_WHITE);
}

// ==========================================
// TELA DE COMER
// ==========================================
void desenharTelaComer() {
  telaVirtual.fillRect(0 * 5, 36 * 5, 27 * 5, 1 * 5, TFT_WHITE);
  
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  int x = (27 - w) / 2;
  
  unsigned long progresso = millis() - tempoInicioAnimacao;
  int respY = 0;
  if ((progresso / 200) % 2 == 0) {
    respY = 1; // mastigacao (1 px logical)
  }
  
  int y = 36 - h + respY;
  
  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);

  int cx = x + w + 2;
  int comidaY = 36 - 3;

  int cw = 16;
  int ch = 16;
  int cxStart = cx - cw/2;
  int cyStart = comidaY - ch/2;

  if (comidaSelecionada == 0) {
    desenharSpriteEscalado(apple_sprite, apple_width, apple_height, cxStart * 5, cyStart * 5, 5);
  } else {
    desenharSpriteEscalado(candy_sprite, candy_width, candy_height, cxStart * 5, cyStart * 5, 5);
  }

  if (progresso > 500 && progresso < 1000) {
    telaVirtual.fillRect((cxStart + cw/2) * 5, cyStart * 5, (cw/2) * 5, ch * 5, TFT_BLACK);
  } else if (progresso >= 1000) {
    telaVirtual.fillRect((cxStart - 2) * 5, cyStart * 5, (cw + 4) * 5, ch * 5, TFT_BLACK);
  }
}

// ==========================================
// TELA DOS SUBMENUS
// ==========================================
void desenharSubMenu(const char* titulo, const char* opcoes[], int total) {
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(5 * 5, 2 * 5);
  telaVirtual.print(titulo);
  telaVirtual.fillRect(2 * 5, 6 * 5, 23 * 5, 1 * 5, TFT_WHITE);

  for (int i = 0; i < total; i++) {
    int y = 11 + i * 9;

    if (i == subMenuCursor) {
      telaVirtual.fillRect(2 * 5, (y - 1) * 5, 23 * 5, 6 * 5, TFT_WHITE);
      telaVirtual.setTextColor(TFT_BLACK);
    } else {
      telaVirtual.setTextColor(TFT_WHITE);
    }
    telaVirtual.setTextSize(1);
    telaVirtual.setCursor(4 * 5, y * 5);
    telaVirtual.print(opcoes[i]);
  }
}

// ==========================================
// TELA DO MINIJOGO DE PULAR
// ==========================================
void desenharTelaPular(unsigned long agora) {
  telaVirtual.fillRect(0 * 5, 36 * 5, 27 * 5, 1 * 5, TFT_WHITE);
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(1 * 5, 1 * 5);
  telaVirtual.print("P:");
  telaVirtual.print(obsCount);

  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  int x = 6; 
  int y = 36 - h + pularY;
  
  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
  
  telaVirtual.fillRect(obsX * 5, 32 * 5, 4 * 5, 4 * 5, TFT_WHITE);
}

// ==========================================
// DESENHO DE CORAÇÃO E TELA DE CARINHO
// ==========================================
void desenharCoracao(int cx, int cy) {
  telaVirtual.fillRect((cx-1) * 5, (cy-1) * 5, 1 * 5, 1 * 5, TFT_WHITE);
  telaVirtual.fillRect((cx+1) * 5, (cy-1) * 5, 1 * 5, 1 * 5, TFT_WHITE);
  telaVirtual.fillRect((cx-2) * 5, (cy) * 5, 5 * 5, 1 * 5, TFT_WHITE);
  telaVirtual.fillRect((cx-1) * 5, (cy+1) * 5, 3 * 5, 1 * 5, TFT_WHITE);
  telaVirtual.fillRect((cx) * 5, (cy+2) * 5, 1 * 5, 1 * 5, TFT_WHITE);
}

void desenharTelaCarinho(unsigned long agora) {
  int respY = (int)(sin(agora / 800.0) * 1.0);
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  
  int x = (27 - w) / 2;
  int y = (48 - h) / 2 + respY;
  int offsetX = 0;
  int offsetY = 0;

  if (millis() - ultimoCliqueCarinho < 150) {
    offsetX = random(-1, 2);
    offsetY = random(-1, 2);
  }

  desenharSpriteEscalado(sprite, w, h, (x + offsetX) * 5, (y + offsetY) * 5, 5);

  if (millis() - ultimoCliqueCarinho < 450) {
    float progresso = (millis() - ultimoCliqueCarinho) / 450.0;
    int deltaY = (int)(progresso * 8.0);
    desenharCoracao(x - 3, y + 5 - deltaY);
    desenharCoracao(x + w + 3, y + 2 - deltaY);
  }
}

// ==========================================
// MENSAGEM DE SUCESSO TEMPORÁRIA
// ==========================================
void desenharMensagemSucesso() {
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(5 * 5, 16 * 5);
  telaVirtual.print(msgSucessoTitulo);

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(3 * 5, 26 * 5);
  telaVirtual.print(msgSucessoSub);
}

// ==========================================
// GAME OVER
// ==========================================
void desenharGameOver() {
  int w = lapide_width, h = lapide_height;
  int x = (27 - w) / 2;
  int y = 8;
  
  desenharSpriteEscalado(lapide_sprite, w, h, x * 5, y * 5, 5);
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(4 * 5, (y + h + 2) * 5);
  telaVirtual.print("MORTO");

  int minutos = (idade * 5) / 60;
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(4 * 5, (y + h + 8) * 5);
  telaVirtual.print(minutos);
  telaVirtual.print("m");
}
"""

start_str = "void desenharTelaPet(unsigned long agora) {"
end_str = "void reiniciarJogo() {"

start_idx = ino.find(start_str)
end_idx = ino.find(end_str)

if start_idx != -1 and end_idx != -1:
    ino = ino[:start_idx] + new_funcs + "\n" + ino[end_idx:]
else:
    print(f"FAILED TO FIND INDICES! start_idx={start_idx}, end_idx={end_idx}")

# Adjust Pular jump variables in arduino.ino physics
ino = ino.replace("obsX -= 4;", "obsX -= 1;") # Physical was 4
ino = ino.replace("obsX -= 2;", "obsX -= 1;") 
ino = ino.replace("pularY = -25;", "pularY = -5;")
ino = ino.replace("if (obsX > 40 && obsX < 75 && pularY == 0) {", "if (obsX > 8 && obsX < 15 && pularY == 0) {")
ino = ino.replace("if (obsX < -20 && !hitObs) {", "if (obsX < -4 && !hitObs) {")
ino = ino.replace("int obsX = 135;", "int obsX = 27;") # global var init
ino = ino.replace("int pularY = 0;", "int pularY = 0;")

with open('arduino.ino', 'w', encoding='utf-8') as f:
    f.write(ino)
