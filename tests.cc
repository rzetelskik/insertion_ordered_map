#include "insertion_ordered_map.h"

#define BOOST_TEST_MODULE tests

#include <iostream>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

struct cout_redirect {
    cout_redirect(std::streambuf *new_buffer) : old(std::cout.rdbuf(new_buffer)) {}

    ~cout_redirect() {
        std::cout.rdbuf(old);
    }

private:
    std::streambuf *old;
};

BOOST_AUTO_TEST_CASE(insert) {
    insertion_ordered_map<int, int> aaa;
    BOOST_CHECK(aaa.insert(4, 5) && (aaa.size() == 1));
    BOOST_CHECK(!aaa.insert(4, 5) && (aaa.size() == 1));
}

BOOST_AUTO_TEST_CASE(erase) {
    insertion_ordered_map<int, int> aaa;
    BOOST_CHECK(aaa.insert(4, 5) && (aaa.size() == 1));
    BOOST_CHECK_NO_THROW(aaa.erase(4));
    BOOST_CHECK(aaa.empty());
    BOOST_CHECK_THROW(aaa.erase(4), lookup_error);
    BOOST_CHECK_THROW(aaa.erase(5), lookup_error);
}

BOOST_AUTO_TEST_CASE(contains) {
    insertion_ordered_map<int, int> aaa;
    BOOST_CHECK(aaa.insert(4, 5) && (aaa.size() == 1));
    BOOST_CHECK(aaa.contains(4));
    BOOST_CHECK(!aaa.contains(5));
}

BOOST_AUTO_TEST_CASE(copy_constructor) {
    insertion_ordered_map<int, int> aaa;
    BOOST_CHECK(aaa.insert(4, 5) && (aaa.size() == 1));
    BOOST_CHECK(aaa.insert(5, 5) && (aaa.size() == 2));
    insertion_ordered_map<int, int> bbb(aaa);
    BOOST_CHECK(bbb.contains(4));
    BOOST_CHECK(bbb.contains(5));
}

BOOST_AUTO_TEST_CASE(at) {
    insertion_ordered_map<int, int> aaa;
    BOOST_CHECK(aaa.insert(4, 5) && (aaa.size() == 1));
    BOOST_CHECK(aaa.insert(5, 5) && (aaa.size() == 2));
    BOOST_CHECK(aaa.at(4) == 5);
    BOOST_CHECK(aaa.at(5) == 5);
}

BOOST_AUTO_TEST_SUITE(copy_on_write)

    BOOST_AUTO_TEST_CASE(basic) {
        insertion_ordered_map<int, int> aaa;
        BOOST_CHECK(aaa.insert(4, 5));
        BOOST_CHECK(aaa.insert(6, 5));
        BOOST_CHECK(aaa.size() == 2);
        insertion_ordered_map<int, int> bbb(aaa);
        insertion_ordered_map<int, int> ccc(aaa);
        BOOST_CHECK(aaa.insert(5, 1));
        BOOST_CHECK(aaa.contains(5));
        BOOST_CHECK(!bbb.contains(5));
        BOOST_CHECK(!ccc.contains(5));
    }

    BOOST_AUTO_TEST_CASE(list_iter_update) {
        insertion_ordered_map<int, int> aaa;
        BOOST_CHECK_NO_THROW(aaa.insert(4, 5));
        BOOST_CHECK_NO_THROW(aaa.insert(6, 5));
        BOOST_CHECK(aaa.at(4) == 5);
        BOOST_CHECK(aaa.at(6) == 5);
        insertion_ordered_map<int, int> bbb(aaa);
        aaa.clear();
        BOOST_CHECK_NO_THROW(bbb.at(4));
        BOOST_CHECK_NO_THROW(bbb.at(6));
        BOOST_CHECK_THROW(aaa.at(4), lookup_error);
        BOOST_CHECK_THROW(aaa.at(6), lookup_error);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(iterator)

BOOST_AUTO_TEST_CASE(iterator) {
    insertion_ordered_map<int, int> iom;
    insertion_ordered_map<int, int>::iterator it1;

    BOOST_CHECK(iom.insert(1, 2));
    BOOST_CHECK(iom.size() == 1);
    BOOST_CHECK(iom.insert(2, 3));
    BOOST_CHECK(iom.size() == 2);
    BOOST_CHECK(iom.insert(3, 4));
    BOOST_CHECK(iom.size() == 3);

    insertion_ordered_map<int, int>::iterator it2(iom.begin());

    insertion_ordered_map<int, int>::iterator it3(it1);

    it3 = iom.end();

    BOOST_CHECK(it2->first == 1);
    BOOST_CHECK(it2->second == 2);

    ++it2;
    BOOST_CHECK(it2->first == 2);
    BOOST_CHECK(it2->second == 3);

    it1 = it2;

    ++it2;
    BOOST_CHECK(it1->first == 2);
    BOOST_CHECK(it1->second == 3);

    BOOST_CHECK(it2->first == 3);
    BOOST_CHECK(it2->second == 4);

    ++it2;
    BOOST_CHECK(it2 == iom.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(indexing_operator)

BOOST_AUTO_TEST_CASE(indexing_operator) {
    insertion_ordered_map<int, std::string> iom;
    BOOST_CHECK(iom.insert(1, "abc"));
    BOOST_CHECK(iom.insert(2, "bcd"));
    BOOST_CHECK(iom.insert(3, "def"));
    BOOST_CHECK(iom[1] == "abc");
    BOOST_CHECK(iom[2] == "bcd");
    BOOST_CHECK(iom[4] == "");
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(assignment_operator)

BOOST_AUTO_TEST_CASE(assignmnent_operator_1) {
    insertion_ordered_map<int, int> iom1;
    insertion_ordered_map<int, int> iom2;
    BOOST_CHECK(iom1.insert(1, 2));
    BOOST_CHECK(iom1.insert(2, 3));
    iom2 = iom1;
    iom1.clear();
    BOOST_CHECK(iom2.contains(1));
    BOOST_CHECK(iom2.contains(2));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(merge)

BOOST_AUTO_TEST_CASE(merge1) {
    insertion_ordered_map<int, int> iom1;
    insertion_ordered_map<int, int> iom2;
    BOOST_CHECK(iom1.insert(1, 2));
    BOOST_CHECK(iom1.insert(2, 3));
    BOOST_CHECK(iom2.insert(1, 9));
    BOOST_CHECK(iom2.insert(3, 4));
    BOOST_CHECK_NO_THROW(iom1.merge(iom2));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(sfinae)

class Testowa {
private:
    int v;
public:
    Testowa() = delete;
    explicit Testowa(int a) { v = a; };
};

BOOST_AUTO_TEST_CASE(sfinae) {
    insertion_ordered_map<int, int> iom1;
    insertion_ordered_map<int, Testowa> iom2;
    // Should compile!
    BOOST_CHECK(iom1[3] == 0);
////     Should not compile!
//    BOOST_CHECK(iom2[1] == 0);
}

BOOST_AUTO_TEST_SUITE_END()

