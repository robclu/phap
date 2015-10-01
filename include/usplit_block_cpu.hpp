/// @file   usplit_block_cpu.hpp
/// @brief  Header file for the unsplittable block class cpu implementation for the parahaplo library 
// ----------------------------------------------------------------------------------------------------------

#ifndef PARAHAPLO_USPLIT_BLOCK_IMPLEMENTATION_CPU_HPP
#define PARAHAPLO_USPLIT_BLOCK_IMPLEMENTATION_CPU_HPP

#include "block_interface.hpp"
#include "equality_checker.hpp"
#include "tree_cpu.hpp"
#include "usplit_block_interface.hpp"

#include <tbb/concurrent_unordered_map.h>
#include <atomic>
#include <bitset>
#include <numeric>
#include <utility>

namespace haplo {

// Specialization for using the CPU
template <typename BaseBlock, size_t Cores>
class UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores> : BaseBlock {
public:
    // --------------------------------------- ALIAS'S ------------------------------------------------------
    using base_type         = BaseBlock;
    using data_type         = typename base_type::data_type;
    using data_container    = std::vector<data_type>;
    using subinfo_type      = typename base_type::subinfo_type;
    using ston_container    = typename std::array<bool, base_type::rows()>;
    using concurrent_umap   = tbb::concurrent_unordered_map<short, short>;
    using index_umap        = tbb::concurrent_unordered_map<short, std::pair<short, short>>;
private:
    data_container  _data;              //!< Data for the unsplittable block
    ston_container  _singleton_info;    //!< Which rows are singletons and which are not
    concurrent_umap _row_mplicities;    //!< Multiplicity of all non-duplicate rows
    concurrent_umap _col_mplicities;    //!< Multiplicity of all non-duplicate columns
    index_umap      _row_params;        //!< Map for row indices mapping to the duplicates and the multiplicites
    index_umap      _col_params;        //!< Map for col indices mapping to the duplicates and the multiplicites
    size_t          _index;             //!< The index of the unsplittable block in the base block
    size_t          _size;              //!< The size -- total number of elements -- in the unsplittable block
    size_t          _rows;              //!< The number of rows in the unsplittable block
    size_t          _cols;              //!< The number of columns in the unsplitatble block
public:
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Constructor which sets the base block for the unsplittable block 
    /// @param[in]  base_block  The block which is being used as the base of this class. This unsplittable
    ///             block is a sub-region of the base block, 
    /// @param[in]  index       The index of this unsplittable block in the base block
    // ------------------------------------------------------------------------------------------------------
    UnsplittableBlockImplementation(base_type& base_block, const size_t index = 0);
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Accessor for the base block 
    /// @return     A pointer to the base block 
    // ------------------------------------------------------------------------------------------------------
    base_type* base_block() { return static_cast<base_type*>(this); }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the multiplicity of row i, if it has a multiplicity, otherwise returns 0
    /// @return     The multiplicity of row i
    // ------------------------------------------------------------------------------------------------------
    short row_multiplicity(const short i) const 
    { 
        return _row_params.find(i) != _row_params.end() ? _row_params.at(i).second : 99; 
    }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the multiplicity of column i, if it has a multiplicity, otherwise returns 0
    /// @return     The multiplicity of column i
    // ------------------------------------------------------------------------------------------------------
    short col_multiplicity(const short i) const 
    { 
        return _col_params.find(i) != _col_params.end() ? _col_params.at(i).second :99; 
    }
   
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Solves the unsplittable block for the haplotype
    // ------------------------------------------------------------------------------------------------------
    void solve_haplotype();
        
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Access to the elements of the unsplittable block after row/duplicate removal
    /// @param[in]  row     The row of the element in the unsplitatble block to get
    /// @param[in]  col     The column of the element in the unsplittable block to get
    /// @return     The value of the element at position (row, col) without duplicates
    // ------------------------------------------------------------------------------------------------------
    haplo::Data operator()(const short row, const short col) const;
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Access to the elements of the unsplittable block before row/duplicate removal
    /// @param[in]  row     The row of the element in the unsplitatble block to get
    /// @param[in]  col     The column of the element in the unsplittable block to get
    /// @return     The value of the element at position (row, col) with duplicates
    // ------------------------------------------------------------------------------------------------------
    data_type at(const short row, const short col) const { return _data[row * _cols + col]; }    
private:
    // ------------------------------------------------------------------------------------------------------
    /// @brief      First removes all the singleton rows (rows with only 1 value) from the unsplittable block
    ///             from which the number of rows, and hence the total size of the block, can be determined.
    // ------------------------------------------------------------------------------------------------------
    void determine_params();
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Uses the determined parameters to fill the data for the sub-block from the _expression
    // ------------------------------------------------------------------------------------------------------
    void fill();
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Maps rows from the unsplittable block to the base block. Say for example the base block
    ///             has 10 rows, and 5 are singular (2, 3, 6, 8, 9) and must be removed, so the _singletons
    ///             array will look like :                                                                  \n\n
    ///             [1, 1, 0, 0, 1, 1, 0, 1, 0, 0]                                                          \n\n
    ///
    ///             Then the mapping is:                                                                    \n\n
    /// 
    ///             | UnsplittableRow | Base Block Row  | (0 indexing)                                      \n
    ///             |       0         |         0       |                                                   \n
    ///             |       1         |         1       |                                                   \n
    ///             |       2         |         4       |                                                   \n
    ///             |       3         |         5       |                                                   \n
    ///             |       4         |         7       |                                                   \n 
    /// @param[in]  unsplittable_row_index   The unspittable index of the unsplittable row for which the
    ///             corresponding row index in the base block must be found
    /// @return     The corresponding row index in the base block
    // ------------------------------------------------------------------------------------------------------
    size_t map_to_base_row(const size_t unsplittable_row_index);
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Removes all duplicate columns or rows
    /// @param[in]  num_threads     The number of threads to use
    /// @tparam     Checker         The fuctor which checks either row or column equivalence
    // ------------------------------------------------------------------------------------------------------
    template <typename Checker>
    void remove_duplicates(Checker checker);
};

// -------------------------------------- IMPLEMENTATIONS ---------------------------------------------------

// ------------------------------------------ PUBLIC --------------------------------------------------------

template <typename BaseBlock, size_t Cores>
UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::UnsplittableBlockImplementation(
                                                                                    base_type& base_block   , 
                                                                                    const size_t index      )
: base_type(base_block), _data(0), _index(index) 
{
    determine_params();
    fill();
    
    // Create row and column equality checker functors
    EqualityChecker<check::ROWS>    row_checker;
    EqualityChecker<check::COLUMNS> col_checker;
    
    remove_duplicates(row_checker);    // Remove all duplicate rows
    remove_duplicates(col_checker);    // Remove all duplicate columns
}

template <typename BaseBlock, size_t Cores>
void UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::solve_haplotype()
{
    // Doing the parallelization over the rows (outer loop in the mathematical problem description)
    // or y variables, so check if the number of threads to use is greater than the number of rows 
    const size_t y_variables        = _row_params.size();
    const size_t x_variables        = _col_params.size();
    const size_t threads_to_use     = Cores > y_variables ? y_variables : Cores;

    // Create trees to do the search. Each y variable has 2 possibilities and for each of the y
    // variables the are 2^{N + 1} possibilities for the x variables. The y variables are used 
    // as the tree roots and then the tree is searched by branching the x variables, to find the 
    // optimal x variables for a given y variable. The bounding operator reduces the 2^{N-1} possible 
    // branches for each y variable to 2N. So the total possible branches is 2^{M + N + 1}, of which 
    // only N*2^{M} searches are done. The 2M y variables are then compared, which can be done in log2M 
    // time. So create the 2M trees
    std::vector<Tree<Device::CPU>> trees; 
    trees.reserve(2 * y_variables);                                     // Reserve space for the trees
    
    for (size_t y_index = 0; y_index < y_variables; ++y_index) {        // Initialize the trees
        trees.emplace_back(y_index, 0,  x_variables);
        trees.emplace_back(y_index, 1,  x_variables);
    }

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, threads_to_use),
        [&](const tbb::blocked_range<size_t>& thread_indices) 
        {
            for (size_t idx = thread_indices.begin(); idx != thread_indices.end(); ++idx) {
                size_t thread_iters = ops::get_thread_iterations(idx, _rows, threads_to_use);
                for (size_t it = 0; it < thread_iters; ++it) {
                    // Define and index for the thread
                    size_t thread_idx = ops::thread_map(idx, threads_to_use, it);
                    
                    // Solve the tree for the optimal values given the tree root index and value
                    trees[thread_idx].solve(*this);
                    
                    // If this thread_idx has a row multiplicity (i.e is not a duplicate)
                    if (_row_mplicities.find(thread_idx) != _row_mplicities.end()) {
                        // Solve the bost solution for the tree given this index, and the next
                        //trees[
                        
                    }
                    
                }
            }
        }
    );
    
    for (const auto & tree : trees) tree.print();
}

