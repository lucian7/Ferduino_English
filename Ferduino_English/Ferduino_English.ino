// Este programa é compatível com a IDE 1.0.2 desde que as bibliotecas "WConstants.h" e "wiring.h" sejam colocadas na pasta ...arduino-1.0.2\hardware\arduino\cores\arduino.
//Dúvidas, sugestões e elogios: fefegarcia_1@hotmail.com
//---------------------------------------------------------
//Ferduino controlador de aquários versão 2.4.0>> Última atualização:26/11/2012.
//Este programa foi desenvolvido com base na versão Stilo 2.1
//Disponível nesta página http://code.google.com/p/stilo/ 
//---------------------------------------------------------
//Algoritmo de controle de LED é baseado em Krusduino de Hugh Dangerfield
//http://Code.Google.com/p/dangerduino/
//Algoritmo da fase lunar foi escrito por Jamie Jardin vulgo "TheDOdblG", baseado no código de NightAtTheOpera's Lunar Phase 
//http://www.nano-reef.com/forums/index.php?showtopic=217305
//-----------------------------------------------------------
// Este programa é software livre; Você pode redistribuí-lo e/ou
// modificá-lo sob os termos da GNU General Public License
// como publicada pela Free Software Foundation versão 3
// da licença, ou (por sua opção) qualquer versão posterior.
// Este programa é distribuído na esperança que seja útil,
// Mas sem qualquer garantia; sem mesmo a garantia implícita de
// comercialização ou propósito particular. Consulte o
// GNU General Public License para mais detalhes.

//*************************************************************************************************
//*************** Biliotecas utilizadas ***********************************************************
//*************************************************************************************************
#include <ITDB02_Graph16.h> // Comente esta linha caso o seu LCD não seja o ITDB32WC
#include <ITDB02_Touch.h> // Comente esta linha caso o seu LCD não seja o ITDB32WC

//#include <UTFT.h>    // Descomente esta linha caso o seu LCD seja o ITDB32WD
//#include <UTouch.h> // Descomente esta linha caso o seu LCD seja o ITDB32WD

#include <Wire.h>
#include <EEPROM.h>
#include <writeAnything.h> // Funções de leitura e gravação da EEPROM.
#include <DS1307.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <EtherCard.h>

//****************************************************************************************************
//***************** Sensores de temperatura **********************************************************
//****************************************************************************************************
OneWire OneWireBus(47);                   //Sensor de temperatura da água e dissipador ligados ao pino 47.
DallasTemperature sensors(&OneWireBus);  // Pass our oneWire reference to Dallas Temperature. 
DeviceAddress sensor_agua= {
  0x28, 0x9C, 0xA9, 0xAA, 0x03, 0x00, 0x00, 0x44 }; // Atribui os endereços dos sensores de temperatura. Adicionar / Alterar os endereços conforme necessário.
DeviceAddress sensor_dissipador = {
  0x28, 0xE1, 0x96, 0xAA, 0x03, 0x00, 0x00, 0x7D }; // Atribui os endereços dos sensores de temperatura. Adicionar / Alterar os endereços conforme necessário.

//****************************************************************************************************
//****************** Variáveis de textos e fontes ****************************************************
//****************************************************************************************************
#define LARGE true
#define SMALL false
extern uint8_t SmallFont[];   // Declara que fontes vamos usar
extern uint8_t BigFont[];     // Declara que fontes vamos usar

//****************************************************************************************************
//****************** Define funções dos pinos digitais e analógicos **********************************
//****************************************************************************************************

// Pinos disponíveis [TX/RX(14,15,18,19) ANALÓGICOS(A6)]

const int ledPinBlue = 8;      // Pino que liga os leds azuis
const int ledPinWhite = 9;     // Pino que liga os leds brancos
const int ledPinMoon = 7;      // Pino que liga os leds da luz noturna
const int aquecedorPin = 42;   // Pino que liga o aquecedor
const int chillerPin = 43;     // Pino que liga o chiller
const int alarmPin = A13;      // Pino que aciona o alarme
const int fanPin = 13;         // Pino que controla a velocidade das ventoinhas do dissipador
const int sensor1 = A0;        //Pino analógico que verifica se há tensão na bóia da quarentena.
const int sensor2 = A1;        //Pino analógico que verifica se há tensão na bóia inferior do sump.
const int sensor3 = A2;        //Pino analógico que verifica se há tensão na bóia superior do sump.
const int sensor4 = A3;        //Pino analógico que verifica se há tensão na bóia inferior do reservatório.
const int sensor5 = A4;        //Pino analógico que verifica o nível do reef.
const int sensor6 = A5;        //Pino analógico que verifica o nível do fish only.
const int bomba1Pin = A9;      // Bomba que tira água da quarentena.
const int bomba2Pin = A10;     // Bomba que tira água do sump.
const int bomba3Pin = A11;     // Bomba que coloca água no sump.
const int solenoide1Pin = 48;  // Liga a reposicao de água doce.
const int reatorPin = 49;      // Pino que liga o CO2 do reator.
const int ozonizadorPin = A12; // pino que liga o ozonizador
const int multiplexadorS0Pin = A14; // Pino S0 de controle dos stamps
const int multiplexadorS1Pin = A15; // Pino S1 de controle dos stamps
const int dosadora1 = 10;     // Bomba dosadora 1
const int dosadora2 = 11;     // Bomba dosadora 2
const int dosadora3 = 12;     // Bomba dosadora 3
const int temporizador1 = 44;         //Pino que liga o timer 1.
const int temporizador2 = 45;         //Pino que liga o timer 2.
const int temporizador3 = 46;         //Pino que liga o timer 3.
const int temporizador4 = A7;         //Pino que liga o timer 4.
const int temporizador5 = A8;         //Pino que liga o timer 5.

