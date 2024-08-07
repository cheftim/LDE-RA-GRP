#ifndef PATTERN_MATRIX_HPP
#define PATTERN_MATRIX_HPP

#include <map>
#include <string>
#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "case-matrix.hpp"
#include "zmatrix.hpp"

class patternMatrix {
    public:
        patternMatrix();
        patternMatrix(int pattern928Number);  // This will load a 928 pattern by number
        patternMatrix(int patternNumber, std::string matrix);
        patternMatrix(int pNum, std::string matrix, bool newEncoding);
        int id;  // Primary identifier for the pattern
        // These identifiers start at 1 and increase with 0 meaning it's not in the file
        int id2704 = 0;  // Identifier for the 2704 pattern
        int id928 = 0;   // Identifier for the 928 pattern
        int id785 = 0;   // Identifier for the 785 pattern
        // Cases that the pattern could match
        std::vector<caseMatrix> cases;
        int caseMatch;
        char subCaseMatch;
        // These are the flags for the pattern matrix
        bool printDebugInfo = false;  // WIP: This is for printing debug information
        bool singleCaseRearrangement = false;  // This is stop the case rearrangement code after a single solution is found
        // << operator flags
        bool printID = false;
        bool printCaseMatch = false;
        bool printAllIDs = false;
        bool multilineOutput = false;
        bool printOldEncoding = false;
        // TODO refactor to split new vs old encoding
        zmatrix pNewEncoding;  // This is the pattern matrix with the new encoding - 2y + x
        // These are the matrices that are used for comparison
        zmatrix p; // This is the pattern matrix
        zmatrix pT;  // This is the transposed pattern matrix
        zmatrix swap23;  // This is the pattern matrix with 2s swapped for 3s and 3s swapped for 2s
        zmatrix swap23T;  // This is the transposed pattern matrix with 2s swapped for 3s and 3s swapped for 2s
        zmatrix cV;  // This is the pattern matrix changed to match the case style, 0s for 0,1 and 1s for 2,3
        zmatrix cVT;  // This is the transposed pattern matrix changed to match the case style, 0s for 0,1 and 1s for 2,3
        zmatrix pGroupings;  // This is the pattern matrix with the groupings applied
        std::vector<std::vector<std::vector<int>>> possibleValues;  // After LDE reduction, these are the possible values for the pattern
        // TODO: Add a A, B set of matrices for the pattern where: A+Bsqrt(2) = pattern
        //   and use these for normality and orthogonality checking
        //   Essentially, this is keeping the original form of the patterns when they are in binary form 'A B' of (0 0, 0 1, 1 0, 1 1)
        // TODO: Use this to check all the rules that Ming shared such as normality, orthogonality, 3 pair counts, 2/1 pair counts, etc.
        //    See line 160 in the Latex document
        std::string originalMatrix; // This is the original matrix string
        std::map<std::string, bool> caseRearrangements; // This is a map of all the possible case rearrangements
        std::vector<std::vector<int>> rowPairCounts;  // This is the row pair counts for the pattern
        std::vector<std::vector<int>> colPairCounts;  // This is the col pair counts for the pattern
        // LDE Tracking
        int LDE = 0;  // This is the LDE of the pattern
        // This tracks an entry by entry LDE change based on T-Gate operations and factorization
        std::vector<std::vector<int>> entryLDEs;
        // This will track the T-Gate operations applied to the pattern
        std::vector<std::string> tGateOperations;

        void loadFromString(std::string m);
        void updatePairCounts();
        // Case Matching Functions
        void matchOnCases();
        bool matchesCase(int caseIndex);
        bool determineSubCase();
        bool case1SubCaseMatch();
        bool case2SubCaseMatch();
        bool case3SubCaseMatch();
        bool case3aSubCaseCheck();
        bool case3bSubCaseCheck();
        bool case3cSubCaseCheck();
        bool case4SubCaseMatch();
        bool case5SubCaseMatch();
        bool case6SubCaseMatch();
        bool case7SubCaseMatch();
        bool case8SubCaseMatch();
        std::string getFirstCaseRearrangement();

        // Duplicate Pattern Checks
        bool isTranspose(patternMatrix other);
        bool is23Swap(patternMatrix other);
        bool is23SwapT(patternMatrix other);
        // Extra Printing Functions
        void printDebug(std::ostream& os);
        void printLDEs(std::ostream& os);
        void printPossibleValues(std::ostream& os);
        // Get a list of T-Gate operations applied
        std::string printTGateOperations();
        // Get the possible values
        std::string getMaxOfPossibleValues();
        // T-Gate Multiplication Functions
        void leftTGateMultiply(int p, int q);
        void rightTGateMultiply(int p, int q);
        int patternElementAddition(int a, int b);
        // LDE Reduction Functions
        void ldeReductionOnEntry(int row, int col, int ldeReduction);
        void ldeReductionOnPattern(int ldeValue);
        // These could be private but are public for testing
        bool rearrangeMatrix();
        void rearrangeColumns(zmatrix patternVersion, zmatrix caseVersion, int currentColumn);
        void rearrangeRows(zmatrix patternVersion, zmatrix caseVersion, int currentRow);
        std::string toString();
        // TODO: Add a csv output for the pattern matrix
        bool isOrthonormal();

        // Friends
        friend std::ostream& operator<<(std::ostream&,const patternMatrix &);
    
    private:
        void init();
        void loadCases();
        int rows = 6;
        int cols = 6;
};

#endif // PATTERN_MATRIX_HPP
