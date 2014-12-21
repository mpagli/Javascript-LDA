/*
 * helper.hpp
 *
 *  Created on: Dec 20, 2014
 *      Author: mat
 */

#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <sstream>
#include <assert.h>
#include <array>
#include <bitset>
#include <iterator>
#include <math.h>
#include <random>

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

typedef shared_ptr<vector<vector<int>>> SharedDataset;
typedef shared_ptr<vector<int>> SharedDocument;
typedef vector<int>::iterator IterVecInt;
typedef vector<short>::iterator IterVecShort;
typedef vector<vector<int>>::iterator IterDataset;
typedef vector<vector<short>>::iterator IterTopicAssignmt;


#endif /* HELPER_HPP_ */
