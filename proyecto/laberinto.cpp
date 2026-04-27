// ============================================================================
// LIBRERÍAS - Inclusiones de bibliotecas estándar y del sistema
// ============================================================================

#include <cctype>        // Librería: Funciones de caracteres (tolower)
#include <fstream>       // Librería: Manejo de archivos (std::ifstream)
#include <iostream>      // Librería: Entrada/salida estándar (std::cout, std::cerr)
#include <queue>         // Librería: Cola FIFO (std::queue)
#include <termios.h>     // Librería: Control de terminal UNIX
#include <unistd.h>      // Librería: Funciones del sistema UNIX (read, STDIN_FILENO)
#include <vector>        // Librería: Contenedor dinámico (std::vector)

// ============================================================================
// CONSTANTES - Definiciones de macros para dimensiones del laberinto
// ============================================================================
#define FILAS 17         // Constante: Número de filas del laberinto
#define COLUMNAS 32      // Constante: Número de columnas del laberinto

// ============================================================================
// CLASE: TerminalManager - Gestiona la configuración de la terminal
// ============================================================================
class TerminalManager {
private:
    // ATRIBUTOS - Estructuras para guardar configuración de terminal
    struct termios oldt, newt;  // Atributo: Configuraciones de terminal antigua y nueva

public:
    // MÉTODO: Configura la terminal para lectura sin eco y sin esperar Enter
    void configurar() {
        tcgetattr(STDIN_FILENO, &oldt);              // Obtiene configuración actual de terminal
        newt = oldt;                                  // Copia la configuración antigua
        newt.c_lflag &= ~(ICANON | ECHO);            // Desactiva modo canónico y eco
        newt.c_cc[VMIN] = 1;                          // Mínimo de caracteres a leer: 1
        newt.c_cc[VTIME] = 0;                         // Tiempo de espera: 0 (sin espera)
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);    // Aplica nueva configuración inmediatamente
    }

    // MÉTODO: Restaura la configuración original de la terminal
    void restaurar() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);    // Restaura configuración antigua
    }

    // MÉTODO: Lee una tecla presionada sin esperar Enter
    char obtenerTecla() {
        char ch;                                      // Variable local: Almacena caractér leído
        read(STDIN_FILENO, &ch, 1);                  // Lee 1 caractér desde entrada estándar
        return ch;                                    // Retorna el caractér leído
    }

    // MÉTODO: Espera a que el usuario presione Enter
    void esperarEnter() {
        char ch;                                      // Variable local: Almacena caractér leído
        do {
            read(STDIN_FILENO, &ch, 1);              // Lee caractér
        } while (ch != '\n');                         // Repite hasta que sea Enter (\n)
    }
};

// ============================================================================
// CLASE: Laberinto - Gestiona el laberinto, movimientos y búsqueda de salida
// ============================================================================
class Laberinto {
private:
    // ATRIBUTOS - Datos del laberinto
    char laberinto[FILAS][COLUMNAS];  // Atributo: Matriz 2D que representa el laberinto
    int x, y;                          // Atributos: Posición actual del jugador (fila, columna)
    int movimientos;                   // Atributo: Contador de movimientos realizados
    int salidaX, salidaY;              // Atributos: Posición de la salida (fila, columna)

    // ESTRUCTURA ANIDADA - Nodo para algoritmo BFS
    struct Nodo {
        int x, y, dist;                // Estructura: Posición (x,y) y distancia desde inicio
    };

public:
    // CONSTRUCTOR - Inicializa el laberinto y posición del jugador
    Laberinto() : x(1), y(1), movimientos(0), salidaX(0), salidaY(0) {
        // Inicializa lista de miembros: x=1, y=1, movimientos=0, salidaX=0, salidaY=0
        for (int i = 0; i < FILAS; ++i) {           // Bucle: Itera sobre filas
            for (int j = 0; j < COLUMNAS; ++j) {    // Bucle: Itera sobre columnas
                laberinto[i][j] = ' ';              // Inicializa cada celda con espacio
            }
        }
    }

