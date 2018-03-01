/**
  @progName emuladorChido.cpp
  @desc Simulador de la ejecucion de instrucciones en lenguaje pseudo-ensamblador.
  @author Humberto Gonzalez, Mariano Alipi, Rodrigo Bilbao.
  @date 23 de febrero de 2018
*/
/*
----------COSAS A CONSIDERAR----------

Memoria de 1000 palabras.

Una palabra consiste de seis números:
[ 5 | 4 | 3 | 2 | 1 | 0 ]

Instrucción:
5 y 4     Código de operación
3         Tipo de direccionamiento
2, 1 y 0  Parámetro: dirección/valor

Dato:
5               Signo
4, 3, 2, 1 y 0  Números

Instrucciones:
LDA, STA, CLA, ADD, SUB, NOP, NEG, HLT, JMP (opcional)

Tipos de direccionamiento:
Absoluto (1), indirecto (2), inmediato (3), relativo (4).

---------------LOG---------------
(Si cambian algo pongan qué cambiaron y el día y hora. :))
  + Adición al programa
  * Modificación del programa
  - Eliminación del programa

02/feb 11:30 + Archivo creado.
15/feb 02:30 Mariano:
             + Agregué todo el código que he estado probando.
15/feb 16:00 Mariano:
             * Modifiqué el orden de las opciones del menú.
             + Agregué código en la opción de modificar memoria con ensamblador.
16/feb 14:30 Mariano:
             + Agregué opciones, su funcionamiento y la capacidad de cambiarlas.
20/feb 14:30 Mariano y Humberto:
             + Comentarios de cada función para explicar lo que hacen.
             + Agregamos funciones para cada opción del menú y para refrescar la pantalla.
             * Limpieza y orden del menú.
22/feb 18:00 Mariano y Humberto:
             + Agregamos validación al modificar la memoria directamente o con ensamblador.
             + Agregamos la estructura de ejecución de las instrucciones (aún falta implementación).
             + Agregamos funciones para ejecutar cada operación del simulador (aún falta implementación).
23/feb 21:30 Mariano y Humberto:
             + Agregamos todas las funciones de las operaciones.
             + Agregamos la funcion que muestra como se van realizando las microoperaciones.
             + Agregamos la opcion de decidir el tiempo que toma cada microoperacion en ejecutarse.
             + Acabamos todo.
*/

// Identificar y hacer la configuración necesaria según el sistema operativo para la función de esperar.
#ifdef _WIN32
   // Library and definitions for Windows (32-bit and 64-bit).
   #include <windows.h>
   #define WAIT Sleep
   #define CONV 1000
#elif __APPLE__
    // Library and definitios for Apple devices.
    #include <unistd.h>
    #define WAIT usleep
    #define CONV 1
#elif __linux__
    // Library  and definitios for Linux systems.
    #include <unistd.h>
    #define WAIT usleep
	#define CONV 1
#elif __unix__
	// All Unices not caught above.
    // Unix
    #include <unistd.h>
    #define WAIT usleep
		#define CONV 1
#else
    #error "Unknown compiler"
#endif

#include <iostream>
#include <locale.h>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>

#define MEMSIZE 1000

using namespace std;

// Arreglos con los códigos de operación.
//                 00     01     02      03     04     05    06     07     08
string codes[] = {"NOP", "CLA", "LDA", "STA", "ADD", "SUB", "NEG", "JMP", "HLT"};
// Arreglo de la memoria del simulador.
string data[MEMSIZE];
// Opciones.
bool showWholeMemory = false, onlyShowErrors = false;
// Valor del PC inicial
int PC = 0, PCprev;
// Otros registros
string MDR, AC, MAR, IR;
// Duración del intervalo de ejecución de las microoperaciones.
int secs = 3;

// Función que obtiene el código de operación según un string.
// Parámetro: el string con la operación (por ejemplo: "LDA").
// Valor de retorno: int del código de operación (por ejemplo: 2).
int getOpCode(string operation) {
    for(int i=0; i<9; i++) {
        if(codes[i] == operation)
            return i;
    }
    return -1;
}

// Función que obtiene el tipo de direccionamiento según un string.
// Parámetro: el string con una letra que representa el tipo (por ejemplo: "ABS").
// Valor de retorno: int del tipo de direccionamiento (por ejemplo: 1).
int getAddrType(string input) {
    if(input == "ABS")
        return 1;
    if(input == "IND")
        return 2;
    if(input == "INM")
        return 3;
    if(input == "REL")
        return 4;
    return -1;
}

