#include <TFT_eSPI.h>
#include <SPI.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <WiFi.h>
#include <pgmspace.h>
#include <time.h>
#include <sys/time.h>

#include "sprites.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite telaVirtual = TFT_eSprite(&tft); // Double Buffer
Preferences preferences;

#define BUZZER_PIN 27

// Pinos dos Botões
const int BTN_ESQUERDO = 0;   // IO0  - Abre Menu / Navega opções
const int BTN_DIREITO  = 35;  // IO35 - Seleciona opção

// Status do Pet
int fome = 100;
int felicidade = 100;
int idade = 0;
bool vivo = true;
bool temCoco = false;
bool taDoente = false;
bool curouDoenca = false;
bool comidaBloqueada = false;
int turnosSujo = 0;
int turnosZerado = 0;

// Controles de Tempo
unsigned long ultimoTurno = 0;
const unsigned long TICK_INTERVAL = 60000; // 60 segundos por turno
unsigned long ultimaInteracao = 0;
const unsigned long TEMPO_REPOUSO = 60000; // 60 segundos (1 minuto) de inatividade para dormir

// Variáveis do Ajuste de Hora
int ajusteHora = 12;
int ajusteMinuto = 0;
int campoAjuste = 0; // 0 = Horas, 1 = Minutos, 2 = Confirmar
bool horaDefinida = false;
time_t ultimoTurnoTime = 0;

// Debounce dos Botões
unsigned long ultimoCliqueEsq = 0;
unsigned long ultimoCliqueDir = 0;

// Estado da Interface (Máquina de Estados)
const int TELA_PET = 0;
const int TELA_MENU = 1;
const int TELA_STATUS = 2;
const int TELA_GAMEOVER = 3;
const int TELA_COMER = 4;
const int TELA_MSG_SUCESSO = 5;
const int TELA_CARINHO = 6;
const int TELA_SUBMENU_COMER = 7;
const int TELA_SUBMENU_BRINCAR = 8;
const int TELA_PULAR = 9;
const int TELA_BANHO = 10;
const int TELA_REMEDIO = 11;
const int TELA_AJUSTE_HORA = 12;
const int TELA_SUBMENU_OPCOES = 13;
const int TELA_CONFIRM_RESET = 14;
int estadoAtual = TELA_PET;

bool estaDormindo() {
  if (!horaDefinida) return false;
  if (idade < 60) return false; // Ovo/bebê não dorme
  time_t agora_t = time(NULL);
  struct tm *t_info = localtime(&agora_t);
  int hora = t_info->tm_hour;
  return (hora >= 22 || hora < 8);
}

// Variáveis de Sucesso Temporário
unsigned long tempoMensagemSucesso = 0;
const char* msgSucessoTitulo = "";
const char* msgSucessoSub = "";

// Menu de Ações
int menuCursor = 0;
const int MENU_TOTAL = 7;
String menuOpcoes[MENU_TOTAL] = {"Comer", "Brincar", "Carinho", "Banho", "Remedio", "Status", "Opcoes"};

// Submenus
int subMenuCursor = 0;
int resetConfirmCursor = 1; // 0 = Sim, 1 = Nao
const char* menuOpcoesSub[3] = {"Hora", "Resetar", "Voltar"};

const char* menuComida[4] = {"Maca", "Pizza", "Hamburguer", "Cogumelo"};
const char* menuBrincar[] = {"Pular"};

// Comida Selecionada
int comidaSelecionada = 0;

// Temporizador de Animações Automáticas
unsigned long tempoInicioAnimacao = 0;