    // MÉTODO: Carga el laberinto desde un archivo de texto
    bool cargar(const std::string& nombreArchivo) {
        std::ifstream archivo(nombreArchivo);       // Abre archivo para lectura
        if (!archivo.is_open()) {                   // Condicional: Verifica si se abrió correctamente
            std::cerr << "Error: " << std::endl;
            std::cerr << "No se pudo abrir el archivo " << nombreArchivo << std::endl;
            return false;                            // Retorna false si no se pudo abrir
        }

        // Validación: Verifica que el archivo tenga exactamente FILAS líneas
        int lineasTotales = 0;                       // Variable local: Contador de líneas
        std::string linea;                           // Variable local: Almacena cada línea leída
        while (std::getline(archivo, linea)) {       // Bucle: Lee cada línea del archivo
            lineasTotales++;                         // Incrementa contador
        }

        if (lineasTotales != FILAS) {                // Condicional: Verifica cantidad de líneas
            std::cerr << "Error: " << std::endl;
            std::cerr << "El archivo debe tener exactamente " << FILAS  
                      << " líneas. Tiene: " << lineasTotales << std::endl;
            return false;                            // Retorna false si cantidad es incorrecta
        }

        // Reinicia el archivo para leerlo desde el principio
        archivo.clear();                             // Limpia flags de error
        archivo.seekg(0);                            // Posiciona al inicio del archivo

        // Lectura: Carga el contenido del archivo en la matriz laberinto
        for (int i = 0; i < FILAS; ++i) {           // Bucle: Itera sobre filas
            if (!std::getline(archivo, linea)) {    // Condicional: Intenta leer línea
                std::cerr << "Error: " << std::endl;
                std::cerr << "No se pudo leer la línea " << i + 1 << std::endl;
                return false;                        // Retorna false si no se puede leer
            }

            // Limpieza: Elimina caracteres de salto de línea (\n y \r)
            if (!linea.empty() && linea.back() == '\n') linea.pop_back();
            if (!linea.empty() && linea.back() == '\r') linea.pop_back();

            // Validación: Verifica que cada línea tenga exactamente COLUMNAS caracteres
            if ((int)linea.length() != COLUMNAS) {  // Condicional: Verifica longitud
                std::cerr << "Error: " << std::endl;
                std::cerr << "Línea " << i + 1 << " debe tener exactamente " 
                          << COLUMNAS << " caracteres. Tiene: " << linea.length() << std::endl;
                return false;                        // Retorna false si longitud es incorrecta
            }

            // Asignación: Copia cada caractér de la línea a la matriz
            for (int j = 0; j < COLUMNAS; ++j) {    // Bucle: Itera sobre columnas
                laberinto[i][j] = linea[j];         // Asigna caractér a posición [i][j]
            }
        }

        return true;                                 // Retorna true si carga fue exitosa
    }