// Función que convierte un string a mayúsculas.
// Parámetros: el string por modificar.
// Valor de retorno: el string en mayúsculas.
string toUpper(string str) {
    string result = "";
    for(int i = 0; i < str.length(); i++) {
        result += toupper(str[i]);
    }
    return result;
}

// Función que convierte un entero a un string.
// Parámetro: el número entero.
// Valor de retorno: string con el entero convertido.
string toString(int num) {
	ostringstream str;
  str << num;
  return str.str();
}


// Función que vacía la memoria del simulador.
// Parámetros: ninguno.
// Valor de retorno: ninguno.
void emptyMemory() {
  for(int i = 0; i < MEMSIZE; i++) {
    data[i] = "";
  }
}

/*
	Función que limpia la pantalla.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void refreshScreen() {
  cout << string(80, '\n');
}

/*
	Funcion que completa el PC con los ceros requeridos para mostrarlo.
  Parametros: PC.
	Valor de retorno: string con el PC completo.
*/
string completePC(int iPC) {
 	string myPC = toString(iPC);
  ostringstream complete;

  for(int i = 0; i < 3 - myPC.length(); i++) {
  	complete << 0;
  }

  complete << myPC;
  return complete.str();
}

// Función que convierte de maquinal a ensamblador.
// Parámetros: string con la instrucción en maquinal.
// Valor de retorno: string con la instrucción en esamblador.
string convertAssemb(string inst) {

  string opCode = inst.substr(0,2), code;
  char addrType = inst[2];
  string addr;
  string parameter = inst.substr(3);

  code = codes[atoi(opCode.c_str())];

  switch(addrType) {
  	case '1':
    	addr = "ABS";
    	break;
    case '2':
    	addr = "IND";
    	break;
    case '3':
    	addr = "INM";
    	break;
    case '4':
    	addr = "REL";
    	break;
  }

  if(code == "NOP" || code == "CLA" || code == "NEG" || code == "HLT")
    parameter = "";

  return code + " " + addr + " " + parameter;;
}

/*
  Funcion que muestra las direcciones de memoria, su contenido y la dirección ejecutándose actualmente.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void showMemoryReg() {
	int iSpaces;
  for(int i = 0; i < MEMSIZE; i++) {
    if (data[i] != "") {
      if(data[i][0] != '+' && data[i][0] != '-') {
        cout << setw(3) << setfill('0') << i << "\t" << data[i];

        iSpaces = 15 - convertAssemb(data[i]).length();

        cout << "  " << convertAssemb(data[i]);
        if(i == PCprev)
          cout << setw(iSpaces) << setfill(' ') << "<==";
        cout << endl;
      } else {
      	cout << setw(3) << setfill('0') << i << "\t" << data[i];
    		cout << endl;
      }
    }
  }
  cout << endl;
}

/*
	Funcion que muestra en pantalla los registros y sus cambios
  Parametros: ninguno.
  valor de retorno: ninguno.
*/
void displayChanges() {
  WAIT(secs * CONV);
  refreshScreen();

  cout << "\t\tR E G I S T R O S" << endl << endl;
	cout << setfill(' ') << setw(5) << "|"  << setw(5) << "PC" << setw(4) << "|" << setw(6) << "MAR" << setw(4) << "|"  << setw(6) << "MDR" << setw(4) << "|"  << setw(5) << "IR" << setw(4) << "|" << endl;
  cout << setw(10) << completePC(PC) << " " << setw(9) << MAR << " " << setw(10) << MDR << " " << setw(9) << IR << endl << endl;
  cout << setw(11) << "AC" << ": " << setw(8) << AC << endl;

  cout << endl;
  showMemoryReg();
}

// Función que regresa un string que contiene cómo se mostrará la opción de acuerdo con su estado (activado/desactivado).
// Parámetros: una variable booleana.
// Valor de retorno: un string que contiene cómo se mostrará la opción.
string getBoolX(bool var) {
    if(var)
        return "[X]";
    else
        return "[ ]";
}

