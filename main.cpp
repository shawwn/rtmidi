#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <node_buffer.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <set>
#include <nan.h>

// module

extern "C" NAN_MODULE_INIT(init) {
}

NODE_MODULE(rtmidi, init)

