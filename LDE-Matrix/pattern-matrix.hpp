#ifndef PATTERN_MATRIX_HPP
#define PATTERN_MATRIX_HPP

#include <map>
#include <string>

#include "case-matrix.hpp"
#include "zmatrix.hpp"

class patternMatrix {
    public:
        patternMatrix();
        patternMatrix(int patternNumber, std::string matrix);
        int id;  // Primary identifier for the pattern
        // These identifiers start at 1 and increase with 0 meaning it's not in the file
        int id2704 = 0;  // Identifier for the 2704 pattern
        int id928 = 0;   // Identifier for the 928 pattern
        int id785 = 0;   // Identifier for the 785 pattern
        std::vector<caseMatrix> cases;
        int caseMatch;
        int subCaseMatch;
        // These are the flags for the pattern matrix
        bool singleCaseRearrangement = false;  // This is stop the case rearrangement code after a single solution is found
        // << operator flags
        bool printID = false;
        bool printCaseMatch = false;
        bool printAllIDs = false;
        // These are the matrices that are used for comparison
        zmatrix p; // This is the pattern matrix
        zmatrix pT;  // This is the transposed pattern matrix
        zmatrix swap23;  // This is the pattern matrix with 2s swapped for 3s and 3s swapped for 2s
        zmatrix swap23T;  // This is the transposed pattern matrix with 2s swapped for 3s and 3s swapped for 2s
        zmatrix cV;  // This is the pattern matrix changed to match the case style, 0s for 0,1 and 1s for 2,3
        zmatrix cVT;  // This is the transposed pattern matrix changed to match the case style, 0s for 0,1 and 1s for 2,3
        // TODO: Add a A, B set of matrices for the pattern where: A+Bsqrt(2) = pattern
        //   and use these for normality and orthogonality checking
        //   Essentially, this is keeping the original form of the patterns when they are in binary form 'A B' of (0 0, 0 1, 1 0, 1 1)
        // TODO: Use this to check all the rules that Ming shared such as normality, orthogonality, 3 pair counts, 2/1 pair counts, etc.
        //    See line 160 in the Latex document
        std::string originalMatrix; // This is the original matrix string
        std::map<std::string, bool> caseRearrangements; // This is a map of all the possible case rearrangements
        void loadFromString(std::string m);
        void matchOnCases();
        bool matchesCase(int caseIndex);
        bool isTranspose(patternMatrix other);
        bool is23Swap(patternMatrix other);
        bool is23SwapT(patternMatrix other);
        void printDebug(std::ostream& os);
        friend std::ostream& operator<<(std::ostream&,const patternMatrix &);
        void leftTGateMultiply(int p, int q);
        void rightTGateMultiply(int p, int q);
        // These could be private but are public for testing
        void printMatchComparison(caseMatrix cm);
        bool rearrangeMatrix();
        void rearrangeColumns(zmatrix patternVersion, zmatrix caseVersion, int currentColumn);
        void rearrangeRows(zmatrix patternVersion, zmatrix caseVersion, int currentRow);
        int patternElementAddition(int a, int b);
        std::string toString();
        // TODO: Add a csv output for the pattern matrix
    private:
        void init();
        void loadCases();
        int rows = 6;
        int cols = 6;
};

#endif // PATTERN_MATRIX_HPP
