#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;

mutex m;

string compress(const string& s) {
    string result = "";
    int i = 0;
    while (i < s.size()) {
        char c = s[i];
        int count = 1;
        while (i + count < s.size() && s[i + count] == c) {
            count++;
        }
        result += c + to_string(count);
        i += count;
    }
    return result;
}

string decompress(const string& s) {
    string result = "";
    for (int i = 0; i < s.size();) {
        char c = s[i++];
        string num = "";
        while (i < s.size() && isdigit(s[i])) {
            num += s[i++];
        }
        result += string(stoi(num), c);
    }
    return result;
}

void compressChunk(string chunk, string& output) {
    string compressed = compress(chunk);
    lock_guard<mutex> lock(m);
    output += compressed;
}

void decompressChunk(string chunk, string& output) {
    string decompressed = decompress(chunk);
    lock_guard<mutex> lock(m);
    output += decompressed;
}

vector<string> splitText(string text, int parts) {
    vector<string> chunks;
    int size = text.size() / parts;
    for (int i = 0; i < parts; i++) {
        int start = i * size;
        int end = (i == parts - 1) ? text.size() : (i + 1) * size;
        chunks.push_back(text.substr(start, end - start));
    }
    return chunks;
}

string readFile(string filename) {
    ifstream fin(filename);
    return string((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
}

void writeFile(string filename, string data) {
    ofstream fout(filename);
    fout << data;
}

int main() {
    string inputFile = "input.txt";
    string compressedFile = "compressed.txt";
    string decompressedFile = "decompressed.txt";
    int threadCount = 4;

    string text = readFile(inputFile);
    cout << "Text length: " << text.length() << "\n\n";

    auto t1 = chrono::high_resolution_clock::now();
    string compressedSingle = compress(text);
    auto t2 = chrono::high_resolution_clock::now();
    chrono::duration<double> singleTime = t2 - t1;

    string compressedMulti = "";
    vector<string> parts = splitText(text, threadCount);
    vector<thread> threads;

    auto t3 = chrono::high_resolution_clock::now();
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(compressChunk, parts[i], ref(compressedMulti));
    }
    for (auto& th : threads) th.join();
    auto t4 = chrono::high_resolution_clock::now();
    chrono::duration<double> multiTime = t4 - t3;

    writeFile(compressedFile, compressedMulti);

    cout << "--- Compression Report ---\n";
    cout << "Single-threaded:   " << singleTime.count() << " seconds\n";
    cout << "Multi-threaded:    " << multiTime.count() << " seconds\n\n";

    string toDecompress = readFile(compressedFile);
    string decompressedMulti = "";
    vector<string> compParts = splitText(toDecompress, threadCount);
    threads.clear();

    auto t5 = chrono::high_resolution_clock::now();
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(decompressChunk, compParts[i], ref(decompressedMulti));
    }
    for (auto& th : threads) th.join();
    auto t6 = chrono::high_resolution_clock::now();
    chrono::duration<double> decompressTime = t6 - t5;

    writeFile(decompressedFile, decompressedMulti);

    cout << "--- Decompression Report ---\n";
    cout << "Multi-threaded:    " << decompressTime.count() << " seconds\n";

    return 0;
}
