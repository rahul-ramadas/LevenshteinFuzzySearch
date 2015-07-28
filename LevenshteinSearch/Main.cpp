#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

using namespace std;


const char* c_WordsFileName = R"(C:\Users\Rahul\Downloads\words.txt)";


int
wmain ()
{
    ifstream fin(c_WordsFileName);

    string word;
    uint32_t wordCount = 0;
    while (getline(fin, word))
    {
        ++wordCount;
    }

    cout << wordCount << endl;

    return 0;
}