    // MÉTODO: Calcula el mínimo número de movimientos para llegar a la salida (BFS)
    int calcularMinimosMovimientos() {
        std::queue<Nodo> cola;                       // Contenedor: Cola para BFS
        std::vector<std::vector<bool>> visitado(FILAS, std::vector<bool>(COLUMNAS, false));
        
        // Atributo local: Matriz booleana para marcar celdas visitadas
        cola.push({1, 1, 0});                        // Inicializa cola con posición inicial (1,1) y distancia 0
        visitado[1][1] = true;                       // Marca posición inicial como visitada

        // Direcciones: Arriba, Abajo, Izquierda, Derecha
        int dx[] = {-1, 1, 0, 0};                    // Array: Cambios en fila para 4 direcciones
        int dy[] = {0, 0, -1, 1};                    // Array: Cambios en columna para 4 direcciones

        while (!cola.empty()) {                      // Bucle: Continúa mientras haya nodos en cola
            Nodo actual = cola.front(); cola.pop();  // Extrae primer nodo de la cola
            int x = actual.x, y = actual.y, dist = actual.dist;  // Desempaqueta datos del nodo

            // Condicional: Verifica si se encontró la salida
            if (laberinto[x][y] == 'S') {            // Si celda actual es 'S' (salida)
                salidaX = x;                         // Guarda coordenada X de salida
                salidaY = y;                         // Guarda coordenada Y de salida
                return dist;                         // Retorna distancia mínima
            }

            // Exploración: Explora los 4 vecinos (arriba, abajo, izquierda, derecha)
            for (int i = 0; i < 4; ++i) {            // Bucle: Itera sobre 4 direcciones
                int nx = x + dx[i], ny = y + dy[i];  // Calcula nueva posición
                // Condicional: Verifica si nueva posición es válida y no visitada
                if (nx >= 0 && nx < FILAS && ny >= 0 && ny < COLUMNAS && !visitado[nx][ny] && laberinto[nx][ny] != '#') {
                    visitado[nx][ny] = true;         // Marca nueva posición como visitada
                    cola.push({nx, ny, dist + 1});   // Agrega nuevo nodo a la cola con distancia incrementada
                }
            }
        }

        return -1;                                   // Retorna -1 si no se encuentra salida
    }

    // MÉTODO: Muestra el laberinto en consola con la posición del jugador
    void mostrar() const {
        std::cout << "\033[2J\033[H";                // Código ANSI: Limpia pantalla y posiciona cursor en inicio
        for (int i = 0; i < FILAS; ++i) {           // Bucle: Itera sobre filas
            for (int j = 0; j < COLUMNAS; ++j) {    // Bucle: Itera sobre columnas
                if (i == x && j == y) std::cout << "J ";  // Si es posición del jugador, muestra 'J'
                else std::cout << laberinto[i][j] << " ";  // Si no, muestra caractér del laberinto
            }
            std::cout << std::endl;                  // Salto de línea al final de cada fila
        }
    }

    // MÉTODO: Procesa el movimiento del jugador según la entrada
    bool mover(char input) {
        int movido = 0;                              // Variable local: Flag para verificar si se movió
        switch (tolower(input)) {                    // Switch: Evalúa entrada en minúsculas
            case 'w':                                // Caso: Movimiento hacia arriba
                if (x > 0 && laberinto[x-1][y] != '#') {  // Condicional: Verifica límite y obstáculo
                    x--;                             // Decrementa fila (sube)
                    movido = 1;                      // Marca como movido
                }
                break;                               // Fin del caso

            case 's':                                // Caso: Movimiento hacia abajo
                if (x < FILAS - 1 && laberinto[x+1][y] != '#') {  // Condicional: Verifica límite y obstáculo
                    x++;                             // Incrementa fila (baja)
                    movido = 1;                      // Marca como movido
                }
                break;                               // Fin del caso

            case 'a':                                // Caso: Movimiento hacia izquierda
                if (y > 0 && laberinto[x][y-1] != '#') {  // Condicional: Verifica límite y obstáculo
                    y--;                             // Decrementa columna (izquierda)
                    movido = 1;                      // Marca como movido
                }
                break;                               // Fin del caso

            case 'd':                                // Caso: Movimiento hacia derecha
                if (y < COLUMNAS - 1 && laberinto[x][y+1] != '#') {  // Condicional: Verifica límite y obstáculo
                    y++;                             // Incrementa columna (derecha)
                    movido = 1;                      // Marca como movido
                }
                break;                               // Fin del caso

            case 'q':                                // Caso: Salir del juego
                return false;                        // Retorna false para terminar
        }

        if (movido) movimientos++;                   // Condicional: Incrementa contador si se movió
        return true;                                 // Retorna true para continuar
    }

    // MÉTODO: Verifica si el jugador está en la salida
    bool estaEnSalida() const {
        return laberinto[x][y] == 'S';               // Retorna true si celda actual es 'S'
    }

