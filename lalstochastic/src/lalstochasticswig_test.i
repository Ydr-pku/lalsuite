// Tests of SWIG interface code
// Author: Karl Wette, 2011, 2012

// Include LAL test code.
#include <lal/lalswig_test.i>

// Test object parent tracking between modules
typedef struct taglalstochasticswig_test_parent_map {
  lalswig_test_struct s;
} lalstochasticswig_test_parent_map_struct;
lalstochasticswig_test_parent_map_struct lalstochasticswig_test_parent_map;