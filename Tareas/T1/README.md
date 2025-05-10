# Tarea 1 - Bash Scripting

**Estudiante:** Andrés Calderón Guardia

## Iniciar juego

Descomprimir `.tar` ejecutando `tar -xvf t1.tar`.

Luego solo se debe ejecutar el archivo `game.sh`.

## Decisiones de diseño

- Nombramiento de las carpetas y archivos

    Se decidió simplemente usar números desde el 1 hasta el ancho definido por simplicidad ya que serán nombres únicos dentro de cada carpeta, lo mismo con los archivos según la cantidad de archivos por directorio definido.

- Generación del contenido de los archivos

    En particular para el modo **name** se mantuvieron los archivos vacíos.

    Para el resto por simpleza y ahorro de código se utilizaron 100 carácteres aleatorios sacados de `dev/urandom`, excluyendo bytes nulos para evitar prompts de advertencia indeseados en la terminal, ya que 2 modos requerían esta aleatoriedad y los otros 2 era libre mientras tuvieran contenido.

- Método de persistencia

  Toda la información que se quiso compartir entre scripts se guardó en la carpeta `/tmp/treasure` lo cual facilitaría la posterior eliminación de estos archivos temporales al agruparlos en un directorio. Entre los distintos archivos que se guardaron están:

  - `game_state`: contiene tan solo el texto indicando el estado del juego (**name**, **content**, **checksum**, **encrypted** o **signed**).
  - `pass`: estando en modo **encrypted**, almacena la primera passphrase utilizada para encriptar los archivos, con el fin de evitar que se utilice la misma al momento de colocar el tesoro.
  - llaves: se almacenan las llaves públicas y privadas al momento de crear el tablero y colocar el tesoro cuando se está en modo **signed**.
  - `hist`: guarda el historial de intentos a la hora de buscar el tesoro para poder acceder a estos valores rápidamente con las flechas del teclado.

- Elección del tesoro

    Se navegó el tablero de forma aleatoria por cada nivel, eligiendo al azar la siguiente carpeta a la que moverse o el respectivo archivo si es que se hallaba al final del árbol.

- Archivo adicional

    Se creó el archivo `utils.sh` para utilizar funciones que se llamaban en más de un solo archivo.

- Limpieza de la terminal

    Se decidió usar `tput` principalmente con el fin de evitar de llenar la terminal con todo el historial de acciones realizadas, mostrando solo lo relevante en todo momento.

    También se utilizó redirección de salidas para mandar contenido innecesario a `/dev/null` para que no se mostrara en pantalla.

- Limpieza de los archivos

    Se utilizó captura de señales para limpiar todos los archivos del juego al momento de terminar el juego por distintos métodos (**SIGTERM**, **SIGINT** y **EXIT**).

- Libertad del jugador

    Finalmente, se hizo que el jugador pudiera decidir todos los valores que definen el juego como lo son la profundidad, ancho y cantidad de archivos por carpeta del tablero, las passphrases y el modo del juego.
