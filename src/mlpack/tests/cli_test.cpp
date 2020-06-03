/**
 * @file cli_test.cpp
 * @author Matthew Amidon, Ryan Curtin
 *
 * Test for the CMD input parameter system.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>
// We'll use CMDOptions.
#include <mlpack/bindings/cli/cli_option.hpp>

namespace mlpack {
namespace util {

template<typename T>
using Option = mlpack::bindings::cmd::CMDOption<T>;

} // namespace util
} // namespace mlpack

static const std::string testName = "";

#include <mlpack/core/util/param.hpp>
#include <mlpack/bindings/cli/parse_command_line.hpp>
#include <mlpack/bindings/cli/end_program.hpp>

#include <boost/test/unit_test.hpp>
#include "test_tools.hpp"

using namespace mlpack;
using namespace mlpack::util;
using namespace mlpack::kernel;
using namespace mlpack::data;
using namespace mlpack::bindings::cmd;
using namespace std;

// When we run these tests, we have to nuke the existing CMD object that's
// created by default.
struct CMDTestDestroyer
{
  CMDTestDestroyer() { CMD::ClearSettings(); }
};

BOOST_FIXTURE_TEST_SUITE(CMDTest, CMDTestDestroyer);

/**
 * Before running a test that uses the CMD options, we have to add the default
 * options that are required for CMD to function, since it will be destroyed at
 * the end of every test that uses CMD in this test suite.
 */
void AddRequiredCMDOptions()
{
  CMD::ClearSettings();

  // These will register with CMD immediately.
  CMDOption<bool> help(false, "help", "Default help info.", "h", "bool");
  CMDOption<string> info("", "info", "Get help on a specific module or option.",
      "", "string");
  CMDOption<bool> verbose(false, "verbose", "Display information messages and "
      "the full list of parameters and timers at the end of execution.", "v",
      "bool");
  CMDOption<bool> version(false, "version", "Display the version of mlpack.",
      "V", "bool");
}

/**
 * Tests that CMD works as intended, namely that CMD::Add propagates
 * successfully.
 */
BOOST_AUTO_TEST_CASE(TestCMDAdd)
{
  AddRequiredCMDOptions();

  // Check that the CMD::HasParam returns false if no value has been specified
  // on the commandline and ignores any programmatical assignments.
  CMDOption<bool> b(false, "global/bool", "True or false.", "a", "bool");

  // CMD::HasParam should return false here.
  BOOST_REQUIRE(!CMD::HasParam("global/bool"));

  // Check that our aliasing works.
  BOOST_REQUIRE_EQUAL(CMD::HasParam("global/bool"),
      CMD::HasParam("a"));
  BOOST_REQUIRE_EQUAL(CMD::GetParam<bool>("global/bool"),
      CMD::GetParam<bool>("a"));
}

/**
 * Tests that the various PARAM_* macros work properly.
 */
BOOST_AUTO_TEST_CASE(TestOption)
{
  AddRequiredCMDOptions();

  // This test will involve creating an option, and making sure CMD reflects
  // this.
  PARAM_IN(int, "test_parent/test", "test desc", "", 42, false);

  BOOST_REQUIRE_EQUAL(CMD::GetParam<int>("test_parent/test"), 42);
}

/**
 * Test that duplicate flags are filtered out correctly.
 */
BOOST_AUTO_TEST_CASE(TestDuplicateFlag)
{
  AddRequiredCMDOptions();

  PARAM_FLAG("test", "test", "t");

  int argc = 3;
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--test";
  argv[2] = "--test";

  // This should not throw an exception.
  BOOST_REQUIRE_NO_THROW(
      ParseCommandLine(argc, const_cast<char**>(argv)));
}

/**
 * Test that duplicate options throw an exception.
 */
BOOST_AUTO_TEST_CASE(TestDuplicateParam)
{
  AddRequiredCMDOptions();

  int argc = 5;
  const char* argv[5];
  argv[0] = "./test";
  argv[1] = "--info";
  argv[2] = "test1";
  argv[3] = "--info";
  argv[4] = "test2";

  // This should throw an exception.
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(ParseCommandLine(argc, const_cast<char**>(argv)),
      runtime_error);
  Log::Fatal.ignoreInput = false;
}

