// ----------------------------------------------------------------------------------------------------------
/// @file   parahaplo_cpp_tests.cpp
/// @brief  Test suites for the c++ implementation of the parahaplo library using Bost.Unit.
// ----------------------------------------------------------------------------------------------------------

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE       ParahaploCppTests
#include <boost/test/unit_test.hpp>

#include "cpp/data.hpp"
#include "cpp/block.hpp"

#include <iostream>

// ------------------------------------------ DATA TESTS ----------------------------------------------------

BOOST_AUTO_TEST_SUITE( DataTestSuite )

BOOST_AUTO_TEST_CASE( canCreateDataTypeAndGetValueForCorrectInput )
{
    haplo::Data data_zero('0');
    haplo::Data data_one( '1');
    haplo::Data data_dash('-');
    std::cout << sizeof(data_one) << "\n";
    
    BOOST_CHECK( data_zero.value() == 0 );
    BOOST_CHECK( data_one.value()  == 1 );
    BOOST_CHECK( data_dash.value() == 2 );
}

BOOST_AUTO_TEST_CASE( canCreateDataTypeAndGetValueForIncorrectInput )
{
    haplo::Data data_wrong_1('b');
    haplo::Data data_wrong_2('z');
    
    BOOST_CHECK( data_wrong_1.value() == 3 );
    BOOST_CHECK( data_wrong_2.value() == 3 );
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------- BLOCK TESTS -----------------------------------------------------
// 
BOOST_AUTO_TEST_SUITE( BlockTestSuite )
    
BOOST_AUTO_TEST_CASE( canCreateABlockOfAnyTypeAnSize )
{
    haplo::Block<3, 4>    block_34;
    haplo::Block<9, 9>    block_99;
    
    BOOST_CHECK( block_34.size() == 12 );
    BOOST_CHECK( block_99.size() == 81 );
}

BOOST_AUTO_TEST_CASE( canAssignAndGetBlockData )
{
    haplo::Block<2, 2>  block_22;
    haplo::Data         data[4] = {'0', '-', '1', '1'};
    
    // Assign data - static assert for dimensions mismatch
    block_22.assign_data(data);
    
    // Get a reference to the data
    const std::vector<haplo::Data>& block_22_data = block_22.get_data();
    
    BOOST_CHECK( block_22_data[0] == 0 );
    BOOST_CHECK( block_22_data[1] == 2 );
    BOOST_CHECK( block_22_data[2] == 1 );
    BOOST_CHECK( block_22_data[3] == 1 );
}

BOOST_AUTO_TEST_CASE( canCreateBlockFromInputFile ) 
{ 
    const std::string input_file = "data/test_input_file.txt";
    haplo::Block<10, 7> block_10_7(input_file, 8);         // Use 8 threads
    
    block_10_7.print();                                                           
    
    // Check random values
    BOOST_CHECK( block_10_7(0, 0) == 0 );
    BOOST_CHECK( block_10_7(0, 1) == 2 );
    BOOST_CHECK( block_10_7(1, 0) == 1 );
    BOOST_CHECK( block_10_7(1, 1) == 0 );
    BOOST_CHECK( block_10_7(2, 2) == 0 );
    BOOST_CHECK( block_10_7(5, 3) == 0 );
    BOOST_CHECK( block_10_7(8, 5) == 2 );
}

BOOST_AUTO_TEST_CASE( canGetReadInfoCorrectly )
{
    const std::string input_file = "data/test_input_file.txt";
    haplo::Block<10, 7> block_10_7(input_file, 8);         // Use 8 threads
    
    // Get the read info of each of the reads
    block_10_7.get_read_info(8);                            // Also use 8 threads
    
    for (auto& read : block_10_7._read_info) {
        std::cout << read.start() << " : " << read.end() << " : " << read.length() << "\n";
    }

    // Check the splittable columns
    block_10_7.find_unsplittable_columns(4);
    
    for (int i = 0; i < block_10_7._column_info.size(); ++i) {
        std::cout << block_10_7._column_info[i] << "\n";
    }
}

BOOST_AUTO_TEST_SUITE_END()
