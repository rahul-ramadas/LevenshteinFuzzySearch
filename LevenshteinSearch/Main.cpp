#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <forward_list>
#include <memory>
#include <algorithm>
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
        );

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
    )
{
    TrieNode* node = Traverse(Word, false);
    return (node && node->m_IsWordEnd);
}


int
wmain ()
{
    Trie trie;

    ifstream fin(c_WordsFileName);

    string word;
    while (getline(fin, word))
    {
        trie.InsertWord(word);
    }

    fin.clear();
    fin.seekg(0, ios::beg);

    while (getline(fin, word))
    {
        if (!trie.ContainsWord(word))
        {
            cout << "Could not find word \"" << word << "\"";
        }
    }

    size_t memUsageInBytes = GetProcessMemUsageInBytes();
    size_t memUsageInMB = memUsageInBytes / (1024 * 1024);

    cout << "Memory usage: " << memUsageInMB << " MB." << endl;

    return 0;
}