//*******************************************************************************************************
//********************** Funções do RTC ********************************************************************
//*******************************************************************************************************
DS1307 rtc(20, 21); // Indica em quais pinos o RTC está conectado.
Time t_temp, t;
boolean ct=false;
boolean cd=false;

//*******************************************************************************************************
//********************** Variáveis das fuções do touch screen e tela inicial ****************************
//*******************************************************************************************************
ITDB02        myGLCD(38,39,40,41,1); // Comente esta linha caso o seu LCD não seja o ITDB32WC
ITDB02_Touch  myTouch(6,5,4,3,2); // Comente esta linha caso o seu LCD não seja o ITDB32WC

//UTFT        myGLCD(ITDB32WD, 38,39,40,41);   //Descomente esta linha caso o seu LCD seja o ITDB32WD
//UTouch      myTouch(6,5,4,3,2); // Descomente esta linha caso o seu LCD seja o ITDB32WD

int x, y;                  // Coordenadas do touch screen
long previousMillis = 0;
byte data[56];
String day; 
int whiteLed, blueLed;    // Valor anterior de PWM.
int dispScreen = 0;

// tela inicio =0, 
//tela de menu = 1, 
//tela do relogio = 2,
// tela de teste = 3,
//tela controle de temperatua = 4, 
//tela alterar valores dos led = 5
// tela para alterar potencia dos leds = 6,
// tela tpa = 7, 
// tela config. bombas dosadoras = 8,
// tela graficos = 9,
// tela wavemaker = 10, 
// tela grafico temperatura = 11,
// tela grafico orp = 12, 
// tela grafico ph do aquario = 13,
// tela configuracao parametros = 14, 
// tela configuracao de ph do reator de calcio = 15, 
// tela configuracao orp = 16, 
// tela grafico de ph do reator de calcio = 17, 
// tela configuracao de ph do aquario = 18, 
// tela grafico de densidade = 19,
// tela configurar densidade = 20, 
// tela esolha uma dosadora = 21, 
// tela dosadora 1 = 22, 
//tela dosadora 2 = 23, tela dosadora 3 = 24, tela dosadora 4 = 25, tela calibra dosadoras = 26

//*****************************************************************************************
//*********************** Variáveis do controle de temperatura da água ********************
//*****************************************************************************************
float tempC = 0;              // Temperatura da água
float setTempC = 0;          // Temperatura desejada
float offTempC = 0;          // Variacao permitida da temperatura
float alarmTempC = 0;        // Variacao para acionar o alarme de temperatura da água
boolean tempCflag = 0;         // Sinaliza que o aquecedor está ligado / desligado
boolean tempHflag = 0;         // Sinaliza que o chiller está ligado / desligado
boolean tempAflag = 0;         // Sinaliza que o alarme de temperatura está ativo
int contador_temp = 0;
float temperatura_agua_temp = 0; // Temperatura temporária
float temperatura_dissipador_temp = 0; // Temperatura temporária


//*****************************************************************************************
//************************ Variáveis do controle do PH do aquário *************************
//*****************************************************************************************
float PHA = 0;               // PH do aquário
float setPHA = 0;           // PH desejado do aquário
float offPHA = 0;           // Variaçãoo permitida do PH do aquário
float alarmPHA = 0;         // Variação para acionar o alarme de ph do aquário
boolean PHAAflag = 0;       // Sinaliza que o PH do aquário esta fora do especificado

//*****************************************************************************************
//************************ Variáveis de controle de densidade *****************************
//*****************************************************************************************
int DEN = 0;                 // Densidade
int setDEN = 0;             // Densidade desejada
int offDEN = 0;             // Variação permitida da densidade
int alarmDEN = 0;           // Variação para acionar o alarme de densidade
boolean DENAflag = 0;       // Sinaliza que a densidade esta fora do especificado

//*****************************************************************************************
//************************ Variáveis de controle do PH do reator de cálcio ****************
//*****************************************************************************************
float PHR =0;               // Valores PH reator
float setPHR = 0;           // PH do reator desejado
float offPHR = 0;           // Variacao permitida do PH do reator
float alarmPHR = 0;         // Variacao para acionar o alarme do PH do reator de calcio
boolean PHRflag = 0;        // Sinaliza que CO2 esta ligado / desligado
boolean PHRAflag = 0;       // Sinaliza que o PH do reator de cálcio esta fora do especificado

//*****************************************************************************************
//************************ Variáveis de controle da  ORP **********************************
//*****************************************************************************************
int ORP =0;                 // Valores ORP
float setORP = 0;           // ORP desejada
float offORP = 0;           // Variação permitida da ORP
float alarmORP = 0;         // Variacão para acionar o alarme da ORP
boolean ORPflag=0;          // Sinaliza que o ozonizador esta ligado / desligado
float ORPAflag = 0;        // Sinaliza que a ORP esta fora do especificado

