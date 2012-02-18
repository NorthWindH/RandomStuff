#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <cstdint>

using namespace std;

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

int32_t isel( int32_t a, int32_t x, int32_t y )
{
    int mask = (a - 1) >> 31; // arithmetic shift right, splat out the sign bit
    // mask is 0xFFFFFFFF if (a < 0) and 0x00 otherwise.
    return x + ((y - x) & mask);
};

int main()
{
    size_t poolSize = 5000000;
    Tester tester;

    std::vector<unsigned char> srcA, srcB;
    uint32_t bAccum;

    // Create big random pool of numbers
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < poolSize; ++i)
    {
        srcA.push_back(rand() % 256);
        srcB.push_back(rand() % 256);
    }

    bAccum = 0;
    unsigned char* srcAPtr = &srcA.front();
    unsigned char* srcBPtr = &srcB.front();

    tester.beginTest("C Looped Branching Select And Sum");
    for (size_t i = 0; i < poolSize; ++i, ++srcBPtr, ++srcAPtr)
    {
        bAccum += (*srcBPtr > *srcAPtr) ? *srcBPtr : 0;
    }
    tester.endTest();

    cout << "bAccum: " << bAccum << endl;

    bAccum = 0;
    srcAPtr = &srcA.front();
    srcBPtr = &srcB.front();

    tester.beginTest("C Looped Branchless Select And Sum");
    for (size_t i = 0; i < poolSize; ++i, ++srcBPtr, ++srcAPtr)
    {
        bAccum += isel(*srcBPtr > *srcAPtr, *srcBPtr, 0);
    }
    tester.endTest();

    cout << "bAccum: " << bAccum << endl;

    bAccum = 0;
    srcAPtr = &srcA.front();
    srcBPtr = &srcB.front();

    tester.beginTest("Assembly Branching Select And Sum");
    __asm
    {
        mov esi, dword ptr [srcAPtr]
        mov edi, dword ptr [srcBPtr]
        mov ebx, -1
        mov ecx, [poolSize]
        xor eax, eax
        xor edx, edx
        cld
Lbl_Loop:   cmpsb
            jae Lbl_Skip
            mov dl, [edi][ebx]
            add eax, edx
Lbl_Skip:   loop Lbl_Loop
        mov [bAccum], eax
    }
    tester.endTest();

    cout << "bAccum: " << bAccum << endl;

    tester.printResults();

    return 0;
}