/**
 * Ensure that a Boolean option which we define is set correctly.
 */
BOOST_AUTO_TEST_CASE(TestBooleanOption)
{
  AddRequiredCMDOptions();

  PARAM_FLAG("flag_test", "flag test description", "");

  BOOST_REQUIRE_EQUAL(CMD::HasParam("flag_test"), false);

  // Now check that CMD reflects that it is false by default.
  BOOST_REQUIRE_EQUAL(CMD::GetParam<bool>("flag_test"), false);

  // Now, if we specify this flag, it should be true.
  int argc = 2;
  const char* argv[2];
  argv[0] = "programname";
  argv[1] = "--flag_test";

  ParseCommandLine(argc, const_cast<char**>(argv));

  BOOST_REQUIRE_EQUAL(CMD::GetParam<bool>("flag_test"), true);
  BOOST_REQUIRE_EQUAL(CMD::HasParam("flag_test"), true);
}

/**
 * Test that a vector option works correctly.
 */
BOOST_AUTO_TEST_CASE(TestVectorOption)
{
  AddRequiredCMDOptions();

  PARAM_VECTOR_IN(size_t, "test_vec", "test description", "t");

  int argc = 5;
  const char* argv[5];
  argv[0] = "./test";
  argv[1] = "--test_vec";
  argv[2] = "1";
  argv[3] = "2";
  argv[4] = "4";

  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  BOOST_REQUIRE(CMD::HasParam("test_vec"));

  vector<size_t> v = CMD::GetParam<vector<size_t>>("test_vec");

  BOOST_REQUIRE_EQUAL(v.size(), 3);
  BOOST_REQUIRE_EQUAL(v[0], 1);
  BOOST_REQUIRE_EQUAL(v[1], 2);
  BOOST_REQUIRE_EQUAL(v[2], 4);
}

/**
 * Test that we can use a vector option by specifying it many times.
 */
BOOST_AUTO_TEST_CASE(TestVectorOption2)
{
  AddRequiredCMDOptions();

  PARAM_VECTOR_IN(size_t, "test2_vec", "test description", "T");

  int argc = 7;
  const char* argv[7];
  argv[0] = "./test";
  argv[1] = "--test2_vec";
  argv[2] = "1";
  argv[3] = "--test2_vec";
  argv[4] = "2";
  argv[5] = "--test2_vec";
  argv[6] = "4";

//  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
//  Log::Fatal.ignoreInput = false;

  BOOST_REQUIRE(CMD::HasParam("test2_vec"));

  vector<size_t> v = CMD::GetParam<vector<size_t>>("test2_vec");

  BOOST_REQUIRE_EQUAL(v.size(), 3);
  BOOST_REQUIRE_EQUAL(v[0], 1);
  BOOST_REQUIRE_EQUAL(v[1], 2);
  BOOST_REQUIRE_EQUAL(v[2], 4);
}