//*****************************************************************************************
//************************ Variáveis de controle de velocidade dos coolers ****************
//*****************************************************************************************
const int HtempMin = 30;    // Declara a temperatura para iniciar o funcionamento das ventoinhas do dissipador 
const int HtempMax = 40;    // Declara que as ventoinhas devem estar em sua velocidade máxima quando o dissipador estiver com 40°c

//*****************************************************************************************
//************************ Variável de controle da temperatura do dissipador **************
//*****************************************************************************************
float tempH = 0;            // Temperatura do dissipador

//*****************************************************************************************
//************************ Variáveis temporárias de controle de temperatura da água *******
//*****************************************************************************************
float temp2beS;           
float temp2beO;
float temp2beA;

//*****************************************************************************************
//************************ Variáveis temporárias de controle do PH do reator de cálcio ****
//*****************************************************************************************
float PHR2beS;             
float PHR2beO;
float PHR2beA;

//*****************************************************************************************
//************************ Variáveis temporárias de controle do PH do aquário *************
//*****************************************************************************************
float PHA2beS;             
float PHA2beO;
float PHA2beA;

//*****************************************************************************************
//************************ Variáveis temporárias de controle da ORP ***********************
//*****************************************************************************************
float ORP2beS;               //orp temporaria
float ORP2beO;
float ORP2beA;

//*****************************************************************************************
//************************ Variáveis temporárias de controle da densidade *****************
//*****************************************************************************************
float DEN2beS;            
float DEN2beO;
float DEN2beA;

//*****************************************************************************************
//************************ Variáveis de controle da iluminação ****************************
//*****************************************************************************************
int LedChangTime = 0;             // Página de alteração do leds, tempo e valores.
boolean WorB;                     // Alnerna entre as tabela de potência dos leds. Brancos "true" ou azuis "false"
boolean MeanWell = true;  // Se estiver usando drivers cuja potência máxima seja obtida aplicando zero volt e a mínima seja 5 volts altere de "true" para "false".
boolean LEDtestTick = false;   // Acelerar o tempo durante o teste dos leds. 
int min_cnt ;
int bled_out ;
int wled_out ;
int moonled_out;
#define WHITE true
#define BLUE false               

//*****************************************************************************************
//************************ Variáveis da fase lunar ******************************************
//*****************************************************************************************
float LC = 29.53059;  // 1 ciclo lunar = 29.53059 dias.
String LP;
double AG;
int MaxI, tMaxI;  // Potência  máxima na Lua cheia.             
int MinI, tMinI;  // Potência  mínima na Lua nova.

//*****************************************************************************************
//************************ Variáveis da TPA automática ************************************
//*****************************************************************************************
int hora = 0;
int minuto = 0;
int duracaomaximatpa = 0;
int segunda = 0;
int terca = 0;
int quarta = 0;
int quinta = 0;
int sexta = 0;
int sabado = 0;
int domingo = 0;
int tpa = 0;                             // Controla os estágios da TPA automática
boolean tpa_em_andamento = false;           // Sinaliza TPA automática em andamento
boolean falha_tpa = false;           // Sinaliza falha durante a TPA automática
unsigned long tempo = 0;                 // Duração de cada estágio da TPA automática
unsigned long marcadoriniciotpa = 0;   // Evita que uma tpa inicie próximo do millis zerar
unsigned long shiftedmillis = 0;       // Evita que uma tpa inicie próximo do millis zerar

//*****************************************************************************************
//************************ Variáveis temporárias da TPA (Troca Parcial de Água) automática ************************************
//*****************************************************************************************
int temp2hora;
int temp2minuto;
int temp2duracaomaximatpa;
int temp2segunda;
int temp2terca;
int temp2quarta;
int temp2quinta;
int temp2sexta;
int temp2sabado;
int temp2domingo;

//****************************************************************************************
//*********************** Variáveis de controle das funções que utilizam o cartao SD *****
//****************************************************************************************
unsigned long logtempminutoantes = 0;  // Variável que controla o tempo para gravação dos parâmetros no cartão SD 
const int chipselect = 4;            // Para utilizar o Sd card do LCD altere para 53
Sd2Card card;
SdFile file;
SdFile root;
SdVolume volume;
char time1;
char time2;
char time3;
char time4;
char time5;

//*****************************************************************************************
//*********************** Variável do controle de níveis **********************************
//*****************************************************************************************
boolean nivel_status = 0;             // Sinaliza nivel baixo em um dos aquários

//*****************************************************************************************
//************************ Variável de controle da reposição de água doce *****************
//*****************************************************************************************
boolean ato = 0;                      // Sinaliza reposição ligada / desligada

//*****************************************************************************************
//************************ Variáveis de envio da informações ao cosm.com ******************
//*****************************************************************************************
long previous_Millis = 0; // Variável que controla o tempo para envio dos dados
int chiller_status;   
int aquecedor_status;
int reator_status;
int ozonio_status;
int reposicao_status;
int niveis_status;
int tpa_status;

//*****************************************************************************************
//************************* Funções do ethernet shield ************************************
//*****************************************************************************************

boolean Ethernet_Shield = true; // Altere para "false" caso não tenha um Ethernet Shield conectado ao Arduino.

