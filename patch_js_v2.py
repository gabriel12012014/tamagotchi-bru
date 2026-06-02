import re

with open('index.html', 'r', encoding='utf-8') as f:
    html = f.read()

# Update basic draw functions to scale x,y,w,h by 5 (except text which just scales coordinates)
html = html.replace("function fillRect(x, y, w, h, c) { ctx.fillStyle = c; ctx.fillRect(x, y, w, h); }",
                    "function fillRect(x, y, w, h, c) { ctx.fillStyle = c; ctx.fillRect(x*5, y*5, w*5, h*5); }")
html = html.replace("function drawRect(x, y, w, h, c) { ctx.strokeStyle = c; ctx.lineWidth = 1; ctx.strokeRect(x+0.5, y+0.5, w-1, h-1); }",
                    "function drawRect(x, y, w, h, c) { ctx.strokeStyle = c; ctx.lineWidth = 1; ctx.strokeRect(x*5, y*5, w*5, h*5); }")

html = html.replace("""    function drawText(txt, x, y, c, size) {
      ctx.fillStyle = c;
      const s = size === 1 ? '10px' : size === 2 ? '16px' : '20px';
      ctx.font = s + ' "Press Start 2P", monospace';
      ctx.fillText(txt, x, y + (size===1?10:size===2?16:20));
    }""", 
"""    function drawText(txt, x, y, c, size) {
      ctx.fillStyle = c;
      const s = size === 1 ? '10px' : size === 2 ? '16px' : '20px';
      ctx.font = s + ' "Press Start 2P", monospace';
      ctx.fillText(txt, x*5, y*5 + (size===1?10:size===2?16:20));
    }""")

new_render_functions = """    // =============================================
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
"""

start_str = "    // =============================================\n    // TELA PET (principal)"
end_str = "    // =============================================\n    // RENDER LOOP"

start_idx = html.find(start_str)
end_idx = html.find(end_str)

if start_idx != -1 and end_idx != -1:
    html = html[:start_idx] + new_render_functions + "\n" + html[end_idx:]

# Also adjust Pular physical physics to logical physics in the render loop!
html = html.replace("obsX -= 8;", "obsX -= 2;") # Logical speed
html = html.replace("pularY = -25;", "pularY = -5;") # Logical jump height
html = html.replace("if (obsX > 40 && obsX < 75 && pularY === 0) {", "if (obsX > 8 && obsX < 15 && pularY === 0) {")
html = html.replace("if (obsX < -20 && !hitObs) {", "if (obsX < -4 && !hitObs) {")
html = html.replace("obsX = 135;", "obsX = 27;") # Replace init values
html = html.replace("obsX = 135;", "obsX = 27;")

# Re-save
with open('index.html', 'w', encoding='utf-8') as f:
    f.write(html)
