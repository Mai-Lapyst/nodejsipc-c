# node_ipc

This project enables your c (and c++) code to setup a ipc connection to nodejs.
For now its only designed to work for child processes of nodejs.

## Implementations

### uv_node_ipc
Implementation using the libuv.

## Usage
First you have to include the header file.
To activate the implementation of the header, simply put once in any of your c files before you include the header following code:
```
#define NODEJSIPC_C_IMPL
```

The api has 4 methods:
- `int nodejsipc_start(uv_loop_t* loop, bool prevent_loopexit, uv_read_cb read_cb)`: this starts the ipc connection.
- `int nodejsipc_stop()`: this stops the ipc connection.
- `char* nodejsipc_channel_serialization()`: returns the serialization nodejs will be using
- `bool nodejsipc_active()`: returns true if the ipc connection exists

If you want to use your own allocator callback for `uv_read_start`, you can use the `NODEJSIPC_C_ALLOC` macro.
Example:
```
void myalloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
    // some other fancy stuff
}

#define NODEJSIPC_C_ALLOC myalloc_cb

#define NODEJSIPC_C_IMPL
#include "uv_nodejs_ipc.h"
```

## Testing
To run the tests do the following:
- execute `build.sh` inside the test folder.
- run `node test.js <testname>` inside the test folder. Use for `testname` one of the foldernames that are inside of the test folder.

## License
This project is licensed under the GPL3 license.
A copy of the license can be found in the [LICENSE](LICENSE) file in this folder.