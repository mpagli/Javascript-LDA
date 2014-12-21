#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "LDAProcessor.hpp"

void InitAll(v8::Handle<v8::Object> exports) {
		LDAProcessor::Init(exports);
}

NODE_MODULE(javascriptLDA, InitAll)