// ==========================================
// FUNÇÕES DE SOM
// ==========================================
void tocarSomBeep() { tone(BUZZER_PIN, 1000, 50); }
void tocarSomPulo() { tone(BUZZER_PIN, 600, 100); delay(100); tone(BUZZER_PIN, 800, 100); }
void tocarSomSucesso() { tone(BUZZER_PIN, 1000, 100); delay(100); tone(BUZZER_PIN, 1200, 150); delay(150); tone(BUZZER_PIN, 1500, 300); }
void tocarSomFalha() { tone(BUZZER_PIN, 300, 200); delay(200); tone(BUZZER_PIN, 200, 300); }
void tocarSomComer() { tone(BUZZER_PIN, 800, 50); delay(100); tone(BUZZER_PIN, 900, 50); delay(100); tone(BUZZER_PIN, 1000, 50); }
void tocarSomCarinho() { tone(BUZZER_PIN, 1200, 100); delay(150); tone(BUZZER_PIN, 1300, 200); }
void tocarSomBanho() { tone(BUZZER_PIN, 500, 100); delay(100); tone(BUZZER_PIN, 600, 100); delay(100); tone(BUZZER_PIN, 500, 100); }
void tocarSomRemedio() { tone(BUZZER_PIN, 1500, 100); delay(100); tone(BUZZER_PIN, 1200, 150); delay(150); tone(BUZZER_PIN, 1000, 200); }
void tocarSomBoot() { tone(BUZZER_PIN, 440, 150); delay(150); tone(BUZZER_PIN, 554, 150); delay(150); tone(BUZZER_PIN, 659, 200); delay(200); tone(BUZZER_PIN, 880, 400); }
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
    // Acorda a cada 4 segundos (tempo menor para o powerbank não desarmar)
    esp_sleep_enable_timer_wakeup(4000000ULL); 
    
    // Entra em repouso mantendo a RAM
    esp_light_sleep_start();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
      // Acordou pelo timer: dá um pulso agressivo no WiFi para o powerbank
      WiFi.mode(WIFI_STA);
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
      WiFi.begin("KeepAlive", "12345678"); // Tenta conectar
        
      // Mantém o WiFi tentando conectar por 2.5 segundos
      // Os Powerbanks modernos filtram picos rápidos. Precisamos de pelo menos 2s.
      unsigned long tInicio = millis();
      while(millis() - tInicio < 2500) {
        yield(); // Deixa o rádio WiFi trabalhar no background
      }
        
      WiFi.disconnect(true);
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
  temCoco = preferences.getBool("temCoco", false);
  taDoente = preferences.getBool("taDoente", false);
  turnosSujo = preferences.getInt("turnosSujo", 0);
  turnosZerado = preferences.getInt("tZerado", 0);
  esp_reset_reason_t r_reason = esp_reset_reason();
  if (r_reason == ESP_RST_POWERON || r_reason == ESP_RST_BROWNOUT || r_reason == ESP_RST_EXT) {
    horaDefinida = false;
  } else {
    horaDefinida = preferences.getBool("horaDefinida", false);
  }
  ultimoTurnoTime = (time_t)preferences.getUInt("uTurnoTime", 0);

  // Inicializa a Tela
  tft.init();
  tft.setRotation(0); // Orientação Vertical
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // Cria a Tela Virtual na RAM (135x240)
  telaVirtual.createSprite(135, 240);

  // Mantém o Wi-Fi ligado e impede o sleep do modem para aumentar o consumo (~120mA)
  // Isso evita que o powerbank desligue por baixo consumo enquanto o pet está acordado
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  ultimaInteracao = millis();
  
  if (!horaDefinida) {
    estadoAtual = TELA_AJUSTE_HORA;
  }
  
  tocarSomBoot();
}