template <typename BaseBlock, size_t Cores>
haplo::Data UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::operator()(const short row,
                                                                                       const short col) const
{
   // Check that the row and column is valid
   if (_row_params.count(row) && _col_params.count(col))
       return _data[_row_params.at(row).second * _cols + _col_params.at(col).second];
}

// ------------------------------------------- PRIVATE ------------------------------------------------------

template <typename BaseBlock, size_t Cores>
void UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::determine_params()
{
    // Create threads where each checks for singletons
    const size_t threads_to_use = Cores > base_type::rows() ? base_type::rows() : Cores;
    
    // Information for the sub-block
    const subinfo_type& info = base_block()->subblock_info(_index);
        
    // Check which rows are singletons
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, threads_to_use),
        [&](const tbb::blocked_range<size_t>& thread_indices) 
        {
            for (size_t idx = thread_indices.begin(); idx != thread_indices.end(); ++idx) {
                size_t thread_iters = ops::get_thread_iterations(idx, base_type::rows(), threads_to_use);
                for (size_t it = 0; it < thread_iters; ++it) {
                    size_t non_gaps     = 0;                        // Number of row elements which aren't gaps
                    size_t base_row     = ops::thread_map(idx, threads_to_use, it);
                    // Now we need to go through all elements in the row 
                    // of base block's rows which are part of this unspittable 
                    // block and check of there is only a single element
                    for (size_t base_col = info.start(); base_col <= info.end() && non_gaps < 2; ++base_col) {
                        if (base_block()->operator()(base_row, base_col).value() != 2) 
                            ++non_gaps;                             // Not a gap, so increment num_elements
                    }
                    // If we have found more than a single element then the row needs to be added
                    // to the UnsplittableBlock, so set the relevant value to 1 (not a singleton)
                    _singleton_info[base_row] = non_gaps > 1 ? 1 : 0;
                }
            }
        }
    );
    
    // This acclally counts the number of rows which aren't singletons
    _rows = std::accumulate(_singleton_info.begin(), _singleton_info.end(), 0);
    _cols = info.columns();
    _size = _rows * info.columns();
    _data.resize(_size, static_cast<uint8_t>(0));   
}

