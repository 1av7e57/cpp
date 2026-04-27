
# 🎮 **Laberinto en C++:**

Un juego de laberinto interactivo para terminal Linux, ahora desarrollado en C++. Navega por un laberinto lleno de paredes, encuentra la salida y compite contra ti mismo para resolverlo en el menor número de movimientos.

---

## 📌 **Descripción**

Este programa es un juego de laberinto que puedes jugar directamente en la terminal de tu computadora Linux. El objetivo es salir de un laberinto lleno de paredes (`#`), moviéndote desde una posición inicial hasta la salida (`S`) usando las teclas **W, A, S, D**. Presiona **Q** en cualquier momento para salir.

---

## 🚀 **Características**

- Menú principal con opción de jugar o salir.
- Carga automática del laberinto desde un archivo `laberinto.txt`.
- Movimiento en tiempo real con teclas WASD.
- Indicador de jugador (`J`) y salida (`S`) en pantalla.
- Cálculo automático de la ruta más corta al inicio del juego.
- Contador de movimientos y mensaje de felicitación al completar el nivel.

---

## 🛠️ **Cómo compilar y ejecutar**

1. Descarga los archivos del proyecto en una carpeta de tu equipo.

⚠️ **Importante**: asegúrate que estén descomprimidos antes de usarlos.

2. Asegúrate de tener un compilador de C++ (como `g++`) instalado.
3. Abre una terminal desde la carpeta donde están los archivos.
4. Compila el programa escribiendo en la terminal:

   ```bash
   g++ -o laberinto laberinto.cpp
   ```

5. Ejecútalo escribiendo en la terminal:

   ```bash
   ./laberinto
   ```
⚠️ **Importante**: Asegúrate de tener el archivo `laberinto.txt` en el mismo directorio.

---

## 📄 **Estructura del archivo `laberinto.txt`**

El laberinto debe estar representado con:
- `#` → paredes
- ` ` (espacio) → caminos
- `S` → salida
- El jugador comienza en la esquina superior izquierda.

Ejemplo:
```
########
#      #
# #### #
# #    #
# # ## #
# #    #
# #### #
#S     #
########
```

---

## 🏆 **Objetivo**

Encontrar la salida (`S`) en el **menor número de movimientos posible**. El juego te muestra cuántos pasos mínimos necesitas al inicio — ¡intenta igualarlo superando tu record!

---

## 📜 **Licencia**

Este proyecto está bajo la licencia MIT. Puedes usarlo, modificarlo y distribuirlo libremente.

---

¡Diviértete resolviendo el laberinto en el menor número de pasos! 🧩

---

> ✅ **Nota**: El código ha sido modernizado a C++ para mejorar su estructura y mantenibilidad, pero la experiencia de juego es idéntica a la versión original en C.
