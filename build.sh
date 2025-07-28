cc \
src/*.c \
-Iincludes -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib  -L/opt/homebrew/lib -lssl -lcrypto -lcurl -pthread -Wno-deprecated-declarations -W -Wall -Wextra -Wno-unused-parameter -g3 -fno-omit-frame-pointer -fsanitize=address -o irc
