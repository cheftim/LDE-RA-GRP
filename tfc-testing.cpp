#include <algorithm>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <map>
#include <regex>
#include <future>

#include "LDE-Matrix/pattern-matrix.hpp"
#include "LDE-Matrix/zmatrix.hpp"
#include "LDE-Matrix/data/patterns928.hpp"
#include "LDE-Matrix/pattern-deduper.hpp"

std::string TFC_OUT_DIR = "tfc-output";

std::map<int, std::vector<char>> caseSubcases = {
    {1, {'-'}},
    {2, {'-'}},
    {3, {'a', 'b', 'c'}},
    {4, {'a', 'b'}},
    {5, {'a', 'b'}},
    {6, {'a', 'b', 'c'}},
    {7, {'-'}},
    {8, {'a', 'b'}},
};

std::string matchedCasesDirectory = "tfc-output";
std::string matchedSubcasesDirectory = "tfc-output";
bool useNewEncoding = false;

std::string replace_all(
    const std::string & str ,   // where to work
    const std::string & find ,  // substitute 'find'
    const std::string & replace //      by 'replace'
) {
    using namespace std;
    string result;
    size_t find_len = find.size();
    size_t pos,from=0;
    while ( string::npos != ( pos=str.find(find,from) ) ) {
        result.append( str, from, pos-from );
        result.append( replace );
        from = pos + find_len;
    }
    result.append( str, from , string::npos );
    return result;
}

std::vector<patternMatrix> loadPatterns(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file:" << filename << std::endl;
        return {}; // Indicate error
    }
    std::vector<patternMatrix> patterns;
    std::string line;

    int lineNumber = 0;
    while (std::getline(file, line)) {
        // ignore comments
        if (line[0] == '#') {
            std::cout << "Ignoring comment: " << line << std::endl;
            continue;
        }
        patterns.push_back(patternMatrix(++lineNumber, line, useNewEncoding));
    }
    file.close();
    return patterns;
}

std::vector<patternMatrix> loadPatternsExtraBrackets(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file:" << filename << std::endl;
        return {}; // Indicate error
    }
    std::vector<patternMatrix> patterns;
    std::string line;

    int lineNumber = 0;
    while (std::getline(file, line)) {
        // ignore comments
        if (line[0] == '#') {
            std::cout << "Ignoring comment: " << line << std::endl;
            continue;
        }
        // Remove the leading and trailing brackets
        line = line.substr(1, line.size() - 2);
        // Replace spaces with commas
        std::replace(line.begin(), line.end(), ' ', ',');
        // Replace all instances of "],[" with "]["
        line = replace_all(line, "],[", "][");
        patterns.push_back(patternMatrix(++lineNumber, line, false));
        if (lineNumber % 1000 == 0) {
            std::cout << "Loaded " << lineNumber << " patterns" << std::endl;
        }
    }
    file.close();
    return patterns;
}

