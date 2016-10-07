/*
	HASH_TABLE.H
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Thread-safe hash table.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <atomic>
#include <algorithm>

#include "allocator_pool.h"
#include "binary_tree.h"
#include "hash_pearson.h"

namespace JASS
	{
	/*
		CLASS HASH_TABLE
		----------------
	*/
	/*!
		@brief Thread-safe hash table.
		@details The thread-safe hash table is implemented as an array of pointers to the thread-safe binary tree.  The only
		way to access elements is with operator[], thus making it easier to access and easier to implement as thread safe.
		As a thread-safe lock-free structure using a pool allocator, its possible for there to be a leak if multiple threads
		try and allocate the same node (or same node in the same tree) at the same time. Destructors of ELEMENT and KEY are
		never called as all memory is managed by the pool allocator.
		@tparam KEY The type used as the key to the element (must include KEY(allocator, KEY) copy constructor).
		@tparam ELEMENT The element data returned given the key (must include ELEMENT(allocator) constructur).
		@tparam BITS The size of the hash table will be 2^BITS (must be 8, 16, or 24).
	*/
	template <typename KEY, typename ELEMENT, size_t BITS = 24>
	class hash_table
		{
		template<typename A, typename B, size_t C> friend std::ostream &operator<<(std::ostream &stream, const hash_table<A, B, C> &tree);

		protected:
			/*
				HASH_TABLE::SIZE()
				------------------
			*/
			/*!
				@brief Return the size of the hash table (in elements) given the size of the hash table in 2^bits.
				@details as this method is evaluated at compile time, it calls static_assert() if bits is not value (it must be 8, 16, or 24).
				@param bits [in] The size of the hash table is 2^bits
				@return 2^bits.
			*/
			static  constexpr size_t size(size_t bits)
				{
				return bits == 8 ? 256 : bits == 16 ? 65536 : bits == 24 ? 16777216 : 0;
				}

		protected:
			static constexpr size_t hash_table_size = size(BITS);			///< The size of the hash table (in elements)
			
		protected:
			std::atomic<binary_tree<KEY, ELEMENT> *> *table;	///< The hash table is just an array of binary trees
			allocator &memory_pool;															///< The pool allocator
			
		public:
			/*
				HASH_TABLE::HASH_TABLE()
				------------------------
			*/
			/*!
				@brief Constructor
				@param pool [in] All memmory associated with this object is allocated using the pool.
			*/
			hash_table(allocator &pool) : memory_pool(pool)
				{
				table = new (pool.malloc(sizeof(*table) * hash_table_size, sizeof(void *))) std::atomic<binary_tree<KEY, ELEMENT> *>[hash_table_size];
				std::fill(table, table + hash_table_size, nullptr);
				}

			/*
				HASH_TABLE::~HASH_TABLE()
				-------------------------
			*/
			/*!
				@brief Destructor
			*/
			~hash_table()
				{
				/*
					Nothing
				*/
				}
			/*
				HASH_TABLE::TEXT_RENDER()
				-------------------------
			*/
			/*!
				@brief Write the contents of this object to the output steam.
				@param stream [in] The stream to write to.
			*/
			void text_render(std::ostream &stream) const
				{
				for (size_t element = 0; element < hash_table_size; element++)
					if (table[element] != nullptr)
						stream << *table[element];
				}
			
			/*
				HASH_TABLE::OPERATOR[]()
				------------------------
			*/
			/*!
				@brief Return a reference to the element associated with the key.  If there is no element the create an empty one.
				@param key [in] The key to look up.
				@return The element associated with the key.
			*/
			ELEMENT &operator[](const KEY &key)
				{
				size_t hash = hash_pearson::hash<BITS>(key);
				
				if (table[hash] == nullptr)
					{
					binary_tree<KEY, ELEMENT> *empty = nullptr;
					binary_tree<KEY, ELEMENT> *new_tree = new (memory_pool.malloc(sizeof(*new_tree), sizeof(void *))) binary_tree<KEY, ELEMENT>(memory_pool);
					
					table[hash].compare_exchange_strong(empty, new_tree);
					/*
						If the Compare and Swap fails then ignore as it simply means there is now a tree in the table, it doesn't mean the key is in the tree.
					*/
					}
				/*
					Turn the atomic pointer into a binary_tree and call operator[]
				*/
				return (*table[hash].load())[key];
				}

			/*
				HASH_TABLE::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				allocator_pool pool;
				hash_table<slice, slice> map(pool);
				
				map[slice("5")] = slice("five");
				map[slice("3")] = slice("three");
				map[slice("7")] = slice("seven");
				map[slice("4")] = slice("four");
				map[slice("2")] = slice("two");
				map[slice("1")] = slice("one");
				map[slice("9")] = slice("nine");
				map[slice("6")] = slice("six");
				map[slice("8")] = slice("eight");
				map[slice("0")];

				const char *answer = "0->\n6->six\n1->one\n4->four\n5->five\n3->three\n8->eight\n7->seven\n2->two\n9->nine\n";

				std::ostringstream serialised;
				serialised << map;
				JASS_assert(strcmp(serialised.str().c_str(), answer) == 0);
	
				puts("hash_table::PASSED");
				}
		};
		
	/*
		OPERATOR<<()
		------------
		@brief Dump the contents of a binary_tree down an output stream.
		@param stream [in] The stream to write to.
		@param tree [in] The tree to write.
		@return The stream once the tree has been written.
	*/
	template <typename KEY, typename ELEMENT, size_t BITS>
	inline std::ostream &operator<<(std::ostream &stream, const hash_table<KEY, ELEMENT, BITS> &map)
		{
		map.text_render(stream);
		return stream;
		}
	}
