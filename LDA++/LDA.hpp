/*
 * LDA.hpp
 *
 *  Created on: Dec 20, 2014
 *      Author: Pagliardini Matteo
 */

#ifndef LDA_HPP_
#define LDA_HPP_

#include "helper.hpp"
#include "TopicAssignment.hpp"

using namespace std;

/**
 * Base class for LDA methods
 */
class LDA {
	private:
		TopicAssignment data; //a structure to store for each token of the dataset the attributed topic, also contains the dictionary

		int numTopics;
		int numDocuments;	//the number of documents in the corpus
		int dictionarySize;		//number of tokens in dictionary
		int globalNumTokens;	//number of tokens in the corpus
		double* alpha;			//for each topic, one alpha value, used for the dirichlet prior over theta=P(topic|doc)
		double alphaSum;
		double beta;			//used for the dirichlet prior over phi=P(token|topic)
		double betaSum;
		bool symmetricAlpha;	//if true then alphas are all the same
		int burninPeriod;		//the length of the burnin period
		int numIterations;

		int topicMask;	//we follow the mallet implementation to store the tokenIdx,topic_count information
		int topicBits;	//for numTopics=5 we need 3 bits to store all the topic indices, topicMask = 00000...0111, topicBits = 3

		double smoothingOnlyMass = 0.0;
		double* cachedCoefficients;

		int* doclength;				//doclength[docIdx] = size of doc[docIdx]
		vector<vector<int>> n_tokenIdx_topicIdx;	//n_tokenIdx_topicIdx[tokenIdx][topicIdx] = count
		int* n_topicIdx;			//n_topicIdx[topicIdx] = count

		int randomSeed;

	public:
		LDA(){
			this->numTopics = 100;
			this->numDocuments = 0;
			this->dictionarySize = 0;
			this->globalNumTokens = 0;
			this->symmetricAlpha = true;
			this->alphaSum = 50.0;
			this->beta = 0.001;
			fill_alpha_symmetric();
			this->burninPeriod = 200;
			this->numIterations = 1000;
			this->randomSeed = -1;
		}

		LDA(int numTopics, double alphaSum, double beta){
			this->numTopics = numTopics;
			this->numDocuments = 0;
			this->dictionarySize = 0;
			this->globalNumTokens = 0;
			this->symmetricAlpha = true;
			this->alphaSum = alphaSum;
			this->beta = beta;
			fill_alpha_symmetric();
			this->burninPeriod = 200;
			this->numIterations = 1000;
			this->randomSeed = -1;
		}

		LDA(int numTopics, double alpha[], double beta){
			this->numTopics = numTopics;
			this->numDocuments = 0;
			this->dictionarySize = 0;
			this->globalNumTokens = 0;
			this->symmetricAlpha = false;
			this->alphaSum = 0.0;
			for(int idx; idx<numTopics;++idx){
				alphaSum += alpha[idx];
			}
			this->beta = beta;
			this->alpha = alpha;
			this->burninPeriod = 200;
			this->numIterations = 1000;
			this->randomSeed = -1;
		}

		void add_document(const vector<string>& document){
			this->data.add_document(document);
		}

		void add_documents(const vector<vector<string>>& documents){
			for(const auto& document: documents){
				this->data.add_document(document);
			}
		}

		/**
		 * If alpha is symmetric, all the alpha_i have the same value. No topic is favored
		 * in the corpus.
		 */
		void fill_alpha_symmetric(){
			alpha = new double[this->numTopics];
			for(int alpha_idx; alpha_idx<this->numTopics; ++alpha_idx){
				this->alpha[alpha_idx] = this->alphaSum/double(this->numTopics);
			}
		}

		void initialize_bit_masks(){
			//initializes bit masks
			bitset<sizeof(int)*8> numTopicsBits(numTopics);
			if (numTopicsBits.count() == 1) {	//if the number of topics is a power of two then we can use the
												//bits up to that power to represent the topic
				topicMask = numTopics - 1;
				topicBits = bitset<sizeof(int)>(topicMask).count();
			}
			else {	//else we need one more bit
				for(int idx=numTopicsBits.size()-1;idx>=0;--idx){
					if(numTopicsBits[idx] == 1){
						topicMask = pow(2,idx)*2-1;
						break;
					}
				}
				topicBits = bitset<sizeof(int)>(topicMask).count();
			}
		}