/*
  Funcion que muestra las direcciones de memoria y su contenido.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void showMemory() {
  if(showWholeMemory) {
    for(int i = 0; i < 10; i++) {
        cout << "\t" << setw(2) << setfill('0') << i;
    }

    cout << endl;

    for(int i = 0; i < MEMSIZE; i += 10) {
        cout << setw(3) << setfill('0') << i;

        for(int j = i; j < i + 10; j++) {
                cout << "\t" << data[j];
        }
        cout << endl;
    }

    cout << endl << endl;
  }
  else {
    cout << "Se muestran solo las direcciones de memoria no vacias:" << endl << endl;
    for(int i = 0; i < MEMSIZE; i++) {
      if (data[i] != "") {
        if(data[i][0] != '+' && data[i][0] != '-') {
              cout << setw(3) << setfill('0') << i << "\t" << data[i] << "  " << convertAssemb(data[i]) << endl;
        }
        else {
         cout << setw(3) << setfill('0') << i << "\t" << data[i] << endl;
        }
      }
    }
  }

  cout << endl;
}


/*
  Función que edita el contenido de una dirección de memoria directamente.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void editMemoryDirectly() {
  int dir;
  string val;

  cout << "Introduzca la direccion de memoria por modificar: ";
  cin >> dir;

  cout << "La dirección " << setw(3) << setfill('0') << dir << " contiene: ";
  if(data[dir] == "")
      cout << "(vacío)";
  else
      cout << data[dir] << "   (" << convertAssemb(data[dir]) << ")";
  cout << endl;

  cout << "Introduzca el nuevo valor: ";
  cin >> val;
  cout << endl;

  // Validation process starts here.

  // If it's data/value...
  if(val[0] == '+' || val[0] == '-') {
  	data[dir] = val;
  } else {
  	// If it's an instruction...
    string opCode, addr, param;
    int iOpCode, iAddr;

    opCode = val.substr(0, 2);
    iOpCode = atoi(opCode.c_str());
    // If it's a valid operation code...
    if(iOpCode >= 0 && iOpCode < 9) {
    	addr = val.substr(2, 1);
		iAddr = atoi(addr.c_str());

      // If it's an instruction which doesn't take parameters, set addressing type to 1 to make it valid.
      if(iOpCode == 0 || iOpCode == 1 || iOpCode == 6 || iOpCode == 8)
        iAddr = 1;

      // If it's a valid addressing type...
      if(iAddr >= 1 && iAddr <= 4) {
    		param = val.substr(3);

        // If parameter is three characters long...
        if(param.length() == 3) {

          // If addressing type is ABS or IND...
          if(iAddr == 1 || iAddr == 2) {
            if(param[0] == '+'  || param[0] == '-') {
              cout << "  ERROR: El parámetro no puede tener signo para ese tipo de direccionamiento." << endl;
              return;
            }
          }

			// Success. Save to memory.
        	data[dir] = val;
  				cout << "Dirección de memoria modificada exitosamente.";
        } else {
        	cout << "ERROR: el parámetro debe ser de tres caracteres.";
        }
      } else {
        cout << "ERROR: el tipo de direccionamiento no es válido.";
      }
    } else {
    	cout << "ERROR: el código de operación no es válido.";
    }

  }
   cout << endl;
}

/*
  Función que edita el contenido de una dirección de memoria con la instrucción dada.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void editMemory() {
  int dir;
  string line;

  cout << "Introduzca la dirección de memoria por modificar: ";
  cin >> dir;
  cin.ignore();

  cout << "La dirección " << setw(3) << setfill('0') << dir << " contiene: ";
  if(data[dir] == "")
      cout << "(vacío)";
  else
      cout << data[dir] << "   (" << convertAssemb(data[dir]) << ")";
  cout << endl;

  cout << "Introduzca el nuevo contenido en ensamblador (por ejemplo, LDA ABS 003): ";
  getline(cin, line);
  line = toUpper(line);

  // "Compiling" process starts here.

  // ints to store operation code, addressing type and parameter value. They will be merged to form an instruction.
  int opCode, addrType;
  string segment, param;

      // If the line is empty, store as empty string("").
      if(line.empty()) {
          data[dir] = "";
      } else {
          istringstream inStream(line);
          ostringstream outStream;

          // This should contain the operation (e.g. "LDA" or "CLA").
          inStream >> segment;
          opCode = getOpCode(segment);

          // If operation is an instruction which doesn't take parameters...
          if(codes[opCode] == "HLT" || codes[opCode] == "NEG" || codes[opCode] == "CLA" || codes[opCode] == "NOP") {
                  outStream << setw(2) << setfill('0') << opCode;
                  outStream << "0000";
                  data[dir] = outStream.str();

            			cout << data[dir] << endl << endl;

            // If operation code is valid...
          } else if(opCode != -1) {

              // This should contain the addressing type (e.g. "ABS" or "INM").
              inStream >> segment;
              addrType = getAddrType(segment);

            	// If the addressing type is valid...
              if(addrType != -1) {

                  // This should contain the parameter value, a three digit number (e.g. "020" or "123").
                  inStream >> segment;

                  if(segment.length() == 3) {

                    	// If addressing type is ABS or IND...
                      if(addrType == 1 || addrType == 2) {
                        if(segment[0] == '+'  || segment[0] == '-') {
                        	cout << "ERROR: El parámetro no puede tener signo para ese tipo de direccionamiento." << endl;
                          return;
                        }
                      }

                      param = segment;

                      outStream << setw(2) << setfill('0') << opCode;
                      outStream << addrType;
                      outStream << param;

                      data[dir] = outStream.str();
                      cout << data[dir] << endl << endl;

                  } else {
                      cout << "ERROR: no se encontró un valor de parámetro válido." << endl;
                      return;
                  }
              } else {
                  cout << "ERROR: no se encontró un tipo de direccionamiento válido." << endl;
                  return;
              }

          // If operation code is invalid but it's a value (values start with the sign and must be six characters long)...
          } else if( (line[0] == '+' || line[0] == '-') &&  line.length() == 6 ) {
              data[dir] = line;
            	cout << data[dir] << endl << endl;

          // If operation code is invalid and it's not a value...
          } else {
              cout << "ERROR: no se encontró una operación o valor/dato válido." << endl;
              return;
          }
      }

  	cout << endl << "Dirección de memoria modificada exitosamente." << endl;
}

/*
  Función que carga la memoria del simulador con los datos de un archivo.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void loadFile() {
  ifstream file;
  string line, segment, fileName;
  bool compileSuccess = true;

  cin.ignore();
  cout << "Se sobreescribirán las direcciones de memoria empalmadas." << endl << endl;
  cout << "Introduzca el nombre del archivo por leer: ";
  getline(cin, fileName);

  file.open(fileName.c_str());

  if(!file.is_open()) {
      cout << endl << "No se pudo abrir el archivo. Verifique que el archivo exista y que el nombre sea correcto." << endl;
      return;
  } else {
      cout << endl << "Leyendo archivo..." << endl << endl;
  }

  // ints to store operation code, addressing type and parameter value. They will be merged to form an instruction.
  int opCode, addrType;
  string param;
  int i = 0;
  while(getline(file, line)) {

      if(!onlyShowErrors)
          cout << "Leyendo línea " << setw(3) << setfill('0') << i << "..." << endl;

      line = toUpper(line);

      // If the line is empty, store as empty string("").
      if(line.empty()) {
          if(!onlyShowErrors)
              cout << "  Línea vacía encontrada." << endl;

          data[i] = "";
          cout << endl;
      } else {
          istringstream inStream(line);
          ostringstream outStream;

          // This should contain the operation (e.g. "LDA" or "CLA").
          inStream >> segment;
          opCode = getOpCode(segment);

          // If operation is an instruction which doesn't take parameters...
          if(codes[opCode] == "HLT" || codes[opCode] == "NEG" || codes[opCode] == "CLA" || codes[opCode] == "NOP") {
                  outStream << setw(2) << setfill('0') << opCode;
                  outStream << "0000";
                  data[i] = outStream.str();

                  if(!onlyShowErrors) {
                      cout << "  Operación que no necesita parámetros encontrada." << endl;
                      cout << "  " << data[i] << endl << endl;
                  }

            // If operation code is valid...
          } else if(opCode != -1) {
              if(!onlyShowErrors)
                  cout << "  Operación encontrada." << endl;

              // This should contain the addressing type (e.g. "ABS" or "INM").
              inStream >> segment;
              addrType = getAddrType(segment);

              if(addrType != -1) {
                  if(!onlyShowErrors)
                      cout << "  Tipo de direccionamiento encontrado." << endl;


                  // This should contain the parameter value, a three digit number (e.g. "020" or "123").
                  inStream >> segment;

                  if(segment.length() == 3) {

                      // If addressing type is ABS or IND...
                      if(addrType == 1 || addrType == 2) {
                        if(segment[0] == '+'  || segment[0] == '-') {
                        	cout << "  ERROR: El parámetro no puede tener signo para ese tipo de direccionamiento." << endl;
                          return;
                        }
                      }

                      param = segment;

                      outStream << setw(2) << setfill('0') << opCode;
                      outStream << addrType;
                      outStream << param;

                      data[i] = outStream.str();

                      if(!onlyShowErrors) {
                          cout << "  Valor de parámetro encontrado." << endl;
                          cout << "  " << data[i] << endl << endl;
                      }

                  } else {
                      if(onlyShowErrors)
                          cout << "Línea " << setw(3) << setfill('0') << i << ": ";

                      cout << "  ERROR: no se encontró un valor de parámetro válido." << endl;
                      compileSuccess = false;
                      return;
                  }
              } else {
                  if(onlyShowErrors)
                          cout << "Línea " << setw(3) << setfill('0') << i << ": ";

                  cout << "  ERROR: no se encontró un tipo de direccionamiento válido." << endl;
                  compileSuccess = false;
                  return;
              }

          // If operation code is invalid but it's a value (values start with the sign and must be six characters long)...
          } else if( (line[0] == '+' || line[0] == '-') &&  line.length() == 6 ) {
              data[i] = line;

              if(!onlyShowErrors) {
                  cout << "  Valor/dato encontrado." << endl;
                  cout << "  " << data[i] << endl << endl;
              }

          // If operation code is invalid and it's not a value...
          } else {

              if(onlyShowErrors)
                          cout << "Línea " << setw(3) << setfill('0') << i << ": ";

            cout << "  ERROR: no se encontró una operación o valor/dato válido." << endl;
              compileSuccess = false;
              return;
          }
      }
      i++;
  }

  cout << "Lectura de archivo finalizada." << endl;

  if(compileSuccess) {
      cout << "Carga exitosa." << endl;
  } else {
      cout << "No fue posible cargar el contenido del archivo por uno o mas errores.";
  }

  file.close();
}

/*
  Función que vacía la memoria del simulador.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void clearMemory() {
  int option;
  cout << "¿Está seguro de que desea vaciar la memoria?\n  1 Si\n  2 No\n => ";
  cin >> option;
  cout << endl;

  if(option == 1) {
      emptyMemory();
      cout << "Memoria vaciada." << endl;
  } else {
      cout << "No se vació la memoria." << endl;
  }

  // Reset option value.
  option = 3;
}

/*
  Función que muestra el submenú de opciones.
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void showOptions() {
 	// Opciones sobre cómo mostrar la memoria, velocidad de procesamiento, etcétera.
  int option;
  do {
      cout << "Seleccione la opción que desea cambiar (X = activado):" << endl;
      cout << "  1 " << getBoolX(showWholeMemory) << " Mostrar el contenido completo de la memoria" << endl;
      cout << "  2 " << getBoolX(onlyShowErrors)  << " Mostrar sólo los errores al cargar un archivo en memoria" << endl;
    	cout << "  3 " << "[" << secs << "] Intervalo en segundos entre la ejecución de microoperaciones" << endl;
      cout << "  0 Volver al menú principal" << endl;
      cout << " => ";
      cin >> option;
      cout << endl;

      if(option == 1)
        showWholeMemory = !showWholeMemory;
      else if(option == 2)
      	onlyShowErrors = !onlyShowErrors;
      else if(option == 3) {
        cout << endl << "Introduzca la nueva duración del intervalo en segundos: ";
        double secsDouble;
        cin >> secsDouble;
        secs = static_cast<int>(secsDouble);
        cout << endl;
      }

      refreshScreen();

  } while(option != 0);


  // Reset option value.
  option = 6;
}

// Función que completa una palabra al número necesario de caracteres, rellenando con 0.
// Parámetros: un número entero.
// Valor de retorno: string con el AC ajustado.
string completeAC(int iTemp) {

  string str = toString(iTemp);
  ostringstream complete;

  if(str[0] == '-') {
    str = str.substr(1);
    complete << '-';
  } else {
  	complete << '+';
  }

  for(int i = 0; i < 5 - str.length(); i++) {
  	complete << 0;
  }

  complete << str;

  return complete.str();
}

/*
  Funcion que realiza la operacion CLA y pone en 0 el acumulador.
  Parametros: Ninguno.
  Valor de retorno: Ninguno.
*/
void opCLA() {
 	 AC = "+00000";
  displayChanges();
}

