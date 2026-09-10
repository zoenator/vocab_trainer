#!/bin/bash

# Try Wayland Primary Selection
WORD=$(wl-paste -p 2>/dev/null)

# Fallback for apps lacking Primary Selection support
if [ -z "$WORD" ] || [ "$WORD" = "Nothing is copied" ]; then
    OLD_CLIP=$(wl-paste 2>/dev/null)
    wl-copy -c
    
    wtype -m logo -m ctrl -m alt -m shift 2>/dev/null
    sleep 0.4
    wtype -M ctrl -k c -m ctrl
    
    for i in {1..10}; do
        WORD=$(wl-paste 2>/dev/null)
        [ -n "$WORD" ] && break
        sleep 0.1
    done
    
    if [ -z "$WORD" ]; then
        echo -n "$OLD_CLIP" | wl-copy
        notify-send "Vocab Error" "Copy failed. Release Super key faster/ Highlight something."
        exit 1
    fi
    
    echo -n "$OLD_CLIP" | wl-copy
fi

# Trim whitespace
WORD=$(echo "$WORD" | xargs)

if [ -z "$WORD" ]; then
    notify-send "Vocab Error" "No text selected!"
    exit 1
fi

CODE=$(trans -b -id "$WORD")
TRANSLATION=$(trans -b "$WORD")

if [ -z "$TRANSLATION" ]; then
    notify-send "Vocab Error" "Translation for '$WORD' failed."
    exit 1
fi

Vocab_cli add "$CODE" "$TRANSLATION" "$WORD" 0 "auto"

if [ $? -eq 0 ]; then
    notify-send "Vocab saved!" "$WORD -> $TRANSLATION"
else
    notify-send "Vocab Error" "Failed to save to database."
fi

