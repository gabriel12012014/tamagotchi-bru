
    // =============================================
    // BITMAP FONT 5x7 (rǸplica do TFT_eSPI)
    // =============================================
    const FONT_5x7 = {
      32:[0x00,0x00,0x00,0x00,0x00],33:[0x00,0x00,0x5F,0x00,0x00],
      34:[0x00,0x07,0x00,0x07,0x00],37:[0x23,0x13,0x08,0x64,0x62],
      40:[0x00,0x1C,0x22,0x41,0x00],41:[0x00,0x41,0x22,0x1C,0x00],
      45:[0x08,0x08,0x08,0x08,0x08],46:[0x00,0x60,0x60,0x00,0x00],
      47:[0x20,0x10,0x08,0x04,0x02],
      48:[0x3E,0x51,0x49,0x45,0x3E],49:[0x00,0x42,0x7F,0x40,0x00],
      50:[0x42,0x61,0x51,0x49,0x46],51:[0x21,0x41,0x45,0x4B,0x31],
      52:[0x18,0x14,0x12,0x7F,0x10],53:[0x27,0x45,0x45,0x45,0x39],
      54:[0x3C,0x4A,0x49,0x49,0x30],55:[0x01,0x71,0x09,0x05,0x03],
      56:[0x36,0x49,0x49,0x49,0x36],57:[0x06,0x49,0x49,0x29,0x1E],
      58:[0x00,0x36,0x36,0x00,0x00],
      60:[0x08,0x14,0x22,0x41,0x00],62:[0x00,0x41,0x22,0x14,0x08],
      65:[0x7E,0x11,0x11,0x11,0x7E],66:[0x7F,0x49,0x49,0x49,0x36],
      67:[0x3E,0x41,0x41,0x41,0x22],68:[0x7F,0x41,0x41,0x22,0x1C],
      69:[0x7F,0x49,0x49,0x49,0x41],70:[0x7F,0x09,0x09,0x09,0x01],
      71:[0x3E,0x41,0x49,0x49,0x7A],72:[0x7F,0x08,0x08,0x08,0x7F],
      73:[0x00,0x41,0x7F,0x41,0x00],74:[0x20,0x40,0x41,0x3F,0x01],
      75:[0x7F,0x08,0x14,0x22,0x41],76:[0x7F,0x40,0x40,0x40,0x40],
      77:[0x7F,0x02,0x0C,0x02,0x7F],78:[0x7F,0x04,0x08,0x10,0x7F],
      79:[0x3E,0x41,0x41,0x41,0x3E],80:[0x7F,0x09,0x09,0x09,0x06],
      82:[0x7F,0x09,0x19,0x29,0x46],83:[0x46,0x49,0x49,0x49,0x31],
      84:[0x01,0x01,0x7F,0x01,0x01],85:[0x3F,0x40,0x40,0x40,0x3F],
      86:[0x1F,0x20,0x40,0x20,0x1F],87:[0x3F,0x40,0x38,0x40,0x3F],
      88:[0x63,0x14,0x08,0x14,0x63],89:[0x07,0x08,0x70,0x08,0x07],
      90:[0x61,0x51,0x49,0x45,0x43],95:[0x80,0x80,0x80,0x80,0x80],
      97:[0x20,0x54,0x54,0x54,0x78],98:[0x7F,0x48,0x44,0x44,0x38],
      99:[0x38,0x44,0x44,0x44,0x20],100:[0x38,0x44,0x44,0x48,0x7F],
      101:[0x38,0x54,0x54,0x54,0x18],102:[0x08,0x7E,0x09,0x01,0x02],
      103:[0x0C,0x52,0x52,0x52,0x3E],104:[0x7F,0x08,0x04,0x04,0x78],
      105:[0x00,0x44,0x7D,0x40,0x00],106:[0x20,0x40,0x44,0x3D,0x00],
      107:[0x7F,0x10,0x28,0x44,0x00],108:[0x00,0x41,0x7F,0x40,0x00],
      109:[0x7C,0x04,0x18,0x04,0x78],110:[0x7C,0x08,0x04,0x04,0x78],
      111:[0x38,0x44,0x44,0x44,0x38],112:[0x7C,0x14,0x14,0x14,0x08],
      113:[0x08,0x14,0x14,0x18,0x7C],114:[0x7C,0x08,0x04,0x04,0x08],
      115:[0x48,0x54,0x54,0x54,0x20],116:[0x04,0x3F,0x44,0x40,0x20],
      117:[0x3C,0x40,0x40,0x20,0x7C],118:[0x1C,0x20,0x40,0x20,0x1C],
      119:[0x3C,0x40,0x30,0x40,0x3C],120:[0x44,0x28,0x10,0x28,0x44],
      121:[0x0C,0x50,0x50,0x50,0x3C],122:[0x44,0x64,0x54,0x4C,0x44],
      124:[0x00,0x00,0x7F,0x00,0x00],
    };

    const canvas = document.getElementById('tft-canvas');
    const ctx = canvas.getContext('2d');

    function drawChar(ch, x, y, color, size) {
      const glyph = FONT_5x7[ch.charCodeAt(0)];
      if (!glyph) return;
      ctx.fillStyle = color;
      for (let col = 0; col < 5; col++) {
        let line = glyph[col];
        for (let row = 0; row < 7; row++) {
          if (line & 0x01) ctx.fillRect(x + col * size, y + row * size, size, size);
          line >>= 1;
        }
      }
    }

    function drawText(str, x, y, color, size) {
      for (let i = 0; i < str.length; i++) drawChar(str[i], x + i * 6 * size, y, color, size);
    }

    // =============================================
    // =============================================
    // SPRITE CACHE
    // =============================================
    const spriteCache = new Map();
    function getSpriteImage(spriteArray, w, h) {
      if (spriteCache.has(spriteArray)) {
        return spriteCache.get(spriteArray);
      }
      const c = document.createElement('canvas');
      c.width = w; c.height = h;
      const tCtx = c.getContext('2d');
      const img = tCtx.createImageData(w, h);
      for (let i = 0; i < spriteArray.length; i++) {
        const v = spriteArray[i], idx = i * 4;
        if (v === 0x0000) { 
          img.data[idx]=0; img.data[idx+1]=0; img.data[idx+2]=0; img.data[idx+3]=0; 
        } else {
          img.data[idx]=((v>>11)&0x1F)<<3; img.data[idx+1]=((v>>5)&0x3F)<<2;
          img.data[idx+2]=(v&0x1F)<<3; img.data[idx+3]=255;
        }
      }
      tCtx.putImageData(img, 0, 0);
      spriteCache.set(spriteArray, c);
      return c;
    }
    
    function getPetSprite() {
      if (idade < 1) {
        return { img: getSpriteImage(ovo_sprite, ovo_width, ovo_height), w: ovo_width, h: ovo_height };
      } else {
        return { img: getSpriteImage(capivara_sprite, capivara_width, capivara_height), w: capivara_width, h: capivara_height };
      }
    }

    // =============================================
    // HELPERS DE DESENHO
    // =============================================
    function fillSprite(color) { ctx.fillStyle = color; ctx.fillRect(0,0,135,240); }
    function drawRect(x,y,w,h,color) { ctx.strokeStyle=color; ctx.lineWidth=1; ctx.strokeRect(x+.5,y+.5,w-1,h-1); }
    function fillRect(x,y,w,h,color) { ctx.fillStyle=color; ctx.fillRect(x,y,w,h); }
    function drawHLine(x,y,w,color) { ctx.fillStyle=color; ctx.fillRect(x,y,w,1); }
    function mapVal(x,a,b,c,d) { return Math.round((x-a)*(d-c)/(b-a)+c); }



    // =============================================
    // AUDIO API (Sons)
    // =============================================
    const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    function tocarSom(freq, dur) {
      if (audioCtx.state === 'suspended') audioCtx.resume();
      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();
      osc.type = 'square';
      osc.frequency.value = freq;
      osc.connect(gain);
      gain.connect(audioCtx.destination);
      osc.start();
      gain.gain.exponentialRampToValueAtTime(0.00001, audioCtx.currentTime + dur/1000);
      osc.stop(audioCtx.currentTime + dur/1000);
    }

    function tocarSomBeep() { tocarSom(1000, 50); }
    function tocarSomPulo() { tocarSom(600, 100); setTimeout(() => tocarSom(800, 100), 100); }
    function tocarSomSucesso() { tocarSom(1000, 100); setTimeout(() => tocarSom(1200, 150), 100); setTimeout(() => tocarSom(1500, 300), 250); }
    function tocarSomFalha() { tocarSom(300, 200); setTimeout(() => tocarSom(200, 300), 200); }

    // =============================================
    // ESTADO DO JOGO
    // =============================================
    let fome = 100, felicidade = 100, idade = 0, vivo = true;
    
    // Carregar progresso salvo
    const salvoFome = localStorage.getItem('pet_fome');
    if (salvoFome !== null) {
      fome = parseInt(salvoFome);
      felicidade = parseInt(localStorage.getItem('pet_felicidade'));
      idade = parseInt(localStorage.getItem('pet_idade'));
      vivo = localStorage.getItem('pet_vivo') === 'true';
    }

    function salvarProgresso() {
      localStorage.setItem('pet_fome', fome);
      localStorage.setItem('pet_felicidade', felicidade);
      localStorage.setItem('pet_idade', idade);
      localStorage.setItem('pet_vivo', vivo);
    }

    let speedMultiplier = 1;
    const TICK_INTERVAL = 5000;

    // Mǭquina de Estados
    const TELA_PET = 0, TELA_MENU = 1, TELA_STATUS = 2, TELA_GAMEOVER = 3, TELA_COMER = 4, TELA_MSG_SUCESSO = 5, TELA_CARINHO = 6;
    const TELA_SUBMENU_COMER = 7, TELA_SUBMENU_BRINCAR = 8, TELA_PULAR = 9;
    let estadoAtual = TELA_PET;
    
    let cliquesComida = 0;
    let cliquesCarinho = 0;
    let ultimoTremorComida = 0;
    let ultimoCliqueCarinho = 0;
    let tempoMensagemSucesso = 0;
    let msgSucessoTitulo = "";
    let msgSucessoSub = "";
    
    let menuCursor = 0;
    const MENU_TOTAL = 3;
    const menuOpcoes = ['Comer', 'Brincar', 'Voltar'];

    let subMenuCursor = 0;
    const SUBMENU_TOTAL = 2;
    const menuComida = ['Maca', 'Doce'];
    const menuBrincar = ['Carinho', 'Pular'];
    let comidaSelecionada = 0;

    // Minigame Pular
    let pularY = 0;
    let isPular = false;
    let tempoPulo = 0;
    let obsX = 27;
    let obsCount = 0;
    let hitObs = false;
    let ultimoMoveObs = 0;

    // =============================================
    // TELA PET (principal)
    // =============================================
    function desenharTelaPet(now) {
      const respY = Math.round(Math.sin(now / 800) * 1);
      const pet = getPetSprite();
      const x = Math.floor((27 - pet.w) / 2);
      const y = Math.floor((48 - pet.h) / 2) + respY;

      if (fome < 30 || felicidade < 30) {
        drawText('(o_O)', 4, 21 + respY, '#ff9f43', 1);
      } else {
        ctx.imageSmoothingEnabled = false;
        ctx.drawImage(pet.img, x*5, y*5, pet.w*5, pet.h*5);
      }
    }

    // =============================================
    // TELA MENU (tela cheia com opções)
    // =============================================
    function desenharMenu() {
      for (let i = 0; i < MENU_TOTAL; i++) {
        const y = 9 + i * 9;
        if (i === menuCursor) {
          fillRect(2, y - 1, 23, 6, '#ffffff');
          drawText(menuOpcoes[i] === 'Brincar' ? 'Jogar' : menuOpcoes[i], 4, y, '#000000', 1);
        } else {
          drawText(menuOpcoes[i] === 'Brincar' ? 'Jogar' : menuOpcoes[i], 4, y, '#ffffff', 1);
        }
      }
    }

    // =============================================
    // TELA STATUS
    // =============================================
    function desenharTelaStatus() {
      drawText('STAT', 5, 2, '#00ffff', 1);
      drawHLine(2, 6, 23, '#424242');

      const minutos = Math.floor((idade * 5) / 60);
      drawText('Id:' + minutos + 'm', 2, 9, '#ffffff', 1);

      drawText('Fome', 2, 16, '#ffffff', 1);
      const corFome = fome > 50 ? '#00ff00' : (fome > 25 ? '#ffff00' : '#ff0000');
      drawText(fome + '%', 17, 16, corFome, 1);
      fillRect(2, 20, 23, 3, '#ffffff');
      const fomeW = Math.round((fome/100)*21);
      if (fomeW > 0) fillRect(3, 21, fomeW, 1, corFome);

      drawText('Felz', 2, 27, '#ffffff', 1);
      const corFel = felicidade > 50 ? '#001fff' : (felicidade > 25 ? '#ffff00' : '#ff0000');
      drawText(felicidade + '%', 17, 27, corFel, 1);
      fillRect(2, 31, 23, 3, '#ffffff');
      const felW = Math.round((felicidade/100)*21);
      if (felW > 0) fillRect(3, 32, felW, 1, corFel);
    }

    // =============================================
    // DESENHO DE CORAÇÃO E TELA DE CARINHO
    // =============================================
    function desenharCoracao(cx, cy) {
      fillRect(cx-1, cy-1, 1, 1, '#ff007f');
      fillRect(cx+1, cy-1, 1, 1, '#ff007f');
      fillRect(cx-2, cy, 5, 1, '#ff007f');
      fillRect(cx-1, cy+1, 3, 1, '#ff007f');
      fillRect(cx, cy+2, 1, 1, '#ff007f');
    }

    function desenharTelaCarinho(now) {
      const respY = Math.round(Math.sin(now / 800) * 1);
      const pet = getPetSprite();
      const x = Math.floor((27 - pet.w) / 2);
      const y = Math.floor((48 - pet.h) / 2) + respY;
      let offsetX = 0; let offsetY = 0;

      if (now - ultimoCliqueCarinho < 150) {
        offsetX = Math.floor(Math.random() * 3) - 1;
        offsetY = Math.floor(Math.random() * 3) - 1;
      }
      ctx.imageSmoothingEnabled = false;
      ctx.drawImage(pet.img, (x + offsetX)*5, (y + offsetY)*5, pet.w*5, pet.h*5);

      if (now - ultimoCliqueCarinho < 450) {
        const progresso = (now - ultimoCliqueCarinho) / 450;
        const deltaY = Math.floor(progresso * 8);
        desenharCoracao(x - 3, y + 5 - deltaY);
        desenharCoracao(x + pet.w + 3, y + 2 - deltaY);
      }
    }

    // =============================================
    // TELA DO MINIJOGO DE COMER
    // =============================================
    function desenharTelaComer() {
      if (cliquesComida >= 0 && cliquesComida < 10) {
        let cx = 13; let cy = 24; const raio = 6;
        const agora = performance.now();
        if (agora - ultimoTremorComida < 150) {
          cx += Math.floor(Math.random() * 3) - 1; cy += Math.floor(Math.random() * 3) - 1;
        }

        if (comidaSelecionada === 0) {
          fillRect(cx, cy - raio - 2, 1, 3, '#5c4033'); // Cabinho
          fillRect(cx + 1, cy - raio - 1, 2, 1, '#00ff00'); // Folha
          fillRect(cx - 2, cy - 2, 5, 5, '#ff0000'); // Corpo
        } else {
          fillRect(cx - 1, cy - 1, 3, 3, '#ff00ff');
          drawRect(cx - 3, cy - 3, 7, 7, '#ffffff');
          fillRect(cx - 5, cy, 2, 1, '#ff00ff');
          fillRect(cx + 4, cy, 2, 1, '#ff00ff');
        }

        if (cliquesComida >= 3 && cliquesComida < 6) {
          fillRect(cx + 2, cy - raio - 2, 9, 2*raio+4, '#000000');
        } else if (cliquesComida >= 6 && cliquesComida < 10) {
          fillRect(cx - 3, cy - raio - 2, 14, 2*raio+4, '#000000');
        }
      }
    }

    // =============================================
    // TELA DOS SUBMENUS
    // =============================================
    function desenharSubMenu(titulo, opcoes, total) {
      drawText(titulo, 5, 2, '#00ffff', 1);
      drawHLine(2, 6, 23, '#424242');

      for (let i = 0; i < total; i++) {
        const y = 11 + i * 9;
        if (i === subMenuCursor) {
          fillRect(2, y - 1, 23, 6, '#ffffff');
          drawText(opcoes[i], 4, y, '#000000', 1);
        } else {
          drawText(opcoes[i], 4, y, '#ffffff', 1);
        }
      }
    }

    // =============================================
    // TELA PULAR
    // =============================================
    function desenharTelaPular(now) {
      drawHLine(0, 36, 27, '#ffffff');
      drawText('P:' + obsCount, 1, 1, '#ffffff', 1);

      const pet = getPetSprite();
      const x = 6; 
      const y = 36 - pet.h + pularY;
      
      ctx.imageSmoothingEnabled = false;
      ctx.drawImage(pet.img, x*5, y*5, pet.w*5, pet.h*5);
      fillRect(obsX, 32, 4, 4, '#ff0000');
    }

    // =============================================
    // TELA DE MENSAGEM DE SUCESSO TEMPORÁRIA
    // =============================================
    function desenharMensagemSucesso() {
      drawText(msgSucessoTitulo, 5, 16, '#00ff00', 1);
      drawText(msgSucessoSub, 3, 26, '#ffffff', 1);
    }

    // =============================================
    // TELA GAME OVER
    // =============================================
    function desenharGameOver() {
      const w = lapide_width, h = lapide_height;
      const x = Math.floor((27 - w) / 2);
      const y = 8;
      
      const lapideImg = getSpriteImage(lapide_sprite, w, h);
      ctx.imageSmoothingEnabled = false;
      ctx.drawImage(lapideImg, x*5, y*5, w*5, h*5);
      
      drawText('MORTO', 4, y + h + 2, '#ff0000', 1);
      const minutos = Math.floor((idade * 5) / 60);
      drawText(minutos + 'm', 4, y + h + 8, '#ffffff', 1);
    }

    // =============================================
    // RENDER LOOP
    // =============================================
    function render() {
      fillSprite('#000000');
      const now = performance.now();

      if (estadoAtual === TELA_MSG_SUCESSO && (now - tempoMensagemSucesso > 1500)) {
        estadoAtual = TELA_PET;
      }
      
      // L�gica do Minijogo de Pular
      if (estadoAtual === TELA_PULAR) {
        if (now - ultimoMoveObs > 50) {
          ultimoMoveObs = now;
          if (!hitObs) {
            obsX -= 2;
            if (isPular) {
              const tPulo = now - tempoPulo;
              if (tPulo < 250) {
                pularY = -5;
              } else if (tPulo < 500) {
                pularY = 0;
              } else {
                isPular = false;
              }
            }
            if (obsX > 8 && obsX < 15 && pularY === 0) {
              hitObs = true;
              tocarSomFalha();
              fome -= 10;
              felicidade -= 10;
              estadoAtual = TELA_MSG_SUCESSO;
              tempoMensagemSucesso = now;
              msgSucessoTitulo = "Ops!";
              msgSucessoSub = "Bateu :(";
              salvarProgresso();
            }
            if (obsX < -4 && !hitObs) {
              obsX = 27;
              obsCount++;
              if (obsCount >= 5) {
                felicidade = Math.min(100, felicidade + 20);
                estadoAtual = TELA_MSG_SUCESSO;
                tempoMensagemSucesso = now;
                msgSucessoTitulo = "Uhuu!";
                msgSucessoSub = "+ feliz";
                tocarSomSucesso();
                salvarProgresso();
              }
            }
          }
        }
      }

      switch (estadoAtual) {
        case TELA_PET:      desenharTelaPet(now); break;
        case TELA_MENU:     desenharMenu(); break;
        case TELA_STATUS:   desenharTelaStatus(); break;
        case TELA_GAMEOVER: desenharGameOver(); break;
        case TELA_COMER:    desenharTelaComer(); break;
        case TELA_MSG_SUCESSO: desenharMensagemSucesso(); break;
        case TELA_CARINHO:  desenharTelaCarinho(now); break;
        case TELA_SUBMENU_COMER: desenharSubMenu("COMIDA", menuComida, SUBMENU_TOTAL); break;
        case TELA_SUBMENU_BRINCAR: desenharSubMenu("BRINCAR", menuBrincar, SUBMENU_TOTAL); break;
        case TELA_PULAR:    desenharTelaPular(now); break;
      }

      // Atualiza painel lateral
      document.getElementById('live-fome').textContent = fome;
      document.getElementById('live-felicidade').textContent = felicidade;
      const minutos = Math.floor((idade * 5) / 60);
      document.getElementById('live-idade').textContent = minutos;

      document.getElementById('stat-fome-box').classList.toggle('low-warn', fome < 40 && vivo);
      document.getElementById('stat-felicidade-box').classList.toggle('low-warn', felicidade < 40 && vivo);

      // Badge de tela
      const badge = document.getElementById('screen-badge');
      const badgeMap = {
        [TELA_PET]:      { text: '🐾 Pet',       cls: 'screen-mode-badge pet' },
        [TELA_MENU]:     { text: '📋 Menu',      cls: 'screen-mode-badge status' },
        [TELA_STATUS]:   { text: '📊 Status',    cls: 'screen-mode-badge status' },
        [TELA_GAMEOVER]: { text: '💀 Game Over', cls: 'screen-mode-badge gameover' },
        [TELA_COMER]:    { text: '🍎 Comendo',   cls: 'screen-mode-badge status' },
        [TELA_MSG_SUCESSO]: { text: '🎉 Sucesso',  cls: 'screen-mode-badge pet' },
        [TELA_CARINHO]:  { text: '💖 Carinho',   cls: 'screen-mode-badge status' },
        [TELA_SUBMENU_COMER]: { text: '🍎 Comida', cls: 'screen-mode-badge status' },
        [TELA_SUBMENU_BRINCAR]: { text: '🎮 Brincar', cls: 'screen-mode-badge status' },
        [TELA_PULAR]:    { text: '🏃 Pular', cls: 'screen-mode-badge status' },
      };
      const b = badgeMap[estadoAtual] || { text: '❓ ' + estadoAtual, cls: 'screen-mode-badge status' };
      badge.textContent = b.text;
      badge.className = b.cls;

      requestAnimationFrame(render);
    }

    // =============================================
    // GAME TICK
    // =============================================
    function gameTick() {
      if (vivo) {
        fome -= 1;
        felicidade -= 2;
        idade++;
        if (fome <= 0 || felicidade <= 0) {
          fome = Math.max(0, fome);
          felicidade = Math.max(0, felicidade);
          vivo = false;
          estadoAtual = TELA_GAMEOVER;
          tocarSomFalha();
        }
        salvarProgresso();
      }
    }

    let gameIntervalId = null;
    function startTimer() {
      if (gameIntervalId) clearInterval(gameIntervalId);
      gameIntervalId = setInterval(gameTick, TICK_INTERVAL / speedMultiplier);
    }

    // =============================================
    // A�ES DO MENU
    // =============================================
    function executarMenuAcao() {
      tocarSomBeep();
      switch (menuCursor) {
        case 0:
          subMenuCursor = 0;
          estadoAtual = TELA_SUBMENU_COMER;
          break;
        case 1:
          subMenuCursor = 0;
          estadoAtual = TELA_SUBMENU_BRINCAR;
          break;
        case 2:
          estadoAtual = TELA_PET;
          break;
      }
    }
    
    function executarSubMenuComida() {
      tocarSomBeep();
      comidaSelecionada = subMenuCursor;
      cliquesComida = 0;
      estadoAtual = TELA_COMER;
    }
    
    function executarSubMenuBrincar() {
      tocarSomBeep();
      if (subMenuCursor === 0) {
        cliquesCarinho = 0;
        estadoAtual = TELA_CARINHO;
      } else {
        obsX = 27;
        obsCount = 0;
        isPular = false;
        hitObs = false;
        pularY = 0;
        estadoAtual = TELA_PULAR;
      }
    }

    function reiniciarJogo() {
      fome = 100; felicidade = 100; idade = 0; vivo = true;
      estadoAtual = TELA_PET;
    }

    // =============================================
    // M�?QUINA DE ESTADOS - INPUT
    // =============================================
    function pressBtn(id) {
      const b = document.getElementById(id);
      b.classList.add('pressed');
      setTimeout(() => b.classList.remove('pressed'), 100);
    }

    let holdTimeout = null;
    let segurandoStatus = false;
    let botaoEsqPressionado = false;

    function iniciarHoldEsquerdo() {
      if (!vivo) {
        botaoEsqPressionado = true;
        return;
      }
      botaoEsqPressionado = true;
      segurandoStatus = false;

      if (estadoAtual === TELA_STATUS) return;

      holdTimeout = setTimeout(() => {
        if (botaoEsqPressionado && vivo && (estadoAtual === TELA_PET || estadoAtual === TELA_MENU)) {
          segurandoStatus = true;
          estadoAtual = TELA_STATUS;
        }
      }, 400);
    }

    function liberarEsquerdo() {
      if (!botaoEsqPressionado) return;
      botaoEsqPressionado = false;

      if (holdTimeout) {
        clearTimeout(holdTimeout);
        holdTimeout = null;
      }

      if (segurandoStatus) {
        segurandoStatus = false;
        if (estadoAtual === TELA_STATUS) {
          estadoAtual = TELA_PET;
        }
      } else {
        executarCliqueEsquerdo();
      }
    }

    function executarCliqueEsquerdo() {
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
          ultimoTremorComida = performance.now();
          cliquesComida++;
          if (cliquesComida >= 10) {
            if (comidaSelecionada === 0) {
              fome += 15;
              msgSucessoTitulo = "Nham!";
              msgSucessoSub = "+ fome";
            } else {
              fome += 5;
              felicidade += 10;
              msgSucessoTitulo = "Doce!";
              msgSucessoSub = "+ feliz";
            }
            fome = Math.min(100, fome);
            felicidade = Math.min(100, felicidade);
            estadoAtual = TELA_MSG_SUCESSO;
            tempoMensagemSucesso = performance.now();
            tocarSomSucesso();
            salvarProgresso();
          } else {
            tocarSomBeep();
          }
          break;
        case TELA_CARINHO:
          ultimoCliqueCarinho = performance.now();
          cliquesCarinho++;
          if (cliquesCarinho >= 10) {
            felicidade = Math.min(100, felicidade + 15);
            estadoAtual = TELA_MSG_SUCESSO;
            tempoMensagemSucesso = performance.now();
            msgSucessoTitulo = "Eba!";
            msgSucessoSub = "+ carinho";
            tocarSomSucesso();
            salvarProgresso();
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
          estadoAtual = TELA_PET;
          tocarSomBeep();
          break;
      }
    }

    // =============================================
    // EVENT LISTENERS
    // =============================================
    const btnEsq = document.getElementById('btn-esq');
    const btnDir = document.getElementById('btn-dir');

    function onPressEsqStart(e) {
      if (e) e.preventDefault();
      btnEsq.classList.add('pressed');
      iniciarHoldEsquerdo();
    }

    function onPressEsqEnd(e) {
      if (e) e.preventDefault();
      btnEsq.classList.remove('pressed');
      liberarEsquerdo();
    }

    btnEsq.addEventListener('mousedown', onPressEsqStart);
    btnEsq.addEventListener('touchstart', onPressEsqStart, { passive: false });
    
    window.addEventListener('mouseup', onPressEsqEnd);
    window.addEventListener('touchend', onPressEsqEnd, { passive: false });
    window.addEventListener('touchcancel', onPressEsqEnd, { passive: false });

    function onBtnDireito() {
      pressBtn('btn-dir');
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
          ultimoTremorComida = performance.now();
          cliquesComida++;
          if (cliquesComida >= 10) {
            if (comidaSelecionada === 0) {
              fome += 15;
              msgSucessoTitulo = "Nham!";
              msgSucessoSub = "+ fome";
            } else {
              fome += 5;
              felicidade += 10;
              msgSucessoTitulo = "Doce!";
              msgSucessoSub = "+ feliz";
            }
            fome = Math.min(100, fome);
            felicidade = Math.min(100, felicidade);
            estadoAtual = TELA_MSG_SUCESSO;
            tempoMensagemSucesso = performance.now();
            tocarSomSucesso();
            salvarProgresso();
          } else {
            tocarSomBeep();
          }
          break;
        case TELA_CARINHO:
          ultimoCliqueCarinho = performance.now();
          cliquesCarinho++;
          if (cliquesCarinho >= 10) {
            felicidade = Math.min(100, felicidade + 15);
            estadoAtual = TELA_MSG_SUCESSO;
            tempoMensagemSucesso = performance.now();
            msgSucessoTitulo = "Eba!";
            msgSucessoSub = "+ carinho";
            tocarSomSucesso();
            salvarProgresso();
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
            tempoPulo = performance.now();
            tocarSomPulo();
          }
          break;
        default:
          break;
      }
    }

    btnDir.addEventListener('click', onBtnDireito);

    window.addEventListener('keydown', (e) => {
      if (e.repeat) return;
      if (e.key === 'ArrowLeft' || e.key === 'a' || e.key === 'A') {
        btnEsq.classList.add('pressed');
        iniciarHoldEsquerdo();
      }
      if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {
        onBtnDireito();
      }
    });

    window.addEventListener('keyup', (e) => {
      if (e.key === 'ArrowLeft' || e.key === 'a' || e.key === 'A') {
        btnEsq.classList.remove('pressed');
        liberarEsquerdo();
      }
    });

    // Reset
    document.getElementById('btn-reset-virtual').addEventListener('click', () => {
      if (vivo) { fome = 0; felicidade = 0; vivo = false; estadoAtual = TELA_GAMEOVER; }
      else { reiniciarJogo(); }
    });

    // Speed slider
    const speedSlider = document.getElementById('speed-slider');
    const speedIndicator = document.getElementById('speed-indicator');
    speedSlider.addEventListener('input', (e) => {
      speedMultiplier = parseInt(e.target.value);
      let d = `${speedMultiplier}x`;
      if (speedMultiplier === 1) d += ' (Padrǜo)';
      else if (speedMultiplier >= 10) d += ' (Ultra)';
      else if (speedMultiplier >= 5) d += ' (Rǭpido)';
      speedIndicator.textContent = d;
      startTimer();
    });

    // =============================================
    // GERAR PINOS
    // =============================================
    const pinLabelsLeft  = ['G','3V','3V','36','37','38','39','32','33','25','26','27'];
    const pinLabelsRight = ['G','5V','5','17','22','21','19','18','5','15','2','4'];

    function buildPins(containerId, labels, side) {
      const container = document.getElementById(containerId);
      labels.forEach(lbl => {
        const hole = document.createElement('div');
        hole.className = 'pin-hole';
        const label = document.createElement('span');
        label.className = 'pin-label';
        label.textContent = lbl;
        label.style[side === 'left' ? 'left' : 'right'] = '14px';
        hole.appendChild(label);
        container.appendChild(hole);
      });
    }

    buildPins('pins-left', pinLabelsLeft, 'left');
    buildPins('pins-right', pinLabelsRight, 'right');

    // =============================================
    // INICIALIZA�ǟO
    // =============================================
    requestAnimationFrame(render);
    startTimer();
  