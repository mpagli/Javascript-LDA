/*
 * dataset.hpp
 *
 *  Created on: Dec 20, 2014
 *      Author: mat
 */

#ifndef DATASET_HPP_
#define DATASET_HPP_

#include "helper.hpp"
#include "dictionary.hpp"
#include "sstream"

using namespace std;

/**
 * A dataset is a set of documents with a dictionary
 */
class Dataset {
	private:
		Dictionary dictionary;
		vector<vector<int>> documents;

	public:
		Dataset(){}

		/**
		 * Builds a dataset from a set of documents, fills the dictionary.
		 */
		Dataset(const vector<vector<string>>& documents){
			this->dictionary = Dictionary();
			for(const vector<string>& document: documents){
				this->dictionary.add_token_sequence(document);
				vector<int> idxSequence(document.size(),0);
				int idx = 0;
				for(const string& token: document){
					idxSequence[idx] = this->dictionary.get_idx_for_token(token);
					idx++;
				}
				this->documents.push_back(idxSequence);
			}
		}

		~Dataset(){}

		/**
		 * Appends a document to documents, updates the dictionary.
		 */
		void add_document(const vector<string>& document){
			this->dictionary.add_token_sequence(document);
			vector<int> idxSequence(document.size(),0);
			int idx = 0;
			for(const string& token: document){
				idxSequence[idx] = this->dictionary.get_idx_for_token(token);
				idx++;
			}
			this->documents.push_back(idxSequence);
		}

		Dictionary& get_dictionary(){
			return dictionary;
		}

		vector<int>& get_document(int idx){
			return documents[idx];
		}

		IterDataset get_iter_documents(){
			return documents.begin();
		}

		IterDataset get_iter_end_documents(){
			return documents.end();
		}

		int size(){
			return this->documents.size();
		}

		int doc_size(int& docIdx){
			return this->documents[docIdx].size();
		}

		/**
		 * Switch of representation, takes all the documents and create a new set of document where
		 * each document is a word.
		 */
		void lda_on_words(){

		}

		/**
		 * Removes from the documents all the tokens not present in the dictionary.
		 */
		void filter_from_dictionary(){

		}

		float get_average_doc_size(){
			float sum = 0.0;
			for(const auto& doc : documents){
				sum += float(doc.size());
			}
			return sum/float(documents.size());
		}

		string to_string(){
			stringstream out;
			out << " ###### Dataset ######\n number of documents: "<< documents.size() << endl
					<< " number of tokens: " << dictionary.get_dictionary_size() << endl
					<< " average doc size: " << get_average_doc_size() << endl;
			return out.str();
		}
};



#endif /* DATASET_HPP_ */