BOOST_AUTO_TEST_CASE(InputColVectorParamTest)
{
  AddRequiredCMDOptions();

  PARAM_COL_IN("vector", "Test vector", "l");

  // Fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "iris_test_labels.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("vector"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("vector_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::vec vec1 = CMD::GetParam<arma::vec>("vector");
  arma::vec vec2 = CMD::GetParam<arma::vec>("vector");

  BOOST_REQUIRE_EQUAL(vec1.n_rows, 63);
  BOOST_REQUIRE_EQUAL(vec2.n_rows, 63);

  for (size_t i = 0; i < vec1.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(vec1[i], vec2[i], 1e-10);
}

BOOST_AUTO_TEST_CASE(InputUnsignedColVectorParamTest)
{
  AddRequiredCMDOptions();

  PARAM_UCOL_IN("vector", "Test vector", "l");

  // Fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "iris_test_labels.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("vector"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("vector_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::Col<size_t> vec1 = CMD::GetParam<arma::Col<size_t>>("vector");
  arma::Col<size_t> vec2 = CMD::GetParam<arma::Col<size_t>>("vector");

  BOOST_REQUIRE_EQUAL(vec1.n_rows, 63);
  BOOST_REQUIRE_EQUAL(vec2.n_rows, 63);

  for (size_t i = 0; i < vec1.n_elem; ++i)
    BOOST_REQUIRE_EQUAL(vec1[i], vec2[i]);
}

BOOST_AUTO_TEST_CASE(InputRowVectorParamTest)
{
  AddRequiredCMDOptions();

  PARAM_ROW_IN("row", "Test vector", "l");

  // Fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "testRes.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("row"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("row_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::rowvec vec1 = CMD::GetParam<arma::rowvec>("row");
  arma::rowvec vec2 = CMD::GetParam<arma::rowvec>("row");

  BOOST_REQUIRE_EQUAL(vec1.n_cols, 7);
  BOOST_REQUIRE_EQUAL(vec2.n_cols, 7);

  for (size_t i = 0; i < vec1.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(vec1[i], vec2[i], 1e-10);
}

BOOST_AUTO_TEST_CASE(InputUnsignedRowVectorParamTest)
{
  AddRequiredCMDOptions();

  PARAM_UROW_IN("row", "Test vector", "l");

  // Fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "testRes.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("row"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("row_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::Row<size_t> vec1 = CMD::GetParam<arma::Row<size_t>>("row");
  arma::Row<size_t> vec2 = CMD::GetParam<arma::Row<size_t>>("row");

  BOOST_REQUIRE_EQUAL(vec1.n_cols, 7);
  BOOST_REQUIRE_EQUAL(vec2.n_cols, 7);

  for (size_t i = 0; i < vec1.n_elem; ++i)
    BOOST_REQUIRE_EQUAL(vec1[i], vec2[i]);
}

BOOST_AUTO_TEST_CASE(OutputColParamTest)
{
  AddRequiredCMDOptions();

  // --vector is an output parameter.
  PARAM_COL_OUT("vector", "Test vector", "l");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("vector"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("vector_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::vec dataset = arma::randu<arma::vec>(100);
  CMD::GetParam<arma::vec>("vector") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the vector back and make sure it was saved correctly.
  arma::vec dataset2;
  data::Load("test.csv", dataset2);

  BOOST_REQUIRE_EQUAL(dataset.n_rows, dataset2.n_rows);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(OutputUnsignedColParamTest)
{
  AddRequiredCMDOptions();

  // --vector is an output parameter.
  PARAM_UCOL_OUT("vector", "Test vector", "l");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --vector parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("vector"));
  // The --vector_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("vector_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::Col<size_t> dataset = arma::randi<arma::Col<size_t>>(100);
  CMD::GetParam<arma::Col<size_t>>("vector") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the vector back and make sure it was saved correctly.
  arma::Col<size_t> dataset2;
  data::Load("test.csv", dataset2);

  BOOST_REQUIRE_EQUAL(dataset.n_rows, dataset2.n_rows);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_EQUAL(dataset[i], dataset2[i]);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(OutputRowParamTest)
{
  AddRequiredCMDOptions();

  // --row is an output parameter.
  PARAM_ROW_OUT("row", "Test vector", "l");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --row parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("row"));
  // The --row_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("row_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::rowvec dataset = arma::randu<arma::rowvec>(100);
  CMD::GetParam<arma::rowvec>("row") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the row vector back and make sure it was saved correctly.
  arma::rowvec dataset2;
  data::Load("test.csv", dataset2);

  BOOST_REQUIRE_EQUAL(dataset.n_cols, dataset2.n_cols);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(OutputUnsignedRowParamTest)
{
  AddRequiredCMDOptions();

  // --row is an output parameter.
  PARAM_UROW_OUT("row", "Test vector", "l");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-l";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --row parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("row"));
  // The --row_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("row_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::Row<size_t> dataset = arma::randi<arma::Row<size_t>>(100);
  CMD::GetParam<arma::Row<size_t>>("row") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the row vector back and make sure it was saved correctly.
  arma::Row<size_t> dataset2;
  data::Load("test.csv", dataset2);

  BOOST_REQUIRE_EQUAL(dataset.n_cols, dataset2.n_cols);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_EQUAL(dataset[i], dataset2[i]);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(InputMatrixParamTest)
{
  AddRequiredCMDOptions();

  // --matrix is an input parameter; it won't be transposed.
  PARAM_MATRIX_IN("matrix", "Test matrix", "m");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-m";
  argv[2] = "test_data_3_1000.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  Log::Fatal.ignoreInput = true;
  ParseCommandLine(argc, const_cast<char**>(argv));
  Log::Fatal.ignoreInput = false;

  // The --matrix parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("matrix"));
  // The --matrix_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("matrix_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::mat dataset = CMD::GetParam<arma::mat>("matrix");
  arma::mat dataset2 = CMD::GetParam<arma::mat>("matrix");

  BOOST_REQUIRE_EQUAL(dataset.n_rows, 3);
  BOOST_REQUIRE_EQUAL(dataset.n_cols, 1000);
  BOOST_REQUIRE_EQUAL(dataset2.n_rows, 3);
  BOOST_REQUIRE_EQUAL(dataset2.n_cols, 1000);

  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);
}

BOOST_AUTO_TEST_CASE(InputMatrixNoTransposeParamTest)
{
  AddRequiredCMDOptions();

  // --matrix is a non-transposed input parameter.
  PARAM_TMATRIX_IN("matrix", "Test matrix", "m");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--matrix_file";
  argv[2] = "test_data_3_1000.csv";

  int argc = 3;

  // The const-cast is a little hacky but should be fine...
  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --matrix parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("matrix"));
  // The --matrix_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("matrix_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  arma::mat dataset = CMD::GetParam<arma::mat>("matrix");
  arma::mat dataset2 = CMD::GetParam<arma::mat>("matrix");

  BOOST_REQUIRE_EQUAL(dataset.n_rows, 1000);
  BOOST_REQUIRE_EQUAL(dataset.n_cols, 3);
  BOOST_REQUIRE_EQUAL(dataset2.n_rows, 1000);
  BOOST_REQUIRE_EQUAL(dataset2.n_cols, 3);

  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);
}

BOOST_AUTO_TEST_CASE(OutputMatrixParamTest)
{
  AddRequiredCMDOptions();

  // --matrix is an output parameter.
  PARAM_MATRIX_OUT("matrix", "Test matrix", "m");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-m";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --matrix parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("matrix"));
  // The --matrix_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("matrix_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::mat dataset = arma::randu<arma::mat>(3, 100);
  CMD::GetParam<arma::mat>("matrix") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the matrix back and make sure it was saved correctly.
  arma::mat dataset2;
  data::Load("test.csv", dataset2);

  BOOST_REQUIRE_EQUAL(dataset.n_cols, dataset2.n_cols);
  BOOST_REQUIRE_EQUAL(dataset.n_rows, dataset2.n_rows);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(OutputMatrixNoTransposeParamTest)
{
  AddRequiredCMDOptions();

  // --matrix is an output parameter.
  PARAM_TMATRIX_OUT("matrix", "Test matrix", "m");

  // Set some fake arguments.
  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-m";
  argv[2] = "test.csv";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // The --matrix parameter should exist.
  BOOST_REQUIRE(CMD::HasParam("matrix"));
  // The --matrix_file parameter should not exist (it should be transparent from
  // inside the program).
  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(CMD::HasParam("matrix_file"), runtime_error);
  Log::Fatal.ignoreInput = false;

  // Since it's an output parameter, we don't need any input and don't need to
  // call ParseCommandLine().
  arma::mat dataset = arma::randu<arma::mat>(3, 100);
  CMD::GetParam<arma::mat>("matrix") = dataset;

  // Write the file.
  EndProgram();
  CMD::ClearSettings();
  AddRequiredCMDOptions();

  // Now load the matrix back and make sure it was saved correctly.
  arma::mat dataset2;
  data::Load("test.csv", dataset2, true, false);

  BOOST_REQUIRE_EQUAL(dataset.n_cols, dataset2.n_cols);
  BOOST_REQUIRE_EQUAL(dataset.n_rows, dataset2.n_rows);
  for (size_t i = 0; i < dataset.n_elem; ++i)
    BOOST_REQUIRE_CLOSE(dataset[i], dataset2[i], 1e-10);

  // Remove the file.
  remove("test.csv");
}

BOOST_AUTO_TEST_CASE(IntParamTest)
{
  AddRequiredCMDOptions();

  PARAM_INT_IN("int", "Test int", "i", 0);

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "-i";
  argv[2] = "3";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  BOOST_REQUIRE(CMD::HasParam("int"));
  BOOST_REQUIRE_EQUAL(CMD::GetParam<int>("int"), 3);
}

BOOST_AUTO_TEST_CASE(StringParamTest)
{
  AddRequiredCMDOptions();

  PARAM_STRING_IN("string", "Test string", "s", "");

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--string";
  argv[2] = "3";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  BOOST_REQUIRE(CMD::HasParam("string"));
  BOOST_REQUIRE_EQUAL(CMD::GetParam<string>("string"), string("3"));
}

BOOST_AUTO_TEST_CASE(DoubleParamTest)
{
  AddRequiredCMDOptions();

  PARAM_DOUBLE_IN("double", "Test double", "d", 0.0);

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--double";
  argv[2] = "3.12";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  BOOST_REQUIRE(CMD::HasParam("double"));
  BOOST_REQUIRE_CLOSE(CMD::GetParam<double>("double"), 3.12, 1e-10);
}

BOOST_AUTO_TEST_CASE(RequiredOptionTest)
{
  AddRequiredCMDOptions();

  PARAM_DOUBLE_IN_REQ("double", "Required test double", "d");

  const char* argv[1];
  argv[0] = "./test";

  int argc = 1;

  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(ParseCommandLine(argc, const_cast<char**>(argv)),
      runtime_error);
  Log::Fatal.ignoreInput = false;
}

BOOST_AUTO_TEST_CASE(UnknownOptionTest)
{
  AddRequiredCMDOptions();

  const char* argv[2];
  argv[0] = "./test";
  argv[1] = "--unknown";

  int argc = 2;

  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(ParseCommandLine(argc, const_cast<char**>(argv)),
      runtime_error);
  Log::Fatal.ignoreInput = false;
}

/**
 * Test that GetPrintableParam() works.
 */
BOOST_AUTO_TEST_CASE(UnmappedParamTest)
{
  AddRequiredCMDOptions();

  PARAM_MATRIX_IN("matrix", "Test matrix", "m");
  PARAM_MATRIX_OUT("matrix2", "Test matrix", "M");
  PARAM_MODEL_IN(GaussianKernel, "kernel", "Test kernel", "k");
  PARAM_MODEL_OUT(GaussianKernel, "kernel2", "Test kernel", "K");

  const char* argv[9];
  argv[0] = "./test";
  argv[1] = "--matrix_file";
  argv[2] = "test_data_3_1000.csv";
  argv[3] = "-M";
  argv[4] = "file2.csv";
  argv[5] = "-k";
  argv[6] = "kernel.txt";
  argv[7] = "-K";
  argv[8] = "kernel2.txt";

  int argc = 9;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Now check that we can get unmapped parameters.
  BOOST_REQUIRE_EQUAL(CMD::GetPrintableParam<arma::mat>("matrix"),
      "'test_data_3_1000.csv' (3x1000 matrix)");
  // This will have size 0x0 since it's an output parameter, and it hasn't been
  // set since ParseCommandLine() was called.
  BOOST_REQUIRE_EQUAL(CMD::GetPrintableParam<arma::mat>("matrix2"),
      "'file2.csv' (0x0 matrix)");
  BOOST_REQUIRE_EQUAL(CMD::GetPrintableParam<GaussianKernel*>("kernel"),
      "kernel.txt");
  BOOST_REQUIRE_EQUAL(CMD::GetPrintableParam<GaussianKernel*>("kernel2"),
      "kernel2.txt");

  remove("kernel.txt");
}

/**
 * Test that we can serialize a model and then deserialize it through the CMD
 * interface.
 */
BOOST_AUTO_TEST_CASE(SerializationTest)
{
  AddRequiredCMDOptions();

  PARAM_MODEL_OUT(GaussianKernel, "kernel", "Test kernel", "k");

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--kernel_file";
  argv[2] = "kernel.txt";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Create the kernel we'll save.
  GaussianKernel* gk = new GaussianKernel(0.5);

  CMD::GetParam<GaussianKernel*>("kernel") = gk;

  // Save it.
  EndProgram();
  CMD::ClearSettings();

  // Now create a new CMD object and load it.
  AddRequiredCMDOptions();

  PARAM_MODEL_IN(GaussianKernel, "kernel", "Test kernel", "k");

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Load the kernel from file.
  GaussianKernel* gk2 = CMD::GetParam<GaussianKernel*>("kernel");

  BOOST_REQUIRE_CLOSE(gk2->Bandwidth(), 0.5, 1e-5);

  // Clean up the memory...
  delete gk2;

  // Now remove the file we made.
  remove("kernel.txt");
}

/**
 * Test that an exception is thrown when a required model is not specified.
 */
BOOST_AUTO_TEST_CASE(RequiredModelTest)
{
  AddRequiredCMDOptions();

  PARAM_MODEL_IN_REQ(GaussianKernel, "kernel", "Test kernel", "k");

  // Don't specify any input parameters.
  const char* argv[1];
  argv[0] = "./test";

  int argc = 1;

  Log::Fatal.ignoreInput = true;
  BOOST_REQUIRE_THROW(ParseCommandLine(argc, const_cast<char**>(argv)),
      runtime_error);
  Log::Fatal.ignoreInput = false;
}

/**
 * Test that we can load both a dataset and its associated info.
 */
BOOST_AUTO_TEST_CASE(MatrixAndDatasetInfoTest)
{
  AddRequiredCMDOptions();

  // Write test file to load.
  fstream f;
  f.open("test.arff", fstream::out);
  f << "@relation test" << endl;
  f << endl;
  f << "@attribute one STRING" << endl;
  f << "@attribute two REAL" << endl;
  f << endl;
  f << "@attribute three STRING" << endl;
  f << endl;
  f << "%% a comment line " << endl;
  f << endl;
  f << "@data" << endl;
  f << "hello, 1, moo" << endl;
  f << "cheese, 2.34, goodbye" << endl;
  f << "seven, 1.03e+5, moo" << endl;
  f << "hello, -1.3, goodbye" << endl;
  f.close();

  // Add options.
  typedef tuple<DatasetInfo, arma::mat> TupleType;
  PARAM_MATRIX_AND_INFO_IN("dataset", "Test dataset", "d");

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--dataset_file";
  argv[2] = "test.arff";

  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Get the dataset and info.
  DatasetInfo info = move(get<0>(CMD::GetParam<TupleType>("dataset")));
  arma::mat dataset = move(get<1>(CMD::GetParam<TupleType>("dataset")));

  BOOST_REQUIRE_EQUAL(info.Dimensionality(), 3);

  BOOST_REQUIRE(info.Type(0) == Datatype::categorical);
  BOOST_REQUIRE_EQUAL(info.NumMappings(0), 3);
  BOOST_REQUIRE(info.Type(1) == Datatype::numeric);
  BOOST_REQUIRE(info.Type(2) == Datatype::categorical);
  BOOST_REQUIRE_EQUAL(info.NumMappings(2), 2);

  BOOST_REQUIRE_EQUAL(dataset.n_rows, 3);
  BOOST_REQUIRE_EQUAL(dataset.n_cols, 4);

  // The first dimension must all be different (except the ones that are the
  // same).
  BOOST_REQUIRE_EQUAL(dataset(0, 0), dataset(0, 3));
  BOOST_REQUIRE_NE(dataset(0, 0), dataset(0, 1));
  BOOST_REQUIRE_NE(dataset(0, 1), dataset(0, 2));
  BOOST_REQUIRE_NE(dataset(0, 2), dataset(0, 0));

  BOOST_REQUIRE_CLOSE(dataset(1, 0), 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 1), 2.34, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 2), 1.03e5, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 3), -1.3, 1e-5);

  BOOST_REQUIRE_EQUAL(dataset(2, 0), dataset(2, 2));
  BOOST_REQUIRE_EQUAL(dataset(2, 1), dataset(2, 3));
  BOOST_REQUIRE_NE(dataset(2, 0), dataset(2, 1));

  remove("test.arff");
}

/**
 * Test that we can access a parameter before we load it.
 */
BOOST_AUTO_TEST_CASE(RawIntegralParameter)
{
  AddRequiredCMDOptions();

  PARAM_DOUBLE_IN("double", "Test double", "d", 0.0);

  const char* argv[1];
  argv[0] = "./test";
  int argc = 1;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Set the double.
  CMD::GetRawParam<double>("double") = 3.0;

  // Now when we get it, it should be what we just set it to.
  BOOST_REQUIRE_CLOSE(CMD::GetParam<double>("double"), 3.0, 1e-5);
}

/**
 * Test that we can load a dataset with a pre-set mapping through
 * CMD::GetRawParam().
 */
BOOST_AUTO_TEST_CASE(RawDatasetInfoLoadParameter)
{
  AddRequiredCMDOptions();

  // Create the ARFF that we will read.
  fstream f;
  f.open("test.arff", fstream::out);
  f << "@relation test" << endl;
  f << endl;
  f << "@attribute one STRING" << endl;
  f << "@attribute two REAL" << endl;
  f << endl;
  f << "@attribute three STRING" << endl;
  f << endl;
  f << "%% a comment line " << endl;
  f << endl;
  f << "@data" << endl;
  f << "hello, 1, moo" << endl;
  f << "cheese, 2.34, goodbye" << endl;
  f << "seven, 1.03e+5, moo" << endl;
  f << "hello, -1.3, goodbye" << endl;
  f.close();

  PARAM_MATRIX_AND_INFO_IN("tuple", "Test tuple", "t");

  const char* argv[3];
  argv[0] = "./test";
  argv[1] = "--tuple_file";
  argv[2] = "test.arff";
  int argc = 3;

  ParseCommandLine(argc, const_cast<char**>(argv));

  // Create a pre-filled DatasetInfo object.
  DatasetInfo info(3);
  info.Type(0) = Datatype::categorical;
  info.Type(2) = Datatype::categorical;
  info.MapString<size_t>("seven", 0); // This will have mapped value 0.
  info.MapString<size_t>("cheese", 0); // This will have mapped value 1.
  info.MapString<size_t>("hello", 0); // This will have mapped value 2.
  info.MapString<size_t>("goodbye", 2); // This will have mapped value 0.
  info.MapString<size_t>("moo", 2); // This will have mapped value 1.

  // Now set the dataset info.
  std::get<0>(CMD::GetRawParam<tuple<DatasetInfo, arma::mat>>("tuple")) = info;

  // Now load the dataset.
  arma::mat dataset =
      std::get<1>(CMD::GetParam<tuple<DatasetInfo, arma::mat>>("tuple"));

  // Check the values.
  BOOST_REQUIRE_CLOSE(dataset(0, 0), 2.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 0), 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(2, 0), 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(0, 1), 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 1), 2.34, 1e-5);
  BOOST_REQUIRE_SMALL(dataset(2, 1), 1e-5);
  BOOST_REQUIRE_SMALL(dataset(0, 2), 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 2), 1.03e+5, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(2, 2), 1.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(0, 3), 2.0, 1e-5);
  BOOST_REQUIRE_CLOSE(dataset(1, 3), -1.3, 1e-5);
  BOOST_REQUIRE_SMALL(dataset(2, 3), 1e-5);

  remove("test.arff");
}

/**
 * Make sure typenames are properly stored.
 */
BOOST_AUTO_TEST_CASE(CppNameTest)
{
  AddRequiredCMDOptions();

  // Add a few parameters.
  PARAM_MATRIX_IN("matrix", "Test matrix", "m");
  PARAM_DOUBLE_IN("double", "Test double", "d", 0.0);

  // Check that the C++ typenames are right.
  BOOST_REQUIRE_EQUAL(CMD::Parameters().at("matrix").cppType, "arma::mat");
  BOOST_REQUIRE_EQUAL(CMD::Parameters().at("help").cppType, "bool");
  BOOST_REQUIRE_EQUAL(CMD::Parameters().at("double").cppType, "double");
}

BOOST_AUTO_TEST_SUITE_END();
