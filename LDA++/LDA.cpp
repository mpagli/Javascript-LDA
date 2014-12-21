//============================================================================
// Name        : LDA.cpp
// Author      : Matteo Pagliardini
// Version     :
// Copyright   : MIT
// Description :
//============================================================================

#include <iostream>
#include "dictionary.hpp"
#include "dataset.hpp"
#include "TopicAssignment.hpp"
#include "LDA.hpp"

using namespace std;

int main() {
	cout << "Test" << endl; // prints

	Dictionary dict;
	vector<string> vec1 = {"un", "deux", "trois","un","un"};
	vector<string> vec2 = {"un", "4", "2","un","6"};

	Dataset dataset;

	dataset.add_document(vec1);
	dataset.add_document(vec2);

	cout<<dataset.to_string()<<endl;



	for(const int& i: dataset.get_document(0)){
		cout<<i<<endl;
	}

	TopicAssignment t;
	t.add_document(vec1);

	cout<<t.to_string();
	t.add_document(vec2);
	cout<<t.to_string();

	double alpha[] = {1.0,1.0,1.0};
	LDA lda(5,alpha,0.01);
	lda.add_document(vec1);
	lda.add_document(vec2);
	lda.initialize();

	cout<<endl<<"done no segfault"<<endl;
	return 0;
}