void case352AllPossible() {
    std::filesystem::create_directory(TFC_OUT_DIR);
    std::ofstream tfcout = std::ofstream(TFC_OUT_DIR + "/tfc-output.txt");
    std::ofstream tfcUniques = std::ofstream(TFC_OUT_DIR + "/tfc-uniques.txt");

    if (!tfcout.is_open()) {
        std::cerr << "Error opening file:" << TFC_OUT_DIR + "/tfc-output.txt" << std::endl;
        return;
    }

    if (!tfcUniques.is_open()) {
        std::cerr << "Error opening file:" << TFC_OUT_DIR + "/tfc-uniques.txt" << std::endl;
        return;
    }

    patternMatrix test = patternMatrix(352);
    test.multilineOutput = true;
    test.printDebugInfo = true;

    std::cout << "Before T-Gate multiplication " << test.printTGateOperations() << ":" << std::endl;
    std::cout << test << std::endl;

    test.rightTGateMultiply(1,4);
    test.rightTGateMultiply(2,3);

    std::cout << "After T-Gate multiplication " << test.printTGateOperations() << ":" << std::endl;
    std::cout << test << std::endl;

    // If you want to reduce and see the LDEs then see possible values, uncomment this block
    test.ldeReductionOnPattern(1);
    std::cout << "LDEs:" << std::endl;
    test.printLDEs(std::cout);
    std::cout << "Possible values:" << std::endl;
    test.printPossibleValues(std::cout);
    std::cout << "Max of possible values: " << test.getMaxOfPossibleValues() << std::endl;
    
    std::cout << "Starting to generate all possible patterns" << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    // This generates all possible value patterns and stores them in the old encoding scheme
    test.generateAllPossibleValuePatterns();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Time to generate all possible patterns: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
    std::cout << "Time to generate 1 possible pattern: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / test.allPossibleValuePatterns.size() << " microseconds" << std::endl;
    std::cout << "Number of possible value patterns: " << test.allPossibleValuePatterns.size() << std::endl;
    tfcout << "Time to generate all possible patterns: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
    tfcout << "Time to generate 1 possible pattern: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / test.allPossibleValuePatterns.size() << " microseconds" << std::endl;
    tfcout << "Number of possible value patterns: " << test.allPossibleValuePatterns.size() << std::endl;
    tfcout << "Deduping:" << std::endl;
    patternDeduper pd = patternDeduper();
    int newPatternID = 352000000;
    std::map<int, int> dupCount;
    for (auto pm : test.allPossibleValuePatterns) {
        int duplicateID = -1;
        // By default, these are in the old encoding but this could change :(
        patternMatrix pmCopy = patternMatrix(++newPatternID, pm.first);
        if (pd.isDuplicate(pmCopy, duplicateID, true)) {
            std::cout << pmCopy.id << " is a duplicate of " << duplicateID << std::endl;
            tfcout << pmCopy.id << " is a duplicate of " << duplicateID << std::endl;
            dupCount[duplicateID]++;
        } else {
            std::cout << pmCopy.id << " is unique" << std::endl;
            tfcout << pmCopy.id << " is unique" << std::endl;
            tfcUniques << pmCopy << std::endl;
        }
    }
    tfcout << "Duplicate Counts:" << std::endl;
    tfcUniques << "Duplicate Counts:" << std::endl;
    for (auto const& [id, count] : dupCount) {
        std::cout << "Duplicate ID: " << id << " Count: " << count << std::endl;
        tfcout << "Duplicate ID: " << id << " Count: " << count << std::endl;
        tfcUniques << "Duplicate ID: " << id << " Count: " << count << std::endl;
    }
    tfcout.close();
    tfcUniques.close();
    std::cout << "Done" << std::endl;
}

