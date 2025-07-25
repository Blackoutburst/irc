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
    _drawBox(0, 0, w, HEADER_HEIGHT, TB_WHITE, TB_DEFAULT);

    _drawString(2, 2, state->current_channel, TB_CYAN, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 2, 2, state->server_name, TB_GREEN, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 2 + strlen(state->server_name) + 2 , 2, state->server_motd, TB_MAGENTA, TB_DEFAULT);
}

void _uiDrawChat(ChatState* state, I32 w, I32 h) {
    _drawBox(USERNAME_WIDTH - 1, 0, w - USERNAME_WIDTH + 1, h - INPUT_HEIGHT + 1, TB_WHITE, TB_DEFAULT);
    
    I32 msg_area_h = h - INPUT_HEIGHT - HEADER_HEIGHT;
    
    I32 total_lines = 0;
    I32* message_lines = malloc(state->message_count * sizeof(I32));
    
    for (I32 i = 0; i < state->message_count; i++) {
        message_lines[i] = _calculateMessageLines(state, i, w);
        total_lines += message_lines[i];
    }
    
    I32 start_msg = 0;
    
    if (total_lines > msg_area_h) {
        I32 lines_from_end = 0;
        for (I32 i = state->message_count - 1; i >= 0; i--) {
            if (lines_from_end + message_lines[i] > msg_area_h) {
                start_msg = i + 1;
                break;
            }
            lines_from_end += message_lines[i];
        }
    }
    
    I32 current_y = HEADER_HEIGHT;
    for (I32 i = start_msg; i < state->message_count && current_y < h - INPUT_HEIGHT; i++) {
        I32 lines_drawn = __color_user_name_message(state, i, current_y, w);
        current_y += lines_drawn;
    }
    
    free(message_lines);
}

void _uiDrawInput(ChatState* state, I32 w, I32 h) {
    _drawBox(USERNAME_WIDTH - 1, h - INPUT_HEIGHT, w - USERNAME_WIDTH + 1, INPUT_HEIGHT, TB_WHITE, TB_DEFAULT);
    
    _drawString(USERNAME_WIDTH + 2, h - INPUT_HEIGHT + 2, "> ", TB_WHITE, TB_DEFAULT);
    _drawString(USERNAME_WIDTH + 4, h - INPUT_HEIGHT + 2, state->input, TB_WHITE, TB_DEFAULT);
}

void _uiDrawUsers(ChatState* state, I32 w, I32 h) {
    _drawBox(0, 0, USERNAME_WIDTH, h, TB_WHITE, TB_DEFAULT);
    
    for (I32 i = 0; i < state->user_count && i < h - INPUT_HEIGHT - 2; i++) {
        __color_user_name(state, i);
    }
}

void _uiDrawJoint(I32 w, I32 h) {
    tb_set_cell(USERNAME_WIDTH - 1, 0, BOX_VERTICAL_SPLIT_TOP, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, h - 1, BOX_VERTICAL_SPLIT_BOTTOM, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, h - INPUT_HEIGHT, BOX_HORIZONTAL_SPLIT_LEFT, TB_WHITE, TB_DEFAULT);
    tb_set_cell(w - 1, h - INPUT_HEIGHT, BOX_HORIZONTAL_SPLIT_RIGHT, TB_WHITE, TB_DEFAULT);
    tb_set_cell(USERNAME_WIDTH - 1, HEADER_HEIGHT - 1, BOX_MIDDLE, TB_WHITE, TB_DEFAULT);
    tb_set_cell(0, HEADER_HEIGHT - 1, BOX_HORIZONTAL_SPLIT_LEFT, TB_WHITE, TB_DEFAULT);
    tb_set_cell(w - 1, HEADER_HEIGHT - 1, BOX_HORIZONTAL_SPLIT_RIGHT, TB_WHITE, TB_DEFAULT);
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

I32 _calculateMessageLines(ChatState* state, I32 msg_index, I32 w) {
    if (!strncmp(state->messages[msg_index].username, ">>>", strlen(">>>")) || 
        !strncmp(state->messages[msg_index].username, "<<<", strlen("<<<"))) {
        I32 total_len = strlen(state->messages[msg_index].username) + 1 + strlen(state->messages[msg_index].content);
        I32 available_width = w - USERNAME_WIDTH - 4;
        return (total_len + available_width - 1) / available_width;
    } else {
        I32 content_width = w - USERNAME_WIDTH - 5 - strlen(state->messages[msg_index].username);
        if (content_width <= 0) content_width = 1;
        
        I32 content_len = strlen(state->messages[msg_index].content);
        if (content_len == 0) return 1;
        
        return (content_len + content_width - 1) / content_width;
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

I32 __color_user_name_message(ChatState* state, I32 i, I32 y_pos, I32 w) {
    I8 buffer[MAX_MESSAGE_LEN + 5];
    I32 color = __color_by_name(state, state->messages[i].username);
    I32 lines_used = 0;

    if (!strncmp(state->messages[i].username, ">>>", strlen(">>>")) || 
        !strncmp(state->messages[i].username, "<<<", strlen("<<<"))) {
        snprintf(buffer, MAX_MESSAGE_LEN, "%s %s", state->messages[i].username, state->messages[i].content);
        
        I32 available_width = w - USERNAME_WIDTH - 4;
        I32 total_len = strlen(buffer);
        I32 offset = 0;
        
        while (offset < total_len) {
            I32 chunk_len = (total_len - offset > available_width) ? available_width : total_len - offset;
            snprintf(buffer, chunk_len + 1, "%s", buffer + offset);
            _drawString(USERNAME_WIDTH + 2, y_pos + lines_used, buffer, color, TB_DEFAULT);
            offset += chunk_len;
            lines_used++;
        }
    } else {
        _drawString(USERNAME_WIDTH + 2, y_pos, state->messages[i].username, color, TB_DEFAULT);
        
        I32 content_width = w - USERNAME_WIDTH - 5 - strlen(state->messages[i].username);
        if (content_width <= 0) content_width = 1;
        
        I32 content_len = strlen(state->messages[i].content);
        I32 offset = 0;
        
        while (offset < content_len) {
            I32 chunk_len = (content_len - offset > content_width) ? content_width : content_len - offset;
            
            if (lines_used == 0) {
                snprintf(buffer, chunk_len + 3, ": %.*s", chunk_len, state->messages[i].content + offset);
            } else {
                snprintf(buffer, chunk_len + 2, " %.*s", chunk_len, state->messages[i].content + offset);
            }
            
            _drawString(USERNAME_WIDTH + 2 + strlen(state->messages[i].username), y_pos + lines_used, buffer, TB_WHITE, TB_DEFAULT);
            offset += chunk_len;
            lines_used++;
        }
        
        if (content_len == 0) {
            _drawString(USERNAME_WIDTH + 2 + strlen(state->messages[i].username), y_pos, ":", TB_WHITE, TB_DEFAULT);
            lines_used = 1;
        }
    }

    return lines_used;
}

