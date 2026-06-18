import re

def patch_html():
    with open('simulador_arduino.html', 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Add TELA_STATUS_VIDA
    content = content.replace(
        "TELA_BOOT = 15, TELA_DESLIGADO = -1;",
        "TELA_BOOT = 15, TELA_STATUS_VIDA = 16, TELA_DESLIGADO = -1;"
    )

    # 2. Add desenharTelaStatusVida function
    new_func = """    function desenharTelaStatusVida() {
      fillRect(0, 0, 27, 48, '#000000');
      const dias = Math.floor(idade / 1440);
      const textDias = "Dias:" + dias;
      const textMinutos = "Min:" + idade;

      const wDias = textDias.length * 12;
      const xDias = (135 - wDias) / 2 / 5;
      drawText(textDias, xDias, 10, '#ffffff', 2);

      const wMinutos = textMinutos.length * 12;
      const xMinutos = (135 - wMinutos) / 2 / 5;
      drawText(textMinutos, xMinutos, 25, '#ffffff', 2);
    }
"""
    if 'function desenharTelaStatusVida' not in content:
        content = content.replace(
            "function desenharCoracao(cx, cy)",
            new_func + "\n    function desenharCoracao(cx, cy)"
        )

    # 3. Add to switch in render
    if 'case TELA_STATUS_VIDA:' not in content:
        content = content.replace(
            "case TELA_BOOT:     desenharTelaBoot(); break;",
            "case TELA_BOOT:     desenharTelaBoot(); break;\n        case TELA_STATUS_VIDA: desenharTelaStatusVida(); break;"
        )

    # 4. Add to badgeMap
    if '[TELA_STATUS_VIDA]' not in content:
        content = content.replace(
            "[TELA_BOOT]:     { text: '🚀 Inicializando', cls: 'screen-mode-badge status' },",
            "[TELA_BOOT]:     { text: '🚀 Inicializando', cls: 'screen-mode-badge status' },\n        [TELA_STATUS_VIDA]: { text: '📈 Status de Vida', cls: 'screen-mode-badge status' },"
        )

    # 5. Right button hold logic
    hold_logic = """
      // Botão Direito Hold Logic
      if (botaoDirPressionado && !segurandoStatusVida && (now - tempoBotaoDir > 400) && vivo && (estadoAtual === TELA_PET || estadoAtual === TELA_MENU)) {
        segurandoStatusVida = true;
        estadoAtual = TELA_STATUS_VIDA;
      }
"""
    if 'Botão Direito Hold Logic' not in content:
        content = content.replace(
            "// Avanço rápido no hold do botão esquerdo",
            hold_logic + "\n      // Avanço rápido no hold do botão esquerdo"
        )

    # 6. Variables and functions for right button
    right_btn_logic = """
    let tempoBotaoDir = 0;
    let segurandoStatusVida = false;
    let botaoDirPressionado = false;

    function iniciarHoldDireito() {
      if (!vivo) {
        botaoDirPressionado = true;
        return;
      }
      if (!botaoDirPressionado) {
        botaoDirPressionado = true;
        segurandoStatusVida = false;
        tempoBotaoDir = performance.now();
      }
    }

    function liberarDireito() {
      if (!botaoDirPressionado) return;
      botaoDirPressionado = false;

      if (segurandoStatusVida) {
        segurandoStatusVida = false;
        if (estadoAtual === TELA_STATUS_VIDA) {
          estadoAtual = TELA_PET;
        }
      } else {
        executarCliqueDireito();
      }
    }

    function executarCliqueDireito() {
      pressBtn('btn-dir');
      if (estadoAtual === TELA_AJUSTE_HORA) {
"""
    if 'let tempoBotaoDir = 0;' not in content:
        content = content.replace(
            "function onBtnDireito() {\n      pressBtn('btn-dir');\n      if (estadoAtual === TELA_AJUSTE_HORA) {",
            right_btn_logic
        )

        content = content.replace(
            "function onBtnDireito(e) {\n      pressBtn('btn-dir');\n      if (estadoAtual === TELA_AJUSTE_HORA) {",
            right_btn_logic
        )
    
    # Replace calls to onBtnDireito with iniciarHoldDireito / liberarDireito in mousedown/up/touchstart/end/cancel/keydown/up
    content = content.replace("onBtnDireito(e);", "iniciarHoldDireito();")
    content = content.replace("onBtnDireito();", "iniciarHoldDireito();")
    
    # 7. Rename the original onBtnDireito body execution properly, except we already replaced the signature.
    # We still need event listeners for the right button physical button on screen.
    btn_dir_listeners = """
    function onPressDirStart(e) {
      if (e) e.preventDefault();
      btnDir.classList.add('pressed');
      iniciarHoldDireito();
    }

    function onPressDirEnd(e) {
      if (e) e.preventDefault();
      btnDir.classList.remove('pressed');
      liberarDireito();
    }

    btnDir.addEventListener('mousedown', onPressDirStart);
    btnDir.addEventListener('touchstart', onPressDirStart, { passive: false });
    
    // Add to window mouseup/touchend
"""
    if 'function onPressDirStart(e)' not in content:
        content = content.replace(
            "btnEsq.addEventListener('touchstart', onPressEsqStart, { passive: false });",
            "btnEsq.addEventListener('touchstart', onPressEsqStart, { passive: false });\n" + btn_dir_listeners
        )
    
    content = content.replace("liberarEsquerdo();\n    });\n    window.addEventListener('touchstart', (e) => {", "liberarEsquerdo();\n      liberarDireito();\n    });\n    window.addEventListener('touchstart', (e) => {")
    content = content.replace("liberarEsquerdo();\n    }, { passive: false });", "liberarEsquerdo();\n      liberarDireito();\n    }, { passive: false });")
    content = content.replace("liberarEsquerdo();\n    });\n    window.addEventListener('touchcancel', (e) => {", "liberarEsquerdo();\n      liberarDireito();\n    });\n    window.addEventListener('touchcancel', (e) => {")
    content = content.replace("liberarEsquerdo();\n    });\n\n    window.addEventListener('keydown'", "liberarEsquerdo();\n      liberarDireito();\n    });\n\n    window.addEventListener('keydown'")
    content = content.replace("if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        onBtnDireito();\n      }", "if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        iniciarHoldDireito();\n      }")
    content = content.replace("if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        iniciarHoldDireito();\n      }", "if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        iniciarHoldDireito();\n      }")
    
    if "if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        liberarDireito();\n      }" not in content:
        content = content.replace(
            "liberarEsquerdo();\n      }\n    });",
            "liberarEsquerdo();\n      }\n      if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {\n        liberarDireito();\n      }\n    });"
        )
    
    # Add TELA_STATUS_VIDA to left click state handler
    content = content.replace(
        "case TELA_STATUS:\n          estadoAtual = TELA_PET;\n          break;",
        "case TELA_STATUS:\n        case TELA_STATUS_VIDA:\n          estadoAtual = TELA_PET;\n          break;"
    )

    with open('simulador_arduino.html', 'w', encoding='utf-8') as f:
        f.write(content)


def patch_arduino():
    with open('arduino/arduino.ino', 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Add TELA_STATUS_VIDA
    if 'const int TELA_STATUS_VIDA' not in content:
        content = content.replace(
            "const int TELA_CONFIRM_RESET = 14;",
            "const int TELA_CONFIRM_RESET = 14;\nconst int TELA_STATUS_VIDA = 15;"
        )

    # 2. Add function desenharTelaStatusVida
    new_func = """
void desenharTelaStatusVida() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  int dias = idade / 1440;
  String textDias = "Dias:" + String(dias);
  String textMinutos = "Min:" + String(idade);
  
  int wDias = textDias.length() * 12;
  int xDias = (135 - wDias) / 2;
  
  int wMinutos = textMinutos.length() * 12;
  int xMinutos = (135 - wMinutos) / 2;
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(xDias, 10 * 5);
  telaVirtual.print(textDias);
  
  telaVirtual.setCursor(xMinutos, 25 * 5);
  telaVirtual.print(textMinutos);
}
"""
    if 'void desenharTelaStatusVida()' not in content:
        content = content.replace(
            "void desenharTelaCarinho(unsigned long agora)",
            new_func + "\nvoid desenharTelaCarinho(unsigned long agora)"
        )

    # 3. Add to switch in render
    if 'case TELA_STATUS_VIDA:' not in content:
        content = content.replace(
            "case TELA_CONFIRM_RESET: desenharTelaConfirmReset(); break;",
            "case TELA_CONFIRM_RESET: desenharTelaConfirmReset(); break;\n    case TELA_STATUS_VIDA: desenharTelaStatusVida(); break;"
        )

    # 4. Modify button right logic
    new_btn_logic = """
  static unsigned long tempoBotaoDir = 0;
  static bool botaoDirPressionado = false;
  static bool segurandoStatusVida = false;

  // Botão Direito (Hold de 400ms)
  if (digitalRead(BTN_DIREITO) == LOW) {
    ultimaInteracao = agora;
    if (!botaoDirPressionado) {
      botaoDirPressionado = true;
      tempoBotaoDir = agora;
    } else {
      if (!segurandoStatusVida && (agora - tempoBotaoDir > 400) && vivo && (estadoAtual == TELA_PET || estadoAtual == TELA_MENU)) {
        segurandoStatusVida = true;
        estadoAtual = TELA_STATUS_VIDA;
      }
    }
  } else {
    if (botaoDirPressionado) {
      botaoDirPressionado = false;
      unsigned long duracao = agora - tempoBotaoDir;
      if (segurandoStatusVida) {
        segurandoStatusVida = false;
        if (estadoAtual == TELA_STATUS_VIDA) {
          estadoAtual = TELA_PET;
        }
      } else {
        if (duracao > 50) { // Debounce mínimo para clique
          dirPress = true;
        }
      }
    }
  }
"""
    if 'static bool segurandoStatusVida = false;' not in content:
        old_btn_logic = """  // Botão Direito (Debounce clássico)
  if (digitalRead(BTN_DIREITO) == LOW && (agora - ultimoCliqueDir > 250)) {
    ultimaInteracao = agora;
    dirPress = true;
    ultimoCliqueDir = agora;
  }"""
        content = content.replace(old_btn_logic, new_btn_logic)

    # 5. Add TELA_STATUS_VIDA to left click state handler
    content = content.replace(
        "case TELA_STATUS:\n        estadoAtual = TELA_PET;\n        break;",
        "case TELA_STATUS:\n      case TELA_STATUS_VIDA:\n        estadoAtual = TELA_PET;\n        break;"
    )
    
    # And right click state handler (if any, wait right click shouldn't exit TELA_STATUS, wait, left click exits it. But right click might also exit it). Let's see:
    content = content.replace(
        "case TELA_STATUS:\n        estadoAtual = TELA_PET;\n        break;",
        "case TELA_STATUS:\n      case TELA_STATUS_VIDA:\n        estadoAtual = TELA_PET;\n        break;"
    )

    with open('arduino/arduino.ino', 'w', encoding='utf-8') as f:
        f.write(content)

patch_html()
patch_arduino()
print("Patched successfully")
