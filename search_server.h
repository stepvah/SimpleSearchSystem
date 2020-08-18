#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
using namespace std;

struct Item {
	size_t dockid;
	size_t hitcount;
};

class InvertedIndex {
public:
	void Add(const string& document);
	const vector<Item>& Lookup(const string& word) const;
	InvertedIndex() = default;
	InvertedIndex(InvertedIndex&& other);
	void operator=(InvertedIndex&& other);
private:
	unordered_map<string, vector<Item>> index;
	size_t num = 0;
};

class SearchServer {
public:
	SearchServer() = default;
	explicit SearchServer(istream& document_input);
	void UpdateDocumentBase(istream& document_input);
	void AddQueriesStream(istream& query_input, ostream& search_results_output);
private:
	InvertedIndex index;
};