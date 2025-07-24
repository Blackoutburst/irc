cc \
src/*.c \
-Iincludes -L/opt/homebrew/lib -lcurl -pthread -Wno-deprecated-declarations -W -Wall -Wextra -Wno-unused-parameter -g3 -fno-omit-frame-pointer -fsanitize=address -o irc

#FAILED (remote: 'No such file or directory')
