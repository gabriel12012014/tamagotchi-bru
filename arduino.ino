#include <TFT_eSPI.h>
#include <SPI.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <WiFi.h>
#include "sprites.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite telaVirtual = TFT_eSprite(&tft); // Double Buffer
Preferences preferences;

#define BUZZER_PIN 26

// Pinos dos Botões
const int BTN_ESQUERDO = 0;   // IO0  - Abre Menu / Navega opções
const int BTN_DIREITO  = 35;  // IO35 - Seleciona opção

// Status do Pet
int fome = 100;
int felicidade = 100;
int idade = 0;
bool vivo = true;

// Controles de Tempo
unsigned long ultimoTurno = 0;
const unsigned long TICK_INTERVAL = 5000; // 5 segundos por turno
unsigned long ultimaInteracao = 0;
const unsigned long TEMPO_REPOUSO = 60000; // 60 segundos (1 minuto) de inatividade para dormir

// Debounce dos Botões
unsigned long ultimoCliqueEsq = 0;
unsigned long ultimoCliqueDir = 0;

// Estado da Interface (Máquina de Estados)
enum Estado { TELA_PET, TELA_MENU, TELA_STATUS, TELA_GAMEOVER, TELA_COMER, TELA_MSG_SUCESSO, TELA_CARINHO, TELA_SUBMENU_COMER, TELA_SUBMENU_BRINCAR, TELA_PULAR };
Estado estadoAtual = TELA_PET;

// Variáveis de Sucesso Temporário
unsigned long tempoMensagemSucesso = 0;
const char* msgSucessoTitulo = "";
const char* msgSucessoSub = "";

// Variáveis do Minijogo de Comer
int cliquesComida = 0;
unsigned long ultimoTremorComida = 0;

// Variáveis do Minijogo de Carinho
int cliquesCarinho = 0;
unsigned long ultimoCliqueCarinho = 0;

// Menu de Ações
int menuCursor = 0;
const int MENU_TOTAL = 3;
const char* menuOpcoes[] = {"Comer", "Brincar", "Voltar"}; // Substitui Carinho por Brincar

// Submenus
int subMenuCursor = 0;
const int SUBMENU_TOTAL = 2;
const char* menuComida[] = {"Maca", "Doce"};
const char* menuBrincar[] = {"Carinho", "Pular"};

// Comida Selecionada
int comidaSelecionada = 0; // 0 = Maca, 1 = Doce

// Variáveis do Minijogo de Pular
int pularY = 0;
bool isPular = false;
unsigned long tempoPulo = 0;
int obsX = 27;
int obsCount = 0;
bool hitObs = false;
unsigned long ultimoMoveObs = 0;

// ==========================================
// FUNÇÕES DE SOM
// ==========================================
void tocarSomBeep() { tone(BUZZER_PIN, 1000, 50); }
void tocarSomPulo() { tone(BUZZER_PIN, 600, 100); delay(100); tone(BUZZER_PIN, 800, 100); }
void tocarSomSucesso() { tone(BUZZER_PIN, 1000, 100); delay(100); tone(BUZZER_PIN, 1200, 150); delay(150); tone(BUZZER_PIN, 1500, 300); }
void tocarSomFalha() { tone(BUZZER_PIN, 300, 200); delay(200); tone(BUZZER_PIN, 200, 300); }
void pararSom() { noTone(BUZZER_PIN); }

