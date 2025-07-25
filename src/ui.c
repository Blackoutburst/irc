#include "ui.h"
#include "irc.h"

static struct tb_event ev;

I32 uiInit(void) {
    U32 ret = tb_init();
    if (ret) return 0;

    tb_set_input_mode(TB_INPUT_ESC);
    tb_set_output_mode(TB_OUTPUT_NORMAL);

    return 1;
}

void uiUpdate(U8 force) {
    ChatState* state = chatGetState();
    I32 w = tb_width();
    I32 h = tb_height();
    
    tb_clear();

    _uiDrawHeader(state, w, h);
    _uiDrawChat(state, w, h);
    _uiDrawUsers(state, w, h);
    _uiDrawInput(state, w, h);
    _uiDrawJoint(w, h);

    tb_present();

    if (!force) tb_poll_event(&ev);
    
    _uiInput(state, &ev);
}

void uiClean(void) {
    tb_shutdown();
}

void _drawBox(I32 x, I32 y, I32 w, I32 h, I32 fg, I32 bg) {
    for (I32 i = 1; i < w - 1; i++) {
        tb_set_cell(x + i, y, BOX_HORIZONTAL, fg, bg);
        tb_set_cell(x + i, y + h - 1, BOX_HORIZONTAL, fg, bg);
    }
    
    for (I32 i = 1; i < h - 1; i++) {
        tb_set_cell(x, y + i, BOX_VERTICAL, fg, bg);
        tb_set_cell(x + w - 1, y + i, BOX_VERTICAL, fg, bg);
    }
    
    tb_set_cell(x, y, BOX_TOP_LEFT, fg, bg);
    tb_set_cell(x + w - 1, y, BOX_TOP_RIGHT, fg, bg);
    tb_set_cell(x, y + h - 1, BOX_BOTTOM_LEFT, fg, bg);
    tb_set_cell(x + w - 1, y + h - 1, BOX_BOTTOM_RIGHT, fg, bg);
}

void _drawString(I32 x, I32 y, const I8* str, I32 fg, I32 bg) {
    for (I32 i = 0; str[i]; i++) {
        tb_set_cell(x + i, y, str[i], fg, bg);
    }
}

void _uiDrawHeader(ChatState* state, I32 w, I32 h) {
    _drawBox(0, 0, w - 1, HEADER_HEIGHT, TB_WHITE, TB_DEFAULT);

    _drawString(2, 2, state->current_channel, TB_CYAN, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 2, 2, state->server_name, TB_GREEN, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 2 + strlen(state->server_name) + 2 , 2, state->server_motd, TB_MAGENTA, TB_DEFAULT);
}

void _uiDrawChat(ChatState* state, I32 w, I32 h) {
    _drawBox(USERNAME_WIDTH - 1, 0, w - USERNAME_WIDTH, h - INPUT_HEIGHT + 1, TB_WHITE, TB_DEFAULT);
    
    I32 msg_area_h = h - INPUT_HEIGHT - HEADER_HEIGHT;
    I32 start_msg = state->message_count > msg_area_h ? state->message_count - msg_area_h : 0;
    for (I32 i = start_msg; i < state->message_count && i - start_msg < msg_area_h; i++) {
        __color_user_name_message(state, i, start_msg);
        //snprintf(buffer, MAX_MESSAGE_LEN, "%s: %s", state->messages[i].username, state->messages[i].content);
        //_drawString(USERNAME_WIDTH + 2, i - start_msg + HEADER_HEIGHT, buffer, TB_WHITE, TB_DEFAULT);
    }
}

void _uiDrawInput(ChatState* state, I32 w, I32 h) {
    _drawBox(USERNAME_WIDTH - 1, h - INPUT_HEIGHT, w - USERNAME_WIDTH, INPUT_HEIGHT, TB_WHITE, TB_DEFAULT);
    
    _drawString(USERNAME_WIDTH + 2, h - INPUT_HEIGHT + 2, "> ", TB_WHITE, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 4, h - INPUT_HEIGHT + 2, state->input, TB_WHITE, TB_DEFAULT);
}

