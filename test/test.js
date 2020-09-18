var cp = require('child_process');
var n  = cp.spawn('./' + process.argv[2] + '/child.run', [], {
    stdio: [ 'inherit', 'inherit', 'inherit', 'ipc' ]
});

n.on('error', (err) => console.log("[ERROR] got error on child: ", err));
n.on('close', () => console.log("[INFO] child closed"));
n.on('message', (data) => console.log("[INFO] recived data: ", data));

if (typeof n.send === 'function') {
    n.send(
        { mystr: 'hello world' },
        undefined,
        undefined,
        (err) => {
            if (err) {
                console.log("[js-error] got error while sending:", err);
            }
        }
    );
} else {
    console.log("Got error: the send function dosnt exists!!");
}