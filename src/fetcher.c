#include "fetcher.h"
#include "logger.h"

static CURL* curl;

I32 fetcherInit(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (!curl) {
        logE("Failed to initialize CURL");
        return 0;
    }
    
    return 1;
}

void fetcherClean(void) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

ANY WriteMemoryCallback(void* contents, ANY size, ANY nmemb, void* userp) {
    ANY realsize = size * nmemb;
    HTTP_RES* mem = (HTTP_RES*)userp;

    I8* ptr = realloc(mem->content, mem->size + realsize + 1);
    if (!ptr) { logW("Not enough memory"); return 0; }

    mem->content = ptr;
    memcpy(&(mem->content[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->content[mem->size] = 0;

    return realsize;
}


I8* fetcherPost(const I8* url, const I8* body) {
    HTTP_RES chunk;
    chunk.content = malloc(1);
    chunk.size = 0;
    
    CURLcode res;

    
    curl_easy_setopt(curl, CURLOPT_URL, url);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) { logW("POST failed"); return NULL; }

    curl_slist_free_all(headers);

    return chunk.content;
}
