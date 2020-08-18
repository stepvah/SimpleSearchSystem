#pragma comment(linker, "/STACK:16777216")
#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
using namespace std;

vector<string> SplitIntoWords(const string& line) {
	istringstream words_input(line);
	return { istream_iterator<string>(words_input), istream_iterator<string>() };
}

SearchServer::SearchServer(istream& document_input) {
	UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
	InvertedIndex new_index;

	for (string current_document; getline(document_input, current_document);) {
		new_index.Add(move(current_document));
	}

	index = move(new_index);
}

void SearchServer::AddQueriesStream(
	istream& query_input, ostream& search_results_output
) {
	vector<size_t> docid_count(50'000, 0);
	vector<pair<size_t, size_t*>> search_results;
	search_results.reserve(50'000);

	for (string current_query; getline(query_input, current_query);) {
		const auto words = SplitIntoWords(current_query);

		fill(docid_count.begin(), docid_count.end(), 0);
		search_results.clear();

		for (const auto& word : words) {
			for (const Item& item : index.Lookup(word)) {
				if (docid_count[item.dockid] == 0) {
					search_results.push_back(
						make_pair(item.dockid, &docid_count[item.dockid]));
				}
				docid_count[item.dockid] += item.hitcount;
			}
		}

		auto MiddleIt = search_results.size() < 5 ? search_results.end() : search_results.begin() + 5;
		partial_sort(begin(search_results),
			MiddleIt,
			end(search_results),
			[](pair<size_t, size_t*> lhs, pair<size_t, size_t*> rhs) {
			int64_t lhs_docid = lhs.first;
			auto lhs_hit_count = *lhs.second;
			int64_t rhs_docid = rhs.first;
			auto rhs_hit_count = *rhs.second;
			return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
		});

		search_results_output << current_query << ":";
		for (auto [docid, hitcount] : Head(search_results, 5)) {
			if (hitcount != 0) {
				search_results_output << " {docid: "
					<< docid << ", hitcount: " << *hitcount << '}';
			}
		}
		search_results_output << '\n';
	}
}

InvertedIndex::InvertedIndex(InvertedIndex&& other)
	: index(move(other.index)),
	num(other.num) {
	other.num = 0;
}

void InvertedIndex::operator=(InvertedIndex&& other)
{
	index = move(other.index);
	num = other.num;
	other.num = 0;
}

void InvertedIndex::Add(const string& document) {
	const size_t docid = num++;
	unordered_map<string_view, size_t*> words;

	for (auto& word : SplitIntoWords(document)) {
		if (auto it = words.find(word); it == words.end()) {
			auto& vec = index[word];
			vec.push_back({ docid, 1 });
			words[word] = &vec[vec.size() - 1].hitcount;
		} else {
			(*(it->second))++;
		}
	}
}

const vector<Item> empty__;

const vector<Item>& InvertedIndex::Lookup(const string& word) const {
	if (auto it = index.find(word); it != index.end()) {
		return it->second;
	}
	else {
		return empty__;
	}
}