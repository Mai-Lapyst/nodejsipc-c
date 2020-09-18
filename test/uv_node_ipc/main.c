#define NODEJSIPC_C_IMPL
#include "uv_nodejs_ipc.h"

#define UV_BASE_LOG(lvl, msg, r) \
    if (r == 0) { printf("[c-" lvl "] %s: ok (0)\n", msg); } else { printf("[c-info] %s: %s - %s (%d)\n", msg, uv_err_name(r), uv_strerror(r), r); }
#define UV_LOG_INFO(msg, r) UV_BASE_LOG("info", msg, r)
#define UV_LOG_ERR(msg, r)  UV_BASE_LOG("err", msg, r)

void on_new_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
    printf("[c-info] on_new_connection(%p, %ld, %p)\n", q, nread, buf);

    if (nread < 0) {
        if (nread != UV_EOF) {
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

#include <unistd.h>

int main() {
    int i = 0;
    while (true) {
        char* str = __environ[i];
        if (str == NULL) { break; }
        if (str[0] == 'N' && str[1] == 'O' && str[2] == 'D' && str[3] == 'E') {
            printf("[c-info] environ[%d] = %s\n", i, str);
        }
        i++;
    }

    uv_loop_t* loop = uv_default_loop();

    int r = nodejsipc_start(loop, true, on_new_connection);
    if (r > 0) {
        printf("[c-err] nodejsipc error: %d\n", r);
    }
    if (r < 0) {
        UV_LOG_ERR("nodejsipc got libuv error", r);
    }

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;

}