    // MÉTODO GETTER: Retorna cantidad de movimientos realizados
    int getMovimientos() const { return movimientos; }

    // MÉTODO GETTER: Retorna posición X (fila) del jugador
    int getX() const { return x; }

    // MÉTODO GETTER: Retorna posición Y (columna) del jugador
    int getY() const { return y; }

    // MÉTODO: Reinicia la posición del jugador y contador de movimientos
    void reiniciarJugador() {
        x = 1;                                       // Resetea fila a 1
        y = 1;                                       // Resetea columna a 1
        movimientos = 0;                             // Resetea contador de movimientos a 0
    }
};

// ============================================================================
// CLASE: Presentador - Gestiona la interfaz de usuario y mensajes
// ============================================================================
class Presentador {
private:
    // ATRIBUTO - Referencia al gestor de terminal
    TerminalManager& terminal;                       // Atributo: Referencia a TerminalManager

public:
    // CONSTRUCTOR - Inicializa el presentador con referencia a terminal
    Presentador(TerminalManager& term) : terminal(term) {}
    // Constructor con lista de inicialización: terminal se inicializa con 'term'

    // MÉTODO: Muestra el menú principal del juego
    void mostrarMenuInicio() {
        std::cout << "\033[2J\033[H";                // Código ANSI: Limpia pantalla
        std::cout << "╔══════════════════════════════════════╗\n";  // Dibuja borde superior
        std::cout << "║            LABERINTO                 ║\n";  // Título del menú
        std::cout << "╠══════════════════════════════════════╣\n";  // Separador
        std::cout << "║ 1. Jugar                             ║\n";  // Opción 1
        std::cout << "║ 2. Salir                             ║\n";  // Opción 2
        std::cout << "╚══════════════════════════════════════╝\n";  // Borde inferior
        std::cout << "Ingresa tu opción (1/2): ";              // Prompt para entrada
        std::cout.flush();                           // Fuerza salida del buffer
    }

    // MÉTODO: Muestra mensaje de bienvenida con información del juego
    void mostrarBienvenida(int minMovimientos) {
        std::cout << "\033[2J\033[H";                // Código ANSI: Limpia pantalla
        std::cout << "¡Bienvenido al Juego de Laberinto!\n";  // Mensaje de bienvenida
        std::cout << "Tu eres el jugador: J\n";       // Explica símbolo del jugador
        std::cout << "Encuentra la salida: S\n";      // Explica símbolo de salida
        std::cout << "Intenta salir con: " << minMovimientos << " movimientos\n";  // Muestra mínimo de movimientos
        std::cout << "Presiona ENTER para comenzar...\n";  // Instrucción para continuar
        std::cout.flush();                           // Fuerza salida del buffer
        terminal.esperarEnter();                     // Espera a que presione Enter
    }

    // MÉTODO: Muestra mensaje de finalización con estadísticas
    void mostrarFin(int movimientos) {
        std::cout << "\n¡Felicidades! Has completado el laberinto.\n";  // Mensaje de éxito
        std::cout << "Movimientos totales: " << movimientos << std::endl;  // Muestra cantidad de movimientos
        std::cout << "Presiona ENTER para volver al menú...\n";  // Instrucción para continuar
        std::cout.flush();                           // Fuerza salida del buffer
        terminal.esperarEnter();                     // Espera a que presione Enter
    }

    // MÉTODO: Muestra prompt pidiendo movimiento del jugador
    void mostrarPromptMovimiento() {
        std::cout << "Ingresa tu movimiento (W/A/S/D) o Q para regresar al Menú: ";  // Prompt
        std::cout.flush();                           // Fuerza salida del buffer
    }

    // MÉTODO: Muestra un mensaje de error
    void mostrarError(const std::string& mensaje) {
        std::cout << mensaje << "\n";                // Muestra mensaje de error
        std::cout << "Presiona ENTER para continuar...\n";  // Instrucción para continuar
        std::cout.flush();                           // Fuerza salida del buffer
        terminal.esperarEnter();                     // Espera a que presione Enter
    }
};