// ==========================================
// FUNÇÃO DE MODO REPOUSO (SLEEP)
// ==========================================
void entrarModoRepouso() {
  // Desliga o backlight (Pino 4 no TTGO T-Display)
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  // Desliga o painel LCD
  tft.writecommand(TFT_DISPOFF);
  tft.writecommand(TFT_SLPIN);

  // Configura os pinos dos botões para acordar do Light Sleep
  gpio_wakeup_enable((gpio_num_t)BTN_ESQUERDO, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)BTN_DIREITO, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  // Loop de sleep e keep-alive para manter o power bank ligado
  bool acordouPorBotao = false;
  while (!acordouPorBotao) {
    // Acorda a cada 15 segundos
    esp_sleep_enable_timer_wakeup(15000000ULL); 
    
    // Entra em repouso mantendo a RAM
    esp_light_sleep_start();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
      // Acordou pelo timer: dá um pulso no WiFi
      WiFi.mode(WIFI_STA);
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
      delay(150); // Liga rádio por 150ms para gerar pico de corrente
      WiFi.mode(WIFI_OFF);
    } else {
      // Acordou por GPIO (botão esquerdo ou direito)
      acordouPorBotao = true;
    }
  }

  // --- ACORDOU ---
  
  // Liga o painel LCD
  tft.writecommand(TFT_SLPOUT);
  delay(120);
  tft.writecommand(TFT_DISPON);
  
  // Liga o backlight
  digitalWrite(4, HIGH);
  
  // Espera os botões serem soltos para não acionar comandos indesejados
  while(digitalRead(BTN_ESQUERDO) == LOW || digitalRead(BTN_DIREITO) == LOW) {
    delay(10);
  }

  // Reseta os temporizadores
  unsigned long tempoAtual = millis();
  ultimoTurno = tempoAtual;
  ultimaInteracao = tempoAtual;
}

// ===============================================

