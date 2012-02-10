/*****************************************************************************
 * FlatAccess
 *
 * This file tests whether flat access is faster than other modes of access.
 */

#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

using namespace std;

/* Branchless selector */
// if a >= 1, return x, else y
int32_t isel( int32_t a, int32_t x, int32_t y )
{
    int mask = (a - 1) >> 31; // arithmetic shift right, splat out the sign bit
    // mask is 0xFFFFFFFF if (a < 0) and 0x00 otherwise.
    return x + ((y - x) & mask);
};

// if a >= 1, return x, else y
intptr_t isel_ptr( intptr_t a, intptr_t x, intptr_t y )
{
    int mask = (a - 1) >> sizeof(intptr_t); // arithmetic shift right, splat out the sign bit
    // mask is 0xFFFFFFFF if (a < 0) and 0x00 otherwise.
    return x + ((y - x) & mask);
};

class Tester
{
public:
    void beginTest(const string& name)
    {
        Test t;
        t.name = name;
        t.begin = clock();
        t.end = 0;
        _tests.push_back(t);
        cout << "Beginning " << name << " test..." << endl;
    }

    void endTest()
    {
        Test& t = _tests.back();
        t.end = clock();
        cout << "Ending " << t.name << " test." << endl;
    }

    void printResults()
    {
        cout << endl;
        cout << "RESULTS:" << endl;
        cout << "========" << endl << endl;

        size_t testsSize = _tests.size();
        Test* t = &_tests.front();
        for (size_t i = 0; i < testsSize; ++i, ++t)
        {
            cout << "Test " << t->name << ":" << endl;
            cout << "    Begin   :" << t->begin << endl;
            cout << "    End     :" << t->end << endl;
            cout << "    Duration:" << t->end - t->begin << endl << endl;
        }

        cout << "Type any string to continue..." << endl;
        string s;
        cin >> s;
    }

private:
    struct Test
    {
        string name;
        clock_t begin;
        clock_t end;
    };
    vector<Test> _tests;
};

int main(int argc, char** argv)
{
    // Create big random pool of numbers
    const int poolSize = 100000000;
    vector<int> pool1, pool2, pool3;
    pool1.reserve(poolSize);
    pool2.reserve(poolSize);
    pool3.reserve(poolSize);
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < poolSize; ++i)
    {
        pool1.push_back(rand());
        pool2.push_back(rand());
        pool3.push_back(0);
    }

    Tester tester;
    tester.beginTest("Random Access Vector");
    for (size_t i = 0; i < poolSize; ++i)
    {
        pool3[i] = (((pool1[i] * pool2[i]) / 3) + 16) % 100;
    }
    tester.endTest();

    pool3.clear();
    pool3.reserve(poolSize);
    pool3.assign(poolSize, 0);

    tester.beginTest("Iterator Vector");
    for (auto p1 = pool1.begin(), end = pool1.end(), p2 = pool2.begin(), p3 = pool3.begin(); p1 != end; ++p1, ++p2, ++p3)
    {
        *p3 = (((*p1 * *p2) / 3) + 16) % 100;
    }
    tester.endTest();

    pool3.clear();
    pool3.reserve(poolSize);
    pool3.assign(poolSize, 0);

    tester.beginTest("Flat Access Vector");
    int* p1 = pool1.size() ? &pool1.front() : nullptr;
    int* p2 = pool2.size() ? &pool2.front() : nullptr;
    int* p3 = pool3.size() ? &pool3.front() : nullptr;
    for (size_t i = 0; i < poolSize; ++i, ++p1, ++p2, ++p3)
    {
        *p3 = (((*p1 * *p2)/ 3) + 16) % 100;
    }
    tester.endTest();

    tester.beginTest("Branch");
    p1 = pool1.size() ? &pool1.front() : nullptr;
    p2 = pool2.size() ? &pool2.front() : nullptr;
    p3 = pool3.size() ? &pool3.front() : nullptr;
    for (size_t i = 0; i < poolSize; ++i, ++p1, ++p2, ++p3)
    {
        if (*p1 < *p2)
        {
            *p3 = *p1;
        }
        else
        {
            *p3 = *p2;
        }
    }
    tester.endTest();

    tester.beginTest("isel");
    p1 = pool1.size() ? &pool1.front() : nullptr;
    p2 = pool2.size() ? &pool2.front() : nullptr;
    p3 = pool3.size() ? &pool3.front() : nullptr;
    for (size_t i = 0; i < poolSize; ++i, ++p1, ++p2, ++p3)
    {
        *p3 = isel(*p1 < *p2, *p1, *p2);
    }
    tester.endTest();

    // Selection tests
    size_t sum;
    tester.beginTest("Branch Select and Sum");
    p1 = pool1.size() ? &pool1.front() : nullptr;
    p2 = pool2.size() ? &pool2.front() : nullptr;
    p3 = pool3.size() ? &pool3.front() : nullptr;
    sum = 0;
    for (size_t i = 0; i < poolSize; ++i, ++p1, ++p2, ++p3)
    {
        if (*p1 < *p2)
        {
            sum += *p2;
        }
    }
    cout << sum << endl;
    tester.endTest();

    tester.beginTest("isel Select and Sum");
    p1 = pool1.size() ? &pool1.front() : nullptr;
    p2 = pool2.size() ? &pool2.front() : nullptr;
    p3 = pool3.size() ? &pool3.front() : nullptr;
    sum = 0;
    for (size_t i = 0; i < poolSize; ++i, ++p1, ++p2, ++p3)
    {
        sum += isel(*p1 < *p2, *p2, 0);
    }
    cout << sum << endl;
    tester.endTest();

    tester.printResults();

    return 0;
}
