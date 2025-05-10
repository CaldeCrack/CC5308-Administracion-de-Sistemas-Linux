#!/bin/bash

source utils.sh

place_treasure() {
  state=$(cat /tmp/treasure/game_state)
  rand=$(ls -Isign_\* | shuf -n 1)
  if [[ -z $1 ]]; then
    path=$rand
  else
    path="$1/$rand"
  fi

  if [[ $rand == "file_"*  ]]; then
    if [[ $state == "name" ]]; then
      key="$path"
    elif [[ $state == "content" ]]; then
      key="$(cat $rand | tr -d '\0')"
    elif [[ $state == "checksum" ]]; then
      key="$(md5sum $rand | cut -d ' ' -f 1)"
    elif [[ $state == "encrypted" ]]; then
      read -p "Ingresa una nueva passphrase: " pass
      delete_prior_lines 1

      while [[ $pass == $(cat /tmp/treasure/pass) ]]; do
        echo "Debes elegir una passphrase distinta a la anterior."
        read -p "Ingresa una nueva passphrase: " pass
        delete_prior_lines 2
      done

      gpg --batch --passphrase "$pass" -c $rand
      mv $rand.gpg $rand
      key=$pass
    elif [[ $state == "signed" ]]; then
      openssl genrsa -out /tmp/treasure/privatekey_treasure.pem 2048
      openssl rsa -in /tmp/treasure/privatekey_treasure.pem -outform PEM -pubout -out /tmp/treasure/publickey_treasure.pem 2> /dev/null
     
      rm sign_$rand
      openssl dgst -sha256 -sign /tmp/treasure/privatekey_treasure.pem -out sign_$rand $rand
      key="$(cat /tmp/treasure/publickey_treasure.pem)"
    fi
  else
    cd $rand
    place_treasure $path
  fi
}

verify() {
  state=$(cat /tmp/treasure/game_state)
  if [[ $state == "name" ]]; then
    return $(string_equals "$1" "$(realpath --relative-to=$(pwd) $2)")
  elif [[ $state == "content" ]]; then
    return $(string_equals "$1" "$(cat $2 | tr -d '\0')")
  elif [[ $state == "checksum" ]]; then
    return $(string_equals "$1" "$(md5sum $2 | cut -d ' ' -f 1)")
  elif [[ $state == "encrypted" ]]; then
    gpg --batch --passphrase "$1" -d "$2" 2> /dev/null > /dev/null
    return $(($? ^ 1))
  elif [[ $state == "signed" ]]; then
    dir=$(dirname "$2")
    file=$(basename "$2")
    sign="$dir/sign_$file"

    openssl dgst -sha256 -verify /tmp/treasure/publickey_treasure.pem -signature "$sign" "$2" 2> /dev/null > /dev/null
    return $(($? ^ 1))
  fi
}