/*
  Funcion que realiza la operacion LDA.
  de memoria.
  Parametros: el tipo de direccionamiento y el parametro de la instruccion ([IR]2-0).
  Valor de retorno: ninguno.
*/
void opLDA(string sDireccionamiento, string sExtra) {
  int iDir, iTemp;
  string sContenido;

  switch (sDireccionamiento[0]) {
    // Absoluto
    case '1': {
     	MAR = sExtra;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir];
      displayChanges();
      AC = MDR;
      displayChanges();
      break;
    }
    // Indirecto
    case '2': {
    	MAR = sExtra;
      displayChanges();
    	iDir = atoi(MAR.c_str());
      MDR = data[iDir];
      displayChanges();
      MAR = MDR;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir];
      displayChanges();
      AC = MDR;
      displayChanges();
      break;
    }
    // Inmediato
    case '3': {
    	sContenido = completeAC(atoi(sExtra.c_str())); // Completa el string con 0 y signo respectivamente
      AC = sContenido;
      displayChanges();
      break;
    }
    // Relativo
    case '4': {
     	iTemp = atoi(sExtra.c_str());
      if (PC + iTemp < 0 || PC + iTemp > 999) {
       	cout << "OUT OF BOUNDS" << endl;
      }
      else {
        if (PC + iTemp < 100) {
          if (PC + iTemp < 10) {
            MAR = "00" + toString(PC + iTemp);
          }
          else {
            MAR = "0" + toString(PC + iTemp);
          }
        }
        else {
          MAR = toString(PC + iTemp);
        }
        displayChanges();
        MDR = data[PC + iTemp]; // MMRead
        displayChanges();
        AC = MDR;
        displayChanges();
      }
      break;
    }
    default: {
     	cout << "INSTRUCCION NO VALIDA" << endl;
    }
  }
}

