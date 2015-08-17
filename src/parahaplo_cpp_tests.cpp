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
    
    BOOST_CHECK( data_zero.value() == 0 );
    BOOST_CHECK( data_one.value()  == 1 );
    BOOST_CHECK( data_dash.value() == 2 );
}

BOOST_AUTO_TEST_CASE( canCreateDataTypeAndGetValueForIncorrectInput )
{
    haplo::Data data_wrong_1('b');
    haplo::Data data_wrong_2('z');
    
    BOOST_CHECK( data_wrong_1.value() == 3 );
    BOOST_CHECK( data_wring_2.value() == 3 );
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------- BLOCK TESTS -----------------------------------------------------
// 
BOOST_AUTO_TEST_SUITE( BlockTestSuite )
    
BOOST_AUTO_TEST_CASE( canCreateABlockOfAnyTypeAnSize )
{
    haplo::Block<float, 3, 4>    block_34;
    haplo::Block<int  , 9, 9>    block_99;
    
    BOOST_CHECK( block_34.size() == 12 );
    BOOST_CHECK( block_99.size() == 81 );
}

BOOST_AUTO_TEST_CASE( canAssignAndGetBlockData )
{
    haplo::Block<int, 2, 2>     block_22;
    int                         data[4] = {1, 2, 3, 4};
    
    // Assign data - static assert for dimensions mismatch
    block_22.assign_data(data);
    
    // Get a reference to the data
    const std::vector<int>& block_22_data = block_22.get_data();
    
    BOOST_CHECK( block_22_data[0] == 1 );
    BOOST_CHECK( block_22_data[1] == 2 );
    BOOST_CHECK( block_22_data[2] == 3 );
    BOOST_CHECK( block_22_data[3] == 4 );
}

BOOST_AUTO_TEST_CASE( canCreateBlockFromInputFile ) 
{   
    const std::string input_file = "test_input_file.txt";
    haplo::Block<char, 3, 8> block_38(input_file, 24);

    const std::vector<char>& block_data = block_38.get_data();
    
    for (auto& input : block_data) {
        std::cout << input << " ";
    }
}

BOOST_AUTO_TEST_SUITE_END()