void case352SubCaseMatching() {
    std::filesystem::create_directory(matchedSubcasesDirectory);
    std::map<int, std::string> patternFiles = {
        // {1, matchedCasesDirectory + "/case1-matches-patterns928.txt"},  // No subcases
        // {2, matchedCasesDirectory + "/case2-matches-patterns928.txt"},  // No subcases
        //{3, matchedCasesDirectory + "/case3-matches-patterns928.txt"},
        {3, matchedCasesDirectory + "/case3-patterns-tfc.txt"},
        //{4, matchedCasesDirectory + "/case4-matches-patterns928.txt"},
        //{5, matchedCasesDirectory + "/case5-matches-patterns928.txt"},
        //{6, matchedCasesDirectory + "/case6-matches-patterns928.txt"},
        // {7, matchedCasesDirectory + "/case7-matches-patterns928.txt"},  // No subcases
        //{8, matchedCasesDirectory + "/case8-matches-patterns928.txt"},
        };
    for (auto const& pair : patternFiles) {
        int caseNumber = pair.first;
        std::string patternFile = pair.second;
        std::vector<patternMatrix> patterns = loadPatterns(patternFile);
        std::vector<std::ofstream> matchedCasesFiles;
        std::vector<std::ofstream> matchedCasesFilesHumanReadable;
        matchedCasesFiles.push_back(std::ofstream(matchedSubcasesDirectory+ "/case" + std::to_string(caseNumber) + "-no-subcase-match.txt"));
        matchedCasesFilesHumanReadable.push_back(std::ofstream(matchedSubcasesDirectory+ "/case" + std::to_string(caseNumber) + "-no-subcase-matches-human-readable.txt"));
        for (char subcase : caseSubcases[caseNumber]) {
            std::string filename = matchedSubcasesDirectory+ "/case" + std::to_string(caseNumber) + subcase + "-matches.txt";
            matchedCasesFiles.push_back(std::ofstream(filename));
            if (!matchedCasesFiles[matchedCasesFiles.size()-1].is_open()) {
                std::cerr << "Error opening file:" << filename << std::endl;
            }
            matchedCasesFiles[matchedCasesFiles.size()-1] << "# Using the new encoding: 2y + x" << std::endl;

            std::string filenameHumanReadable = matchedSubcasesDirectory+ "/case" + std::to_string(caseNumber) + subcase + "-matches-human-readable.txt";
            matchedCasesFilesHumanReadable.push_back(std::ofstream(filenameHumanReadable));
            if (!matchedCasesFilesHumanReadable[matchedCasesFilesHumanReadable.size()-1].is_open()) {
                std::cerr << "Error opening file:" << filenameHumanReadable << std::endl;
            }
            matchedCasesFilesHumanReadable[matchedCasesFilesHumanReadable.size()-1] << "# Using the new encoding: 2y + x" << std::endl;
        }

        for (patternMatrix pm : patterns) {
            std::cout << "Pattern " << pm.id << std::endl;
            pm.singleCaseRearrangement = true;
            pm.rearrangeMatrix();
            patternMatrix pm2 = patternMatrix(pm.id, pm.getFirstCaseRearrangement());
            pm2.updatePairCounts();
            if(!pm2.matchesCase(caseNumber)) {
                std::cout << "Pattern " << pm2.id << " does not match case " << caseNumber << " and is in the wrong file" << std::endl;
                std::cout << pm2 << std::endl;
                continue;
            }
            pm2.caseMatch = caseNumber;
            pm2.printID = true;
            
            if(pm2.determineSubCase()) {
                int index = pm2.subCaseMatch - 'a' + 1;
                matchedCasesFiles[index] << pm2 << std::endl;
                pm2.multilineOutput = true;
                matchedCasesFilesHumanReadable[index] << pm2 << std::endl;
            } else {
                std::cout << "Pattern " << pm2.id << " has no subcase matches." << std::endl;
                matchedCasesFiles[0] << pm2 << std::endl;
                pm2.multilineOutput = true;
                matchedCasesFilesHumanReadable[0] << pm2 << std::endl;
            
            }
        }
        for (std::ofstream& matchedCasesFile : matchedCasesFiles) matchedCasesFile.close();
        for (std::ofstream& matchedCasesFile : matchedCasesFilesHumanReadable) matchedCasesFile.close();
    }
}

void test352Possible() {
    patternMatrix test = patternMatrix(67,"[1,1,1,1,0,0][1,1,3,3,0,0][0,0,0,0,2,2][0,0,0,0,2,2][1,3,1,3,0,0][1,3,3,1,0,0]", true);
    test.multilineOutput = true;

    test.rearrangeMatrix();
    patternMatrix test2 = patternMatrix(67, test.getFirstCaseRearrangement());
    test2.multilineOutput = true;

    test2.printPairCounts(std::cout);
    
    
    std::cout << "Before T-Gate multiplication " << test2.printTGateOperations() << ":" << std::endl;
    std::cout << test2 << std::endl;

    // Here, you can mulitply the pattern with T-Gates.  Feel free to update as desired.
    //  You can use the following functions to multiply the pattern with T-Gates:
    //    test.rightTGateMultiply(column1, column2) - Multiply the pattern with T-Gate on the right
    //    test.leftTGateMultiply(row1, row2) - Multiply the pattern with T-Gate on the left
    test2.rightTGateMultiply(1,4);
    test2.rightTGateMultiply(2,3);

    std::cout << "After T-Gate multiplication " << test2.printTGateOperations() << ":" << std::endl;
    std::cout << test2 << std::endl;

    // If you want to reduce and see the LDEs then see possible values, uncomment this block
    test2.ldeReductionOnPattern(1);
    std::cout << "LDEs:" << std::endl;
    test2.printLDEs(std::cout);
    std::cout << "Possible values:" << std::endl;
    test2.printPossibleValues(std::cout);
    std::cout << "Max of possible values: " << test.getMaxOfPossibleValues() << std::endl;
}