/*
  Funcion que realiza la operacion STA.
  Parametros: el tipo de direccionamiento y el parametro de la instruccion ([IR]2-0).
  Valor de retorno: ninguno.
*/
void opSTA(string sDireccionamiento, string sExtra) {
	int iDir, iTemp;
  string sContenido;

  switch (sDireccionamiento[0]) {
    // Absoluto
    case '1': {
     	MAR = sExtra;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = AC;
      displayChanges();
      data[iDir] = MDR; // MMWrite
      displayChanges();
      break;
    }
    // Indirecto
    case '2': {
    	MAR = sExtra;
      displayChanges();
    	iDir = atoi(MAR.c_str());
      MDR = data[iDir];
      displayChanges();
      MAR = MDR;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = AC;
      displayChanges();
      data[iDir] = MDR; // MMWrite
      displayChanges();
      break;
    }
    // Relativo
    case '4': {
     	iTemp = atoi(sExtra.c_str());
      if (PC + iTemp < 0 || PC + iTemp > 999) {
       	cout << "OUT OF BOUNDS" << endl;
      }
      else {
      	if (PC + iTemp < 100) {
          if (PC + iTemp < 10) {
            MAR = "00" + toString(PC + iTemp);
          }
          else {
            MAR = "0" + toString(PC + iTemp);
          }
        }
        else {
          MAR = toString(PC + iTemp);
        }
        displayChanges();
        MDR = AC;
        displayChanges();
        data[PC + iTemp] = MDR; // MMWrite
        displayChanges();
      }
      break;
    }
    default: {
     	cout << "INSTRUCCION NO VALIDA" << endl;
    }
  }
}