#define FEED    "xxxxx"               // Número do projeto(cosm.com).
 #define APIKEY  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //Senha do projeto(cosm.com)

static byte mymac[] = {0x54, 0x55, 0x58, 0x10, 0x00, 0x26}; // Este mac deve ser único na rede.
static byte myip[] = {192, 168, 2, 105};
static byte gwip[] = {192, 168, 2, 1};
char website[] PROGMEM = "api.cosm.com";
byte Ethernet::buffer[500];
Stash stash;

//*****************************************************************************************
//************************** Variáveis de controle do multiplexador ***********************
//*****************************************************************************************
boolean Stamps = true; // Altere para "false" caso não tenha ao menos um dos circuitos de PH, ORP e EC da Atlas Scientific.
long millis_antes = 0;
int DENT; // Densidade temporária.
float PHT; // PH temporário.
int ORPT; // ORP temporária
char sensorstring[15];
byte holding;
short ph1=0; // Y0
short ph2=1; // Y1
short orp=2; // Y2
short ec=3; // Y3
int done = 0;

//*****************************************************************************************
//************************** Variáveis da solicitação de senha ****************************
//*****************************************************************************************
char stCurrent[7]="";
char limpar_senha [7] = "";
int stCurrentLen=0;
char senha [7] = {
  '1','2','3','4','5','6','\0'}; // Insira sua senha aqui. O caracter '\0' não deve ser alterado.

//*****************************************************************************************
//************************** Variáveis dosadoras ******************************************
//*****************************************************************************************
boolean dosadoras = true; //Altere para "false" caso não tenha as dosadoras.
char time6;
char time7;
char time8;
char time9;
char time10;
char time11;
int contador = 0;
int minuto01 = 0;
int minuto11 = 0;
int deslocamento_botao_x = 155;
int deslocamento_botao_y = 90;
long tempo_dosagem_1 = 0;
int pwm_dosadora_1 = 255;
int pwm_dosadora_2 = 255;
int pwm_dosadora_3 = 255;
float dose_dosadora_1_manual = 0.0;
float dose_dosadora_2_manual = 0.0;
float dose_dosadora_3_manual = 0.0;
boolean modo_manual = false;
boolean modo_personalizado = false;
boolean modo_automatico = false;
boolean modo_calibrar = false;
boolean dosadora_1_selecionada = false;
boolean dosadora_2_selecionada = false;
boolean dosadora_3_selecionada = false;
int ativar_desativar_1 = 0;
int ativar_desativar_2 = 0;
int ativar_desativar_3 = 0;
int modo_personalizado_on_1 = 0;
int modo_automatico_on_1 = 0;
int modo_personalizado_on_2 = 0;
int modo_automatico_on_2 = 0;
int modo_personalizado_on_3 = 0;
int modo_automatico_on_3 = 0;
float fator_calib_dosadora_1 = 0.0;
float fator_calib_dosadora_2 = 0.0;
float fator_calib_dosadora_3 = 0.0;
float dose_dosadora_1_personalizada = 0.0;
float dose_dosadora_2_personalizada = 0.0;
float dose_dosadora_3_personalizada = 0.0;
float dose_dosadora_1_automatica = 0.0;
float dose_dosadora_2_automatica = 0.0;
float dose_dosadora_3_automatica = 0.0;
int hora_inicial_dosagem_personalizada_1 = 0;
int minuto_inicial_dosagem_personalizada_1 = 0;
int hora_final_dosagem_personalizada_1 = 0;
int minuto_final_dosagem_personalizada_1 = 0;
int segunda_dosagem_personalizada_1 = 0;
int terca_dosagem_personalizada_1 = 0;
int quarta_dosagem_personalizada_1 = 0;
int quinta_dosagem_personalizada_1 = 0;
int sexta_dosagem_personalizada_1 = 0;
int sabado_dosagem_personalizada_1 = 0;
int domingo_dosagem_personalizada_1 = 0;
int hora_inicial_dosagem_personalizada_2 = 0;
int minuto_inicial_dosagem_personalizada_2 = 0;
int hora_final_dosagem_personalizada_2 = 0;
int minuto_final_dosagem_personalizada_2 = 0;
int segunda_dosagem_personalizada_2 = 0;
int terca_dosagem_personalizada_2 = 0;
int quarta_dosagem_personalizada_2 = 0;
int quinta_dosagem_personalizada_2 = 0;
int sexta_dosagem_personalizada_2 = 0;
int sabado_dosagem_personalizada_2 = 0;
int domingo_dosagem_personalizada_2 = 0;
int hora_inicial_dosagem_personalizada_3 = 0;
int minuto_inicial_dosagem_personalizada_3 = 0;
int hora_final_dosagem_personalizada_3 = 0;
int minuto_final_dosagem_personalizada_3 = 0;
int segunda_dosagem_personalizada_3 = 0;
int terca_dosagem_personalizada_3 = 0;
int quarta_dosagem_personalizada_3 = 0;
int quinta_dosagem_personalizada_3 = 0;
int sexta_dosagem_personalizada_3 = 0;
int sabado_dosagem_personalizada_3 = 0;
int domingo_dosagem_personalizada_3 = 0;
int quantidade_dose_dosadora_1_personalizada = 0;
int quantidade_dose_dosadora_2_personalizada = 0;
int quantidade_dose_dosadora_3_personalizada = 0;
int quantidade_dose_dosadora_1_automatica = 0;
int quantidade_dose_dosadora_2_automatica = 0;
int quantidade_dose_dosadora_3_automatica = 0;
int hora_inicial_dosagem_automatica_1 = 0;
int minuto_inicial_dosagem_automatica_1 = 0;
int hora_final_dosagem_automatica_1 = 0;
int minuto_final_dosagem_automatica_1 = 0;
int hora_inicial_dosagem_automatica_2 = 0;
int minuto_inicial_dosagem_automatica_2 = 0;
int hora_final_dosagem_automatica_2 = 0;
int minuto_final_dosagem_automatica_2 = 0;
int hora_inicial_dosagem_automatica_3 = 0;
int minuto_inicial_dosagem_automatica_3 = 0;
int hora_final_dosagem_automatica_3 = 0;
int minuto_final_dosagem_automatica_3 = 0;

