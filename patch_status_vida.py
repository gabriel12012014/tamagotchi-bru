def patch_html():
    with open('simulador_arduino.html', 'r', encoding='utf-8') as f:
        content = f.read()

    old_func = """    function desenharTelaStatusVida() {
      fillRect(0, 0, 27, 48, '#000000');
      const dias = Math.floor(idade / 1440);
      
      const labelDias = "Dias";
      const valDias = String(dias);
      const labelMin = "Minutos";
      const valMin = String(idade);

      const xDiasL = (135 - labelDias.length * 12) / 2 / 5;
      drawText(labelDias, xDiasL, 2, '#ffffff', 2);
      const xDiasV = (135 - valDias.length * 12) / 2 / 5;
      drawText(valDias, xDiasV, 11, '#ffffff', 2);

      const xMinL = (135 - labelMin.length * 12) / 2 / 5;
      drawText(labelMin, xMinL, 25, '#ffffff', 2);
      const xMinV = (135 - valMin.length * 12) / 2 / 5;
      drawText(valMin, xMinV, 34, '#ffffff', 2);
    }"""

    new_func = """    function desenharTelaStatusVida() {
      fillRect(0, 0, 27, 48, '#000000');
      const dias = Math.floor(idade / 1440);
      
      const labelDias = "Dias";
      const valDias = String(dias);
      const labelMin = "Minutos";
      const valMin = String(idade);

      const xDiasL = (135 - labelDias.length * 12) / 2 / 5;
      drawText(labelDias, xDiasL, 8, '#ffffff', 2);
      const xDiasV = (135 - valDias.length * 12) / 2 / 5;
      drawText(valDias, xDiasV, 13, '#ffffff', 2);

      const xMinL = (135 - labelMin.length * 12) / 2 / 5;
      drawText(labelMin, xMinL, 26, '#ffffff', 2);
      const xMinV = (135 - valMin.length * 12) / 2 / 5;
      drawText(valMin, xMinV, 31, '#ffffff', 2);
    }"""

    if old_func in content:
        content = content.replace(old_func, new_func)
        with open('simulador_arduino.html', 'w', encoding='utf-8') as f:
            f.write(content)
        print("HTML patched successfully")
    else:
        print("HTML old function not found")


def patch_arduino():
    with open('arduino/arduino.ino', 'r', encoding='utf-8') as f:
        content = f.read()

    old_func = """void desenharTelaStatusVida() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  int dias = idade / 1440;
  String lblDias = "Dias";
  String valDias = String(dias);
  String lblMin = "Minutos";
  String valMin = String(idade);
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  
  telaVirtual.setCursor((135 - lblDias.length() * 12) / 2, 2 * 5);
  telaVirtual.print(lblDias);
  
  telaVirtual.setCursor((135 - valDias.length() * 12) / 2, 11 * 5);
  telaVirtual.print(valDias);
  
  telaVirtual.setCursor((135 - lblMin.length() * 12) / 2, 25 * 5);
  telaVirtual.print(lblMin);
  
  telaVirtual.setCursor((135 - valMin.length() * 12) / 2, 34 * 5);
  telaVirtual.print(valMin);
}"""

    new_func = """void desenharTelaStatusVida() {
  telaVirtual.fillScreen(TFT_BLACK);
  
  int dias = idade / 1440;
  String lblDias = "Dias";
  String valDias = String(dias);
  String lblMin = "Minutos";
  String valMin = String(idade);
  
  telaVirtual.setTextColor(TFT_WHITE);
  telaVirtual.setTextSize(2);
  
  telaVirtual.setCursor((135 - lblDias.length() * 12) / 2, 8 * 5);
  telaVirtual.print(lblDias);
  
  telaVirtual.setCursor((135 - valDias.length() * 12) / 2, 13 * 5);
  telaVirtual.print(valDias);
  
  telaVirtual.setCursor((135 - lblMin.length() * 12) / 2, 26 * 5);
  telaVirtual.print(lblMin);
  
  telaVirtual.setCursor((135 - valMin.length() * 12) / 2, 31 * 5);
  telaVirtual.print(valMin);
}"""

    if old_func in content:
        content = content.replace(old_func, new_func)
        with open('arduino/arduino.ino', 'w', encoding='utf-8') as f:
            f.write(content)
        print("Arduino patched successfully")
    else:
        print("Arduino old function not found")

patch_html()
patch_arduino()
