#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <unordered_set>
#include <vector>

#include "word.h"

class Dictionary {
public:
	Dictionary();

	bool contains(
			const std::string& word)
		const;

	std::vector<std::string> get_suggestions(
			const std::string& word)
		const;

private:
	void add_trigram_suggestions(
			std::vector<std::string>& suggestions,
			const std::string& word)
		const;

	void rank_suggestions(
			std::vector<std::string>& suggestions,
			const std::string& word)
		const;

	void trim_suggestions(
			std::vector<std::string>& suggestions)
		const;

	bool contains_at_least_half(
			const Word& word,
			const std::vector<std::string>& trigrams_misspelled)
		const;

	unsigned int edit_distance(
			const std::string& s1,
			const std::string& s2)
		const;

	static const unsigned int MAX_LENGTH = 25;
	static const unsigned int TRIM_SIZE  = 5;

	// The 0:th element (words with 0 characters) will be ignored.
	// word[i] will return the vector containing words of length i.
	std::vector<Word> words[MAX_LENGTH + 1];
};

#endif