/*
  Funcion que realiza la operacion ADD.
  Parametros: el tipo de direccionamiento y el parametro de la instruccion ([IR]2-0).
  Valor de retorno: ninguno.
*/
void opADD(string sDireccionamiento, string sExtra) {
	int iDir, iTemp, iTemp2;
  string sContenido;

  switch (sDireccionamiento[0]) {
    // Absoluto
    case '1': {
     	MAR = sExtra;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      iTemp = atoi(MDR.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp + iTemp2 > 99999 || iTemp + iTemp2 < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp + iTemp2);
        displayChanges();
      }
      break;
    }
    // Indirecto
    case '2': {
    	MAR = sExtra;
      displayChanges();
    	iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      MAR = MDR;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      iTemp = atoi(MDR.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp + iTemp2 > 99999 || iTemp + iTemp2 < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp + iTemp2);
        displayChanges();
      }
      break;
    }
    // Inmediato
    case '3': {
      iTemp = atoi(sExtra.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp + iTemp2 > 99999 || iTemp + iTemp2 < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp + iTemp2);
        displayChanges();
      }
      break;
    }
    // Relativo
    case '4': {
     	iTemp = atoi(sExtra.c_str());
      if (PC + iTemp < 0 || PC + iTemp > 999) {
       	cout << "OUT OF BOUNDS" << endl;
      }
      else {
        if (PC + iTemp < 100) {
          if (PC + iTemp < 10) {
            MAR = "00" + toString(PC + iTemp);
          }
          else {
            MAR = "0" + toString(PC + iTemp);
          }
        }
        else {
          MAR = toString(PC + iTemp);
        }
        displayChanges();
        MDR = data[PC + iTemp]; // MMRead
        displayChanges();
        iTemp = atoi(MDR.c_str());
        iTemp2 = atoi(AC.c_str());
        AC = completeAC(iTemp + iTemp2);
        displayChanges();
      }
      break;
    }
    default: {
     	cout << "INSTRUCCION NO VALIDA" << endl;
    }
  }
}

