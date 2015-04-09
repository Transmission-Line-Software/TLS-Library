// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "base/polynomial.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

class PolynomialTest : public ::testing::Test {
 protected:
  PolynomialTest() {

    std::vector<double> coefficients(4, 0);
    coefficients.at(0) = 0;
    coefficients.at(1) = 1000;
    coefficients.at(2) = 0;
    coefficients.at(3) = -10;

    p_.set_coefficients(coefficients);
  }

  Polynomial p_;
};

TEST_F(PolynomialTest, Derivative) {

  // first derivative
  Polynomial d1 = p_.Derivative();
  std::vector<double> c1 = d1.coefficients();
  EXPECT_EQ(1000, c1.at(0));
  EXPECT_EQ(0, c1.at(1));
  EXPECT_EQ(-30, c1.at(2));

  // second derivative
  Polynomial d2 = d1.Derivative();
  std::vector<double> c2 = d2.coefficients();
  EXPECT_EQ(0, c2.at(0));
  EXPECT_EQ(-60, c2.at(1));
}

TEST_F(PolynomialTest, OrderMax) {

  // existing coefficients
  EXPECT_EQ(3, p_.OrderMax());

  // remove a coefficient
  std::vector<double> c = p_.coefficients();
  c.pop_back();
  p_.set_coefficients(c);
  EXPECT_EQ(2, p_.OrderMax());
}

TEST_F(PolynomialTest, Slope) {

  Polynomial d = p_.Derivative();
  const double x = 1;
  EXPECT_EQ(d.Y(x), p_.Slope(x));
}

TEST_F(PolynomialTest, X) {

  EXPECT_EQ(-1, std::round(p_.X(-990, 3, 0)));
  EXPECT_EQ(0, std::round(p_.X(0, 3, 0)));
  EXPECT_EQ(1, std::round(p_.X(990, 3, 0)));
}

TEST_F(PolynomialTest, Y) {

  EXPECT_EQ(-990, std::round(p_.Y(-1)));
  EXPECT_EQ(0, std::round(p_.Y(0)));
  EXPECT_EQ(990, std::round(p_.Y(1)));
}