//*****************************************************************************************
//************************** Variáveis temporárias das dosadoras **************************
//*****************************************************************************************
float fator_calib_dosadora_1_temp2;
float fator_calib_dosadora_2_temp2;
float fator_calib_dosadora_3_temp2;
float dose_dosadora_1_personalizada_temp2;
float dose_dosadora_2_personalizada_temp2;
float dose_dosadora_3_personalizada_temp2;
float dose_dosadora_1_automatica_temp2;
float dose_dosadora_2_automatica_temp2;
float dose_dosadora_3_automatica_temp2;
float dose_dosadora_1_manual_temp2;
float dose_dosadora_2_manual_temp2;
float dose_dosadora_3_manual_temp2;
int temp2hora_inicial_dosagem_personalizada_1;
int temp2minuto_inicial_dosagem_personalizada_1;
int temp2hora_final_dosagem_personalizada_1;
int temp2minuto_final_dosagem_personalizada_1;
int temp2segunda_dosagem_personalizada_1;
int temp2terca_dosagem_personalizada_1;
int temp2quarta_dosagem_personalizada_1;
int temp2quinta_dosagem_personalizada_1;
int temp2sexta_dosagem_personalizada_1;
int temp2sabado_dosagem_personalizada_1;
int temp2domingo_dosagem_personalizada_1;
int temp2hora_inicial_dosagem_personalizada_2;
int temp2minuto_inicial_dosagem_personalizada_2;
int temp2hora_final_dosagem_personalizada_2;
int temp2minuto_final_dosagem_personalizada_2;
int temp2segunda_dosagem_personalizada_2;
int temp2terca_dosagem_personalizada_2;
int temp2quarta_dosagem_personalizada_2;
int temp2quinta_dosagem_personalizada_2;
int temp2sexta_dosagem_personalizada_2;
int temp2sabado_dosagem_personalizada_2;
int temp2domingo_dosagem_personalizada_2;
int temp2hora_inicial_dosagem_personalizada_3;
int temp2minuto_inicial_dosagem_personalizada_3;
int temp2hora_final_dosagem_personalizada_3;
int temp2minuto_final_dosagem_personalizada_3;
int temp2segunda_dosagem_personalizada_3;
int temp2terca_dosagem_personalizada_3;
int temp2quarta_dosagem_personalizada_3;
int temp2quinta_dosagem_personalizada_3;
int temp2sexta_dosagem_personalizada_3;
int temp2sabado_dosagem_personalizada_3;
int temp2domingo_dosagem_personalizada_3;
int quantidade_dose_dosadora_1_personalizada_temp2;
int quantidade_dose_dosadora_2_personalizada_temp2;
int quantidade_dose_dosadora_3_personalizada_temp2;
int quantidade_dose_dosadora_1_automatica_temp2;
int quantidade_dose_dosadora_2_automatica_temp2;
int quantidade_dose_dosadora_3_automatica_temp2;
int modo_personalizado_on_1_temp2;
int modo_automatico_on_1_temp2;
int modo_personalizado_on_2_temp2;
int modo_automatico_on_2_temp2;
int modo_personalizado_on_3_temp2;
int modo_automatico_on_3_temp2;
int hora_inicial_dosagem_automatica_1_temp2;
int minuto_inicial_dosagem_automatica_1_temp2;
int hora_final_dosagem_automatica_1_temp2;
int minuto_final_dosagem_automatica_1_temp2;
int hora_inicial_dosagem_automatica_2_temp2;
int minuto_inicial_dosagem_automatica_2_temp2;
int hora_final_dosagem_automatica_2_temp2;
int minuto_final_dosagem_automatica_2_temp2;
int hora_inicial_dosagem_automatica_3_temp2;
int minuto_inicial_dosagem_automatica_3_temp2;
int hora_final_dosagem_automatica_3_temp2;
int minuto_final_dosagem_automatica_3_temp2;

//*****************************************************************************************
//************************** Variáveis dos timers *****************************************
//*****************************************************************************************
boolean temporizador_1 = false;
boolean temporizador_2 = false;
boolean temporizador_3 = false;
boolean temporizador_4 = false;
boolean temporizador_5 = false;
int temporizador_1_ativado = 0;
int temporizador_2_ativado = 0;
int temporizador_3_ativado = 0;
int temporizador_4_ativado= 0;
int temporizador_5_ativado= 0;
int on1_minuto = 0;
int on1_hora = 0;
int on2_minuto = 0;
int on2_hora = 0;
int on3_minuto = 0;
int on3_hora = 0;
int on4_minuto = 0;
int on4_hora = 0;
int on5_minuto = 0;
int on5_hora = 0;
int off1_minuto = 0;
int off1_hora = 0;
int off2_minuto = 0;
int off2_hora = 0;
int off3_minuto = 0;
int off3_hora = 0;
int off4_minuto = 0;
int off4_hora = 0;
int off5_minuto = 0;
int off5_hora= 0;

