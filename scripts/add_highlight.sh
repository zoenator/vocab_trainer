#!/bin/bash

WORD=$(cliphist list | head -n 1 | cliphist decode)

if [ -z "$WORD" ]; then
    notify-send "Vocab Error" "Cliphist is empty"
    exit 1
fi

CODE=$(trans -b -id "$WORD")
TRANSLATION=$(trans -b "$WORD")


#Vocab_cli add "$CODE" "$TRANSLATION" "$WORD" 0 "auto"
notify-send "Vocab saved!" "$WORD -> $TRANSLATION"