void _uiDrawUsers(ChatState* state, I32 w, I32 h) {
    _drawBox(0, 0, USERNAME_WIDTH, h, TB_WHITE, TB_DEFAULT);
    
    for (I32 i = 0; i < state->user_count && i < h - INPUT_HEIGHT - 2; i++) {
        __color_user_name(state, i);
        //_drawString(2, HEADER_HEIGHT + i + 1, state->users[i], TB_WHITE, TB_DEFAULT);
    }
}

void _uiDrawJoint(I32 w, I32 h) {
    tb_set_cell(USERNAME_WIDTH - 1, 0, BOX_VERTICAL_SPLIT_TOP, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, h - 1, BOX_VERTICAL_SPLIT_BOTTOM, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, h - INPUT_HEIGHT, BOX_HORIZONTAL_SPLIT_LEFT, TB_WHITE, TB_DEFAULT);
    tb_set_cell(w - 2, h - INPUT_HEIGHT, BOX_HORIZONTAL_SPLIT_RIGHT, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, HEADER_HEIGHT - 1, BOX_MIDDLE, TB_WHITE, TB_DEFAULT);
}

void _uiInput(ChatState* state, struct tb_event *ev) {
    if (ev->type == TB_EVENT_KEY) {
        if (ev->key == TB_KEY_ESC) {
            tb_shutdown();
            exit(0);
        } else if (ev->key == TB_KEY_ENTER && state->input_len > 0) {
            if (state->message_count < MAX_MESSAGES) {
                if (!ircRunCommand(state, state->input)) {
                    chatAddMessage(state->username, state->input);
                    IRC_PRIVMSG(state->current_channel, state->input);
                }
            }
            state->input[0] = '\0';
            state->input_len = 0;
        } else if (ev->key == TB_KEY_BACKSPACE || ev->key == TB_KEY_BACKSPACE2) {
            if (state->input_len > 0) {
                state->input[--state->input_len] = '\0';
            }
        } else if (ev->ch && state->input_len < MAX_MESSAGE_LEN - 1) {
            state->input[state->input_len++] = ev->ch;
            state->input[state->input_len] = '\0';
        }
    }
}

// |!| ASS SECTION |!|

I32 __color_by_name(ChatState* state, const I8* username) {
    I32 color = TB_WHITE;

    if (!strncmp(username, "IRC", 3) || !strncmp(username, state->server_name, strlen(state->server_name))) color = TB_YELLOW;
    if (!strncmp(username, "Ichika", strlen("Ichika"))) color = TB_RED;
    if (!strncmp(username, "shiroko", strlen("shiroko"))) color = TB_CYAN;
    if (!strncmp(username, "user", strlen("user"))) color = TB_GREEN;
    if (!strncmp(username, "Venice", strlen("Venice"))) color = TB_BLUE;

    if (!strncmp(username, ">>>", strlen(">>>"))) color = TB_GREEN;
    if (!strncmp(username, "<<<", strlen("<<<"))) color = TB_RED;

    return color;
}

void __color_user_name(ChatState* state, I32 i) {
    I32 color = __color_by_name(state, state->users[i]);
    _drawString(2, HEADER_HEIGHT + i, state->users[i], color, TB_DEFAULT);
}

void __color_user_name_message(ChatState* state, I32 i, I32 start_msg) {
    I8 buffer[MAX_MESSAGE_LEN];
    
    I32 color = __color_by_name(state, state->messages[i].username);

    if (!strncmp(state->messages[i].username, ">>>", strlen(">>>")) || !strncmp(state->messages[i].username, "<<<", strlen("<<<"))) {
        snprintf(buffer, MAX_MESSAGE_LEN, "%s %s", state->messages[i].username, state->messages[i].content);
        _drawString(USERNAME_WIDTH + 2, i - start_msg + HEADER_HEIGHT, buffer, color, TB_DEFAULT);
    } else {
        _drawString(USERNAME_WIDTH + 2, i - start_msg + HEADER_HEIGHT, state->messages[i].username, color, TB_DEFAULT);
        
        snprintf(buffer, MAX_MESSAGE_LEN, ": %s", state->messages[i].content);
        _drawString(USERNAME_WIDTH + 2 + strlen(state->messages[i].username), i - start_msg + HEADER_HEIGHT, buffer, TB_WHITE, TB_DEFAULT);
    }
}