//*****************************************************************************************
//************************** Variáveis temporárias dos timers *****************************
//*****************************************************************************************
int on1_minuto_temp2;
int on1_hora_temp2;
int on2_minuto_temp2;
int on2_hora_temp2;
int on3_minuto_temp2;
int on3_hora_temp2;
int on4_minuto_temp2;
int on4_hora_temp2;
int on5_minuto_temp2;
int on5_hora_temp2;
int off1_minuto_temp2;
int off1_hora_temp2;
int off2_minuto_temp2;
int off2_hora_temp2;
int off3_minuto_temp2;
int off3_hora_temp2;
int off4_minuto_temp2;
int off4_hora_temp2;
int off5_minuto_temp2;
int off5_hora_temp2;
int temporizador_1_ativado_temp2;
int temporizador_2_ativado_temp2;
int temporizador_3_ativado_temp2;
int temporizador_4_ativado_temp2;
int temporizador_5_ativado_temp2;
//*****************************************************************************************
//************************** Variáveis de controle da potência dos leds *******************
//*****************************************************************************************
byte bled[96] = {                       // Potência de saída dos leds azuis 255 = 100% da potência
  0, 0, 0, 0, 0, 0, 0, 0,       // 0 e 2
  0, 0, 0, 0, 0, 0, 0, 0,       // 2 e 4
  0, 0, 0, 0, 12, 21, 30, 39,       // 4 e 6
  48, 57, 66, 75, 84, 93, 102, 111,     // 6 e 8 
  120, 129, 138, 147, 156, 165, 174, 183, // 8 e 10
  192, 201, 210, 219, 228, 237, 246, 255, // 10 e 12
  255, 246, 237, 228, 219, 210, 201, 192, // 12 e 14
  183, 174, 165, 156, 147, 138, 129, 120,  // 14 e 16
  111, 102, 93, 84, 75, 66, 57, 48,       // 16 e 18
  39, 30, 21, 12, 0, 0, 0, 0,         // 18 a 20
  0, 0, 0, 0, 0, 0, 0, 0,         // 20 e 22
  0, 0, 0, 0, 0, 0, 0, 0          // 22 a 24
};  

byte wled[96] = {                         //Potência de saída dos leds brancos 255 = 100% da potência
  0, 0, 0, 0, 0, 0, 0, 0,       // 0 e 2
  0, 0, 0, 0, 0, 0, 0, 0,       // 2 e 4
  0, 0, 0, 0, 12, 21, 30, 39,       // 4 e 6
  48, 57, 66, 75, 84, 93, 102, 111,     // 6 e 8 
  120, 129, 138, 147, 156, 165, 174, 183, // 8 e 10
  192, 201, 210, 219, 228, 237, 246, 255, // 10 e 12
  255, 246, 237, 228, 219, 210, 201, 192, // 12 e 14
  183, 174, 165, 156, 147, 138, 129, 120,  // 14 e 16
  111, 102, 93, 84, 75, 66, 57, 48,       // 16 e 18
  39, 30, 21, 12, 0, 0, 0, 0,         // 18 a 20
  0, 0, 0, 0, 0, 0, 0, 0,         // 20 e 22
  0, 0, 0, 0, 0, 0, 0, 0          // 22 a 24
};
byte tled[96];

