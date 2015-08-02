#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <forward_list>
#include <memory>
#include <algorithm>
#include <chrono>
#include <Windows.h>
#include <Psapi.h>

using namespace std;


const char* c_WordsFileName = R"(C:\Users\Rahul\Downloads\words.txt)";


size_t
GetProcessMemUsageInBytes ()
{
    PROCESS_MEMORY_COUNTERS_EX memCounters;
    BOOL retVal = GetProcessMemoryInfo(GetCurrentProcess(),
                                       reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&memCounters),
                                       sizeof(memCounters));
    if (!retVal)
    {
        cout << "GetProcessMemoryInfo failed - 0x" << hex << GetLastError() << endl;
        return 0;
    }

    return memCounters.PrivateUsage;
}


struct TrieNode
{
    TrieNode ()
        : m_IsWordEnd(false)
    {
    }

    bool m_IsWordEnd;
    forward_list<pair<char, unique_ptr<TrieNode>>> m_Edges;
};


class Trie
{
public:

    void
    InsertWord (
        const string& Word
        );

    bool
    ContainsWord (
        const string& Word
        ) const;

private:

    TrieNode*
    Traverse (
        const string& Word,
        bool AddNodes
        );

    TrieNode m_Root;
};


TrieNode*
Trie::Traverse (
    const string& Word,
    bool AddNodes
    )
{
    TrieNode* node = &m_Root;

    for (char ch : Word)
    {
        auto edgeIt = find_if(node->m_Edges.cbegin(),
                              node->m_Edges.cend(),
                              [ch](const auto& Edge)
                              {
                                  return Edge.first == ch;
                              });

        if (edgeIt == node->m_Edges.cend())
        {
            if (!AddNodes)
            {
                return nullptr;
            }

            node->m_Edges.emplace_front(make_pair(ch, make_unique<TrieNode>()));
            edgeIt = node->m_Edges.cbegin();
        }

        node = edgeIt->second.get();
    }

    return node;
}


void
Trie::InsertWord (
    const string& Word
    )
{
    TrieNode* node = Traverse(Word, true);
    node->m_IsWordEnd = true;
}


bool
Trie::ContainsWord (
    const string& Word
    ) const
{
    TrieNode* node = const_cast<Trie*>(this)->Traverse(Word, false);
    return (node && node->m_IsWordEnd);
}


Trie
CreateTrie ()
{
    Trie trie;

    ifstream fin(c_WordsFileName);

    chrono::steady_clock::duration creationTime(0);

    string word;
    while (getline(fin, word))
    {
        auto start = chrono::steady_clock::now();

        trie.InsertWord(word);

        auto end = chrono::steady_clock::now();

        auto diff = end - start;
        creationTime += diff;
    }

    cout << "Time to create trie: " << chrono::duration_cast<chrono::milliseconds>(creationTime).count() << " ms" << endl;

    return trie;
}


void
TestLookup (
    const Trie& TheTrie
    )
{
    ifstream fin(c_WordsFileName);

    chrono::steady_clock::duration avgLookupTime(0);
    chrono::steady_clock::duration maxLookupTime(0);
    chrono::steady_clock::duration minLookupTime = (chrono::steady_clock::duration::max)();
    string maxWord;
    string minWord;
    uint32_t numWords = 0;

    string word;
    while (getline(fin, word))
    {
        ++numWords;

        auto start = chrono::steady_clock::now();

        bool foundWord = TheTrie.ContainsWord(word);

        auto end = chrono::steady_clock::now();

        if (!foundWord)
        {
            cout << "Could not find word \"" << word << "\"";
        }

        auto diff = end - start;

        if (diff > maxLookupTime)
        {
            maxLookupTime = diff;
            maxWord = word;
        }

        if (diff < minLookupTime)
        {
            minLookupTime = diff;
            minWord = word;
        }

        avgLookupTime += diff;
    }

    avgLookupTime /= numWords;

    cout << "Average lookup time: " << chrono::duration_cast<chrono::nanoseconds>(avgLookupTime).count() << " ns" << endl;
    cout << "Max lookup time: " << chrono::duration_cast<chrono::nanoseconds>(maxLookupTime).count() << " ns (" << maxWord << ")" << endl;
    cout << "Min lookup time: " << chrono::duration_cast<chrono::nanoseconds>(minLookupTime).count() << " ns (" << minWord << ")" << endl;
}


void
MeasureMemUsage ()
{
    size_t memUsageInBytes = GetProcessMemUsageInBytes();
    size_t memUsageInMB = memUsageInBytes / (1024 * 1024);

    cout << "Memory usage: " << memUsageInMB << " MB." << endl;
}


int
wmain ()
{
    Trie trie = CreateTrie();

    MeasureMemUsage();

    TestLookup(trie);

    return 0;
}