/*
  Funcion que realiza la operacion SUB.
  Parametros: el tipo de direccionamiento y el parametro de la instruccion ([IR]2-0).
  Valor de retorno: ninguno.
*/
void opSUB(string sDireccionamiento, string sExtra) {
	int iDir, iTemp, iTemp2;
  string sContenido;

  switch (sDireccionamiento[0]) {
    // Absoluto
    case '1': {
     	MAR = sExtra;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      iTemp = atoi(MDR.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp2 - iTemp > 99999 || iTemp2 - iTemp < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp2 - iTemp);
        displayChanges();
      }
      break;
    }
    // Indirecto
    case '2': {
    	MAR = sExtra;
      displayChanges();
    	iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      MAR = MDR;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      iTemp = atoi(MDR.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp2 - iTemp > 99999 || iTemp2 - iTemp < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp2 - iTemp);
        displayChanges();
      }
      break;
    }
    // Inmediato
    case '3': {
      iTemp = atoi(sExtra.c_str());
      iTemp2 = atoi(AC.c_str());
      if (iTemp2 - iTemp > 99999 || iTemp2 - iTemp < -99999) {
       	cout << "OVERFLOW" << endl;
      }
      else {
        AC = completeAC(iTemp2 - iTemp);
        displayChanges();
      }
      break;
    }
    // Relativo
    case '4': {
     	iTemp = atoi(sExtra.c_str());
      if (PC + iTemp < 0 || PC + iTemp > 999) {
       	cout << "OUT OF BOUNDS" << endl;
      }
      else {
        if (PC + iTemp < 100) {
          if (PC + iTemp < 10) {
            MAR = "00" + toString(PC + iTemp);
          }
          else {
            MAR = "0" + toString(PC + iTemp);
          }
        }
        else {
          MAR = toString(PC + iTemp);
        }
        displayChanges();
        MDR = data[PC + iTemp]; // MMRead
        displayChanges();
        iTemp = atoi(MDR.c_str());
        iTemp2 = atoi(AC.c_str());
        AC = completeAC(iTemp2 - iTemp);
        displayChanges();
      }
      break;
    }
    default: {
     	cout << "INSTRUCCION NO VALIDA" << endl;
    }
  }
}

/*
  Funcion que realiza la operacion NEG.
  Parametros: ninguno.
  Valor de retorno: ninguno.
*/
void opNEG() {
	int iTemp;
  iTemp = atoi(AC.c_str());
  iTemp *= -1;
  AC = completeAC(iTemp);
  displayChanges();
}

/*
  Funcion que realiza la operacion JMP.
  Parametros: ninguno.
  Valor de retorno: ninguno.
*/
void opJMP(string sDireccionamiento, string sExtra) {
	int iDir, iTemp;
  string sContenido;

  switch (sDireccionamiento[0]) {
    // Absoluto
    case '1': {
      PCprev = PC;
     	PC = atoi(sExtra.c_str());
      displayChanges();
      break;
    }
    // Indirecto
    case '2': {
    	MAR = sExtra;
      displayChanges();
    	iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMREad
      displayChanges();
      MAR = MDR;
      displayChanges();
      iDir = atoi(MAR.c_str());
      MDR = data[iDir]; // MMRead
      displayChanges();
      PCprev = PC;
      PC = atoi(MDR.c_str());
      displayChanges();
      break;
    }
    // Relativo
    case '4': {
     	iTemp = atoi(sExtra.c_str());
      if (PC + iTemp < 0 || PC + iTemp > 999) {
       	cout << "OUT OF BOUNDS" << endl;
      }
      else {
        if (PC + iTemp < 100) {
          if (PC + iTemp < 10) {
            MAR = "00" + toString(PC + iTemp);
          }
          else {
            MAR = "0" + toString(PC + iTemp);
          }
        }
        else {
          MAR = toString(PC + iTemp);
        }
        displayChanges();
        PCprev = PC;
        PC = atoi(MAR.c_str());
        displayChanges();
      }
    	break;
		}
    default: {
    	cout << "INPUT ERROR" << endl;
    }
  }
}