		void initialize_counts(){
			//initializes arrays
			cachedCoefficients = new double[numTopics];
			numDocuments = data.get_num_documents();
			dictionarySize = data.get_dictionary().get_dictionary_size();
			globalNumTokens = data.get_total_num_tokens();
			betaSum = dictionarySize*beta;
			doclength = new int[numDocuments];
			for(int idx=0;idx<numDocuments;++idx){
				doclength[idx]=data.get_document_size(idx);
			}
			//--- n_tokenIdx_topicIdx
			int tokenIdxCount[dictionarySize]; //for each tokenIdx stores its count in the whole corpus
			n_tokenIdx_topicIdx.resize(dictionarySize,vector<int>());
			for(int& count: tokenIdxCount){
				count = 0;
			}
			IterDataset itData = data.get_iter_idxSequences();
			for(;itData!=data.get_iter_end_idxSequences();++itData){
				IterVecInt itTokenIdx=(*itData).begin();
				for(;itTokenIdx!=(*itData).end();++itTokenIdx){
					tokenIdxCount[*itTokenIdx]++;
				}
			}
			for(int idx=0; idx<dictionarySize; ++idx){
				n_tokenIdx_topicIdx[idx].resize(min(tokenIdxCount[idx],numTopics),0);
			}
			//--- sample the topics at random
			random_device rnd;
			mt19937 mt(rnd());
			if(randomSeed != -1){
				mt.seed(randomSeed);
			}
			uniform_int_distribution<> dist(0, numTopics-1);
			IterTopicAssignmt iterTopicAssignmt = data.get_iter_assignmentSequences();
			for(;iterTopicAssignmt!=data.get_iter_end_assignmentSequences();++iterTopicAssignmt){
				IterVecShort iterTopics = (*iterTopicAssignmt).begin();
				for(;iterTopics!=(*iterTopicAssignmt).end();++iterTopics){
					*iterTopics = dist(mt);
				}
			}
			//--- n_topicIdx_docIdx and n_topicIdx
			n_topicIdx = new int[numTopics];
			for(int topicIdx=0;topicIdx<numTopics;topicIdx++){
				n_topicIdx[topicIdx] = 0;
			}
			for(int docIdx=0;docIdx<this->numDocuments;++docIdx){ //iterate over all documents
				IterVecShort currentTopic = data.get_iter_begin_assignmentSequence(docIdx);
				IterVecInt currentToken = data.get_iter_begin_idxSequences(docIdx);

				for(;currentToken!=data.get_iter_end_idxSequences(docIdx);++currentToken){

					n_topicIdx[*currentTopic]++;

					//the n_tokenIdx_topicIdx[tokenId] stores the counts of tokenId for each topic sorted per counts number.
					//n_tokenIdx_topicIdx has the capacity of storing one count per topic. The counts are kept sorted, not
					//all the vector is used.
					vector<int>* currentTokenTopic = &n_tokenIdx_topicIdx[*currentToken]; 	//a vector of int, the topicMask rightmost
																						//bits are used to store the topic Id.
					int idx=0;
					int topic=(*currentTokenTopic)[0]&topicMask;
					int currentCount = -1;
					while((*currentTokenTopic)[idx]>0 && topic!=*currentTopic){
						idx++;
						topic=(*currentTokenTopic)[idx]&topicMask;
					}
					currentCount=(*currentTokenTopic)[idx]>>topicBits;

					if(currentCount == 0){
						(*currentTokenTopic)[idx] = (1<<topicBits)+topic;
					}else{ //we need to be sure the list stay sorted
						(*currentTokenTopic)[idx] = ((currentCount+1)<<topicBits)+topic;
						while(idx>0 && (*currentTokenTopic)[idx]>(*currentTokenTopic)[idx-1]){
							int temp=(*currentTokenTopic)[idx];
							(*currentTokenTopic)[idx] = (*currentTokenTopic)[idx-1];
							(*currentTokenTopic)[idx-1] = temp;
							idx--;
						}
					}
					currentTopic++;
				}
			}
		}

		/**
		 * Return true if all the counts work together. The sum of all the counts should be
		 * the total number of tokens in the corpus.
		 */
		bool check_counts_integrity(){
			bool answer=true;
			int sum_doclength=0, sum_topicIdx=0, sum_token_topic=0;
			for(int idx=0;idx<numDocuments;++idx){
				sum_doclength += doclength[idx];
			}
			for(int idx=0;idx<numTopics;++idx){
				sum_topicIdx += n_topicIdx[idx];
			}
			IterDataset it_token_topic=n_tokenIdx_topicIdx.begin();
			for(;it_token_topic!=n_tokenIdx_topicIdx.end();++it_token_topic){
				IterVecInt it_token = (*it_token_topic).begin();
				for(;it_token!=(*it_token_topic).end();++it_token){
					sum_token_topic +=(*it_token)>>topicBits;
				}
			}
			if(sum_doclength!=globalNumTokens || sum_topicIdx!=globalNumTokens ||
					sum_token_topic!=globalNumTokens){
				answer=false;
			}
			return answer;
		}

		/**
		 * Call this function once everything is set. Initializes all the count structures.
		 */
		void initialize(){
			initialize_bit_masks();
			initialize_counts();
			assert(check_counts_integrity());
		}