void loop() {
  unsigned long agora = millis();

  // ==========================================
  // KEEP ALIVE DO POWERBANK (Picos de Wi-Fi)
  // ==========================================
  // Gera um pico de consumo de ~250mA a cada 5 segundos para impedir que o powerbank desligue
  static unsigned long ultimoScan = 0;
  if (agora - ultimoScan > 5000) {
    ultimoScan = agora;
    WiFi.scanNetworks(true); // Scan assíncrono (não trava o jogo)
  }


  // Verifica inatividade para entrar em modo repouso (apenas se for horário de dormir)
  if (estaDormindo() && (agora - ultimaInteracao > TEMPO_REPOUSO)) {
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
  static bool fastForwarded = false;
  static unsigned long ultimoIncrementoHora = 0;

  bool esqPress = false;
  bool dirPress = false;

  // Botão Esquerdo (Hold de 400ms)
  if (digitalRead(BTN_ESQUERDO) == LOW) {
    ultimaInteracao = agora;
    if (!botaoEsqPressionado) {
      botaoEsqPressionado = true;
      tempoBotaoEsq = agora;
      fastForwarded = false;
    } else {
      if (!segurandoStatus && (agora - tempoBotaoEsq > 400) && vivo && (estadoAtual == TELA_PET || estadoAtual == TELA_MENU)) {
        segurandoStatus = true;
        estadoAtual = TELA_STATUS;
      }

      // Avanço rápido no hold do botão esquerdo na tela de ajuste de hora
      if (estadoAtual == TELA_AJUSTE_HORA && (agora - tempoBotaoEsq > 400)) {
        if (agora - ultimoIncrementoHora > 100) {
          ultimoIncrementoHora = agora;
          fastForwarded = true;
          if (campoAjuste == 0) {
            ajusteHora = (ajusteHora + 1) % 24;
            tocarSomBeep();
          } else if (campoAjuste == 1) {
            ajusteMinuto = (ajusteMinuto + 1) % 60;
            tocarSomBeep();
          }
        }
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
        if (duracao > 50 && !fastForwarded) { // Debounce mínimo para clique
          esqPress = true;
        }
        fastForwarded = false;
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
      case TELA_AJUSTE_HORA:
        if (campoAjuste == 0) {
          ajusteHora = (ajusteHora + 1) % 24;
          tocarSomBeep();
        } else if (campoAjuste == 1) {
          ajusteMinuto = (ajusteMinuto + 1) % 60;
          tocarSomBeep();
        } else if (campoAjuste == 2) {
          campoAjuste = 0; // Volta para horas se apertar esquerdo no SALVAR
          tocarSomBeep();
        }
        break;
      case TELA_PET:
        if (estaDormindo()) {
          tocarSomFalha();
        } else {
          estadoAtual = TELA_MENU;
          menuCursor = 0;
        }
        break;
      case TELA_MENU:
        menuCursor++;
        if (menuCursor >= MENU_TOTAL) {
          estadoAtual = TELA_PET;
          menuCursor = 0;
        }
        tocarSomBeep();
        break;
      case TELA_STATUS:
        estadoAtual = TELA_PET;
        break;
      case TELA_GAMEOVER:
        reiniciarJogo();
        break;
      case TELA_COMER:
      case TELA_CARINHO:
      case TELA_PULAR:
      case TELA_BANHO:
        // Ignora clique
        break;
      case TELA_SUBMENU_COMER:
      case TELA_SUBMENU_BRINCAR:
        if (estadoAtual == TELA_SUBMENU_COMER) subMenuCursor = (subMenuCursor + 1) % 2;
        else if (estadoAtual == TELA_SUBMENU_BRINCAR) subMenuCursor = (subMenuCursor + 1) % 1;
        tocarSomBeep();
        break;
      case TELA_SUBMENU_OPCOES:
        subMenuCursor = (subMenuCursor + 1) % 3;
        tocarSomBeep();
        break;
      case TELA_CONFIRM_RESET:
        resetConfirmCursor = (resetConfirmCursor + 1) % 2;
        tocarSomBeep();
        break;
    }
  }

  // Botão Direito: Seleciona opção do menu
  if (dirPress) {
    switch (estadoAtual) {
      case TELA_AJUSTE_HORA:
        if (campoAjuste == 0 || campoAjuste == 1) {
          campoAjuste = (campoAjuste + 1) % 3;
          tocarSomBeep();
        } else if (campoAjuste == 2) {
          struct tm t;
          t.tm_year = 2026 - 1900;
          t.tm_mon = 5;
          t.tm_mday = 5;
          t.tm_hour = ajusteHora;
          t.tm_min = ajusteMinuto;
          t.tm_sec = 0;
          t.tm_isdst = -1;
          time_t epoch = mktime(&t);
          struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
          settimeofday(&tv, NULL);
          
          horaDefinida = true;
          preferences.putBool("horaDefinida", true);
          
          ultimoTurnoTime = epoch;
          preferences.putUInt("uTurnoTime", (uint32_t)ultimoTurnoTime);
          
          estadoAtual = TELA_PET;
          ultimaInteracao = millis();
          tocarSomSucesso();
        }
        break;
      case TELA_PET:
        if (estaDormindo()) {
          tocarSomFalha();
        }
        break;
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
      case TELA_CARINHO:
      case TELA_PULAR:
      case TELA_BANHO:
        // Ignora cliques durante a animação
        break;
      case TELA_SUBMENU_COMER:
        executarSubMenuComida();
        break;
      case TELA_SUBMENU_OPCOES:
        if (subMenuCursor == 0) { // Hora
          ajusteHora = 12;
          ajusteMinuto = 0;
          campoAjuste = 0;
          estadoAtual = TELA_AJUSTE_HORA;
          tocarSomBeep();
        } else if (subMenuCursor == 1) { // Resetar
          resetConfirmCursor = 1; // Default para 'Nao'
          estadoAtual = TELA_CONFIRM_RESET;
          tocarSomBeep();
        } else { // Voltar
          estadoAtual = TELA_PET;
          tocarSomBeep();
        }
        break;
      case TELA_CONFIRM_RESET:
        if (resetConfirmCursor == 0) { // Sim
          reiniciarJogo();
        } else { // Nao
          estadoAtual = TELA_SUBMENU_OPCOES;
          tocarSomBeep();
        }
        break;
      default:
        break;
    }
  }

  // ==========================================
  // 3. GAME TICK - Decaimento dos Stats
  // ==========================================
  if (horaDefinida && vivo && estadoAtual != TELA_AJUSTE_HORA) {
    time_t agoraTime = time(NULL);
    if (ultimoTurnoTime == 0) {
      ultimoTurnoTime = agoraTime;
      preferences.putUInt("uTurnoTime", (uint32_t)ultimoTurnoTime);
    }

    int segundosPassados = agoraTime - ultimoTurnoTime;
    int ticksPassados = segundosPassados / 60;

    if (ticksPassados > 0) {
      for (int i = 0; i < ticksPassados; i++) {
        if (vivo) {
          time_t tickEpoch = ultimoTurnoTime + (i + 1) * 60;
          struct tm *tick_t = localtime(&tickEpoch);
          int tickHora = tick_t->tm_hour;
          bool tickDormindo = (tickHora >= 22 || tickHora < 8) && (idade >= 60);

          if (!tickDormindo) {
            processarTickStatus();
          } else {
            idade++;
          }
        }
      }
      ultimoTurnoTime += ticksPassados * 60;
      preferences.putUInt("uTurnoTime", (uint32_t)ultimoTurnoTime);

      // Salva progresso no Tick
      preferences.putInt("fome", fome);
      preferences.putInt("felicidade", felicidade);
      preferences.putInt("idade", idade);
      preferences.putBool("vivo", vivo);
      preferences.putBool("temCoco", temCoco);
      preferences.putBool("taDoente", taDoente);
      preferences.putInt("turnosSujo", turnosSujo);
      preferences.putInt("tZerado", turnosZerado);
    }
  }

  // ==========================================
  // CONTROLE DE ANIMAÇÕES AUTOMÁTICAS
  // ==========================================
  if (estadoAtual == TELA_COMER || estadoAtual == TELA_CARINHO || estadoAtual == TELA_PULAR || estadoAtual == TELA_BANHO || estadoAtual == TELA_REMEDIO) {
    if (agora - tempoInicioAnimacao > 1500) { // Animação dura 1.5s
      if (estadoAtual == TELA_BANHO) {
        temCoco = false;
        turnosSujo = 0;
        felicidade = min(100, felicidade + 10);
      }
      estadoAtual = TELA_PET;
      // Salva progresso após a ação
      preferences.putInt("fome", fome);
      preferences.putInt("felicidade", felicidade);
      preferences.putBool("temCoco", temCoco);
      preferences.putInt("turnosSujo", turnosSujo);
      preferences.putInt("tZerado", turnosZerado);
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
    case TELA_SUBMENU_COMER: desenharSubMenu("COMIDA", menuComida, 4); break;
    case TELA_SUBMENU_BRINCAR: desenharSubMenu("BRINCAR", menuBrincar, 1); break;
    case TELA_PULAR:    desenharTelaPular(agora); break;
    case TELA_BANHO:    desenharTelaBanho(agora); break;
    case TELA_REMEDIO:  desenharTelaRemedio(agora); break;
    case TELA_AJUSTE_HORA: desenharAjusteHora(); break;
    case TELA_SUBMENU_OPCOES: desenharSubMenuOpcoes(); break;
    case TELA_CONFIRM_RESET: desenharTelaConfirmReset(); break;
  }

  telaVirtual.pushSprite(0, 0);
}

// ==========================================
// Executa a ação selecionada no menu
// ==========================================
void executarMenuAcao() {
  tocarSomBeep();
  switch (menuCursor) {
    case 0:
      if (temCoco) {
        comidaBloqueada = true;
        tocarSomFalha();
        estadoAtual = TELA_COMER;
        tempoInicioAnimacao = millis();
      } else {
        comidaBloqueada = false;
        subMenuCursor = random(0, 4);
        executarSubMenuComida();
      }
      break;
    case 1:
      if (!taDoente && !temCoco) felicidade += 20;
      if (felicidade > 100) felicidade = 100;
      estadoAtual = TELA_PULAR;
      tempoInicioAnimacao = millis();
      tocarSomPulo();
      break;
    case 2:
      if (!taDoente && !temCoco) felicidade += 15;
      if (felicidade > 100) felicidade = 100;
      estadoAtual = TELA_CARINHO;
      tempoInicioAnimacao = millis();
      tocarSomCarinho();
      break;
    case 3:
      estadoAtual = TELA_BANHO;
      tempoInicioAnimacao = millis();
      tocarSomBanho();
      break;
    case 4:
      estadoAtual = TELA_REMEDIO;
      tempoInicioAnimacao = millis();
      if (taDoente) {
        taDoente = false;
        curouDoenca = true;
        tocarSomRemedio();
      } else {
        curouDoenca = false;
        tocarSomFalha();
      }
      break;
    case 5:
      estadoAtual = TELA_STATUS;
      break;
    case 6:
      estadoAtual = TELA_SUBMENU_OPCOES;
      subMenuCursor = 0;
      break;
  }
}

void executarSubMenuComida() {
  tocarSomBeep();
  comidaSelecionada = subMenuCursor;
  if (comidaSelecionada == 0) {
    fome += 40; // Maca
  } else if (comidaSelecionada == 1) {
    fome += 50; // Pizza
    felicidade += 10;
  } else if (comidaSelecionada == 2) {
    fome += 60; // Hamburguer
  } else {
    fome += 20; // Cogumelo
    felicidade += 30;
  }
  if (fome > 100) fome = 100;
  if (felicidade > 100) felicidade = 100;
  
  estadoAtual = TELA_COMER;
  tempoInicioAnimacao = millis();
  tocarSomComer();
}

void executarSubMenuBrincar() {
  tocarSomBeep();
  if (subMenuCursor == 0) {
    felicidade += 20;
    if (felicidade > 100) felicidade = 100;
    estadoAtual = TELA_PULAR;
    tocarSomPulo();
  }
  tempoInicioAnimacao = millis();
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
  if (idade < 60) {
    w = ovo_width;
    h = ovo_height;
    return ovo_sprite;
  } else if (estaDormindo()) {
    w = capivara_sleep_width;
    h = capivara_sleep_height;
    return capivara_sleep_sprite;
  } else if (fome < 30 || felicidade < 30) {
    w = capivara_sad_width;
    h = capivara_sad_height;
    return capivara_sad_sprite;
  } else {
    w = capivara_width;
    h = capivara_height;
    return capivara_sprite;
  }
}

// ==========================================
// TELA PRINCIPAL: Pet centralizado + respiração
// ==========================================
// ==========================================
// TELA PET (principal)
// ==========================================
void desenharTelaPet(unsigned long agora) {
  int respY = (int)(sin(agora / 800.0) * 1.0);
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  
  int x = (27 - w) / 2;
  int y = (48 - h) / 2 + 5 + respY;

  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);

  if (estaDormindo()) {
    // Animação flutuante do ZZZ
    unsigned long ciclo = agora % 3000;
    float floatY = (ciclo / 3000.0) * 12.0;
    int zX = x + w - 4;
    int zY = y - 4 - (int)floatY;
    
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setTextSize(1);
    telaVirtual.setCursor(zX * 5, zY * 5);
    if (ciclo < 1000) {
      telaVirtual.print("z");
    } else if (ciclo < 2000) {
      telaVirtual.print("zZ");
    } else {
      telaVirtual.print("zZZ");
    }
  } else {
    if (taDoente) {
      desenharSpriteEscalado(icone_doenca_sprite, icone_doenca_width, icone_doenca_height, (x + (w - icone_doenca_width) / 2 + 2) * 5, (y - 6) * 5, 5);
    }

    if (temCoco) {
      int px = x + (w - coco_width) / 2;
      int py = y + h - 5;
      desenharSpriteEscalado(coco_sprite, coco_width, coco_height, px * 5, py * 5, 5);
    }
  }

}

// ==========================================
// TELA DO MENU (tela cheia com opções)
// ==========================================
void desenharMenu() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  String texto = menuOpcoes[menuCursor];
  int textWidth = texto.length() * 12; // Size 2: 10px width + 2px spacing
  int x = (135 - textWidth) / 2;
  int y = 112; // (240 - 16) / 2 = 112 (Centro Vertical)
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  
  // Desenho normal
  telaVirtual.setCursor(x, y); 
  telaVirtual.print(texto);
  
  // Desenho deslocado em 1 pixel (Falso Negrito / Bold)
  telaVirtual.setCursor(x + 1, y);
  telaVirtual.print(texto);

  // Acentos manuais
  if (texto == "Remedio") {
    // Acento agudo no segundo 'e' (index 3). Começa em x + 36.
    int x_char = x + 36;
    telaVirtual.drawLine(x_char + 4, y - 2, x_char + 7, y - 5, TFT_WHITE);
    telaVirtual.drawLine(x_char + 5, y - 2, x_char + 8, y - 5, TFT_WHITE);
  } else if (texto == "Opcoes") {
    // Cedilha no 'c' (index 2). Começa em x + 24.
    int x_c = x + 24;
    telaVirtual.drawLine(x_c + 5, y + 14, x_c + 3, y + 16, TFT_WHITE);
    telaVirtual.drawLine(x_c + 3, y + 16, x_c + 5, y + 17, TFT_WHITE);
    telaVirtual.drawLine(x_c + 6, y + 14, x_c + 4, y + 16, TFT_WHITE);
    telaVirtual.drawLine(x_c + 4, y + 16, x_c + 6, y + 17, TFT_WHITE);

    // Til no 'o' (index 3). Começa em x + 36.
    int x_o = x + 36;
    telaVirtual.fillRect(x_o + 2, y - 4, 3, 2, TFT_WHITE);
    telaVirtual.fillRect(x_o + 5, y - 3, 3, 2, TFT_WHITE);
    telaVirtual.fillRect(x_o + 3, y - 4, 3, 2, TFT_WHITE);
    telaVirtual.fillRect(x_o + 6, y - 3, 3, 2, TFT_WHITE);
  }

  // Setinha para baixo
  int arrowY = y + 26;
  telaVirtual.fillRect(11 * 5, arrowY, 5 * 5, 5, TFT_WHITE);
  telaVirtual.fillRect(12 * 5, arrowY + 5, 3 * 5, 5, TFT_WHITE);
  telaVirtual.fillRect(13 * 5, arrowY + 10, 1 * 5, 5, TFT_WHITE);
}

// ==========================================
// TELA DE STATUS
// ==========================================
void desenharTelaStatus() {
  int dias = idade / 1440;
  String textIdade = "Id:" + String(dias) + "d";
  int wIdade = textIdade.length() * 12;
  int xIdade = (135 - wIdade) / 2;

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(xIdade, 2 * 5);
  telaVirtual.print(textIdade);

  // Relógio abaixo da idade
  time_t agora_t = time(NULL);
  struct tm *t_info = localtime(&agora_t);
  char clockBuf[10];
  sprintf(clockBuf, "%02d:%02d", t_info->tm_hour, t_info->tm_min);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(52, 8 * 5);
  telaVirtual.print(clockBuf);

  // --- FOME: ícone label + 3 indicadores ---
  // Ícone label (esquerda)
  desenharSpriteEscalado(icone_fome_sprite, icone_fome_width, icone_fome_height, 1 * 5, 15 * 5, 5);
  // 3 ícones: somem da direita conforme fome cai
  int qtdFome = round((fome / 100.0) * 3.0);
  int espacoFome = icone_fome_width + 2;
  int xInicioFome = icone_fome_width + 3;
  for (int i = 0; i < 3; i++) {
    if (i < qtdFome) {
      desenharSpriteEscalado(icone_fome_sprite, icone_fome_width, icone_fome_height,
        (xInicioFome + i * espacoFome) * 5, 15 * 5, 5);
    }
  }

  // --- FELICIDADE: ícone label + 3 indicadores ---
  // Ícone label (esquerda)
  desenharSpriteEscalado(icone_felicidade_sprite, icone_felicidade_width, icone_felicidade_height, 1 * 5, 30 * 5, 5);
  // 3 ícones: somem da direita conforme felicidade cai
  int qtdFelz = round((felicidade / 100.0) * 3.0);
  int espacoFelz = icone_felicidade_width + 2;
  int xInicioFelz = icone_felicidade_width + 3;
  for (int i = 0; i < 3; i++) {
    if (i < qtdFelz) {
      desenharSpriteEscalado(icone_felicidade_sprite, icone_felicidade_width, icone_felicidade_height,
        (xInicioFelz + i * espacoFelz) * 5, 30 * 5, 5);
    }
  }
}

// ==========================================
// TELA DE COMER
// ==========================================
void desenharTelaComer() {
  unsigned long progresso = millis() - tempoInicioAnimacao;

  if (comidaBloqueada) {
    // Sujo: mostra pet + interrogação (igual ao remédio sem doença)
    int w, h;
    const unsigned short* sprite = getPetSprite(w, h);
    int x = (27 - w) / 2;
    int y = (48 - h) / 2 + 5;
    desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
    desenharSpriteEscalado(interrogacao_sprite, interrogacao_width, interrogacao_height, (x + w - 5) * 5, (y - 8) * 5, 5);
  } else {
    // Limpo: animação normal de comer
    int cx = 13; 
    int cy = 24; 

    if ((progresso / 100) % 2 == 0) {
      cx += random(-1, 2); 
      cy += random(-1, 2);
    }

    int w, h;
    const unsigned short* sprite;
    if (comidaSelecionada == 0) { sprite = maca_sprite; w = maca_width; h = maca_height; }
    else if (comidaSelecionada == 1) { sprite = pizza_sprite; w = pizza_width; h = pizza_height; }
    else if (comidaSelecionada == 2) { sprite = hamburguer_sprite; w = hamburguer_width; h = hamburguer_height; }
    else { sprite = cogumelo_sprite; w = cogumelo_width; h = cogumelo_height; }
    
    int xOffset = w / 2;
    int yOffset = h / 2;
    
    desenharSpriteEscalado(sprite, w, h, (cx - xOffset) * 5, (cy - yOffset) * 5, 5);

    if (progresso > 500 && progresso < 1000) {
      telaVirtual.fillRect((cx + xOffset - 4) * 5, (cy - yOffset) * 5, 6 * 5, h * 5, TFT_BLACK);
    } else if (progresso >= 1000) {
      telaVirtual.fillRect((cx - xOffset) * 5, (cy - yOffset) * 5, w * 5, h * 5, TFT_BLACK);
    }
  }
}

void desenharTelaRemedio(unsigned long agora) {
  unsigned long progresso = agora - tempoInicioAnimacao;

  int cx = 13; 
  int cy = 24; 

  if ((progresso / 100) % 2 == 0) {
    cx += random(-1, 2); 
    cy += random(-1, 2);
  }

  if (curouDoenca) {
    int w = remedio_width;
    int h = remedio_height;
    int xOffset = w / 2;
    int yOffset = h / 2;
    desenharSpriteEscalado(remedio_sprite, w, h, (cx - xOffset) * 5, (cy - yOffset) * 5, 5);
    
    if (progresso > 500 && progresso < 1000) {
      telaVirtual.fillRect((cx + xOffset - 4) * 5, (cy - yOffset) * 5, 6 * 5, h * 5, TFT_BLACK);
    } else if (progresso >= 1000) {
      telaVirtual.fillRect((cx - xOffset) * 5, (cy - yOffset) * 5, w * 5, h * 5, TFT_BLACK);
    }
  } else {
    int w, h;
    const unsigned short* sprite = getPetSprite(w, h);
    int x = (27 - w) / 2;
    int y = (48 - h) / 2 + 5;
    
    desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
    desenharSpriteEscalado(interrogacao_sprite, interrogacao_width, interrogacao_height, (x + w - 5) * 5, (y - 8) * 5, 5);
  }
}

// ==========================================
// TELA DOS SUBMENUS
// ==========================================
void desenharSubMenu(const char* titulo, const char* opcoes[], int total) {
  telaVirtual.fillScreen(TFT_BLACK);
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

    // Acentos manuais para o submenu de comida ("COMIDA")
    if (strcmp(titulo, "COMIDA") == 0) {
      uint16_t corAcento = (i == subMenuCursor) ? TFT_BLACK : TFT_WHITE;
      if (i == 0) { // Maçã
        // c (index 2) em 4 * 5 + 12 = 32. Cedilha em y * 5 + 7.
        int x_c = 32;
        telaVirtual.drawPixel(x_c + 2, y * 5 + 7, corAcento);
        telaVirtual.drawPixel(x_c + 1, y * 5 + 8, corAcento);
        telaVirtual.drawPixel(x_c + 2, y * 5 + 9, corAcento);
        // a (index 3) em 4 * 5 + 18 = 38. Til em y * 5 - 2 e y * 5 - 1.
        int x_a = 38;
        telaVirtual.drawPixel(x_a + 1, y * 5 - 2, corAcento);
        telaVirtual.drawPixel(x_a + 2, y * 5 - 2, corAcento);
        telaVirtual.drawPixel(x_a + 3, y * 5 - 1, corAcento);
        telaVirtual.drawPixel(x_a + 4, y * 5 - 1, corAcento);
      } else if (i == 2) { // Hambúrguer
        // u (index 4) em 4 * 5 + 24 = 44. Acento agudo acima de y * 5.
        int x_u = 44;
        telaVirtual.drawLine(x_u + 1, y * 5 - 2, x_u + 3, y * 5 - 4, corAcento);
      }
    }
  }
}