template <typename BaseBlock, size_t Cores>
void UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::fill()
{
    // Check that we aren't using too many threads
    const size_t threads_to_use = Cores >  _rows ? _rows : Cores;
    
    // Reference to the sub-block info for this unsplittable block
    const subinfo_type& info = base_block()->subblock_info(_index);
    
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, threads_to_use),
        [&](const tbb::blocked_range<size_t>& thread_indices)
        {
            for (size_t idx = thread_indices.begin(); idx != thread_indices.end(); ++idx) {
                size_t thread_iters = ops::get_thread_iterations(idx, _rows, threads_to_use);
                for (size_t it = 0; it < thread_iters; ++it) {
                    // Get the row of the unsplittable block based on the thead it and num threads
                    size_t row_idx = ops::thread_map(idx, threads_to_use, it);
                    // Using the row of this unsplittable block, get the corresponding 
                    // row from the base block -- see function declaration in class declararion
                    size_t base_row = map_to_base_row(row_idx);
                    // Go through the columns and set the data for the 
                    // unsplitable block using the data from the base block
                    for (size_t base_col = info.start(); base_col <= info.end(); ++base_col) {
                        _data[row_idx * _cols       +                               // offset due to row 
                              base_col - info.start()]                              // Column offset
                        = base_block()->operator()(base_row, base_col).value();     // Value
                    }                  
                }               
            }
        }
    );
}

template <typename BaseBlock, size_t Cores>
size_t UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::map_to_base_row(
                                                                        const size_t unsplittable_row_index)
{
    size_t non_singleton_row = -1, base_row = 0;
    // Keep looking through the row information of the base block until we have 
    // found the nonsingleton row index equal to the unsplittable row index we want
    while (non_singleton_row != unsplittable_row_index) {
        // 1's represent a row that is not singleton, so if we find one,
        // increment the current non singleton row in the base block
        if (_singleton_info[base_row++] == 1) ++non_singleton_row;
    }
    return --base_row;
}