		void sample_topics_for_doc(int docIdx){

			int localTopicCounts[numTopics];
			int localTopicIdx[numTopics];
			int oldTopic,token;

			IterVecShort currentTopic = data.get_iter_begin_assignmentSequence(docIdx);
			for(;currentTopic!=data.get_iter_end_assignmentSequence(docIdx);++currentTopic){
				localTopicCounts[*currentTopic]++;
			}

			int denseIdx = 0;
			for(int topicIdx=0;topicIdx<numTopics;++topicIdx){
				if(localTopicCounts[topicIdx]!=0){
					localTopicIdx[denseIdx] = topicIdx;
					denseIdx++;
				}
			}
			int nonZeroTopics = denseIdx;

			double topicBetaMass = 0.0;

			for(denseIdx=0;denseIdx<nonZeroTopics;denseIdx++){
				int topic = localTopicIdx[denseIdx];
				int n = localTopicCounts[topic];
				topicBetaMass += beta*n/(n_topicIdx[topic]+betaSum);
				cachedCoefficients[topic] = (alpha[topic]+n)/(n_topicIdx[topic]+betaSum);
			}

			double topicTermMass = 0.0;
			double topicTokenScores[numTopics];
			int* topicTermIndices;
			int* topicTermValues;
			int i;
			double score;
			IterVecInt currentToken = data.get_iter_begin_idxSequences(docIdx);
			currentTopic = data.get_iter_begin_assignmentSequence(docIdx);
			for(;currentToken!=data.get_iter_end_idxSequences(docIdx);++currentToken){
				token = *currentToken;
				oldTopic = *currentTopic;

				//Remove the influence of the current token
				smoothingOnlyMass -= alpha[oldTopic]*beta / (n_topicIdx[oldTopic]+betaSum);
				topicBetaMass -= beta*localTopicCounts[oldTopic]/(n_topicIdx[oldTopic]+betaSum);
				localTopicCounts[oldTopic]--;

				//maintain the denseIdx list
				if(localTopicCounts[oldTopic] == 0){
					denseIdx = 0;
					while(localTopicIdx[denseIdx]!=oldTopic){
						denseIdx++;
					}
					while (denseIdx < nonZeroTopics) {
						if (denseIdx < numTopics - 1) {
							localTopicIdx[denseIdx] = localTopicIdx[denseIdx + 1];
						}
						denseIdx++;
					}
					nonZeroTopics--;
				}
				//decrement n_tokenIdx_topicIdx
				n_topicIdx[oldTopic]--;

				smoothingOnlyMass += alpha[oldTopic] * beta / (n_topicIdx[oldTopic] + betaSum);
				topicBetaMass += beta * localTopicCounts[oldTopic] /(n_topicIdx[oldTopic] + betaSum);

				cachedCoefficients[oldTopic] = (alpha[oldTopic] + localTopicCounts[oldTopic]) /(n_topicIdx[oldTopic] + betaSum);

				vector<int>* token_topicIdx = &n_tokenIdx_topicIdx[token];
				//the token/oldTopic association should be removed
				bool alreadyDecremented = false;
				int idx = 0, localTopic, localCount;
				while(idx<token_topicIdx->size() && (*token_topicIdx)[idx]>0){
					localTopic = (*token_topicIdx)[idx] & topicMask;
					localCount = (*token_topicIdx)[idx] >> topicBits;

					if(!alreadyDecremented && localTopic == oldTopic){
						localCount--;
						if(localCount == 0){
							(*token_topicIdx)[idx] = 0;
						}else{
							(*token_topicIdx)[idx] = (localCount << topicBits)+oldTopic;
						}

						int subIdx = idx;
						while(subIdx < (*token_topicIdx).size()-1 && (*token_topicIdx)[subIdx] < (*token_topicIdx)[subIdx+1]){
							int temp = (*token_topicIdx)[subIdx];
							(*token_topicIdx)[subIdx] = (*token_topicIdx)[subIdx+1];
							(*token_topicIdx)[subIdx+1] = temp;
							subIdx++;
						}
						alreadyDecremented = true;
					} else {
						score = cachedCoefficients[localTopic] * localCount;
						topicTermMass += score;
						topicTokenScores[idx] = score;
						idx++;
					}
				}

				std::random_device rd;
				std::default_random_engine e1(rd());
				uniform_real_distribution<double> uniformDist(0,1);
				double sample = uniformDist(e1);

				currentTopic++;
			}
		}

		void runLDA(){
			smoothingOnlyMass = 0.0;

			initialize();

			for(int iter=0;iter<numIterations;++iter){
				for(int topicIdx=0;topicIdx<numTopics;++topicIdx){
					smoothingOnlyMass += alpha[topicIdx]*beta / (n_topicIdx[topicIdx]+betaSum);
					cachedCoefficients[topicIdx] = alpha[topicIdx] / (n_topicIdx[topicIdx]+betaSum);
				}

				for(int docIdx=0;docIdx<this->numDocuments;++docIdx){ //iterate over all documents
					sample_topics_for_doc(docIdx);
				}
			}

		}
};



#endif /* LDA_HPP_ */
