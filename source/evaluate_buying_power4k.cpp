/*
	EVALUATE_BUYING_POWER4K.CPP
	---------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_buying_power4k.h"

namespace JASS
	{
	/*
		EVALUATE_BUYING_POWER4K::COMPUTE()
		----------------------------------
	*/
	double evaluate_buying_power4k::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth)
		{
		size_t which = 0;
		double lowest_priced_item = std::numeric_limits<decltype(lowest_priced_item)>::max();
		double total_spending = 0;
		std::vector<double> query_prices;

		/*
			Get the lowest k priced item's price though a linear seach for the assessments for this query
			since this is only going to happen once per run, it doesn't seem worthwhile trying to optimise this.
			We can use a vector of doubles because we don't care which items they are, we only want the lowest prices.
		*/
		for (auto assessment = assessments.find_first(query_id); assessment != assessments.assessments.end(); assessment++)
			if ((*assessment).query_id == query_id && (*assessment).score != 0)
				{
				auto price = prices.find("PRICE", (*assessment).document_id);
				query_prices.push_back(price.score);
				}

		/*
			There is no lowest priced item as there are no relevant items.
		*/
		if (query_prices.size() == 0)
			return 1;

		sort(query_prices.begin(), query_prices.end());
		size_t query_k = maths::minimum(query_prices.size(), top_k);		// if there are fewer then top_k relevant items then reduce k

		double mimimum_cost = 0;
		for (size_t item = 0; item < query_k; item++)
			mimimum_cost += query_prices[item];

		/*
			Compute the spending up to the k-th relevant item.
		*/
		for (const auto &result : results_list)
			{
			/*
				Sum the total spending so far.
			*/
			auto assessment = prices.find("PRICE", result);
			total_spending += assessment.score;

			/*
				Have we found k relevant items yet?
			*/
			assessment = assessments.find(query_id, result);
			if (assessment.score != 0)
				if (--query_k == 0)
					break;

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		return mimimum_cost / total_spending;
		}

	/*
		EVALUATE_BUYING_POWER4K::UNITTEST()
		-----------------------------------
	*/
	void evaluate_buying_power4k::unittest(void)
		{
		/*
			Example results list with one relevant document
		*/
		std::vector<std::string> results_list_one =
			{
			"one",
			"three",
			"two",  		// lowest priced relevant item
			"five",
			"four",
			};

		/*
			Example results list with two relevant document
		*/
		std::vector<std::string> results_list_two =
			{
			"ten",
			"nine",		// relevant
			"eight",		// relevant
			"seven",		// lowest priced relevant item
			"six",
			};

		/*
			Load the sample price list
		*/
		evaluate prices;
		std::string copy = unittest_data::ten_price_assessments_prices;
		prices.decode_assessments_trec_qrels(copy);

		/*
			Load the sample assessments
		*/
		evaluate container;
		copy = unittest_data::ten_price_assessments;
		container.decode_assessments_trec_qrels(copy);

		/*
			Evaluate the first results list
		*/
		evaluate_buying_power4k calculator(2, prices, container);
		double calculated_precision = calculator.compute("1", results_list_one);

		/*
			Compare to 5 decimal places
		*/
		double true_precision_one = 2.0 / 6.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("2", results_list_two);
		double true_precision_two = 15.0 / 27.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));


		puts("evaluate_buying_power4k::PASSED");
		}
	}