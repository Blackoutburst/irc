#pragma once

#include <termbox2.h>

#include "chat.h"

#define USERNAME_WIDTH 20
#define HEADER_HEIGHT 5
#define INPUT_HEIGHT 5

#define BOX_HORIZONTAL L'═'
#define BOX_VERTICAL L'║'
#define BOX_TOP_LEFT L'╔'
#define BOX_TOP_RIGHT L'╗'
#define BOX_BOTTOM_LEFT L'╚'
#define BOX_BOTTOM_RIGHT L'╝'
#define BOX_HORIZONTAL_SPLIT_LEFT L'╠'
#define BOX_HORIZONTAL_SPLIT_RIGHT L'╣'
#define BOX_VERTICAL_SPLIT_TOP L'╦'
#define BOX_VERTICAL_SPLIT_BOTTOM L'╩'
#define BOX_MIDDLE L'╬'

I32 uiInit(void);
void uiUpdate(U8 force);
void uiClean(void);

void _drawBox(I32 x, I32 y, I32 w, I32 h, I32 fg, I32 bg);
void _drawString(I32 x, I32 y, const I8* str, I32 fg, I32 bg);
void _uiDrawHeader(ChatState* state, I32 w, I32 h);
void _uiDrawChat(ChatState* state, I32 w, I32 h);
void _uiDrawUsers(ChatState* state, I32 w, I32 h);
void _uiDrawInput(ChatState* state, I32 w, I32 h);
void _uiDrawJoint(I32 w, I32 h);
void _uiInput(ChatState* state, struct tb_event *ev);

I32 __color_by_name(ChatState* state, const I8* username);
void __color_user_name(ChatState* state, I32 i);
void __color_user_name_message(ChatState* state, I32 i, I32 start_msg);