template <typename BaseBlock, size_t Cores> template <typename Checker>
void UnsplittableBlockImplementation<BaseBlock, Device::CPU, Cores>::remove_duplicates(Checker checker)
{
    // This can check either row or column equivalence, the following variables are defined:
    // 
    // checks       : This is the total number of elements for comparison, so if rows are being checked 
    //                then checks = number of columns since for 2 rows, each corresponding column must 
    //                be checked, and vice versa for columns
    // comparisons  : This is the total number of rows or columns (an index) to compare for equivelance,
    //                if columns are being checked then comparisons = _cols and if rows are are being 
    //                checked then comparisons = _rows

    // Create an concurrent unordered map so that insertion is thread-safe and use the index of a 
    // duplicate row/column as the key, and the row/column of which it is a duplicate as the value
    concurrent_umap duplicates;
   
    // Set the number of checks for each comparison, total number of comparisons, stride and the
    // total number of threads to use depending on if column or row duplicates are being checked
    const size_t checks         = Checker::type == check::ROWS ? _cols : _rows;
    const size_t comparisons    = Checker::type == check::ROWS ? _rows : _cols;
    const size_t stride         = _cols;    
    const size_t threads_to_use = Cores > comparisons ? comparisons : Cores;

    // Start conditions for each of the threads which comapre other indices which the index assigned 
    // to them. Say thread 2 needs to find all duplicates of row 2, then thread 0 and 1 need to have 
    // compared to row 2 before row 2 can compare with rows 3->N, which allows duplicates to be skipped
    std::vector<tbb::atomic<short>> start_conditions(comparisons);
    
    // Linear index for mapping which allows an unordered map with keys as the index values to be used,
    // where the values are then a pair with the non-linear index as the first value and the multiplicity
    // as the other
    tbb::atomic<short> map_size = 0;
    
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, threads_to_use),
        [&](const tbb::blocked_range<size_t>& thread_indices)
        {
            for (size_t idx = thread_indices.begin(); idx != thread_indices.end(); ++idx) {
                size_t thread_iters = ops::get_thread_iterations(idx, comparisons, threads_to_use);
                for (size_t it = 0; it < thread_iters; ++it) {
                    size_t current_idx      = ops::thread_map(idx, threads_to_use, it);
                    size_t multiplicity     = 1;
                    
                    // Wait for the start condition to be set for this thread
                    while (current_idx > 0  && start_conditions[current_idx] < current_idx) {}
                    
                    // Check if this thread is a duplicate (already been found by another thread)
                    if (duplicates.find(current_idx) == duplicates.end()) {
                        // Set the index in the parameter map, which will have indices from 0 -> num non dups
                        // rather than 0 -> total with dups, so we can then iterate over the non dup rows/cols
                        short map_index = map_size; map_size.fetch_and_add(1);
                        size_t duplicates_found = 1;
                        
                        // Not a duplicate so we must look for duplicates for each of the remaining rows/cols
                        for (size_t other_idx = current_idx + 1; other_idx < comparisons; ++other_idx) {
                            // Check if the rows/columns are equal
                            if (checker(&_data[0], current_idx, other_idx, checks, stride)) {
                                // Add that other_idx (a row or column ahead of current_idx) 
                                // is a duplicate of current_idx, and use other_idx as a key
                                // so that when it comes to that idx, no work is done
                                duplicates[other_idx] = current_idx;
                                // Add to the multiplicity of the current_idx as a duplicate was found.
                                // Use a temp variable since it's almost definitely faster to write to
                                // the concurrent container once at the end rather than on each iteration
                                ++multiplicity;
                                // Since other_idx will not check through the threads ahead of it because 
                                // it is a duplicate, increment duplicates_found to tell all other threads
                                // that this duplicate thread has done all it's comparisons
                                start_conditions[other_idx] += duplicates_found++;
                            } else {
                                // Set that idx has checked other_idx so that the thread that will do the
                                // comparisons for indexes against other_idx can start
                                start_conditions[other_idx] += duplicates_found;
                            }
                        }
                        
                        // Add the parameters (multiplicity and index of the row/column) to the appropriate
                        // map
                        // Create a pair with for the row parameter with the first value as the row index
                        // and teh second value as the multiplicity of the row
                        auto param = std::make_pair(static_cast<short>(current_idx), 
                                                    static_cast<short>(multiplicity));
                        // Add the parameter to the appropriate container
                        Checker::type == check::ROWS 
                                       ? _row_params[map_index] = param
                                       : _col_params[map_index] = param;
                        
                    }  
                }   // End iterations for specific thread
            }       // End all thread instances
        }           // End parallel lambda
    );
}

}               // End namespace haplo
#endif          // PARAHAPLO_USPLIT_BLOCK_IMPLEMENTATION_CPU_HPP