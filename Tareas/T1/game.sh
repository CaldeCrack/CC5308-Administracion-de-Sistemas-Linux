#!/bin/bash

source utils.sh
source board.sh
source controller.sh

base_dir=$(pwd)

cleanup() {
  cd $base_dir
  clean_board
  exit 1
}

trap cleanup SIGTERM SIGINT EXIT

options=("name" "content" "checksum" "encrypted" "signed")
for i in "${!options[@]}"; do
    echo "[$((i + 1))] ${options[i]}"
done

read -p "Elige un modo (1-5): " state_num
delete_prior_lines 1
while [[ ! $state_num =~ ^[1-5]$ ]]; do
  echo "Elección inválida. Elegir un número entre 1 y 5."
  read -p "Elige un modo (1-5): " state_num
  delete_prior_lines 2
done
state="${options[$((state_num - 1))]}"
delete_prior_lines 5

read -p "Ingrese una profundidad: " depth
delete_prior_lines 1

read -p "Ingrese un ancho: " width
delete_prior_lines 1

read -p "Ingrese una cantidad de archivos: " files
delete_prior_lines 1

create_board $depth $width $files $state 
place_treasure "" # this defines key
cd $base_dir/tablero

echo "¡El tesoro ha sido enterrado!"

HISTFILE=/tmp/treasure/hist
HISTSIZE=1000
HISTFILESIZE=1000
touch "$HISTFILE"
history -r

while true; do
  read -e -r -p "Ingresa una ruta: " input
  history -s "$input"
  delete_prior_lines 1

  verify "$key" "$input"
  if [ $? -eq 1 ]; then
    break
  fi
done

delete_prior_lines 1
echo "¡Felicidades! Hallaste el tesoro ($input)"