void desenharSubMenuOpcoes() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  String texto = menuOpcoesSub[subMenuCursor];
  int textWidth = texto.length() * 12; // Size 2: 10px width + 2px spacing
  int x = (135 - textWidth) / 2;
  int y = 112; // (240 - 16) / 2 = 112 (Centro Vertical)
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  
  // Desenho normal
  telaVirtual.setCursor(x, y); 
  telaVirtual.print(texto);
  
  // Desenho deslocado em 1 pixel (Falso Negrito / Bold)
  telaVirtual.setCursor(x + 1, y);
  telaVirtual.print(texto);

  // Setinha para baixo
  int arrowY = y + 26;
  telaVirtual.fillRect(11 * 5, arrowY, 5 * 5, 5, TFT_WHITE);
  telaVirtual.fillRect(12 * 5, arrowY + 5, 3 * 5, 5, TFT_WHITE);
  telaVirtual.fillRect(13 * 5, arrowY + 10, 1 * 5, 5, TFT_WHITE);
}

void desenharTelaConfirmReset() {
  telaVirtual.fillScreen(TFT_BLACK);
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(2 * 5, 8 * 5);
  telaVirtual.print("TEM CERTEZA?");

  const char* opcoes[2] = {"Sim", "Nao"};
  for (int i = 0; i < 2; i++) {
    int y = 20 + i * 9;

    if (i == resetConfirmCursor) {
      telaVirtual.fillRect(4 * 5, (y - 1) * 5, 19 * 5, 6 * 5, TFT_WHITE);
      telaVirtual.setTextColor(TFT_BLACK);
    } else {
      telaVirtual.setTextColor(TFT_WHITE);
    }
    telaVirtual.setTextSize(1);
    telaVirtual.setCursor(6 * 5, y * 5);
    telaVirtual.print(opcoes[i]);

    // Desenha o til no 'a' de "Não" (i == 1)
    if (i == 1) {
      uint16_t corAcento = (i == resetConfirmCursor) ? TFT_BLACK : TFT_WHITE;
      int x_a = 6 * 5 + 1 * 6; // N(0), a(1), o(2) -> 30 + 6 = 36
      telaVirtual.drawPixel(x_a + 1, y * 5 - 2, corAcento);
      telaVirtual.drawPixel(x_a + 2, y * 5 - 2, corAcento);
      telaVirtual.drawPixel(x_a + 3, y * 5 - 1, corAcento);
      telaVirtual.drawPixel(x_a + 4, y * 5 - 1, corAcento);
    }
  }
}

