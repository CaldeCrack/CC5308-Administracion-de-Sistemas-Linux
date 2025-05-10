delete_prior_lines() {
  tput cuu $1
  for ((i = 0; i < $1; i++)); do
    tput el
    echo
  done
  tput cuu $1
}

string_equals() {
  if [[ "$1" == "$2" ]]; then
    return 1
  else
    return 0
  fi
}

