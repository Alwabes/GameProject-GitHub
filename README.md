# NINJA DEFENDERS*
// *nome suscetível à alterações.

#**ADAPTAÇÕES** <br/>
Criar um jogo similar ao Space Invaders do Raylib. Porém, agora, inimigos vem de todos os lados, com o tempo de jogo e com progredir das fases, ficam mais fortes. O personagem principal continua com a função de atirar e a contagem dos pontos depende do número de inimigos eliminados. 
O ambiente agora será representado por um background e música temática.

- _personagem principal_ – ninja [nome indefinido] 
- _inimigos_ – indefinido
- _ambiente_ – indefinido

.<br/>
.<br/>
.<br/>

#**NARRATIVA** <br/>
"Este planeta tão vasto e cheio de segredos, como foi acabar assim?". É um situação em que nenhuma pessoa tem o que chamar de lar, estamos no reino dos monstros.
Você é a última esperança do seu vilarejo. Um ninja capaz de salvar seu povo e lutar contra os inimigos que tentar destruir seu lar. Muitos guerreiros já estão caídos, não resta muito tempo... 
Agora é você contra o mundo! Proteja sua casa e derrote todas as hordas que estão por vir. Não há mais esperanças para a Terra, por hora, o que nos resta é lutar.

.<br/>
.<br/>
.<br/>

#**OBJETIVOS** <br/>
Sobreviver o máximo possível, sem limite de tempo, às hordas de inimigos que avançam em sua direção. O jogo é limitado apenas pelos níveis e marca a pontuação de acordo com o número de inimigos eliminados. Ao chegar na fase final, o jogador receberá a mensagem de missão concluída. A pontuação contabilizará no ranking caso esta esteja acima da última colocada.

.<br/>
.<br/>
.<br/>

#**REGRAS** <br/>
-	O jogador possui 3 vidas.
-	Em caso de morte, o jogo finalizará com a mensagem de derrota.
-	Há formas de recuperar vida durante o trajeto.
-	Para passar para o próximo estágio é necessário cumprir “missões”, não sendo obrigatório eliminar todos os inimigos.
[...]

.<br/>
.<br/>
.<br/>

#**CÁLCULO DE PONTUAÇÃO** <br/>
A pontuação é calculada pela eliminação dos inimigos e níveis alcançados.
Cada inimigo, dependendo da dificuldade, resulta em uma certa quantia de pontos. Cada nível alcançado, aumenta o multiplicador de pontos.
_Exemplo:_ 

Inimigo de nível 1: 1 ponto                     Nível 1: multiplicador (x1)<br/>
Inimigo de nível 2: 5 pontos                    Nível 2: multiplicador (x2)<br/>
Inimigo de nível 3: 10 pontos                   Nível 3: multiplicador (x3)<br/>
[...]                                           [...]<br/>
<br/>
**Cálculo da pontuação:** _pontuação das eliminações * multiplicador_
