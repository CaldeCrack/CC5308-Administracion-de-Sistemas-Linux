#!/bin/bash

source utils.sh

create_board_rec() {
  if (( $1 > 0 )); then
    for ((i = 1; i <= $2; i++)); do
      mkdir "dir_$i"
    done
    
    for dir in ./**/; do
      cd $dir
      create_board_rec $(($1 - 1)) $2 $3 $4 $5
      cd ..
    done
  else
    for ((k = 1; k <= $3; k++)); do
      touch "file_$k"
    done
    fill_board $4 $5
  fi
}

create_board() {
  mkdir tablero
  cd tablero

  mkdir /tmp/treasure
  touch /tmp/treasure/game_state
  echo $4 >> /tmp/treasure/game_state

  pass=""
  if [[ $4 == "encrypted" ]]; then
    read -p "Ingresa una passphrase: " pass
    touch /tmp/treasure/pass
    echo "$pass" > /tmp/treasure/pass
    delete_prior_lines 1
  fi

  if [[ $4 == "signed" ]]; then
    openssl genrsa -out /tmp/treasure/privatekey.pem 2048
    openssl rsa -in /tmp/treasure/privatekey.pem -outform PEM -pubout -out /tmp/treasure/publickey.pem 2> /dev/null
  fi

  create_board_rec $1 $2 $3 $4 $pass
}

clean_board() {
  rm -rf ./tablero
  rm -rf /tmp/treasure
}

fill_board() {
  if [[ $1 != "name" ]]; then 
    for file in *; do
      head -c 100 /dev/urandom >> $file
    done

    if [[ $1 == "encrypted" ]]; then
      for file in *; do
        gpg --batch --passphrase "$2" -c $file
        mv $file.gpg $file
      done
    elif [[ $1 == "signed" ]]; then
      for file in *; do
        openssl dgst -sha256 -sign /tmp/treasure/privatekey.pem -out sign_$file $file
      done
    fi
  fi
}