/*
  Funcion que ejecuta las instrucciones que se encuentren en la memoria
  Parámetros: ninguno.
  Valor de retorno: ninguno.
*/
void execute() {
  string sOpCode, sAdType, sExtra;
  bool bContinue = true;
  PC = 0;
  PCprev = 0;
  displayChanges();


  while (PC < 1000 && bContinue) {

  	IR = data[PC];

    if (IR != "" && IR[0] != '+' && IR[0] != '-') {
    	sOpCode = IR.substr(0,2);
    	sAdType = IR.substr(2, 1);
      sExtra = IR.substr(3, 3);

      if (sOpCode != "07") {
      	PCprev = PC++;
      }

       // NOP
      if (sOpCode == "00") {
          // JEJE SOY UN NOP e.e
      }
      // CLA
      else if (sOpCode == "01") {
          opCLA();
      }
      // LDA
      else if (sOpCode == "02") {
          opLDA(sAdType, sExtra);
      }
      // STA
      else if (sOpCode == "03") {
          opSTA(sAdType, sExtra);
      }
      // ADD
      else if (sOpCode == "04") {
          opADD(sAdType, sExtra);
      }
      // SUB
      else if (sOpCode == "05") {
          opSUB(sAdType, sExtra);
      }
      // NEG
      else if (sOpCode == "06") {
          opNEG();
      }
      // JMP
      else if (sOpCode == "07") {
      	opJMP(sAdType, sExtra);
      }
      // HLT
      else if (sOpCode == "08") {
        bContinue = false;
        displayChanges();
      }
    }
    else {
     PCprev = PC++;
    }
  }
}

// Función que muestra el menú, lee la opción del usuario y llama la función correspondiente,
// repitiéndose hasta que el usuario desee salir.
// Parámetros: ninguno.
// Valor de retorno: ninguno.
void showMenu() {
  int option;

  do {
    cout << endl << "Seleccione una opcion:" << endl;
    cout << "  1 Ver memoria\n";
    cout << "  2 Editar dirección de memoria (valor)\n";
    cout << "  3 Editar dirección de memoria (ensamblador)\n";
    cout << "  4 Cargar archivo en memoria\n";
    cout << "  5 Vaciar memoria\n";
    cout << "  6 Configuración\n";
    cout << "  7 Ejecutar programa\n";
    cout << "  0 Salir\n";
    cout << " => ";
    cin >> option;

    cout << endl;

    switch (option) {
      case 1: {
        showMemory();
        break;
      }
      case 2: {
        editMemoryDirectly();
        cout << "Presione cualquier tecla para regresar al menu..." << endl;
        cin.ignore();
        cin.get();
        refreshScreen();
        break;
      }
      case 3: {
        editMemory();
        cout << "Presione cualquier tecla para regresar al menu..." << endl;
        cin.ignore();
        cin.get();
        refreshScreen();
        break;
      }
      case 4: {
        loadFile();
        break;
      }
      case 5: {
        clearMemory();
        cout << "Presione cualquier tecla para continuar..." << endl;
        cin.ignore();
        cin.get();
        refreshScreen();
        break;
      }
      case 6: {
        refreshScreen();
        showOptions();
        refreshScreen();
        break;
      }
      case 7: {
       	execute();
        cout << "Ejecución exitosa." << endl;
        WAIT(3 * CONV);
        break;
      }
      case 0: {
        break;
      }
      default: {
        cout << endl << "Por favor seleccione una opción valida." << endl;
        break;
      }
    }
  } while(option != 0);
}

int main() {

    setlocale(LC_CTYPE, "Spanish");
    emptyMemory();

    showMenu();

    return 0;
}
