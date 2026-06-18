def patch_html():
    with open('simulador_arduino.html', 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the block and replace using regex to be safe about the accented comments
    import re
    
    # Locate the start of the function and the line just before FOME
    match = re.search(r'function desenharTelaStatus\(\) \{.*?const fomeImg', content, re.DOTALL)
    if match:
        old_block = match.group(0)
        
        new_block = """function desenharTelaStatus() {
      const d = getSimulatedTime();
      const hStr = String(d.getHours()).padStart(2, '0');
      const mStr = String(d.getMinutes()).padStart(2, '0');
      const timeStr = `${hStr}:${mStr}`;
      const wTime = timeStr.length * 12; 
      const xTime = (135 - wTime) / 2 / 5; 
      drawText(timeStr, xTime, 4, '#ffffff', 2);

      // --- FOME: 4 ícones (label + 3 indicadores) ---
      // Ícone label (esquerda)
      const fomeImg"""
        
        # We replace some accents just in case regex match needs to be reconstructed, but regex handles it.
        # Actually it's easier to just replace old_block with new_block.
        
        # Re-build exactly to avoid losing the FOME comment
        new_block_full = """function desenharTelaStatus() {
      const d = getSimulatedTime();
      const hStr = String(d.getHours()).padStart(2, '0');
      const mStr = String(d.getMinutes()).padStart(2, '0');
      const timeStr = `${hStr}:${mStr}`;
      const wTime = timeStr.length * 12; 
      const xTime = (135 - wTime) / 2 / 5; 
      drawText(timeStr, xTime, 4, '#ffffff', 2);

      // --- FOME: 4 ícones (label + 3 indicadores) ---
      // Ícone label (esquerda)
      const fomeImg"""
      
        content = content.replace(old_block, new_block_full)
        with open('simulador_arduino.html', 'w', encoding='utf-8') as f:
            f.write(content)
        print("HTML patched successfully")
    else:
        print("HTML block not found")

def patch_arduino():
    with open('arduino/arduino.ino', 'r', encoding='utf-8') as f:
        content = f.read()

    import re
    match = re.search(r'void desenharTelaStatus\(\) \{.*?desenharSpriteEscalado\(icone_fome_sprite', content, re.DOTALL)
    if match:
        old_block = match.group(0)
        
        new_block = """void desenharTelaStatus() {
  time_t agora_t = time(NULL);
  struct tm *t_info = localtime(&agora_t);
  char clockBuf[10];
  sprintf(clockBuf, "%02d:%02d", t_info->tm_hour, t_info->tm_min);
  
  String timeStr = String(clockBuf);
  int wTime = timeStr.length() * 12;
  int xTime = (135 - wTime) / 2;

  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  telaVirtual.setCursor(xTime, 4 * 5);
  telaVirtual.print(clockBuf);

  // --- FOME: ícone label + 3 indicadores ---
  // Ícone label (esquerda)
  desenharSpriteEscalado(icone_fome_sprite"""
        
        content = content.replace(old_block, new_block)
        with open('arduino/arduino.ino', 'w', encoding='utf-8') as f:
            f.write(content)
        print("Arduino patched successfully")
    else:
        print("Arduino block not found")

patch_html()
patch_arduino()
