/*
 * dictionary.hpp
 *
 *  Created on: Dec 19, 2014
 *      Author: Pagliardini Matteo
 */

#ifndef DICTIONARY_HPP_
#define DICTIONARY_HPP_

#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

/**
 *	Handle the mapping from tokens to indexes. Used to create a compact representation
 *	of the dataset in memory and have by-index access to values during the LDA computation.
 */
class Dictionary {
	private:
		unordered_map<string, int> token2idx;	//mapping from token to index
		unordered_map<int, string> idx2token;	//mapping from index to token
		vector<int>	counts;	//total number of occurrences in corpus, access by index

	public:
		Dictionary(){}

		Dictionary(Dictionary& dict):token2idx(dict.token2idx),idx2token(dict.idx2token),
				counts(dict.counts){}

		/**
		 * Adds all the tokens in tokenSeq to the dictionary
		 */
		Dictionary(const vector<string>& tokenSeq):Dictionary(){
			for(string const &token: tokenSeq){
				add_token(token);
			}
		}

		~Dictionary(){}

		int get_idx_for_token(const string& token){
			if (token2idx.find(token)!=token2idx.end()){
				return token2idx[token];
			}
			else {
				return -1;
			}
		}

		string get_token_for_idx(int idx){
			if(idx2token.find(idx)!=idx2token.end()){
				return idx2token[idx];
			} else {
				return "";
			}
		}

		void add_token(const string& token){
			int tokenIdx = -1;
			if(token2idx.find(token) == token2idx.end()){
				tokenIdx = token2idx.size();
				token2idx[token]=tokenIdx;
				idx2token[tokenIdx]=token;
				counts.push_back(0);
			} else{
				tokenIdx = token2idx[token];
			}
			counts[tokenIdx]++;
		}

		void add_token_sequence(const vector<string>& tokenSeq){
			for(string const &token: tokenSeq){
				add_token(token);
			}
		}

		/**
		 * reassign all the indexes for all the token in token2Idx such that there are
		 * no holes in the indices.
		 */
		void compactify(){
			for(int idx=0; idx<counts.size(); ++idx){
				if(idx2token.find(idx) == idx2token.end()){	//if the idx not in counts, we need to fill the gap
					string token = idx2token[counts.size()-1];
					idx2token.erase(counts.size()-1);
					token2idx.erase(token);
					token2idx[token]=idx;
					idx2token[idx]=token;
					counts[idx]=counts.back();
					counts.pop_back();
				}
			}
		}

		/**
		 * Removes all tokens with a count less than threshold
		 */
		void filter_tokens(int threshold){
			for(int countIdx=0; countIdx<counts.size(); ++countIdx){
				if(counts[countIdx] < threshold){
					string tokenToRemove = idx2token[countIdx];
					token2idx.erase(tokenToRemove);
					idx2token.erase(countIdx);
				}
			}
			compactify();
		}

		int get_dictionary_size(){
			return token2idx.size();
		}
};



#endif /* DICTIONARY_HPP_ */
