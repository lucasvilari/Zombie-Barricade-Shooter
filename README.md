# 🧟 Zombie Barricade Shooter

Um **First-Person Shooter estacionário** feito em **C++** com **OpenGL** e **FreeGLUT**. Você está preso atrás de uma barricada, cercado pela névoa de uma floresta sombria, e precisa segurar a horda de mortos-vivos com munição limitada e bons reflexos.

> Trabalho final da disciplina de **Computação Gráfica** — Departamento de Ciência da Computação, Universidade Federal do Piauí (UFPI), Teresina, PI.

---

## 📸 Screenshot

<img width="1181" height="607" alt="print" src="https://github.com/user-attachments/assets/5e0e83b7-8b60-4981-9bb7-02d84cc98ab9" />

---

## 🎮 Sobre o jogo

O jogador permanece fixo atrás de uma barricada resistente, mas não invencível, que protege sua base de sobrevivência. Da névoa da floresta surgem zumbis que avançam continuamente para destruí-la.

**A mecânica central:**

- Cada zumbi só morre com um tiro certeiro na cabeça (headshot).
- O pente da arma é limitado. Quando acaba, é preciso virar as costas para a barricada e recarregar junto às caixas de munição, perdendo os inimigos de vista enquanto eles continuam avançando.
- A recarga é gradual e passiva, acontecendo enquanto o jogador olha para o acampamento/caixas de munição.
- Se a "vida" da barricada chegar a zero, é *Game Over*: o tempo para, os controles são desativados e um grito (`scream.mp3`) é tocado. Qualquer tecla reinicia a partida.

**Objetivo:** sobreviver pelo maior tempo possível, administrando munição e tempo.

### 🧟 Tipos de zumbi

O tipo é sorteado aleatoriamente a cada novo zumbi gerado:

| Tipo | Comportamento |
|---|---|
| **Normal** | Caminha lentamente em linha reta até a barricada |
| **Runner** | Avança em alta velocidade rumo à barricada |
| **Strafer** | Oscila lateralmente em movimento harmônico enquanto avança |

### 🖥️ HUD

- **Barricada:** integridade atual da barricada
- **Balas:** munição disponível no pente

---

## ✨ Recursos técnicos

### Modelagem e texturização
- **Modelos externos (`.obj` + `.mtl`)**: arma, acampamento e caixas de munição, carregados com texturas e normal maps.
- **Modelos procedurais**: árvores, zumbis e barricada são construídos em código com primitivas (cubóides, cones e cilindros) e coloridos via código.

### Raycasting e colisão
- Ao atirar, um raio é lançado da câmera na direção do mouse, usando as funções de projeção do OpenGL.
- Cada zumbi possui uma AABB (Axis-Aligned Bounding Box) envolvendo a cabeça, testada com o algoritmo de interseção Ray vs AABB (método "Slab").
- Um acerto remove o zumbi e dispara um efeito de explosão de sangue.
- A hitbox também é checada contra a colisão da barricada para definir se o zumbi está em modo de ataque.

### Iluminação e atmosfera
- Iluminação global azul escura para simular a noite.
- Spotlight acima do jogador para melhorar a visibilidade.
- Fog intenso a partir de certa distância, fazendo os zumbis surgirem gradualmente.
- Muzzle flash ao disparar, aumentando o brilho na região do cano da arma.

### Movimentação e animações
- Zumbis avançam incrementando a coordenada Z, levando consigo o modelo e a hitbox.
- A virada de câmera entre barricada e acampamento usa interpolação e rotação em torno de um eixo.
- Animação de coice da arma ao disparar e de aproximação das caixas de munição ao recarregar.
- Braços e pernas dos zumbis são controlados individualmente (funções de tempo e ângulos) para as animações de caminhada e ataque.
- Sistema de partículas vermelhas para a explosão do crânio.

### Áudio
- Sons ambiente, disparos e efeitos via MiniAudio, com reprodução simultânea de vários arquivos.

---

## 🧰 Tecnologias e bibliotecas

| Biblioteca | Uso |
|---|---|
| [OpenGL](https://www.opengl.org/) + [FreeGLUT](http://www.opengl.org/resources/libraries/glut/) | Renderização, janela e entrada |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) | Carregamento de modelos `.obj`/`.mtl` |
| [stb_image](https://github.com/nothings/stb) | Carregamento de texturas `.png`/`.jpg` |
| [miniaudio](https://miniaud.io/) | Reprodução de áudio |

Bibliotecas padrão do C++ utilizadas: `<iostream>`, `<cmath>`, `<cstdlib>`, `<ctime>`, `<string>`, `<vector>` e `<map>`.

As bibliotecas tinyobjloader, stb_image e miniaudio são *header-only*: basta incluir os arquivos de cabeçalho na raiz do projeto.

---

## 🚀 Como compilar e executar

### Pré-requisitos
- Compilador C++ (g++/clang++/MSVC)
- OpenGL e FreeGLUT instalados

### Linux (exemplo)

```bash
sudo apt install build-essential freeglut3-dev
g++ main.cpp -o zombie-barricade-shooter -lGL -lGLU -lglut
./zombie-barricade-shooter
```

> ⚠️ Ajuste o nome do(s) arquivo(s)-fonte e as flags de link conforme a estrutura real do seu projeto (o miniaudio pode exigir `-lpthread -ldl -lm` no Linux). Execute o binário a partir da pasta que contém os assets (modelos, texturas e sons).

---

## 🕹️ Controles

| Ação | Controle |
|---|---|
| Mirar | Mouse |
| Atirar | Clique do mouse |
| Virar para o acampamento / recarregar | _(preencher com a tecla)_ |
| Reiniciar após Game Over | Qualquer tecla |

---

## 📁 Estrutura sugerida

```
.
├── main.cpp
├── tiny_obj_loader.h
├── stb_image.h
├── miniaudio.h
├── models/        # .obj e .mtl (arma, acampamento, caixas de munição)
├── textures/      # texturas e normal maps
├── sounds/        # sons ambiente, disparos, scream.mp3
├── docs/img/      # screenshots usadas neste README
└── README.md
```

---

## 🧠 Desafios e aprendizados

O maior desafio foi gerenciar recursos multimídia (áudio e modelos 3D) em um ambiente C++ nativo, o que foi resolvido integrando bibliotecas *header-only* específicas. A mecânica de tiro exigiu precisão matemática, resolvida com Raycasting e o algoritmo Ray vs AABB, garantindo a detecção correta de headshots.

O projeto permitiu praticar modelagem, texturização, iluminação, animação e lógica de jogabilidade com OpenGL.

---

## 👥 Autores

- Lucas Vilarinho C. M. Camarço
- João Pedro Saleh de Sousa

Universidade Federal do Piauí — Departamento de Ciência da Computação

---

## 📚 Referências

- FOSNER, M. *OpenGL Utility Toolkit (GLUT) 3.7*. http://www.opengl.org/resources/libraries/glut/
- YOKOGAWA, S. *tinyobjloader 2.0*. https://github.com/tinyobjloader/tinyobjloader
- GARRETT, N. *stb_image 2.27*. https://github.com/nothings/stb
- DUMONT, D. *miniaudio*. https://miniaud.io/
