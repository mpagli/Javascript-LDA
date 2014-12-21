/*
 * TopicAssignment.hpp
 *
 *  Created on: Dec 20, 2014
 *      Author: mat
 */

#ifndef TOPICASSIGNMENT_HPP_
#define TOPICASSIGNMENT_HPP_

#include "helper.hpp"
#include "dataset.hpp"

using namespace std;

/**
 * Stores the associations (tokenIdx,topic) for all the dataset
 */
class TopicAssignment {
	private:
		Dataset idxSequences;	//vector of sequences of tokenIdx
		vector<vector<short>> assignmentSequences;	//vector of sequences of associated topics

	public:
		TopicAssignment(){}

		/**
		 * Initialize the datastructure, all the topics are set to 0. Uses shared_ptr
		 * to avoid copying the tokenIdx array
		 */
		TopicAssignment(Dataset& dataset):idxSequences(dataset){
			this->assignmentSequences = vector<vector<short>>(dataset.size(),{0});
			for(int docIdx=0; docIdx<dataset.size(); ++docIdx){
				this->assignmentSequences[docIdx] = vector<short>(dataset.doc_size(docIdx),0);
			}
		}

		~TopicAssignment(){}

		/**
		 * Add a document to the dataset, updates the dictionary, append a vector of
		 * topic assignments to assignmentSequences.
		 */
		void add_document(const vector<string>& document){
			this->idxSequences.add_document(document);
			this->assignmentSequences.push_back(vector<short>(document.size(),0));
		}

		Dataset& get_idxSequences(){
			return idxSequences;
		}

		Dictionary& get_dictionary(){
			return idxSequences.get_dictionary();
		}

		vector<int>& get_document(int idx){
			return idxSequences.get_document(idx);
		}

		int get_document_size(int idx){
			return idxSequences.get_document(idx).size();
		}

		int get_num_documents(){
			return assignmentSequences.size();
		}

		IterDataset get_iter_idxSequences(){
			return this->idxSequences.get_iter_documents();
		}

		IterDataset get_iter_end_idxSequences(){
			return this->idxSequences.get_iter_end_documents();
		}

		IterTopicAssignmt get_iter_assignmentSequences(){
			return this->assignmentSequences.begin();
		}

		IterTopicAssignmt get_iter_end_assignmentSequences(){
			return this->assignmentSequences.end();
		}

		IterVecShort get_iter_begin_assignmentSequence(int idx){
			return this->assignmentSequences[idx].begin();
		}

		IterVecInt get_iter_begin_idxSequences(int idx){
			return this->idxSequences.get_document(idx).begin();
		}

		IterVecShort get_iter_end_assignmentSequence(int idx){
			return this->assignmentSequences[idx].end();
		}

		IterVecInt get_iter_end_idxSequences(int idx){
			return this->idxSequences.get_document(idx).end();
		}

		int get_total_num_tokens(){
			double sum = 0.0;
			for(const auto& doc: assignmentSequences){
				sum += doc.size();
			}
			return sum;
		}

		string to_string(){
			stringstream out;
			out<<"number of documents: "<<this->idxSequences.size()<<endl;
			for(int docIdx=0; docIdx<this->idxSequences.size(); docIdx++){
				for(int tokenIdx=0; tokenIdx<this->idxSequences.doc_size(docIdx); ++tokenIdx){
					out<<this->idxSequences.get_document(docIdx)[tokenIdx]<<":"<<this->assignmentSequences[docIdx][tokenIdx]<<"  ";
				}
				out<<endl;
			}
			out<<endl;
			return out.str();
		}

};



#endif /* TOPICASSIGNMENT_HPP_ */