void setup() {
  // Configuração dos Botões e Buzzer
  pinMode(BTN_ESQUERDO, INPUT_PULLUP);
  pinMode(BTN_DIREITO, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Inicializa Persistência
  preferences.begin("pet", false);
  fome = preferences.getInt("fome", 100);
  felicidade = preferences.getInt("felicidade", 100);
  idade = preferences.getInt("idade", 0);
  vivo = preferences.getBool("vivo", true);

  // Inicializa a Tela
  tft.init();
  tft.setRotation(0); // Orientação Vertical

  // Cria a Tela Virtual na RAM (135x240)
  telaVirtual.createSprite(135, 240);

  ultimaInteracao = millis();
}

void loop() {
  unsigned long agora = millis();

  // Verifica inatividade para entrar em modo repouso
  if (agora - ultimaInteracao > TEMPO_REPOUSO) {
    entrarModoRepouso();
    agora = millis(); // Atualiza o tempo atual após acordar
  }

  // Expiração da tela de sucesso após 1500ms
  if (estadoAtual == TELA_MSG_SUCESSO && (agora - tempoMensagemSucesso > 1500)) {
    estadoAtual = TELA_PET;
  }

  // ==========================================
  // 1. INPUT - Leitura dos Botões (Hold/Release)
  // ==========================================
  static unsigned long tempoBotaoEsq = 0;
  static bool botaoEsqPressionado = false;
  static bool segurandoStatus = false;

  bool esqPress = false;
  bool dirPress = false;

  // Botão Esquerdo (Hold de 400ms)
  if (digitalRead(BTN_ESQUERDO) == LOW) {
    ultimaInteracao = agora;
    if (!botaoEsqPressionado) {
      botaoEsqPressionado = true;
      tempoBotaoEsq = agora;
    } else {
      if (!segurandoStatus && (agora - tempoBotaoEsq > 400) && vivo && (estadoAtual == TELA_PET || estadoAtual == TELA_MENU)) {
        segurandoStatus = true;
        estadoAtual = TELA_STATUS;
      }
    }
  } else {
    if (botaoEsqPressionado) {
      botaoEsqPressionado = false;
      unsigned long duracao = agora - tempoBotaoEsq;
      if (segurandoStatus) {
        segurandoStatus = false;
        if (estadoAtual == TELA_STATUS) {
          estadoAtual = TELA_PET;
        }
      } else {
        if (duracao > 50) { // Debounce mínimo para clique
          esqPress = true;
        }
      }
    }
  }

  // Botão Direito (Debounce clássico)
  if (digitalRead(BTN_DIREITO) == LOW && (agora - ultimoCliqueDir > 250)) {
    ultimaInteracao = agora;
    dirPress = true;
    ultimoCliqueDir = agora;
  }

  // ==========================================
  // 2. MÁQUINA DE ESTADOS (Navegação)
  // ==========================================

  // Se morreu, força tela de game over
  if (!vivo && estadoAtual != TELA_GAMEOVER) {
    estadoAtual = TELA_GAMEOVER;
  }

  // Botão Esquerdo: Clique rápido abre menu / Navega
  if (esqPress) {
    switch (estadoAtual) {
      case TELA_PET:
        estadoAtual = TELA_MENU;
        menuCursor = 0;
        break;
      case TELA_MENU:
        menuCursor = (menuCursor + 1) % MENU_TOTAL;
        break;
      case TELA_STATUS:
        estadoAtual = TELA_PET;
        break;
      case TELA_GAMEOVER:
        reiniciarJogo();
        break;
      case TELA_COMER:
        ultimoTremorComida = agora;
        cliquesComida++;
        if (cliquesComida >= 10) {
          if (comidaSelecionada == 0) { // Maca
            fome += 15;
            msgSucessoTitulo = "Nham!";
            msgSucessoSub = "+ fome";
          } else { // Doce
            fome += 5;
            felicidade += 10;
            msgSucessoTitulo = "Doce!";
            msgSucessoSub = "+ feliz";
          }
          if (fome > 100) fome = 100;
          if (felicidade > 100) felicidade = 100;
          estadoAtual = TELA_MSG_SUCESSO;
          tempoMensagemSucesso = agora;
          tocarSomSucesso();
        } else {
          tocarSomBeep();
        }
        break;
      case TELA_CARINHO:
        ultimoCliqueCarinho = agora;
        cliquesCarinho++;
        if (cliquesCarinho >= 10) {
          felicidade += 15;
          if (felicidade > 100) felicidade = 100;
          estadoAtual = TELA_MSG_SUCESSO;
          tempoMensagemSucesso = agora;
          msgSucessoTitulo = "Eba!";
          msgSucessoSub = "+ carinho";
          tocarSomSucesso();
        } else {
          tocarSomBeep();
        }
        break;
      case TELA_SUBMENU_COMER:
      case TELA_SUBMENU_BRINCAR:
        subMenuCursor = (subMenuCursor + 1) % SUBMENU_TOTAL;
        tocarSomBeep();
        break;
      case TELA_PULAR:
        estadoAtual = TELA_PET; // Sair do jogo
        tocarSomBeep();
        break;
    }
  }

  // Botão Direito: Seleciona opção do menu
  if (dirPress) {
    switch (estadoAtual) {
      case TELA_MENU:
        executarMenuAcao();
        break;
      case TELA_STATUS:
        estadoAtual = TELA_PET;
        break;
      case TELA_GAMEOVER:
        reiniciarJogo();
        break;
      case TELA_COMER:
        ultimoTremorComida = agora;
        cliquesComida++;
        if (cliquesComida >= 10) {
          if (comidaSelecionada == 0) { // Maca
            fome += 15;
            msgSucessoTitulo = "Nham!";
            msgSucessoSub = "+ fome";
          } else { // Doce
            fome += 5;
            felicidade += 10;
            msgSucessoTitulo = "Doce!";
            msgSucessoSub = "+ feliz";
          }
          if (fome > 100) fome = 100;
          if (felicidade > 100) felicidade = 100;
          estadoAtual = TELA_MSG_SUCESSO;
          tempoMensagemSucesso = agora;
          tocarSomSucesso();
        } else {
          tocarSomBeep();
        }
        break;
      case TELA_CARINHO:
        ultimoCliqueCarinho = agora;
        cliquesCarinho++;
        if (cliquesCarinho >= 10) {
          felicidade += 15;
          if (felicidade > 100) felicidade = 100;
          estadoAtual = TELA_MSG_SUCESSO;
          tempoMensagemSucesso = agora;
          msgSucessoTitulo = "Eba!";
          msgSucessoSub = "+ carinho";
          tocarSomSucesso();
        } else {
          tocarSomBeep();
        }
        break;
      case TELA_SUBMENU_COMER:
        executarSubMenuComida();
        break;
      case TELA_SUBMENU_BRINCAR:
        executarSubMenuBrincar();
        break;
      case TELA_PULAR:
        if (!isPular && !hitObs) {
          isPular = true;
          tempoPulo = agora;
          tocarSomPulo();
        }
        break;
      default:
        break;
    }
  }

  // ==========================================
  // 3. GAME TICK - Decaimento dos Stats
  // ==========================================
  if (agora - ultimoTurno > TICK_INTERVAL) {
    if (vivo) {
      fome -= 1;
      felicidade -= 2;
      idade++;

      if (fome <= 0 || felicidade <= 0) {
        fome = max(0, fome);
        felicidade = max(0, felicidade);
        vivo = false;
        estadoAtual = TELA_GAMEOVER;
        tocarSomFalha();
      }
      
      // Salva progresso no Tick
      preferences.putInt("fome", fome);
      preferences.putInt("felicidade", felicidade);
      preferences.putInt("idade", idade);
      preferences.putBool("vivo", vivo);
    }
    ultimoTurno = agora;
  }

  // ==========================================
  // LÓGICA DO MINIJOGO DE PULAR
  // ==========================================
  if (estadoAtual == TELA_PULAR) {
    if (agora - ultimoMoveObs > 50) {
      ultimoMoveObs = agora;
      
      if (!hitObs) {
        obsX -= 8;
        
        if (isPular) {
          unsigned long tPulo = agora - tempoPulo;
          if (tPulo < 250) {
            pularY = -5;
          } else if (tPulo < 500) {
            pularY = 0;
          } else {
            isPular = false;
          }
        }
        
        // Colisão
        if (obsX > 8 && obsX < 15 && pularY == 0) {
          hitObs = true;
          tocarSomFalha();
          fome -= 10;
          felicidade -= 10;
          estadoAtual = TELA_MSG_SUCESSO;
          tempoMensagemSucesso = agora;
          msgSucessoTitulo = "Ops!";
          msgSucessoSub = "Bateu :(";
        }
        
        // Passou
        if (obsX < -4 && !hitObs) {
          obsX = 27;
          obsCount++;
          if (obsCount >= 5) {
            felicidade += 20;
            if (felicidade > 100) felicidade = 100;
            estadoAtual = TELA_MSG_SUCESSO;
            tempoMensagemSucesso = agora;
            msgSucessoTitulo = "Uhuu!";
            msgSucessoSub = "+ feliz";
            tocarSomSucesso();
          }
        }
      }
    }
  }

  // ==========================================
  // 4. RENDERIZAÇÃO
  // ==========================================
  telaVirtual.fillSprite(TFT_BLACK);

  switch (estadoAtual) {
    case TELA_PET:      desenharTelaPet(agora); break;
    case TELA_MENU:     desenharMenu(); break;
    case TELA_STATUS:   desenharTelaStatus(); break;
    case TELA_GAMEOVER: desenharGameOver(); break;
    case TELA_COMER:    desenharTelaComer(); break;
    case TELA_MSG_SUCESSO: desenharMensagemSucesso(); break;
    case TELA_CARINHO:  desenharTelaCarinho(agora); break;
    case TELA_SUBMENU_COMER: desenharSubMenu("COMIDA", menuComida, SUBMENU_TOTAL); break;
    case TELA_SUBMENU_BRINCAR: desenharSubMenu("BRINCAR", menuBrincar, SUBMENU_TOTAL); break;
    case TELA_PULAR:    desenharTelaPular(agora); break;
  }

  telaVirtual.pushSprite(0, 0);
}

// ==========================================
// Executa a ação selecionada no menu
// ==========================================
void executarMenuAcao() {
  tocarSomBeep();
  switch (menuCursor) {
    case 0: // Comer (Abre submenu de comidas)
      subMenuCursor = 0;
      estadoAtual = TELA_SUBMENU_COMER;
      break;
    case 1: // Brincar (Abre submenu de brincadeiras)
      subMenuCursor = 0;
      estadoAtual = TELA_SUBMENU_BRINCAR;
      break;
    case 2: // Voltar
      estadoAtual = TELA_PET;
      break;
  }
}

void executarSubMenuComida() {
  tocarSomBeep();
  comidaSelecionada = subMenuCursor;
  cliquesComida = 0;
  estadoAtual = TELA_COMER;
}

void executarSubMenuBrincar() {
  tocarSomBeep();
  if (subMenuCursor == 0) {
    cliquesCarinho = 0;
    estadoAtual = TELA_CARINHO;
  } else {
    // Inicia Pular
    obsX = 27;
    obsCount = 0;
    isPular = false;
    hitObs = false;
    pularY = 0;
    estadoAtual = TELA_PULAR;
  }
}

// ==========================================
// Desenha o sprite escalado (pixel por pixel)
// ==========================================
void desenharSpriteEscalado(const unsigned short* sprite, int w, int h, int x, int y, int escala) {
  for (int linha = 0; linha < h; linha++) {
    for (int col = 0; col < w; col++) {
      uint16_t cor = pgm_read_word(&sprite[linha * w + col]);
      if (cor != TFT_BLACK && cor != 0x0000) { // 0x0000 é transparente
        telaVirtual.fillRect(x + col * escala, y + linha * escala, escala, escala, cor);
      }
    }
  }
}

const unsigned short* getPetSprite(int& w, int& h) {
  if (idade < 1) {
    w = ovo_width;
    h = ovo_height;
    return ovo_sprite;
  } else {
    w = capivara_width;
    h = capivara_height;
    return capivara_sprite;
  }
}

// ==========================================
// TELA PRINCIPAL: Pet centralizado + respiração
// ==========================================
void desenharTelaPet(unsigned long agora) {
  int respY = (int)(sin(agora / 800.0) * 2.0);

  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  
  int spriteW = w * 3;
  int spriteH = h * 3;
  int x = (135 - spriteW) / 2;
  int y = (240 - spriteH) / 2 + respY;

  if (fome < 30 || felicidade < 30) {
    telaVirtual.setTextSize(3);
    telaVirtual.setTextColor(TFT_ORANGE);
    telaVirtual.setCursor(22, 105 + respY);
    telaVirtual.print("(o_O)");
  } else {
    desenharSpriteEscalado(sprite, w, h, x, y, 3);
  }
}

// ==========================================
// TELA DO MENU (tela cheia com opções)
// ==========================================
void desenharMenu() {
  for (int i = 0; i < MENU_TOTAL; i++) {
    int y = 45 + i * 45;

    if (i == menuCursor) {
      // Opção selecionada: fundo branco, texto preto
      telaVirtual.fillRect(10, y - 5, 115, 28, TFT_WHITE);
      telaVirtual.setTextColor(TFT_BLACK);
    } else {
      // Opção normal: texto branco
      telaVirtual.setTextColor(TFT_WHITE);
    }

    telaVirtual.setTextSize(2);
    telaVirtual.setCursor(22, y);
    telaVirtual.print(menuOpcoes[i]);
  }
}

// ==========================================
// TELA DE STATUS
// ==========================================
void desenharTelaStatus() {
  telaVirtual.setTextColor(TFT_CYAN);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(25, 12);
  telaVirtual.print("STATUS");

  telaVirtual.drawFastHLine(10, 32, 115, 0x4208);

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  int minutos = (idade * 5) / 60;
  telaVirtual.setCursor(12, 45);
  telaVirtual.print("Idade: ");
  telaVirtual.print(minutos);
  telaVirtual.print("m");

  // Fome
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(12, 80);
  telaVirtual.print("Fome");

  uint16_t corFome = fome > 50 ? TFT_GREEN : (fome > 25 ? TFT_YELLOW : TFT_RED);
  telaVirtual.setTextColor(corFome);
  telaVirtual.setCursor(85, 80);
  telaVirtual.print(fome);
  telaVirtual.print("%");

  telaVirtual.drawRect(12, 100, 111, 16, TFT_WHITE);
  int fomeW = map(fome, 0, 100, 0, 107);
  if (fomeW > 0) telaVirtual.fillRect(14, 102, fomeW, 12, corFome);

  // Felicidade
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setCursor(12, 135);
  telaVirtual.print("Feliz");

  uint16_t corFel = felicidade > 50 ? TFT_BLUE : (felicidade > 25 ? TFT_YELLOW : TFT_RED);
  telaVirtual.setTextColor(corFel);
  telaVirtual.setCursor(85, 135);
  telaVirtual.print(felicidade);
  telaVirtual.print("%");

  telaVirtual.drawRect(12, 155, 111, 16, TFT_WHITE);
  int felW = map(felicidade, 0, 100, 0, 107);
  if (felW > 0) telaVirtual.fillRect(14, 157, felW, 12, corFel);
}

// ==========================================
// TELA DO MINIJOGO DE COMER (Maçã fatiada e com tremor)
// ==========================================
void desenharTelaComer() {
  if (cliquesComida >= 0 && cliquesComida < 10) {
    int cx = 67;
    int cy = 120;
    int raio = 30;

    // Efeito de Tremor por 150ms
    if (millis() - ultimoTremorComida < 150) {
      cx += random(-2, 3);
      cy += random(-2, 3);
    }

    if (comidaSelecionada == 0) { // Maca
      // Cabinho da Maçã (Marrom: 0x5A00)
      telaVirtual.fillRect(cx - 2, cy - raio - 10, 4, 12, 0x5A00);
      // Folha da Maçã (Verde)
      telaVirtual.fillEllipse(cx + 4, cy - raio - 8, 6, 3, TFT_GREEN);
      // Corpo da Maçã (Vermelho)
      telaVirtual.fillCircle(cx - 8, cy, raio, TFT_RED);
      telaVirtual.fillCircle(cx + 8, cy, raio, TFT_RED);
    } else { // Doce
      // Corpo do Doce (Azul claro ou Rosa)
      telaVirtual.fillCircle(cx, cy, raio - 5, TFT_MAGENTA);
      // Espiral interna do doce
      telaVirtual.drawCircle(cx, cy, raio - 15, TFT_WHITE);
      // Embrulho (Triangulos)
      telaVirtual.fillTriangle(cx - 20, cy, cx - 45, cy - 20, cx - 45, cy + 20, TFT_MAGENTA);
      telaVirtual.fillTriangle(cx + 20, cy, cx + 45, cy - 20, cx + 45, cy + 20, TFT_MAGENTA);
    }

    // Efeito de Fatiamento (Cortes retangulares pretos)
    if (cliquesComida >= 3 && cliquesComida < 6) {
      telaVirtual.fillRect(cx + 10, cy - raio - 12, 45, 2 * raio + 24, TFT_BLACK);
    } else if (cliquesComida >= 6 && cliquesComida < 10) {
      telaVirtual.fillRect(cx - 15, cy - raio - 12, 70, 2 * raio + 24, TFT_BLACK);
    }
  }
}

// ==========================================
// TELA DOS SUBMENUS
// ==========================================
void desenharSubMenu(const char* titulo, const char* opcoes[], int total) {
  telaVirtual.setTextColor(TFT_CYAN);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(25, 12);
  telaVirtual.print(titulo);
  telaVirtual.drawFastHLine(10, 32, 115, 0x4208);

  for (int i = 0; i < total; i++) {
    int y = 55 + i * 45;

    if (i == subMenuCursor) {
      telaVirtual.fillRect(10, y - 5, 115, 28, TFT_WHITE);
      telaVirtual.setTextColor(TFT_BLACK);
    } else {
      telaVirtual.setTextColor(TFT_WHITE);
    }
    telaVirtual.setTextSize(2);
    telaVirtual.setCursor(22, y);
    telaVirtual.print(opcoes[i]);
  }
}

// ==========================================
// TELA DO MINIJOGO DE PULAR
// ==========================================
void desenharTelaPular(unsigned long agora) {
  // Chão
  telaVirtual.drawFastHLine(0, 180, 135, TFT_WHITE);
  
  // Placar
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(5, 5);
  telaVirtual.print("Pts: ");
  telaVirtual.print(obsCount);

  // Pet
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  int spriteW = w * 3;
  int spriteH = h * 3;
  int x = 30; // Fixo no canto esquerdo
  int y = 180 - spriteH + pularY;
  
  desenharSpriteEscalado(sprite, w, h, x, y, 3);
  
  // Obstáculo (bloco vermelho)
  telaVirtual.fillRect(obsX, 160, 20, 20, TFT_RED);
}

// ==========================================
// DESENHO DE CORAÇÃO E TELA DE CARINHO
// ==========================================
void desenharCoracao(int cx, int cy) {
  telaVirtual.fillCircle(cx - 3, cy - 2, 3, 0xF81F); // Rosa / Pink
  telaVirtual.fillCircle(cx + 3, cy - 2, 3, 0xF81F);
  telaVirtual.fillTriangle(cx - 6, cy - 1, cx + 6, cy - 1, cx, cy + 7, 0xF81F);
}

void desenharTelaCarinho(unsigned long agora) {
  int respY = (int)(sin(agora / 800.0) * 2.0);

  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  int spriteW = w * 3;
  int spriteH = h * 3;
  int x = (135 - spriteW) / 2;
  int y = (240 - spriteH) / 2 + respY;

  int offsetX = 0;
  int offsetY = 0;

  // Pet treme por 150ms após o clique
  if (agora - ultimoCliqueCarinho < 150) {
    offsetX = random(-2, 3);
    offsetY = random(-2, 3);
  }

  // Desenhar pet com tremor e respiração
  desenharSpriteEscalado(sprite, w, h, x + offsetX, y + offsetY, 3);

  // Desenhar corações flutuando por 450ms
  if (agora - ultimoCliqueCarinho < 450) {
    float progresso = (agora - ultimoCliqueCarinho) / 450.0;
    int deltaY = (int)(progresso * 40.0);

    // Coração da esquerda
    desenharCoracao(x - 15, y + 25 - deltaY);
    // Coração da direita
    desenharCoracao(x + spriteW + 15, y + 10 - deltaY);
  }
}

// ==========================================
// TELA DE MENSAGEM DE SUCESSO TEMPORÁRIA
// ==========================================
void desenharMensagemSucesso() {
  telaVirtual.setTextColor(TFT_GREEN);
  telaVirtual.setTextSize(3);
  int xTitulo = 37;
  if (strcmp(msgSucessoTitulo, "Nham!") == 0) {
    xTitulo = 22;
  }
  telaVirtual.setCursor(xTitulo, 80);
  telaVirtual.print(msgSucessoTitulo);

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  int xSub = 32;
  if (strcmp(msgSucessoSub, "+ carinho") == 0) {
    xSub = 15;
  }
  telaVirtual.setCursor(xSub, 130);
  telaVirtual.print(msgSucessoSub);
}

// ==========================================
// TELA DE GAME OVER
// ==========================================
void desenharGameOver() {
  int w = lapide_width;
  int h = lapide_height;
  int spriteW = w * 3;
  int spriteH = h * 3;
  int x = (135 - spriteW) / 2;
  int y = 40;
  
  desenharSpriteEscalado(lapide_sprite, w, h, x, y, 3);

  telaVirtual.setTextColor(TFT_RED);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(20, y + spriteH + 10);
  telaVirtual.print("FALECEU");

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  int minutos = (idade * 5) / 60;
  telaVirtual.setCursor(20, y + spriteH + 35);
  telaVirtual.print("Viveu: ");
  telaVirtual.print(minutos);
  telaVirtual.print(" min");
}

// ==========================================
// Resetar o jogo
// ==========================================
void reiniciarJogo() {
  fome = 100;
  felicidade = 100;
  idade = 0;
  vivo = true;
  estadoAtual = TELA_PET;
  ultimoTurno = millis();
  
  preferences.putInt("fome", fome);
  preferences.putInt("felicidade", felicidade);
  preferences.putInt("idade", idade);
  preferences.putBool("vivo", vivo);
}