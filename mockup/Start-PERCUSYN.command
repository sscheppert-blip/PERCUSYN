#!/bin/zsh
cd "${0:A:h}" || exit 1
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
if ! command -v node >/dev/null 2>&1 && [[ -s "$HOME/.nvm/nvm.sh" ]]; then
  source "$HOME/.nvm/nvm.sh"
fi
if ! command -v node >/dev/null 2>&1; then
  print 'Bitte Node.js 20 oder neuer installieren. Danach dieses Skript erneut öffnen.'
  read '?Enter zum Schließen: '
  exit 1
fi
(
  for attempt in {1..40}; do
    if /usr/bin/curl --silent --fail 'http://127.0.0.1:5173' >/dev/null; then
      open 'http://127.0.0.1:5173'
      break
    fi
    sleep 0.1
  done
) &
node server.mjs
