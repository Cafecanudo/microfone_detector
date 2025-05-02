# 🎙️ Microphone Detector

Durante reuniões online (como no Microsoft Teams), é comum esquecermos o microfone aberto e só percebermos quando conversas privadas já foram ouvidas.  
O **Microphone Detector** ajuda a evitar esse problema: ele exibe bordas verdes nos cantos da tela sempre que o microfone está ativo.

> ⚠️ **Nota:** No momento, esta aplicação é compatível apenas com sistemas Linux (testado no Linux Mint e Ubuntu).

---

## ✅ Requisitos

Antes de compilar ou executar o programa, instale as seguintes bibliotecas:

```bash
sudo apt-get install libx11-dev libxfixes-dev libxext-dev
sudo apt-get install libasound-dev portaudio19-dev
```

## 🛠️ Como Compilar
Execute o comando abaixo no terminal:
```
gcc -o mic-detector mic-detector.c -lX11 -lXfixes -lXext -lasound -lportaudio
```

## ▶️ Como Executar
Após a compilação, inicie o programa com:
```
sudo chmod +x mic-detector
./mic-detector
```

## 🚀 Funcionalidades
- ✅ Detecta automaticamente quando o microfone está ativo.
- ✅ Exibe um indicador visual (bordas verdes) para alertar o usuário.
- ✅ Leve e roda em segundo plano sem consumir muitos recursos.

## 💻 Compatibilidade
- ✅ Linux Mint
- ✅ Ubuntu
- ❌ Outros sistemas operacionais ainda não suportados.

## 🚀 Demo
![](demo.gif)