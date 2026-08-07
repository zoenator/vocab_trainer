# C Vocab Daemon

## What is this?

A lightweight, native Linux language learning assistant. Currently focused on vocabulary, this project is designed to eventually evolve into a fully modular, open-source language learning tool that goes far beyond just learning words.

It consists of two main parts:

1. A **CLI tool** (`vocab`) to manage your entries and check your stats.
2. A **background daemon** (`VocabDaemon`) that runs silently and periodically interrupts you with a pop-up quiz using `zenity`.

Under the hood, it uses a custom spaced-repetition algorithm and stores everything locally in a highly efficient raw binary file. No bloat, no web-frameworks, just pure C and Linux IPC (Named Pipes).

## Prerequisites

To build and run this, you need a Linux environment with:

- `gcc` or `clang`
- `cmake` (>= 3.15)
- `zenity` (for the graphical pop-ups)
- `systemd` (for background service management)

## How to build and install

    # 1. Create build directory and compile
    mkdir build && cd build
    cmake ..

    # 2. Install globally (requires sudo)
    sudo cmake --install .

## Systemd Setup

To enable the daemon to run automatically in the background and survive reboots:

    # 1. Copy the service file to your systemd user directory
    mkdir -p ~/.config/systemd/user/
    cp systemd/vocab-daemon.service ~/.config/systemd/user/

    # 2. Reload systemd and enable the daemon
    systemctl --user daemon-reload
    systemctl --user enable --now vocab-daemon.service

## Usage

Once installed and running, you can manage your database from anywhere in your terminal:

    # Add a new entry to your database
    vocab add

    # Check your current learning progress and statistics
    vocab stats
