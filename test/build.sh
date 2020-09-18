gcc -Wall -o plain/child.run plain/main.c -l uv
gcc -Wall -I ../src/ -o uv_node_ipc/child.run uv_node_ipc/main.c -l uv