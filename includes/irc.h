#pragma once

#include <string.h>
#include <stdarg.h>

#include "types.h"
#include "chat.h"
#include "client.h"
#include "strings.h"

#define IRC_LIST() _ircSendPacket("LIST \r\n")
#define IRC_PONG(token) _ircSendPacket("PONG %s\r\n", token)
#define IRC_PRIVMSG(channel, message) _ircSendPacket("PRIVMSG %s :%s\r\n", channel, message)
#define IRC_PART(channel) _ircSendPacket("PART %s\r\n", channel)
#define IRC_JOIN(channel) _ircSendPacket("JOIN %s\r\n", channel)
#define IRC_NICK(nick) _ircSendPacket("NICK %s\r\n", nick)
#define IRC_USER(realname) _ircSendPacket("USER user 0 * :%s\r\n", realname)

I32 ircRunCommand(ChatState* state, const I8* command);
void ircProcessMessage(const I8* message);
I8* ircGetUsername(const I8* message);

void _ircSendPacket(const I8* format, ...);
