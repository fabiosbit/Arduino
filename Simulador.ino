//Created 24 May 2011 by Jim Lindblom SparkFun Electronics https://www.sparkfun.com/products/10182 "Example Code"
//Updated 24 Apr 2012 by Jean David SEDRUE Version betatest26 - 24042012 http://www.gamoover.net/Forums/index.php?topic=25907
//Updated 20 May 2013 by RacingMat in english http://www.x-sim.de/forum
//Modificado em Agosto de 2020 por Fabiosbit.

#define pinMotorDireito 1
#define pinMotorEsquerdo 0
#define pinPotDireito A4
#define pinPotEsquerdo A5
#define pwmMax 255  //velocidade máxima do motor, de 0 a 255 (tensão de 0V a 24V)
#define Horario 1
#define AntHorario 2

//Define os pontos máximos e mínimos dos dois potenciômetros (de 0 a 1023)
//Utilize o monitor serial para encontrar os valores corretos
const int potMin = 40;
const int potMax = 500;

//Definição dos pinos do Arduino:
int ponteInA[2] = {7, 2}; //PonteH entradas MxINA: sentido horário
int ponteInB[2] = {8, 4}; //PonteH entradas MxINB: sentido anti-horário
int pwmPin[2] = {10, 9};  //PonteH entradas MxPWM

//Posição inical (simulador centralizado)
int posDestinoEsquerdo = 225;
int posDestinoDireito = 225;

////////////////////////////////////////////////////////////////////////////////
//Função de inicialização
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < 2; i++)
  {
    pinMode(ponteInA[i], OUTPUT);
    pinMode(ponteInB[i], OUTPUT);
    pinMode(pwmPin[i], OUTPUT);
  }

  //Inicializando com os motores desligados
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(ponteInA[i], LOW);
    digitalWrite(ponteInB[i], LOW);
    analogWrite(pwmPin[i], 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
//Função Loop principal
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  int posAtualEsquerdo, posAtualDireito;

  LerDadosJogo(); //Recebe as novas posições de destino dos motores

  posAtualDireito = analogRead(pinPotDireito);    //Faz a leitura da posição atual do motor direito
  posAtualEsquerdo = analogRead(pinPotEsquerdo);  //Faz a leitura da posição atual do motor esquerdo

  MoverMotor(pinMotorDireito, posAtualDireito, posDestinoDireito);	//Liga o motor direito até chegar na nova posição
  MoverMotor(pinMotorEsquerdo, posAtualEsquerdo, posDestinoEsquerdo);	//Liga o motor esquerdo até chegar na nova posição

//  LerPotenciometros(); //Descomente esta função para verificar e ajustar os potenciômetros
}

////////////////////////////////////////////////////////////////////////////////
//Função: Imprime no console a posição atual dos potenciômetros (somente para ajustes)
////////////////////////////////////////////////////////////////////////////////
void LerPotenciometros()
{
  Serial.println("Potenciometros");
  Serial.print(analogRead(pinPotDireito));
  Serial.print(" ; ");
  Serial.println(analogRead(pinPotEsquerdo));
  Serial.println();
  delay(250);
}

////////////////////////////////////////////////////////////////////////////////
//Função: Recebe e trata os dados de telemetria do jogo capturados pelo simtools.
////////////////////////////////////////////////////////////////////////////////
void LerDadosJogo()
{
  byte dados[3] = {'0', '0', '0'};

  if (Serial.available() > 2)
  {
    //Verifica no primeiro byte se o dado se refere ao lado direito ou esquerdo (Right ou Left)
    dados[0] = Serial.read();
    if (dados[0] == 'L')
    {
      dados[1] = Serial.read();
      dados[2] = Serial.read();
      posDestinoDireito = ConverterDadosHexa(dados);
    }
	
    if (dados[0] == 'R')
    {
      dados[1] = Serial.read();
      dados[2] = Serial.read();
      posDestinoEsquerdo = ConverterDadosHexa(dados);
    }
  }

  if (Serial.available() > 16) Serial.flush();	//Aguarda a transmição de todos os bytes
}

////////////////////////////////////////////////////////////////////////////////
//Função: Calcular os movimentos, freio e velocidade
////////////////////////////////////////////////////////////////////////////////
void MoverMotor(int motor, int posAtual, int posDestino)
{
  int distMinima = 20;	//Define uma distância mínima para evitar movimentos muito curtos
  int distancia;
  int pwm;
  int distanciaFreio = 40;  //Disponibiliza uma distância para o motor parar ou inverter a rotação

  //Certifica que o destino está dentro do range mecânico
  posDestino = constrain(posDestino, potMin + distanciaFreio, potMax - distanciaFreio);

  distancia = abs(posDestino - posAtual); //valor absoluto

  if (distancia <= distMinima)
  {
    DesligarMotor(motor); //Distância muito curta para se mover
  }
  else
  {
    //Calcula a velocidade conforme a distância
    pwm = (pwmMax * 0.7);
    if (distancia > 50)   pwm = (pwmMax * 0.8);
    if (distancia > 80)   pwm = (pwmMax * 0.9);
    if (distancia > 100)  pwm =  pwmMax;

	//Verifica se os motores não estão fora do limite mecânico e faz voltar para o range se preciso
	//Depois chama a função que liga os motores
    if ((posAtual < potMin) || (posAtual < posDestino)) AcionarMotor(motor, Horario, pwm);
    if ((posAtual > potMax) || (posAtual > posDestino)) AcionarMotor(motor, AntHorario, pwm);
  }
}


////////////////////////////////////////////////////////////////////////////////
//Função: Desliga o motor, giro livre.
////////////////////////////////////////////////////////////////////////////////
void DesligarMotor(int motor) 
{
  digitalWrite(ponteInA[motor], HIGH);
  digitalWrite(ponteInB[motor], HIGH);
  analogWrite(pwmPin[motor], 0);
}

////////////////////////////////////////////////////////////////////////////////
//Função: Liga os motores no sentido necessário
////////////////////////////////////////////////////////////////////////////////
void AcionarMotor(int motor, int direcao, int pwm)
{
  if (direcao == 1)
  {
    digitalWrite(ponteInA[motor], HIGH);
    digitalWrite(ponteInB[motor], LOW);
  }
  else
  {
    digitalWrite(ponteInA[motor], LOW);
    digitalWrite(ponteInB[motor], HIGH);
  }

  analogWrite(pwmPin[motor], pwm);
}


////////////////////////////////////////////////////////////////////////////////
// Função: converte de Hexadecimal para Decimal
////////////////////////////////////////////////////////////////////////////////
int ConverterDadosHexa(byte x[3])
{
  int resultado;

  if ((x[2] == 13) || (x[2] == 'R') || (x[2] == 'L'))
  {
    x[2] = x[1];
    x[1] = '0';
  }
  for (int i = 1; i < 3; i++)
  {
    if (x[i] > 47 && x[i] < 58 ) 
    { //para xA até xF
      x[i] = x[i] - 48;
    }
    if (  x[i] > 64 && x[i] < 71 ) 
    { //para x0 até x9
      x[i] = x[i] - 55;
    }
  }
	
  //Mapeia o range do Simtools (0 a 255) para o range mecânico do simulador (potMin a potMax)
  resultado = map((x[1] * 16 + x[2]), 0, 255, potMin, potMax);
  return resultado;
}