void generateCaseSumMaps() {
    // int1 = case number, int2 = sum, int3 = id, string = pattern
    std::map <int, std::map <int, std::map <int, std::string>>> caseSumMap;
    for (int i = 1; i <=928; i++) {
        patternMatrix pm = patternMatrix(i);
        std::cout << pm.id << " " << pm << std::endl;
        pm.matchOnCases();
        caseSumMap[pm.caseMatch][pm.p.zSum][pm.id] = pm.toString();
    }
    
    std::filesystem::create_directory(TFC_OUT_DIR);
    std::ofstream tfcout = std::ofstream(TFC_OUT_DIR + "/case-sum-map.txt");
    if (!tfcout.is_open()) {
        std::cerr << "Error opening file:" << TFC_OUT_DIR + "/case-sum-map.txt" << std::endl;
        return;
    }
    for (auto const& [caseNumber, sumMap] : caseSumMap) {
        tfcout << "    {" << caseNumber << ", {" << std::endl;
        std::cout << "Case: " << caseNumber << std::endl;
        for (auto const& [sum, idMap] : sumMap) {
            tfcout << "        {" << sum << ", {" << std::endl;
            std::cout << "Sum: " << sum << std::endl;
            for (auto const& [id, pattern] : idMap) {
                tfcout << "            {" << id << ", \"" << pattern << "\"}," << std::endl;
            }
            tfcout << "        }}," << std::endl;
        }
        tfcout << "    }}," << std::endl;
    }
}

void outputCaseSumMap() {
    for (auto const& [caseNumber, sumMap] : CASE_SUM_MAP_PATTERNS_928) {
        std::cout << "Case: " << caseNumber << std::endl;
        for (auto const& [sum, idMap] : sumMap) {
            std::cout << "Sum: " << sum << std::endl;
            for (auto const& [id, pattern] : idMap) {
                std::cout << "ID: " << id << " Pattern: " << pattern << std::endl;
            }
        }
    }
}

void patternDeduperTest() {
    patternDeduper pd = patternDeduper();
    patternMatrix pm = patternMatrix(182);
    pm.multilineOutput = true;
    int duplicateID = 0;
    bool isDuplicate = pd.isDuplicate(pm, duplicateID, true);
    std::cout << "Is duplicate: " << isDuplicate << " Duplicate ID: " << duplicateID << std::endl;
}

void dedupeP352() {
    std::filesystem::create_directory(TFC_OUT_DIR);
    std::ofstream tfcout = std::ofstream(TFC_OUT_DIR + "/tfc-output.txt");
    std::ofstream tfcUniques = std::ofstream(TFC_OUT_DIR + "/tfc-uniques.txt");

    std::vector<patternMatrix> patterns = loadPatterns(TFC_OUT_DIR + "/p352-xT14-xT23-all-possible.txt");

    patternDeduper pd = patternDeduper();
    int newPatternID = 352000000;
    std::map<int, int> dupCount;
    for (auto pm : patterns) {
        int duplicateID = -1;
        pm.id = ++newPatternID;
        if (pd.isDuplicate(pm, duplicateID, true)) {
            std::cout << pm.id << " is a duplicate of " << duplicateID << std::endl;
            tfcout << pm.id << " is a duplicate of " << duplicateID << std::endl;
            dupCount[duplicateID]++;
        } else {
            std::cout << pm.id << " is unique" << std::endl;
            tfcout << pm.id << " is unique" << std::endl;
            tfcUniques << pm.id << " " << pm << std::endl;
        }
    }
    tfcout << "Duplicate Counts:" << std::endl;
    tfcUniques << "Duplicate Counts:" << std::endl;
    for (auto const& [id, count] : dupCount) {
        std::cout << "Duplicate ID: " << id << " Count: " << count << std::endl;
        tfcout << "Duplicate ID: " << id << " Count: " << count << std::endl;
        tfcUniques << "Duplicate ID: " << id << " Count: " << count << std::endl;
    }
    tfcout.close();
    tfcUniques.close();
    std::cout << "Done" << std::endl;
}

