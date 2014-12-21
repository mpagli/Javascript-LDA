#ifndef LDAPROCESSOR_H
#define LDAPROCESSOR_H

#include <node.h>
//#include "dataset.h"
#include <iostream>

using namespace std;

class LDAProcessor : public node::ObjectWrap {
	
	private:
		//dataset
		double beta;
		int num_topics;
		double alpha_sum;
		ostream& out;
		//id2tokens //use smart poiteurs
		
		explicit LDAProcessor(int num_topics, double alpha_sum, double beta);
		~LDAProcessor();
		
		static v8::Handle<v8::Value> New(const v8::Arguments& args);
		static v8::Handle<v8::Value> pushDocument(const v8::Arguments& args);
		static v8::Persistent<v8::Function> constructor;
		
	public:
		static void Init(v8::Handle<v8::Object> exports);
		
		
};

#endif
