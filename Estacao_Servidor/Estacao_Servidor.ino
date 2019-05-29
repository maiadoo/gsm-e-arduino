#include <SoftwareSerial.h>

SoftwareSerial serialGSM(10, 11); 
// TX e RX do SIM800L nos pinos 10 e 11 do Arduino (UNO), respectivamente

bool temSMS = false;
String telefoneSMS;
String dataHoraSMS;
String mensagemSMS;
String comandoGSM = "";
String ultimoGSM = "";

#define senhaGsm "1234" //Para receber e enviar SMS, confirmar número
//#define numeroCall "9xxxxxxxx" //Número que irá receber chamadas do módulo///////////////////////



//Funções devem ser declaradas antes do setup, coisas do C para Arduino 
void leGSM();
void enviaSMS(String telefone, String mensagem);
void configuraGSM();

void setup() {

  Serial.begin(9600); //Velociade padrão de leitura e resposta da serial
  serialGSM.begin(9600); 

  Serial.println("Sketch Iniciado!"); //Verificação do módulo
  configuraGSM();
}

void loop() {
  leGSM();

  if (comandoGSM != "") { //Verifica o que foi feito e o comando utilizado anteriormente
      Serial.println(comandoGSM);
      ultimoGSM = comandoGSM;
      comandoGSM = "";
  }

  if (temSMS) { //Verifica recebimento de SMS e envio

     Serial.println("Chegou Mensagem!!");
     Serial.println();
    
     Serial.print("Remetente: ");  
     Serial.println(telefoneSMS);
     Serial.println();
    
     Serial.print("Data/Hora: ");  
     Serial.println(dataHoraSMS);
     Serial.println();
    
     Serial.println("Mensagem:");  
     Serial.println(mensagemSMS);
     Serial.println();
      
     mensagemSMS.trim();      
     /*if ( mensagemSMS == senhaGsm ) {
        Serial.println("Enviando SMS de Resposta.");  
         enviaSMS(telefoneSMS, "SMS Recebido e Senha OK!"); //Envia SMS de confirmação de recebimento
     }*/
     temSMS = false;
  }          
  
}

void leGSM()
{
  static String textoRec = "";
  static unsigned long delay1 = 0;
  static int count=0;  
  static unsigned char buffer[64];

  if (serialGSM.available()) {            
 
     while(serialGSM.available()) {         
   
        buffer[count++] = serialGSM.read();     
        if(count == 64)break;
     }

     textoRec += (char*)buffer;
     delay1   = millis();
     
     for (int i=0; i<count; i++) {
         buffer[i]=NULL;
     } 
     count = 0;                       
  }


  if ( ((millis() - delay1) > 100) && textoRec != "" ) {

     if ( textoRec.substring(2,7) == "+CMT:" ) {
        temSMS = true;
     }

     if (temSMS) {
            
        telefoneSMS = "";
        dataHoraSMS = "";
        mensagemSMS = "";

        byte linha = 0;  
        byte aspas = 0;
        for (int nL=1; nL < textoRec.length(); nL++) {

            if (textoRec.charAt(nL) == '"') {
               aspas++;
               continue;
            }                        
          
            if ( (linha == 1) && (aspas == 1) ) {
               telefoneSMS += textoRec.charAt(nL);
            }

            if ( (linha == 1) && (aspas == 5) ) {
               dataHoraSMS += textoRec.charAt(nL);
            }

            if ( linha == 2 ) {
               mensagemSMS += textoRec.charAt(nL);
            }

            if (textoRec.substring(nL - 1, nL + 1) == "\r\n") {
               linha++;
            }
        }
     } else {
       comandoGSM = textoRec;
     }
     
     textoRec = "";  
  }     
}


void enviaSMS(String telefone, String mensagem) {    //a mensagem será a string de dados de cada sensor da estação
  serialGSM.print("AT+CMGS=\"" + telefone + "\"\n"); //o telefone é o número de telefone do chip do módulo que receberá os dados 
  serialGSM.print(mensagem + "\n");
  serialGSM.print((char)26); 
}


void configuraGSM() {
   serialGSM.print("AT+CMGF=1\n;AT+CNMI=2,2,0,0,0\n;ATX4\n;AT+COLP=1\n"); //Algumas configurações específicas
   //AT+CMGF=1, Formato de SMS (1 = texto, 0 = dados(PDU)), 
   //AT+CNMI=2,2,0,0,0, Indicadores de nova mensagem (Mode 2 = mensagem solicitada diretamente, 
   //                             mt 2 = verifica tipografia das mensagens, itálico, por exemplo,
   //                             bm 0 = roteamento em modem,
   //                             ds 0 = Sem SMS-STATUS-REPORT,
   //                             bfr 0 = Sem mensagem no buffer),
   //ATX4, Monitor de monitoramento de chamada (4 = habilita dial tone e detecção de linha ocupada),
   //AT+COLP=1, Linha conectada, habilitar ou náo notificação (1 = habilitar)      
}
