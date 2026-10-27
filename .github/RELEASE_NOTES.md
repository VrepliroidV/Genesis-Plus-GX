**Genesis Plus GX ULTIMATE 1.1.0**: emulador de Mega Drive / Genesis, Mega CD, Master System, Game Gear e SG-1000 para Wii, baseado no [Genesis Plus GX 1.7.5](https://github.com/ekeeke/Genesis-Plus-GX) (Eke-Eke). *English below.*

## Novidades da 1.1.0: música no menu

- **Pasta de músicas**: coloque arquivos `.ogg` em `genplusultimate/music`, no SD ou no pendrive de onde o emulador abre. Quantas músicas quiser, de qualquer tamanho e com qualquer nome. Elas são lidas direto do cartão enquanto tocam.
- **Options > Menu > Menu Music**: **OFF**, **RANDOM** (sorteia uma música da pasta cada vez que o emulador abre, o padrão) ou cada música da pasta pelo nome. Ao trocar, a música nova já começa a tocar.
- **Mini player** no alto da tela, à direita, acima da logo: voltar, play, pause, stop, avançar e o nome da música. Ele funciona com o ponteiro do Wiimote e só aparece quando a pasta tem músicas. Voltar e avançar passam pelas músicas da pasta em ordem.
- Ao voltar para o menu durante o jogo, a música continua de onde estava (antes ela recomeçava), a não ser que você tenha pausado ou parado. O nome fica cinza com a música pausada ou parada.
- O `genplusultimate/Bg_music.ogg` não é mais usado: coloque a música na pasta `music`.

## O que já vinha na 1.0.0

- **Molduras em PNG** (Options > Video > **Bezel**), para preencher o espaço vazio ao redor do jogo numa TV LCD:
  - Aparecem nos modos de imagem **INTERLACED** (480i) e **PROGRESSIVE** (480p). Em **ORIGINAL** (240p) a imagem fica como sempre, sem moldura.
  - O emulador acha sozinho a área transparente no meio do PNG e encaixa a tela do jogo nela. A moldura é desenhada por cima, então cantos arredondados ficam certinhos. O Auto Crop continua funcionando dentro da moldura.
  - Um PNG sem área transparente no meio vira fundo, com o jogo na frente no tamanho de sempre.
  - PNG de qualquer tamanho, por exemplo os de 1920x1080 feitos para o RetroArch. Com o Wii em 16:9, use molduras de 16:9.
  - Opções: **OFF**, **DEFAULT** (a moldura de TV que já vem no emulador, ligada por padrão), **AUTO** (`<nome do jogo>.png`, senão `default.png`, senão a moldura de TV) e cada PNG da pasta `genplusultimate/bezels`, no SD ou no pendrive. A escolha também entra nas configurações por jogo.
- **Overclock automático** (Options > System > **Main 68k Auto Overclock** e **Sub 68k Auto Overclock**), para a CPU principal e a do Sega CD: o overclock só entra quando o jogo fica lento e fica desligado nas transições. Com Show FPS ligado, aparece a velocidade em uso.
- **Configurações por jogo** (botão da engrenagem **Per-Game**), no arquivo `saves/<jogo>.cfg`.
- **Auto Crop** das bordas pretas (Video > Auto Crop: FIT ou STRETCH) e **Crop Scaling** (SMOOTH ou INTEGER).

**Controles USB do tipo Xbox 360 (XInput)**, com fio ou com receptor 2.4 GHz plugado direto no USB do Wii.
- Testado com o **Retroflag Classic Controller-M wireless** (estilo Mega Drive) e com um **controle sem fio genérico de SNES Classic Mini**. Até 4 controles; o primeiro receptor plugado é o jogador 1. Pode plugar e tirar com o emulador aberto.
- O emulador vê esses controles como **controle de GameCube**. Nas opções de controle, escolha GameCube para a porta do jogador. Todos os botões podem ser configurados.

Botões já configurados, pensados para o Retroflag de 6 botões:

| Retroflag | Mega Drive |
|---|---|
| A, B, C | A, B, C |
| X, Y, Z | X, Y, Z |
| L e R | C |
| Mode | Mode |
| Start | Start |
| **Mode + direcional para a direita** | abre o menu do emulador |

Em outros controles, os gatilhos e o botão de ombro esquerdo dão C, o ombro direito dá Z e Select dá Mode.

## Instalação

1. Baixe o `GenesisPlusGXUltimate-Wii.zip` e extraia na raiz do cartão SD ou do pendrive. Ele cria `apps/genplusultimate` (o emulador) e a pasta `genplusultimate` com `roms`, `bezels` e `music`. Quem já tem a 1.0.0 pode extrair por cima: as configurações e os saves continuam. As outras pastas (saves, snaps, cheats, bios) são criadas quando o emulador abre.
2. É um app separado: pode ficar instalado junto com o Genesis Plus GX, e não mexe na pasta `genplus` dele. Para usar seus saves e BIOS antigos, copie as pastas `saves`, `bios` e `lock-on` de `genplus` para `genplusultimate`. As ROMs podem ser copiadas para `genplusultimate/roms`, ou escolha outra pasta no navegador de arquivos.

## Observações

- Plugue o receptor **direto no USB do Wii**. Ligado num adaptador na entrada do Wiimote, ele não funciona e pode travar o Wii.
- Funciona junto com um HD externo plugado na outra porta USB.

## Créditos

- Genesis Plus GX ULTIMATE: VrepliroidV
- Genesis Plus GX: Eke-Eke (e Charles MacDonald, autor do Genesis Plus original); a lista completa está em View Credits, no menu de saída do emulador.

---

## English

**Genesis Plus GX ULTIMATE 1.1.0**: Mega Drive / Genesis, Mega CD, Master System, Game Gear and SG-1000 emulator for Wii, based on Genesis Plus GX 1.7.5 (Eke-Eke).

New in 1.1.0, menu music:
- OGG files in `genplusultimate/music` (on the SD card or USB drive the emulator starts from), any number and any size, read as they play.
- Options > Menu > Menu Music: OFF, RANDOM (one file of the folder at every start, the default) or one of the files; changing it plays the music at once.
- A mini player at the top right of the menus, above the logo: previous, play, pause, stop, next and the name of the music. It is used with the Wii remote pointer and is shown when the folder has music. Back in the menu during a game, the music goes on where it was.
- `genplusultimate/Bg_music.ogg` is not played any more: put the music in the music folder.

Already in 1.0.0:

- **PNG bezels** (Options > Video > Bezel), in the INTERLACED and PROGRESSIVE display modes (not in ORIGINAL 240p). The transparent window in the middle of the PNG is found by itself and the game fills it, with the bezel drawn over its edges; a PNG without a window is a background behind the game. Any PNG size works (1920x1080 RetroArch bezels included). Choices: OFF, DEFAULT (the TV frame built into the emulator, on by default), AUTO (`<game>.png`, else `default.png`, else the TV frame) and each PNG in `genplusultimate/bezels` on the SD card or USB drive. Kept per game too.
- Automatic 68k overclock (main and Sega CD), per-game settings, Auto Crop of black borders, and Xbox 360 (XInput) USB controllers and 2.4 GHz receivers read as GameCube controllers (up to 4, hot-plug). Default layout for the 6-button Retroflag: A, B, C, X, Y, Z, Mode and Start as labelled; L and R also give C. **Mode + D-pad right** opens the menu.

Install: extract `GenesisPlusGXUltimate-Wii.zip` to the root of the SD card or USB drive (over 1.0.0 too: settings and saves stay). It is a separate app (`apps/genplusultimate`) with its own data folder (`genplusultimate`), so it can sit next to Genesis Plus GX; copy `saves`, `bios` and `lock-on` from `genplus` if you want to keep them.

---

Genesis Plus GX is © Charles MacDonald and Eke-Eke. It may be used and shared for free, but not for commercial purposes; see LICENSE.txt, which is also in the .zip. The source code of this build is in the "Source code" files of this release.
