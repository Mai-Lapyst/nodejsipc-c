#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <uv.h>

uv_loop_t* loop;

#define UV_BASE_LOG(lvl, msg, r) \
    if (r == 0) { printf("[c-" lvl "] %s: ok (0)\n", msg); } else { printf("[c-info] %s: %s - %s (%d)\n", msg, uv_err_name(r), uv_strerror(r), r); }
#define UV_LOG_INFO(msg, r) UV_BASE_LOG("info", msg, r)
#define UV_LOG_ERR(msg, r)  UV_BASE_LOG("err", msg, r)

#define BASE_LOG(lvl, msg) printf("[c-" lvl "] %s\n", msg);
#define LOG_INFO(msg) BASE_LOG("info", msg)
#define LOG_ERR(msg) BASE_LOG("err", msg)

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    printf("[c-info] alloc_buffer(%p, %ld, %p)\n", handle, suggested_size, buf);
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void on_new_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
    printf("[c-info] on_new_connection(%p, %ld, %p)\n", q, nread, buf);

    if (nread < 0) {
        if (nread != UV_EOF) {
            //fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            UV_LOG_ERR("Error while reading", (int)nread);
        }
        uv_close((uv_handle_t*) q, NULL);
        return;
    }

    printf("[c-info] buf.len = %ld\n", buf->len);
    printf("[c-info] buf.base = '%s'\n", buf->base);

    // echo the data back
    uv_write_t* req = (uv_write_t*) malloc(sizeof(uv_write_t));
    uv_write(req, q, buf, 1, NULL);
}

int main(int argc, char* argv[]) {
    LOG_INFO("starting c-app...");

    LOG_INFO("arguments: ");
    for (int i = 0; i < argc; i++) {
        printf("[c-info] argv[%d] = %s\n", i, argv[i]);
    }

    LOG_INFO("environment: ");
    int i = 0;
    while (true) {
        char* str = __environ[i];
        if (str == NULL) { break; }
        if (str[0] == 'N' && str[1] == 'O' && str[2] == 'D' && str[3] == 'E') {
            printf("[c-info] environ[%d] = %s\n", i, str);
        }
        i++;
    }

    //uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    //uv_loop_init(loop);
    loop = uv_default_loop();
    LOG_INFO("loop sucessfully created");

    // constructor (?)
    uv_pipe_t queue;
    int r = uv_pipe_init(loop, &queue, 1);
    UV_LOG_INFO("uv_pipe_init", r);
    if (r != 0) { return r; }
    printf("[c-info] queue = %p\n", (void*)&queue);

    r = uv_pipe_open(&queue, 3);
    UV_LOG_INFO("uv_pipe_open", r);

    r = uv_read_start((uv_stream_t*)&queue, alloc_buffer, on_new_connection);
    UV_LOG_INFO("uv_read_start", r);

    uv_run(loop, UV_RUN_DEFAULT);

    LOG_INFO("quitting...");
    return 0;
}