bool dedupTest(int caseNumber) {
    std::filesystem::create_directory("temp");
    int newPatternID = 1000000 * caseNumber;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    std::cout << "Case: " << caseNumber << std::endl;
    std::ofstream dedupeOut = std::ofstream("temp/case-" + std::to_string(caseNumber) + "-dedupe-out.txt");
    std::ofstream uniquesOut = std::ofstream("temp/case-" + std::to_string(caseNumber) + "-uniques-out.txt");
    if (!dedupeOut.is_open()) {
        std::cerr << "Error opening file:" << "temp/case-" + std::to_string(caseNumber) + "-dedupe-out.txt" << std::endl;
        return false;
    }
    if (!uniquesOut.is_open()) {
        std::cerr << "Error opening file:" << "temp/case-" + std::to_string(caseNumber) + "-uniques-out.txt" << std::endl;
        return false;
    }
    std::string filename = "temp/case-" + std::to_string(caseNumber) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file:" << filename << std::endl;
        return false;
    }
    std::ifstream lineCount(filename);
    int totalLines = std::count(std::istreambuf_iterator<char>(lineCount), std::istreambuf_iterator<char>(), '\n');
    std::cout << "Total lines: " << totalLines << std::endl;
    std::cout << "Files opened" << std::endl;
    std::cout << "Iterating through patterns" << std::endl;
    lineCount.close();
    
    patternDeduper pd = patternDeduper();
    std::cout << "Deduper Loaded" << std::endl;
    std::map<int, int> dupCount;
    std::cout << "Starting dedupe" << std::endl;
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        // ignore comments
        if (line[0] == '#') {
            std::cout << "Ignoring comment: " << line << std::endl;
            continue;
        }
        // Remove the leading and trailing brackets
        line = line.substr(1, line.size() - 2);
        // Replace spaces with commas
        std::replace(line.begin(), line.end(), ' ', ',');
        // Replace all instances of "],[" with "]["
        line = replace_all(line, "],[", "][");
        patternMatrix pm = patternMatrix(++lineNumber, line, false);
        //std::cout << pm.id << " " << pm << std::endl;
        pm.matchOnCases();
        int duplicateID = -1;
        if (pd.isDuplicate(pm, duplicateID, true)) {
            //std::cout << pm.id << " is a duplicate of " << duplicateID << std::endl;
            dupCount[duplicateID]++;
        } else {
            pm.id = ++newPatternID;
            //std::cout << pm.id << " is unique" << std::endl;
            uniquesOut << pm.id << " " << pm << std::endl;
            dedupeOut << pm.id << " " << pm << std::endl;
        }
        if (lineNumber % 10000 == 0) {
            std::cout << "Deduped " << lineNumber << " patterns" << std::endl;
            std::cout << "Deduped 1 pattern: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count() / lineNumber << " microseconds" << std::endl;
            std::cout << "Percent done: " << floorf(lineNumber * 100) / totalLines << "%" << std::endl;
        }
    }
    file.close();
    std::cout << "\nDuplicate Counts:" << std::endl;
    dedupeOut << "\nDuplicate Counts:" << std::endl;
    for (auto const& [id, count] : dupCount) {
        std::cout << "Duplicate ID: " << id << " Count: " << count << std::endl;
        dedupeOut << "Duplicate ID: " << id << " Count: " << count << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Time to dedupe: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
    std::cout << "Time to dedupe 1 pattern: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / lineNumber << " microseconds" << std::endl;
    dedupeOut << "Time to dedupe: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
    dedupeOut << "Time to dedupe 1 pattern: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / lineNumber << " microseconds" << std::endl;
    dedupeOut.close();
    uniquesOut.close();
    return true;
}

int main(int argc, char **argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::future<bool>> futures;
     for (int i = 1; i <= 8; i++) {
        auto res = std::async(std::launch::async, dedupTest, i);
        futures.push_back(std::move(res));
    }
    for (auto &f : futures) {
        f.wait();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Done!" << std::endl;
    std::cout << "Time to dedupe: " << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << " seconds" << std::endl;
    std::cout << "Results are:\n";
    int caseNumber = 1;
    for (auto &f : futures) {
        bool result = f.get();
        std::cout << "Case " << std::to_string(caseNumber)  << "Result: " << result << std::endl;
        caseNumber++;
    }
    return 0;
}