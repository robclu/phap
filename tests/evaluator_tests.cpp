// ----------------------------------------------------------------------------------------------------------
/// @file   evaluator_tests.cpp
/// @brief  Test suite for parahaplo evaluator class
// ----------------------------------------------------------------------------------------------------------

#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
    #define BOOST_TEST_MODULE EvaluatorTests
#endif
#include <boost/test/unit_test.hpp>
#include <iostream>

#include "../haplo/evaluator.hpp"

static constexpr char* input_ref = "input_files/eval_input_zero_ref.txt";
static constexpr char* input_sol = "input_files/eval_input_zero_sol.txt";
static constexpr char* ref_6     = "solutions/input_3_reference.txt";
static constexpr char* sol_6     = "solutions/input_3_soln.txt";

BOOST_AUTO_TEST_SUITE( EvaluatorSuite )
    
BOOST_AUTO_TEST_CASE( canCorrectlyEvaluateSolution )
{
    haplo::Evaluator evaluator;
 
    std::cout << "RR: " << evaluator(ref_6, sol_6) << "\n";
    
    BOOST_CHECK( evaluator(input_ref, input_sol) == 1 );
}

BOOST_AUTO_TEST_SUITE_END()