// ============================================================================
// CLASE: Juego - Controlador principal que coordina todo el juego
// ============================================================================
class Juego {
private:
    // ATRIBUTOS - Componentes principales del juego
    TerminalManager terminal;                        // Atributo: Gestor de terminal
    Laberinto laberinto;                             // Atributo: Objeto laberinto
    Presentador presentador;                         // Atributo: Gestor de presentación

public:
    // CONSTRUCTOR - Inicializa componentes del juego
    Juego() : terminal(), laberinto(), presentador(terminal) {}
    // Constructor con lista de inicialización: inicializa terminal, laberinto y presentador

    // MÉTODO: Inicia el flujo principal del juego
    void iniciar() {
        terminal.configurar();                       // Configura terminal para lectura sin eco

        while (true) {                               // Bucle infinito: Menú principal
            presentador.mostrarMenuInicio();         // Muestra menú
            char opcion = terminal.obtenerTecla();   // Lee opción del usuario

            // Validación: Verifica que opción sea 1 o 2
            if (opcion != '1' && opcion != '2') {    // Condicional: Opción inválida
                presentador.mostrarError("Opción inválida.");  // Muestra error
                continue;                            // Vuelve al inicio del bucle
            }

            // Condicional: Usuario eligió salir
            if (opcion == '2') {                     // Si opción es 2
                std::cout << "Saliendo...\n";        // Mensaje de despedida
                std::cout << "Presiona ENTER para cerrar el programa.\n";  // Instrucción
                std::cout.flush();                   // Fuerza salida del buffer
                terminal.obtenerTecla();             // Espera a que presione una tecla
                terminal.restaurar();                // Restaura configuración de terminal
                return;                              // Termina el programa
            }

            // Carga del laberinto
            if (!laberinto.cargar("laberinto.txt")) {  // Intenta cargar archivo
                presentador.mostrarError("Verifique el archivo antes de continuar.");  // Error si falla
                continue;                            // Vuelve al menú
            }

            // Cálculo de movimientos mínimos
            int minMovimientos = laberinto.calcularMinimosMovimientos();  // Calcula mínimo con BFS

            // Muestra bienvenida
            presentador.mostrarBienvenida(minMovimientos);  // Muestra pantalla de bienvenida

            // Reinicia posición del jugador
            laberinto.reiniciarJugador();            // Pone jugador en posición inicial

            // Bucle de juego: Continúa hasta que jugador llegue a salida o presione Q
            while (true) {                           // Bucle: Juego en progreso
                laberinto.mostrar();                 // Dibuja laberinto con jugador

                // Condicional: Verifica si jugador llegó a la salida
                if (laberinto.estaEnSalida()) {      // Si está en salida
                    presentador.mostrarFin(laberinto.getMovimientos());  // Muestra pantalla de fin
                    break;                           // Sale del bucle de juego
                }

                // Entrada del usuario
                presentador.mostrarPromptMovimiento();  // Muestra prompt
                char input = terminal.obtenerTecla();   // Lee tecla presionada
                input = tolower(input);              // Convierte a minúscula

                // Procesa movimiento
                laberinto.mover(input);              // Ejecuta movimiento

                // Condicional: Verifica si usuario presionó Q
                if (input == 'q') {                  // Si presionó Q
                    std::cout << "\nRegresando al menú...\n";  // Mensaje
                    break;                           // Sale del bucle de juego
                }
            }
        }

        terminal.restaurar();                        // Restaura configuración de terminal
    }
};

// ============================================================================
// FUNCIÓN: main - Punto de entrada del programa
// ============================================================================
int main() {
    Juego juego;                                     // Objeto: Crea instancia de Juego
    juego.iniciar();                                 // Método: Inicia el juego
    return 0;                                        // Retorna 0 (éxito)
}