// ==========================================
// TELA DO MINIJOGO DE PULAR
// ==========================================
void desenharTelaPular(unsigned long agora) {
  telaVirtual.fillRect(0 * 5, 41 * 5, 27 * 5, 1 * 5, TFT_WHITE);
  
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  int x = (27 - w) / 2;
  
  unsigned long progresso = agora - tempoInicioAnimacao;
  int puloAutoY = 0;
  if ((progresso / 250) % 2 == 0) { 
    puloAutoY = -5; // Pula 5 pixels lógicos
  }
  
  int y = 41 - h + puloAutoY;
  
  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
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
  int y = (48 - h) / 2 + 5 + respY;
  
  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);

  unsigned long progresso = agora - tempoInicioAnimacao;
  float floatY = (progresso / 1500.0) * 8.0;
  desenharCoracao(x + 3, y - 2 - (int)floatY);
  desenharCoracao(x + w - 4, y - 5 - (int)floatY);
}

// ==========================================
// TELA DE BANHO
// ==========================================
void desenharTelaBanho(unsigned long agora) {
  unsigned long progresso = agora - tempoInicioAnimacao;
  int w, h;
  const unsigned short* sprite = getPetSprite(w, h);
  
  int x = (27 - w) / 2;
  int y = (48 - h) / 2 + 5;
  
  desenharSpriteEscalado(sprite, w, h, x * 5, y * 5, 5);
  
  if (progresso < 2000) {
    int offset = (progresso / 30) % 48;
    telaVirtual.fillRect((x + 2)*5, ((y - 10 + offset) % 48)*5, 2*5, 2*5, TFT_WHITE);
    telaVirtual.fillRect((x + w - 4)*5, ((y - 20 + (int)(offset * 1.5)) % 48)*5, 3*5, 3*5, TFT_WHITE);
    telaVirtual.fillRect((x + 5)*5, ((y - 5 + (int)(offset * 1.2)) % 48)*5, 1*5, 1*5, TFT_WHITE);
    telaVirtual.fillRect((x + 12)*5, ((y - 30 + (int)(offset * 0.8)) % 48)*5, 2*5, 2*5, TFT_WHITE);
  } else {
    temCoco = false;
    estadoAtual = TELA_PET;
    preferences.putBool("temCoco", temCoco);
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
    
    int dias = idade / 1440;
    String minStr = String(dias) + "d";
    
    int textWidth = minStr.length() * 12;
    int cx = (135 - textWidth) / 2;
    
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setTextSize(2);
    
    // Texto em Bold Centrado
    telaVirtual.setCursor(cx, (y + h + 4) * 5);
    telaVirtual.print(minStr);
    telaVirtual.setCursor(cx + 1, (y + h + 4) * 5);
    telaVirtual.print(minStr);
  }

void processarTickStatus() {
  if (temCoco) {
    turnosSujo++;
    if (turnosSujo >= 120) { // 2 horas sujo
      taDoente = true;
    }
  } else {
    turnosSujo = 0;
  }

  if (idade < 60) {
    // Fase de Ovo (Bebê): Decaimento rápido mas imortal
    if (idade % 5 == 0) fome = max(0, fome - 25);
    if (idade % 3 == 0) felicidade = max(0, felicidade - 25);
  } else {
    // Fase Capivara (Normal)
    if (!taDoente) {
      if (!temCoco) {
        if (idade % 4 == 0) fome = max(0, fome - 1);
        if (idade % 3 == 0) felicidade = max(0, felicidade - 1);
      } else {
        if (idade % 4 == 0) fome = max(0, fome - 1);
        felicidade = max(0, felicidade - 1); // -1 por minuto quando sujo
      }
    } else {
      if (idade % 2 == 0) fome = max(0, fome - 1);
      felicidade = max(0, felicidade - 1); // -1 por minuto quando doente
    }
  }
  
  idade++;

  int cocoChance = (idade < 60) ? 20 : 480;
  if (!temCoco && random(cocoChance) == 0) {
    temCoco = true;
  }

  // Agoniza por 2 horas após crescer se estiver zerado
  if (idade >= 60) {
    if (fome <= 0 || felicidade <= 0) {
      turnosZerado++;
      if (turnosZerado % 15 == 0 && turnosZerado < 120) {
        tocarSomBeep(); // Beepa a cada 15 min de agonia
      }
      if (turnosZerado >= 120) {
        fome = 0;
        felicidade = 0;
        vivo = false;
        estadoAtual = TELA_GAMEOVER;
        tocarSomFalha();
      }
    } else {
      turnosZerado = 0;
    }
  }
}

void desenharAjusteHora() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  char hBuf[4];
  char mBuf[4];
  sprintf(hBuf, "%02d", ajusteHora);
  sprintf(mBuf, "%02d", ajusteMinuto);
  
  telaVirtual.setTextSize(2);
  
  if (campoAjuste == 0) {
    telaVirtual.fillRect(2 * 5, 14 * 5, 10 * 5, 10 * 5, TFT_WHITE);
    telaVirtual.setTextColor(TFT_BLACK);
    telaVirtual.setCursor(3 * 5, 15 * 5);
    telaVirtual.print(hBuf);
    
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setCursor(14 * 5, 15 * 5);
    telaVirtual.print(mBuf);
    
    telaVirtual.setCursor(10 * 5, 15 * 5);
    telaVirtual.print(":");
  } else if (campoAjuste == 1) {
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setCursor(3 * 5, 15 * 5);
    telaVirtual.print(hBuf);
    
    telaVirtual.fillRect(13 * 5, 14 * 5, 11 * 5, 10 * 5, TFT_WHITE);
    telaVirtual.setTextColor(TFT_BLACK);
    telaVirtual.setCursor(14 * 5, 15 * 5);
    telaVirtual.print(mBuf);
    
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setCursor(10 * 5, 15 * 5);
    telaVirtual.print(":");
  } else {
    telaVirtual.setTextColor(TFT_WHITE);
    telaVirtual.setCursor(3 * 5, 15 * 5);
    char tBuf[10];
    sprintf(tBuf, "%02d:%02d", ajusteHora, ajusteMinuto);
    telaVirtual.print(tBuf);
  }
  
  if (campoAjuste == 2) {
    telaVirtual.fillRect(7 * 5, 30 * 5, 13 * 5, 6 * 5, TFT_WHITE);
    telaVirtual.setTextColor(TFT_BLACK);
  } else {
    telaVirtual.drawRect(7 * 5, 30 * 5, 13 * 5, 6 * 5, TFT_WHITE);
    telaVirtual.setTextColor(TFT_WHITE);
  }
  telaVirtual.setTextSize(1);
  telaVirtual.setCursor(11 * 5, 32 * 5);
  telaVirtual.print("SALVAR");
}

void reiniciarJogo() {
  fome = 100;
  felicidade = 100;
  idade = 0;
  vivo = true;
  temCoco = false;
  taDoente = false;
  turnosSujo = 0;
  horaDefinida = false;
  preferences.putBool("horaDefinida", false);
  estadoAtual = TELA_AJUSTE_HORA;
  campoAjuste = 0;
  ajusteHora = 12;
  ajusteMinuto = 0;
  
  preferences.putInt("fome", fome);
  preferences.putInt("felicidade", felicidade);
  preferences.putInt("idade", idade);
  preferences.putBool("vivo", vivo);
  preferences.putBool("temCoco", temCoco);
  preferences.putBool("taDoente", taDoente);
  preferences.putInt("turnosSujo", turnosSujo);
}
