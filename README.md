# IRC
Basic IRL client (WIP)

## Build
For now only the macos build was done, simply run
```
./build.sh
```
you may need to install dependency such as openssl

## Usage
The client uses TSL through OpenSSL, if the server doesn't support TLS it will not work
```
./irc <ip> <port> <nickname> <realname>
```

## Commands
Once logged you can
- !list | That allow you to see available channels
- !join <channel> | Makes you join a channel

## Misc
TUI was made using the amazing [termbox2](https://github.com/termbox/termbox2)
