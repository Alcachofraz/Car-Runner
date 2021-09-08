# Car-Runner
    Pequeno jogo de corrida com obstáculos para sistema embebido (LPC1769, Cortex-M3).
 
![image](https://user-images.githubusercontent.com/75852333/132536528-1f0ae90c-d152-4a8d-8d86-309ac8412ee6.png)
 

## 1.	Introdução

No curso de Engenharia Eletrónica e Telecomunicações e de Computadores do Instituto Superior de Engenharia de Lisboa, ao longo das cadeiras de Sistemas Embebidos 1 e Sistemas Embebidos 2, foi desenvolvida uma API documentada de acesso a alguns periféricos. Esta API foi projetada em C, para o microcontrolador LPC1769 da NXP, que utiliza uma arquitetura Cortex-M3.

A biblioteca desenvolvida tem como objetivo final a projeção de um pequeno videojogo, que utilizará alguns periféricos. Este relatório não vai abordar tal biblioteca em grande detalhe, sendo o foco principal os procedimentos e métodos utilizados na concepção das várias funcionalidades do videojogo. No entanto, a biblioteca será sucintamente descrita, e a sua documentação será enviada em anexo.
	

## 2.	Descrição do Projeto

O projeto final da cadeira de Sistemas Embebidos 2 consistiu em tirar partido da biblioteca de interface a diferentes periféricos, para criar um mini-jogo de obstáculos. 

O programa em si, é composto pelo designado jogo, um menu inicial muito simples, e também um menu com algumas opções configuráveis, como a data, hora, pontuações e nome do utilizador.

O jogo consiste em controlar um carro, visível num pequeno LCD, de forma a não chocar com obstáculos (distribuídos aleatoriamente pelo mapa) e a apanhar os galões de combustível (também distribuídos aleatoriamente) de modo a não ficar com o tanque vazio. Se uma pontuação for boa o suficiente (será discutido mais à frente) será guardada na memória local, e será também enviada para um servidor.

O projeto completo conta com alguns componentes. Passa-se à enumeração:

- NXP LPC1769 Microcontroller;
- Step-Up 3.3V para 5V;
- LCD 16x2 NMTC-S16205DRGHS;
- 3 Botões;
- Acelerómetro ADXL345;
- ESP-01 Wi-Fi Module;
- Uma resistência de 1.8kΩ;
- Uma resistência de 46kΩ;
- Fios de ligação;

Eis os blocos envolvidos neste sistema:
 
![image](https://user-images.githubusercontent.com/75852333/132536584-071b3575-19ef-49ea-84ab-aa80450b88a8.png)
 
 
## 3.	API de Interface aos Periféricos

A API conta com os seguintes módulos:

- adxl.h – Interface ao acelerómetro ADXL345 (utiliza o módulo spi.h). Apresenta um modo de funcionamento adequado ao FreeRTOS;
- button.h – Interface aos 3 botões utilizados no videojogo;
- flash.h – Interface à flash do microcontrolador (servirá para guardar algumas pontuações do videojogo, mesmo quando a energia for desligada);
- lcd.h – Interface ao LCD 16x2 (onde se desencadeará tanto o jogo, como os menus). Apresenta um modo de funcionamento adequado ao FreeRTOS;
- led.h – Interface ao LED interno do microcontrolador (não é utilizado no jogo, server apenas de teste à comunicação com o LPC1769);
- rtc.h – Interface ao relógio em tempo real presente no microcontrolador (utilizado para acertar o dia/hora e disponibilizar no LCD);
- spi.h – Interface a periféricos que utilizam o protocolo de comunicação SPI (Configura o periférico SPI presente no microcontrolador, e efetua transferências genéricas);
- wait.h – Interface ao periférico SYSTICK e aos Timers, ambos presentes no microcontrolador (tendo a componente SYSTICK sido desenvolvida pelo docente). A componente dos Timers apresenta um sistema de interrupções configurável pelo programador. Apresenta um modo de funcionamento adequado ao FreeRTOS;
- i2c.h – Interface a periféricos que utilizam o protocolo de comunicação I2C (Configura o periférico I2C1 ou I2C2 presente no microcontrolador, e efetua transferências genéricas). Funciona com base em rotinas de atendimento a interrupções;
- eeprom.h – Interface à EEPROM presente no microcontrolador, utilizando o protocolo I2C.
- uart.h – Interface a periféricos que utilizam o protocolo de comunicação UART (Configura o periférico UART2 presente no microcontrolador, e efetua transferências genéricas). Funciona com base em rotinas de atendimento a interrupções;
- esp.h – Interface ao módulo Wi-Fi ESP-01.
- network.h – Algumas funcionalidades relacionadas com a rede. Compatível apenas com ambiente FreeRTOS.


## 4.	Esquema elétrico

Em primeiro lugar, os botões. Estão a funcionar em active low, logo estão conectados ao GND e aos GPIOS do LPC. Os botões serão utilizados para interagir com o menu, visto que durante o jogo o utilizador apenas terá de ter em atenção a inclinação da placa, e o acelerómetro tratará do resto.

O step-up foi utilizado para transformar os 3.3V do microcontrolador em 5V, de forma a alimentar a drive do LCD. No entanto o LCD apresenta outra entrada que exige o fornecimento de uma tensão mais baixa para a lógica. É aí que entram as resistências, com o objetivo de realizarem um divisor de tensão, em função dos 5V. Ainda relativamente ao LCD, a entrada R/W foi ligada diretamente ao GND, sendo que o objetivo é efetuar transferências de leitura e nunca de escrita.
	
As 3 ligações SPI (MOSI, MISO e SCK) do ADXL345 foram ligadas a pinos específicos do LPC, que têm funções associadas para lidar com transferências desse tipo. O CS foi ligado a um GPIO normal. O acelerómetro foi alimentado com os 3.3V fornecidos pelo microcontrolador.

Por fim, o ESP-01 será alimentado com 3.3V, e a entrada CHPD será submetida ao nível lógico 1, por tratar-se de um enable active high. Por outro lado, a entrada RST é active low, pelo que será submetida, grande parte do tempo, ao nível lógico 0. De modo a controlar esta entrada (para que seja possível efetuar reset por hardware), será ligada ao GPIO do microcontrolador.

Na figura seguinte é possível observar o esquema que descreve todas estas ligações:
 
 ![image](https://user-images.githubusercontent.com/75852333/132536698-9d24a6f8-c8eb-4953-abcd-436244761177.png)


## 5.	Fluxo da Aplicação

Como já foi referido, o programa dividir-se-á, maioritariamente, em três partes. Um modo de jogo, um menu principal e um modo de configuração. Alguns aspetos do fluxo do programa:

- Assim que o utilizador iniciar o programa, ser-lhe-á solicitada a introdução de um username. Esse será o username que será associado às pontuações que ele obtiver.
- O menu principal apresenta, incialmente, a data e hora. No entanto, se o utilizador clicar no botão L, o menu principal deverá passar a mostrar as três melhores pontuações, que estarão armazenadas na flash do microcontrolador. Para voltar a mostrar a data e hora, o utilizador deverá clicar no botão R.
- Para entrar em modo de configuração, o utilizador deverá manter premido o botão R e L simultaneamente, durante um curto período de tempo, até aparecer o novo menu.
- No menu de configuração, haverá três opções. Uma opção para acertar a data e hora, uma para apagar o registo de pontuações, e outra para alterar o username. O utilizador poderá também voltar a manter premido o botão R e L para voltar ao menu principal.
- De volta ao menu principal, o utilizador pode também premir o botão S para iniciar um jogo. Terá de esperar um máximo de três segundos (de forma a não haver conflitos com os TIMERS), e aparecer-lhe-á uma tela a dizer para premir qualquer botão. O jogo não começa enquanto isso não for feito.
- Assim que o utilizador apertar qualquer botão, o jogo começa. Quando um obstáculo for atingido, ou o tanque ficar vazio, aparecerá uma nova tela, que indica a pontuação do utilizador. Premir qualquer botão para voltar ao menu principal.


## 6.	Máquina de Estados

Criou-se então uma máquina de estados com o seguinte comportamento:
 
 ![image](https://user-images.githubusercontent.com/75852333/132536789-1a6bb3dd-3d69-4a7a-9098-c3ff4f54da5e.png)


Passa-se à descrição individual dos estados representados na figura 3:

- Ask Username: É o estado inicial do programa. Sempre que o programa for iniciado, é pedido o nome do utilizador. Este estado recebe como input os 3 botões (L, R e S), que são utilizados para inserir caracteres. Quando o último caractere for inserido, o estado transita para Idle;
- Idle: Na verdade, representa dois estados que alternam entre si, com base no input dos botões R e L. Haverá duas formas de transitar para outro estado: Premir o botão S para iniciar um jogo (Game), ou manter premido R e L simultaneamente para entrar em modo de configuração (Configuration);
- Configuration: É o menu de configuração. É possível transitar de quatro formas diferentes: Selecionando um dos três tipos de configuração disponíveis, ou voltando ao menu principal (mantendo premido os botões R e L), i.e. ao estado Idle;
- Time Configuration: Configuração da data e hora. Vai selecionando os diferentes campos da data e hora, para o utilizador acertar. Quando se acertar o último campo (minutos), transita para Configuration;
- Score Configuration: Apagar o registo de pontuações. Antes ser apagar com sucesso, o utilizador tem de confirmar a sua intenção. Transita para Configuration;
- Name Configuration: Configuração do nome. Muito semelhante ao estado Ask Username, mas transita para Configuration.
- Pregame: Antes de o jogo principiar, o utilizador deverá clicar em qualquer botão. Transita para Game;
- Game: O jogo propriamente dito. Funciona com base no acelerómetro ADXL345. Transita para PostGame quando o utilizador perder;
- Postgame: Mostra ao utilizador a sua pontuação. Transita para Idle.


## 7.	Aplicação FreeRTOS

Como a aplicação apresenta vários blocos de código que podem ser efetuados concorrentemente, apresenta-se na figura seguinte uma solução utilizando os mecanismos de concorrência e sincronismo oferecidos pelo FreeRTOS.
 
 ![image](https://user-images.githubusercontent.com/75852333/132536877-23620b29-6d1e-41d2-9343-cb3142512538.png)

	
Os mecanismos semSTATE e queueACC (não indicados na figura anterior) são, respetivamente, um semáforo e uma fila. Ambos foram declarados no código, mas não são utilizados em qualquer circunstância. Logo, não estão incluídos no diagrama da aplicação.

As tasks estão identificadas a verde, excepto a taskSTATE_MACHINE por se tratar da task principal, responsável pelo fluxo da aplicação. 

As tasks rodeadas pela linha vermelha a tracejado, representam tasks cuja exclusiva função é atualizar um determinado recurso partilhado, que depois será acedido e, em alguns casos, alterado por outras tasks. Note-se que não é necessário nenhum mecanismo de acesso sincronizado aos recursos blinked e scoreCount, visto que taskSTATE_MACHINE não alterará o seu conteúdo, apenas o lerá. Os recursos fuel e points serão alterados também por taskUPDATE_GAME, pelo que é necessário um semáforo para cada um.

Além dos dois semáforos já referidos, é utilizado mais um para sincronizar o acesso ao recurso gameEnd, por parte das tasks taskSTATE_MACHINE e taskUPDATE_GAME.

A laranja, estão representadas as filas utilizadas para comunicar entre as várias tasks.

As tasks pintadas a carmesim representam tasks que têm um objetivo não periódico, e quando esse objetivo é concluído a task termina.


## 8.	Pontuações

Presente no projeto final, encontra-se um módulo chamado score.h/c. Este módulo responsabiliza-se pelo acesso tanto à flash do LPC1769, como à EEPROM. O armazenamento de pontuações e do respetivo utilizador, é tarefa deste módulo.

Apresenta algumas funções utilizadas pelo programa principal, dada a necessidade de guardar pontuações, de as apagar, de as obter ou até de as ordenar.

A gestão de pontuações consiste no seguinte:

- Só se guardam três pontuações de cada vez, no máximo (será designado o pódio);
- Quando alguma posição do pódio estiver vazia (por prévio apagamento), o LCD mostrará: “Not defined”;
- O pódio nunca poderá conter o mesmo utilizador em duas posições diferentes. Se um utilizador já tiver uma pontuação registada, e obtiver uma pontuação pior, nada acontece. Se obtiver uma melhor, é atualizado o pódio com essa pontuação.
- Se todas as posições do pódio já estiverem registadas, este só atualizará na presença de uma das seguintes situações:
	- Se um utilizador registado no pódio obtiver uma pontuação melhor;
	- Se um utilizador não registado obtiver uma pontuação melhor que o terceiro lugar;

Em suma, o programa comporta-se como se armazenasse todas as pontuações, de forma ordenada e sem repetições do utilizador, mas mantendo apenas o pódio.

O algoritmo programático de armazenamento e gestão de pontuações consiste num ciclo com dois objetivos:

- Percorrer o pódio à procura do utilizador corrente. Se encontrar:
	- Se a pontuação registada é menor, atualiza;
	- Se a pontuação registada é maior, ignora e retorna;
- Percorrer o pódio à procura de uma posição vazia. Se encontrar, atualiza;

Se o utilizador ainda não estiver registado, nem houver espaços vazios, verifica se a pontuação corrente é maior que a menor pontuação do pódio. Se for, substitui. No final, ordena as pontuações.

No caso de uma pontuação bater algum recorde, e entrar para o pódio (ou seja, ser guardada na flash/EEPROM), será também enviada para um servidor, utilizando o protocolo MQTT.


## 9.	Características do Videojogo

Algumas características acerca do videojogo:
- O carro controlado pelo utilizador ocupa duas posições do LCD (2 colunas e 1 linha), isto é, o carro é composto por 2 caracteres. Um caractere para a parte de trás e outro para a parte da frente. Se tanto a parte da frente como a parte de trás embater num obstáculo, o utilizador perde;
- Os obstáculos apresentam sempre um espaço de 2 colunas entre si, no mínimo, de forma a haver sempre possibilidade de não embater num obstáculo;
- Os galões de combustível aparecem duas vezes em cada 40 colunas. A ideia é o utilizador não deixar escapar nenhum, ou fica imediatamente descompensado e habilita-se  a perder muito mais rapidamente;
- O módulo car_runner.h apresenta vários aspetos configuráveis do jogo, como por exemplo:
	- GAME RATE – Intervalo de tempo entre cada frame do jogo;
	- FUEL RATE – Intervalo de tempo entre cada decremento ao tanque de combustível;
	- CAR_POSITION – Coluna onde o utilizador visualizará o carro, durante o jogo;
	- INITIAL_OBSTACLE_GAP – Espaço (em colunas) sem obstáculos no início, de forma a não perder de imediato.
	- INCLINATION_THRESHOLD – O quão será preciso inclinar a placa, de forma provocar uma alteração de faixa por parte do carro.
	
Existem algumas constantes que não devem ser alteradas. No caso, estará devidamente explicitado na documentação.


