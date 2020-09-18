/**
 * uv_nodejs_ipc.h - a single header include to simply setup a ipc connection with node
 * Dependencys: libuv
 * 
 * Usage:
 * c_nodeipc_start(...)     initializes the connection with node
 * c_nodeipc_stop()         stops the connection
 */

#ifndef __NODEJS_IPC__
#define __NODEJS_IPC__

#include <uv.h>
#include <stdlib.h>
#include <stdbool.h>

int nodejsipc_start(uv_loop_t* loop, bool prevent_loopexit, uv_read_cb read_cb);
int nodejsipc_stop();
char* nodejsipc_channel_serialization();
bool nodejsipc_active();

#ifdef NODEJSIPC_C_IMPL

    #ifndef NODEJSIPC_C_ALLOC
        #define NODEJSIPC_C_ALLOC nodejsipc_alloc_buffer

        void nodejsipc_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            buf->base = (char*) malloc(suggested_size);
            buf->len = suggested_size;
        }
    #endif

    uv_pipe_t* nodejsipc_pipe = NULL;

    /**
     * Starts a ipc connection with the parent nodejs process
     * 
     * Parameters:
     *  loop                    The libuv loop the connection is living on
     *  prevent_loopexit        If this is true, the connection can prevent the loop from exit
     *  read_cb                 read-callback for uv_read_start, will get called if the connection has data available
     * 
     * Returns:
     *  0   ok
     *  <0  libuv error
     *  1   connection already present
     *  2   could not find NODE_CHANNEL_FD in the environment, maybe the executable was not spawned by nodejs?
     */
    int nodejsipc_start(uv_loop_t* loop, bool prevent_loopexit, uv_read_cb read_cb) {
        if (nodejsipc_pipe != NULL) { return 1; }

        nodejsipc_pipe = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));

        int r = uv_pipe_init(loop, nodejsipc_pipe, 1);
        if (r != 0) { return r; }

        if (!prevent_loopexit) {
            uv_unref((uv_handle_t*)nodejsipc_pipe);
        }

        char* env_node_fd = getenv("NODE_CHANNEL_FD");
        if (env_node_fd == NULL) {
            uv_close((uv_handle_t*)nodejsipc_pipe, NULL);
            free(nodejsipc_pipe);
            nodejsipc_pipe = NULL;
            return 2;
        }
        int ipc_fd = atoi(env_node_fd);             // TODO: non-numbers are not trackable with this

        r = uv_pipe_open(nodejsipc_pipe, ipc_fd);
        if (r != 0) {
            uv_close((uv_handle_t*)nodejsipc_pipe, NULL);
            free(nodejsipc_pipe);
            nodejsipc_pipe = NULL;
            return r;
        }

        r = uv_read_start((uv_stream_t*)nodejsipc_pipe, NODEJSIPC_C_ALLOC, read_cb);
        if (r != 0) {
            uv_close((uv_handle_t*)nodejsipc_pipe, NULL);
            free(nodejsipc_pipe);
            nodejsipc_pipe = NULL;
            return r;
        }

        return 0;
    }

    /**
     * Stops the current connection
     * 
     * Returns:
     *  0   ok
     *  <0  libuv errorcode
     *  1   connection was not started
     */
    int nodejsipc_stop() {
        if (nodejsipc_pipe == NULL) {
            return 1;
        }
        int r = uv_read_stop((uv_stream_t*)nodejsipc_pipe);
        uv_close((uv_handle_t*)nodejsipc_pipe, NULL);
        free(nodejsipc_pipe);
        nodejsipc_pipe = NULL;
        return r;
    }

    /**
     * Returns the serialization mode of the current connection
     * 
     * Aviable modes:
     *  json, advanced
     */
    char* nodejsipc_channel_serialization() {
        return getenv("NODE_CHANNEL_SERIALIZATION_MODE");
    }

    /**
     * Returns the state of the connection.
     * 
     * true if the connection exist, false otherwise.
     */
    bool nodejsipc_active() {
        return (nodejsipc_pipe != NULL);
    }

    // undef the impl define so if we include the file we used to emit the impl, dosnt also have the impl emited
    #undef NODEIPC_C_IMPL
#endif

#endif