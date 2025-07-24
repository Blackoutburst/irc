#define LOGGER_IMPLEMENTATION
#include "llm.h"
#include "logger.h"

static LLM_MESSAGE** _context = NULL;
static I32 _context_size = 0;
static I8* _model = NULL;

//
// PUBLIC METHODS
//

void llmInit(const I8* model, const I32 context_size, const I8* system_prompt) {
    _context_size = context_size;
    
    _context = malloc(sizeof(LLM_MESSAGE*) * context_size);
    for (I32 i = 0; i < context_size; i++) { 
        _context[i] = malloc(sizeof(LLM_MESSAGE));
        _context[i]->role = NULL;
        _context[i]->content = NULL;
        _context[i]->used = 0;
    }

    I32 model_len = strlen(model);
    _model = malloc(model_len + 1);
    memcpy(_model, model, model_len);
    _model[model_len] = 0;

    _llmFillMessage(_context[0], "system", system_prompt);
}

void llmClean(void) {
    for (I32 i = 0; i < _context_size; i++) {
        LLM_MESSAGE* msg = _context[i];
        free(msg->role);
        free(msg->content);
        free(msg);
    }

    free(_model);
    free(_context);
    _context = NULL;
    _context_size = 0;
}

I8* llmSpeak(const I8* msg) {
    _llmAddMessage("user", msg);
    
    I8* body = _llmFormatInput();

    I8* data = fetcherPost("http://172.20.10.3:1234/v1/chat/completions", body);
    I8* output = _llmGetAnswer(data);

    free(data);
    free(body);

    _llmAddMessage("assistant", output);

    return output;
}

//
// INTERNAL METHODS
//

void _llmAddMessage(const I8* role, const I8* content) {
    I32 full = 1;
    for (I32 i = 0; i < _context_size; i++) {
        if (!_context[i]->used) { 
            full = 0; 
            break;
        }
    }

    if (full) {
        _llmFreeMessage(_context[1]);

        for (int i = 1; i < _context_size - 1; i++) { // Start at 1 to skip system prompt
            _context[i]->role = strdup(_context[i + 1]->role);
            _context[i]->content = strdup(_context[i + 1]->content);
        }

        _llmFreeMessage(_context[_context_size - 1]);
        _llmFillMessage(_context[_context_size - 1], role, content);
    } else {
        for (I32 i = 0; i < _context_size; i++) {
            LLM_MESSAGE* msg = _context[i];
            if (msg->used) continue;

            _llmFillMessage(msg, role, content);
            break;
        }
    }
}

I8* _llmFormatInput(void) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_str(doc, root, "model", _model);

    yyjson_mut_val* messages = yyjson_mut_obj_add_arr(doc, root, "messages");

    for (I32 i = 0; i < _context_size; i++) {
        if (!_context[i]->used) continue;
    
        yyjson_mut_val* message_obj = yyjson_mut_arr_add_obj(doc, messages);
        yyjson_mut_obj_add_str(doc, message_obj, "role", _context[i]->role);
        yyjson_mut_obj_add_str(doc, message_obj, "content", _context[i]->content);
    }
    
    yyjson_mut_val* response_format = yyjson_mut_obj_add_obj(doc, root, "response_format");
    yyjson_mut_obj_add_str(doc, response_format, "type", "text");

    I8* json = yyjson_mut_write(doc, 0, NULL);

    yyjson_mut_doc_free(doc);

    return json;
}

void _llmFillMessage(LLM_MESSAGE* msg, const I8* role, const I8* content) {
    I32 role_len = strlen(role);
    I32 content_len = strlen(content);

    msg->role = malloc(role_len + 1);
    
    memcpy(msg->role, role, role_len);
    msg->role[role_len] = 0;

    msg->content = malloc(content_len + 1);
    
    memcpy(msg->content, content, content_len);
    msg->content[content_len] = 0;

    msg->used = 1;
}

I8* _llmGetAnswer(I8* data) {
    I8* answer = NULL;
    yyjson_doc* doc = yyjson_read((I8*)data, strlen((I8*)data), 0);
    yyjson_val* root = yyjson_doc_get_root(doc);

    yyjson_val* choices = yyjson_obj_get(root, "choices");
    if (!choices) { logW("Invalid JSON, missing property 'choices'"); return NULL; }
    
    yyjson_val* choice = yyjson_arr_get_first(choices);
    if (!choice) { logW("Invalid JSON, 'choices' is empty"); return NULL; }
    
    yyjson_val* message = yyjson_obj_get(choice, "message");
    if (!message) { logW("Invalid JSON, missing property 'message'"); return NULL; }
    
    yyjson_val* content = yyjson_obj_get(message, "content");
    if (!content) { logW("Invalid JSON, missing property 'content'"); return NULL; }

    const I8* content_str = yyjson_get_str(content);
    if (!content_str) { logW("Invalid JSON, 'content' string is NULL"); return NULL; }
    
    I32 length = yyjson_get_len(content);
    if (!length) { logW("Invalid JSON, 'content' length is NULL"); return NULL; }
    
    answer = malloc(length + 1);
    memcpy(answer, content_str, length);
    answer[length] = 0;
    
    yyjson_doc_free(doc);

    return answer;
}

void _llmFreeMessage(LLM_MESSAGE* msg) {
    free(msg->role);
    free(msg->content);
    msg->role = NULL;
    msg->content = NULL;
}