char* tabela_textos[] PROGMEM = 
{
"",        // tabela_textos[0]
"MENU 1",  // tabela_textos[1]
"MENU 2",  // tabela_textos[2]
"HORA E DATA", // tabela_textos[3]
"TESTAR LED", // tabela_textos[4]
"CONTROLE PARAM.", // tabela_textos[5]
"GRAFICOS", // tabela_textos[6]
"ALT. VALORES LED", // tabela_textos[7]
"TPA AUTOMATICA", // tabela_textos[8] // Troca Parcial de Água automática.
"DOSADORAS", // tabela_textos[9]
"WAVEMAKER", // tabela_textos[10]
"INICIO", // tabela_textos[11]
"ALTERAR HORA E DATA", // tabela_textos[12]
"SALVAR", // tabela_textos[13]
"HORA:", // tabela_textos[14]
"DATA:", // tabela_textos[15]
"TESTAR VARIACAO DE LUMINOSIDADE", // tabela_textos[16]
"INICIAR", // tabela_textos[17]
"TESTE", // tabela_textos[18]
"TESTAR VARIACAO DE LUMINOSIDADE", // tabela_textos[19]
"PARAR", // tabela_textos[20]
"TESTE EM ANDAMENTO", // tabela_textos[21]
"TEMPO:", // tabela_textos[22]
"POTENCIA DE SAIDA (0-255):", // tabela_textos[23]
"CONTROLE DE TEMPERATURA DA AGUA", // tabela_textos[24]
"GRAFICO DE TEMPERATURA", // tabela_textos[25]
"GRAFICO DE ORP", // tabela_textos[26]
"GRAFICO DE PH DO AQUARIO", // tabela_textos[27]
"GRAFICO DE PH DO REATOR DE CALCIO",// tabela_textos[28]
"GRAFICO DE DENSIDADE",// tabela_textos[29]
"ALTERAR POTENCIAS DOS LEDS BRANCOS", // tabela_textos[30]
"ALTERAR POTENCIA DOS LEDS AZUIS", // tabela_textos[31]
"CONF. DIAS E HORARIO PARA TPA AUTOMATICA",// tabela_textos[32]
"ALTERAR CONFIGURACAO DAS DOSADORAS",// tabela_textos[33]
"ESCOLHA UM GRAFICO",// tabela_textos[34]
"WAVEMAKER", // tabela_textos[35]
"ESCOLHA UM PARAMETRO", // tabela_textos[36]
"CONTROLE DE PH DO REATOR DE CALCIO", // tabela_textos[37]
"CONTROLE DE ORP", // tabela_textos[38]
"CONTROLE DE PH DO AQUARIO",// tabela_textos[39]
"CONTROLE DE DENSIDADE", // tabela_textos[40]
"ESCOLHA UMA DOSADORA", // tabela_textos[41]
"CALIBRAR DOSADORA", // tabela_textos[42]
"DOSAGEM MANUAL", // tabela_textos[43]
"CONFIGURAR DOSAGEM AUTOMATICA" ,// tabela_textos[44]
"CONFIGURAR DOSAGEM PERSONALIZADA", // tabela_textos[45]
"DIGITE A SENHA", // tabela_textos[46]
"CONFIGURAR DOSAGEM PERSONALIZADA", // tabela_textos[47]
"ALTERAR CONFIGURACAO DAS DOSADORAS", // tabela_textos[48]
"REVER CONF. DA DOSAGEM AUTOMATICA", // tabela_textos[49]
"REVER CONF. DA DOSAGEM PERSONALIZADA", // tabela_textos[50]
"DESATIVAR DOSADORAS", // tabela_textos[51]
"LUZ NOTURNA   255 = 100 %", // tabela_textos[52]
"SELECIONE O TIMER", // tabela_textos[53]
"CONFIGURAR TIMER 1", // tabela_textos[54]
"CONFIGURAR TIMER 2", // tabela_textos[55]
"CONFIGURAR TIMER 3", // tabela_textos[56]
"CONFIGURAR TIMER 4", // tabela_textos[57]
"CONFIGURAR TIMER 5", // tabela_textos[58]
"SEGUNDA", // tabela_textos[59]
"TERCA", // tabela_textos[60]
"QUARTA", // tabela_textos[61]
"QUINTA", // tabela_textos[62]
"SEXTA", // tabela_textos[63]
"SABADO", // tabela_textos[64]
"DOMINGO", // tabela_textos[65]
"VOLTAR", // tabela_textos[66]
"LUA NOVA", // tabela_textos[67]
"LUA CRESCENTE", // tabela_textos[68]
"LUA CHEIA",  // tabela_textos[69]
"S", // tabela_textos[70] // Segunda, Sexta e Sábado
"T",// tabela_textos[71]
"Q", // tabela_textos[72] // Quarta e Quinta
"D", // tabela_textos[73]
"TEMPERATURA DESEJADA:", // tabela_textos[74]
"VARIACAO PERMITIDA:", // tabela_textos[75]
"VARIACAO PARA ACIONAR O ALARME:", // tabela_textos[76]
"POTENCIA DOS LEDS BRANCOS: 255 = 100%", // tabela_textos[77]
"AZUL", // tabela_textos[78]
"POTENCIA LEDS AZUIS: 255 = 100%", // tabela_textos[79]
"BRANCO", // tabela_textos[80]
"ALTERAR", // tabela_textos[81]
"SALVAR EEPROM", // tabela_textos[82]
"HORA:", // tabela_textos[83]
"DURACAO MAXIMA", // tabela_textos[84]
"POR ESTAGIO:", // tabela_textos[85]
"FALHA!", // tabela_textos[86]
"NORMAL", // tabela_textos[87]
"MANUAL", // tabela_textos[88]
"AUTOMATICO", // tabela_textos[89]
"PERSONALIZADO", // tabela_textos[90]
"REVER CONFIG.", // tabela_textos[91]
"DESATIVAR MODOS", // tabela_textos[92]
"TEMPERATURA", // tabela_textos[93]
"PH AQUARIO", // tabela_textos[94]
"PH REATOR", // tabela_textos[95]
"ORP", // tabela_textos[96]
"DENSIDADE", // tabela_textos[97]
"CALIBRAR SONDAS", // tabela_textos[98]
"PH DESEJADO:", // tabela_textos[99]
"ORP DESEJADA:", // tabela_textos[100]
"DENSIDADE DESEJADA:", // tabela_textos[101]
"DOSADORA 1", // tabela_textos[102]
"DOSADORA 2", // tabela_textos[103]
"DOSADORA 3", // tabela_textos[104]
"COLOQUE O RECIPIENTE SOB O BICO", // tabela_textos[105]
"DOSADOR E CLIQUE EM INICIAR.", // tabela_textos[106]
"EM SEGUIDA INSIRA O VALOR OBTIDO", // tabela_textos[107]
"EM ML NO CAMPO ABAIXO.", // tabela_textos[107]
"PARA MELHORES RESULTADOS REPITA O TESTE", // tabela_textos[109]
"ALGUMAS VEZES E USE UMA MEDIA.", // tabela_textos[110]
"DOSADORA 1 SELECIONADA", // tabela_textos[111]
"DOSADORA 2 SELECIONADA",// tabela_textos[112]
"DOSADORA 3 SELECIONADA", // tabela_textos[113]
"SELECIONE O VOLUME A SER DOSADO", // tabela_textos[114]
"EM ML NO CAMPO ABAIXO  E", // tabela_textos[115]
"EM SEGUIDA TOQUE EM INICIAR.", // tabela_textos[116]
"A DOSAGEM INICIARA EM 10 SEGUNDOS",// tabela_textos[117]
"INICIAL:", // tabela_textos[118]
"FINAL:", // tabela_textos[119]
"SELECIONE O", // tabela_textos[120]
"VOLUME ", // tabela_textos[121]
"(ML/DIA):",  // tabela_textos[122]
"QUANTIDADE",  // tabela_textos[123]
"DE DOSES:",  // tabela_textos[124]
"SELECIONADA", // tabela_textos[125]
"ON", // tabela_textos[126]
"OFF", // tabela_textos[127]
"QUANTIDADE DE DOSES:", // tabela_textos[128]
"DOSADORA 1:", // tabela_textos[129]
"DOSADORA 2:", // tabela_textos[130]
"DOSADORA 3:", // tabela_textos[131]
"LUA MINGUANTE", // tabela_textos[132]
"CALIBRAR", // tabela_textos[133]
"DESATIVAR MODOS", // tabela_textos[134]
"PROXIMO", // tabela_textos[135]
"LIMPAR", // tabela_textos[136]
"ENTRAR", // tabela_textos[137]
"ANTERIOR", // tabela_textos[138]
"AUTOMATICO", // tabela_textos[139]
"PERSONALIZADO", // tabela_textos[140]
"USE ESTA", // tabela_textos[141]
"FUNCAO", // tabela_textos[142]
"APENAS PARA", // tabela_textos[143]
"DESATIVAR", // tabela_textos[144]
"OS MODOS", // tabela_textos[145]
"DE DOSAGEM.", // tabela_textos[146]
"ATIVADO", // tabela_textos[147]
"DESATIVADO", // tabela_textos[148]
"UM DOS MODOS", // tabela_textos[149]
"DE DOSAGEM ", // tabela_textos[150]
"ESTA:", // tabela_textos[151]
"DESATIVADOS", // tabela_textos[152]
"AMBOS OS MODOS", // tabela_textos[153]
"ESTAO OU", // tabela_textos[154]
"SERAO:", // tabela_textos[155]
"ILUMINACAO MINIMA:", // tabela_textos[156]
"LUA CHEIA:", // tabela_textos[157]
"LUA NOVA:", //  tabela_textos[158]
"LUZ NOTURNA", // tabela_textos[159]
"TIMERS", // tabela_textos[160]
"LIGAR TIMER", // tabela_textos[161]
"DESLIGAR TIMER", // tabela_textos[162]
"ALTERADO!", // tabela_textos[163]
"MODO MANUAL SELECIONADO", // tabela_textos[164]
"MODO AUTOMATICO SELECIONADO", // tabela_textos[165]
"INTERVALO ", // tabela_textos[166]
"ERRADO!", // tabela_textos[167]
"HORA FINAL", // tabela_textos[168]
"ERRADA!", // tabela_textos[169]
"HORA", // tabela_textos[170]
"MODO PERSONALIZADO SELECIONADO", // tabela_textos[171]
"MODO CALIBRACAO SELECIONADO",  // tabela_textos[172]
"SENHA CORRETA", // tabela_textos[173]
"SENHA INCORRETA!", // tabela_textos[174]
"MAXIMO 6 DIGITOS!", // tabela_textos[175]
"O INTERVALO ENTRE A INICIAL E A FINAL", // tabela_textos[176]
"DEVE SER DE NO MINIMO 10 MINUTOS!", // tabela_textos[177]
"A HORA FINAL NAO PODE SER MENOR", // tabela_textos[178]
"QUE A INICIAL!",// tabela_textos[179]
"ILUMINACAO MAXIMA:", // tabela_textos[180]
"% ILUMINADA", // tabela_textos[181]
"FASE LUNAR:", // tabela_textos[182]
"T.DISSIPADOR:", // tabela_textos[183]
"TEMP. AGUA:", // tabela_textos[184]
"PH DO AQUARIO:",  // tabela_textos[185]
"PH DO REATOR:",  // tabela_textos[186]
"DENSIDADE:", // tabela_textos[187]
"ORP:", // tabela_textos[188]
"CHILLER:", // tabela_textos[189]
"AQUECEDOR:", // tabela_textos[190]
"REATOR:", // tabela_textos[191]
"OZONIO:", // tabela_textos[192]
"REPOSICAO:", // tabela_textos[193]
"NIVEL:", // tabela_textos[194]
"TPA:", // tabela_textos[195]
"BAIXO!", // tabela_textos[196]
"NORMAL" // tabela_textos[197]
};
