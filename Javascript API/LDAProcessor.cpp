
#include <node.h>
#include "LDAProcessor.hpp"

using namespace std;

v8::Persistent<v8::Function> LDAProcessor::constructor;

LDAProcessor::LDAProcessor(int num_topics, double alpha_sum, double beta):
beta(beta), alpha_sum(alpha_sum), num_topics(num_topics), out(cout){
	out << endl << num_topics << endl << alpha_sum << endl << beta <<endl;
	//dataset
}

LDAProcessor::~LDAProcessor(){
}

void LDAProcessor::Init(v8::Handle<v8::Object> exports){
	//constructor template
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(New);
	tpl->SetClassName(v8::String::NewSymbol("LDAProcessor"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//binding functions
	tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("pushDocument"),
		v8::FunctionTemplate::New(pushDocument)->GetFunction());
	constructor = v8::Persistent<v8::Function>::New(tpl->GetFunction());
	exports->Set(v8::String::NewSymbol("LDAProcessor"),constructor);
}

v8::Handle<v8::Value> LDAProcessor::New(const v8::Arguments& args){
	v8::HandleScope scope;
	
	if(args.IsConstructCall()){ //if the method is invoked with a new LDAProcessor(...)
		if(args.Length() != 3){
			v8::ThrowException(v8::Exception::TypeError(v8::String::New("Wrong number of arguments")));
			return scope.Close(v8::Undefined());
		} else if(!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber()){
			v8::ThrowException(v8::Exception::TypeError(v8::String::New("Wrong arguments' type")));
			return scope.Close(v8::Undefined());
		} else {
			int num_topics = args[0]->NumberValue();
			double alpha_sum = args[1]->NumberValue();
			double beta = args[2]->NumberValue();
			LDAProcessor* obj = new LDAProcessor(num_topics,alpha_sum,beta);
			obj->Wrap(args.This());
			return args.This();
		}
	} else {	// Invoked as plain function `LDAProcessor(...)`, turn into construct call.
		const int argc = 3;
		v8::Local<v8::Value> argv[3] = {args[0], args[1], args[2]};
		return scope.Close(constructor->NewInstance(argc, argv));
	}
}

v8::Handle<v8::Value> LDAProcessor::pushDocument(const v8::Arguments& args){
	v8::HandleScope scope;
	
	LDAProcessor* obj = ObjectWrap::Unwrap<LDAProcessor>(args.This());
	//processing obj
	
	return scope.Close(v8::Boolean::New(true));
}


