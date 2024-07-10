#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "case-matrix.hpp"
#include "pattern-matrix.hpp"
#include "zmatrix.hpp"


patternMatrix::patternMatrix() {
    init();
}

void patternMatrix::init() {
    id = 0;
    caseMatch = -1;
    subCaseMatch = -1;
    pNewEncoding = zmatrix(rows, cols, 3);
    p = zmatrix(rows, cols, 3);
    pT = zmatrix(cols, rows, 3);
    swap23 = zmatrix(rows, cols, 3);
    swap23T = zmatrix(cols, rows, 3);
    cV = zmatrix(rows, cols, 1);
    cVT = zmatrix(cols, rows, 1);
    pGroupings = zmatrix(rows, cols, 36);
    entryLDEs.resize(rows);
    for (int i = 0; i < rows; i++) {
        entryLDEs[i].resize(cols);
    }
    originalMatrix = "[0,0,0,0,0][0,0,0,0,0][0,0,0,0,0][0,0,0,0,0][0,0,0,0,0][0,0,0,0,0]";
    loadCases();
}

patternMatrix::patternMatrix(int pNum, std::string matrix) {
    init();
    id = pNum;
    loadFromString(matrix);
}

void patternMatrix::loadFromString(std::string m) {
    if (m.size() == 0) throw std::runtime_error("Empty pattern string");
    std::stringstream ms(m);
    std::string r;
    int group = 1;  // Used to track groupings; initially, all entries are in their own group
    int row = 0;
    while(std::getline(ms, r, ']')) {
        if(row == rows) throw std::runtime_error("Too many rows in pattern string");
        std::stringstream rs(r);
        std::string v;
        int mv;
        int col = 0;
        while(std::getline(rs, v, ',')) {
            if(col == cols) throw std::runtime_error("Too many columns in pattern string");
            if(v[0] == '[') v.erase(0,1);
            
            // This set represents the original patterns we got from the creators
            if(v == "0 0") mv = 0;
            else if (v == "0 1") mv = 1;
            else if (v == "1 0") mv = 2;
            else if (v == "1 1") mv = 3;
            // This set represents our representation of the patterns
            else if(v == "0") mv = 0;
            else if (v == "1") mv = 1;
            else if (v == "2") mv = 2;
            else if (v == "3") mv = 3;
            else {
                std::ostringstream vErr;
                vErr << "Invalid value in pattern string: " << v ;
                throw std::runtime_error(vErr.str());
            }
            pNewEncoding.z[row][col] = (mv == 1) ? 2 : (mv == 2) ? 1 : mv;
            p.z[row][col] = mv;
            pT.z[col][row] = mv;
            swap23.z[row][col] = (mv == 2) ? 3 : (mv == 3) ? 2 : mv;
            swap23T.z[col][row] = (mv == 2) ? 3 : (mv == 3) ? 2 : mv;
            cV.z[row][col] = mv / 2;
            cVT.z[col][row] = mv / 2;
            pGroupings.z[row][col] = group;
            group++;
            col++;
        }
        if(col != cols) {
            std::ostringstream colErr;
            colErr << "Too few columns in pattern string: " << col << " instead of " << cols;
            throw std::runtime_error(colErr.str());
        }
        row++;
    }
    if(row != rows) {
        std::ostringstream rowErr;
        rowErr << "Too few rows in pattern string: " << row << " instead of " << rows;
        throw std::runtime_error(rowErr.str());
    } 
    p.updateMetadata();
    pT.updateMetadata();
    cV.updateMetadata();
    cVT.updateMetadata();
    swap23.updateMetadata();
    swap23T.updateMetadata();
    pGroupings.updateMetadata();  // This probably isn't necessary but it's good to be consistent
    // Now we know we have a valid matrix string so let's save it
    originalMatrix = toString();
}

void patternMatrix::matchOnCases() {
    caseMatch = -1;
    for (int i = 0; i < cases.size(); i++) {
        if (matchesCase(i)) {
            if (caseMatch != -1) {
                std::ostringstream caseErr;
                caseErr << "Pattern " << id << " matches more than 1 case: " << caseMatch << " and " << cases[i].id << std::endl;
                throw std::runtime_error(caseErr.str());
            }
            caseMatch = cases[i].id;
        }
    }
}

bool patternMatrix::matchesCase(int caseIndex) {
    if (cV == cases[caseIndex].c) return true; // pattern matrix case style matches the case matrix
    if (cVT == cases[caseIndex].c) return true; // transposed pattern matrix case style matches the case matrix
    return false;
}

bool patternMatrix::isTranspose(patternMatrix other) {
    // We need to do a complete check to verify that the pattern matrix is a transpose of the other
    bool isTranspose = (pT == other.p);
    return isTranspose;
}

bool patternMatrix::is23Swap(patternMatrix other) {
   // Now to check details starting with rows.
   std::vector<std::vector<int>> zRowCounts = swap23.zRowCounts;
    for (int i = 0; i < other.p.zRowCounts.size(); i++) {
        for (int j = 0; j < zRowCounts.size(); j++) {
            if (zRowCounts[j] == other.p.zRowCounts[i]) {
                zRowCounts.erase(zRowCounts.begin() + j);
                break;
            }
        }
    }
    if (zRowCounts.size() != 0) return false;
    // Now to check details of columns.
    std::vector<std::vector<int>> zColCounts = swap23.zColCounts;
    for (int i = 0; i < other.p.zColCounts.size(); i++) {
        for (int j = 0; j < zColCounts.size(); j++) {
            if ( zColCounts[j] == other.p.zColCounts[i]) {
                zColCounts.erase(zColCounts.begin() + j);
                break;
            }
        }
    }
    if (zColCounts.size() != 0) return false;
    // If we made it this far, then we have a match (I think)
    return true;
}


// This is essentially the same as is23Swap, but we are checking the transposed swap23 matrix
// TODO - Refactor this to use a common function with is23Swap
bool patternMatrix::is23SwapT(patternMatrix other) {
   // Now to check details starting with rows.
   std::vector<std::vector<int>> zRowCounts = swap23T.zRowCounts;
    for (int i = 0; i < other.p.zRowCounts.size(); i++) {
        for (int j = 0; j < zRowCounts.size(); j++) {
            if ( zRowCounts[j] == other.p.zRowCounts[i]) {
                zRowCounts.erase(zRowCounts.begin() + j);
                break;
            }
        }
    }
    if (zRowCounts.size() != 0) return false;
    // Now to check details of columns.
    std::vector<std::vector<int>> zColCounts = swap23T.zColCounts;
    for (int i = 0; i < other.p.zColCounts.size(); i++) {
        for (int j = 0; j < zColCounts.size(); j++) {
            if ( zColCounts[j] == other.p.zColCounts[i]) {
                zColCounts.erase(zColCounts.begin() + j);
                break;
            }
        }
    }
    if (zColCounts.size() != 0) return false;
    // If we made it this far, then we have a match (I think)
    return true;
}

void patternMatrix::printDebug(std::ostream& os) {
    std::cout << "Pattern Number: " << id << std::endl;
    std::cout << "Matched Case Number: " << caseMatch << std::endl;
    std::cout << "Pattern Matrix Debug:" << std::endl;
    p.printDebug(os);
    std::cout << "Transposed Pattern Matrix Debug:" << std::endl;
    pT.printDebug(os);
    std::cout << "Swapped 2s and 3s Pattern Matrix Debug:" << std::endl;
    swap23.printDebug(os);
    std::cout << "Case Style Pattern Matrix Debug:" << std::endl;
    cV.printDebug(os);
    std::cout << "Transposed Case Style Pattern Matrix Debug:" << std::endl;
    cVT.printDebug(os);
}

// TODO - Add a note on which pattern encoding is being used
// TODO - Refactor this mess, especially the new vs old encoding
std::ostream& operator<<(std::ostream& os,const patternMatrix &pm) {
    // Could do this differently by using flags to set the output format
    if (pm.printID) {
        os << pm.id;
    }
    if (pm.printCaseMatch) {
        os << pm.caseMatch;
    }
    if (pm.printAllIDs) {
        os << pm.id2704 << " " << pm.id928 << " " << pm.id785;
    }
    if (pm.multilineOutput) {
        if (pm.printID || pm.printCaseMatch || pm.printAllIDs) {
            os << std::endl;
        }
        if (pm.printOldEncoding) {
            zmatrix temp = pm.p;
            temp.multilineOutput = true;
            os << temp;
        } else {
            zmatrix temp = pm.pNewEncoding;
            temp.multilineOutput = true;
            os << temp;
        }
    } else {
        if (pm.printID || pm.printCaseMatch || pm.printAllIDs) os << " ";
        if (pm.printOldEncoding) {
            os << pm.p;
        } else {
            os << pm.pNewEncoding;
        }
    }
    return os;
}

void patternMatrix::printMatchComparison(caseMatrix cm) {
    std::cout << "NOT IMPLEMENTED YET" << std::endl;
}

// Do something with factorization of the sqrt(2) and the modulo 2 addition
//  Need to make sure that we handle all of the different outcomes as some may produce multiple
//   See your notes for that.



// ====== MULTIPLICATION BY T-GATES ======
// TODO: THIS IS FOR BOTH LEFT AND RIGHT MULTIPLICATION
// TODO: Track LDEs during this process
// TODO: Verify that columns and rows are orthogonal
//  col(i) * col(j) -> i==j then 2^k, i!=j then 0 (covers both orthogonal and normalized)
//  row(i) * row(j) -> i==j then 2^k, i!=j then 0 (covers both orthogonal and normalized)
// TODO: Normalized columns and rows = 2^k

// Left T-gate multiplication means adding rows p, q and replacing both with the result
void patternMatrix::leftTGateMultiply(int pRow, int qRow) {
    //  Handle left T-gate multiplication
    //  This means adding rows p, q and replacing both with the result
    //  This might be different when p > q
    //  TODO: Need to track LDEs here
    int result;
    for (int i = 0; i < p.z[pRow].size(); i++) {
        result = patternElementAddition(p.z[pRow][i], p.z[qRow][i]);
        p.z[pRow][i] = result;
        p.z[qRow][i] = result;
        // Increment the LDEs
        entryLDEs[pRow][i]++;
        entryLDEs[qRow][i]++;
        // Groupings - Going to use the lower group number of the two
        //  It might not matter but might also make it easier for humans to read
        if (pGroupings.z[pRow][i] > pGroupings.z[qRow][i]) {
            pGroupings.z[pRow][i] = pGroupings.z[qRow][i];
        } else {
            pGroupings.z[qRow][i] = pGroupings.z[pRow][i];
        }
    }
    // TODO - Do factorization which includes possible LDE reduction
}

// Right T-gate multiplication means adding columns p, q and replacing both with the result
void patternMatrix::rightTGateMultiply(int pCol, int qCol) {
    //  Handle right T-gate multiplication
    //  This means adding column p, q and replacing both with the result
    //  This might be different when p > q
    // TODO: Need to track LDEs here
    int result;
    for (int i = 0; i < p.z.size(); i++) {
        result = patternElementAddition(p.z[i][pCol], p.z[i][qCol]);
        p.z[i][pCol] = result;
        p.z[i][qCol] = result;
        // Increment the LDEs
        entryLDEs[i][pCol]++;
        entryLDEs[i][qCol]++;
        // Groupings - Going to use the lower group number of the two
        //  It might not matter but might also make it easier for humans to read
        if (pGroupings.z[i][pCol] <= pGroupings.z[i][qCol]) {
            pGroupings.z[i][pCol] = pGroupings.z[i][qCol];
        } else {
            pGroupings.z[i][qCol] = pGroupings.z[i][pCol];
        }
    }
    // TODO - Do factorization which includes possible LDE reduction
}
// ====== MULTIPLICATION BY T-GATES ======

// TODO: Maybe move this out of the class?  It might need to stay in the class depending on how we handle the LDEs
// TODO: We need to return any LDE increases here
//    which probably means returning a struct or modifying a,b by reference and returning the LDE increase
int patternMatrix::patternElementAddition(int a, int b) {
    // TODO: Put the logic for this someplace and reference it here
    // This is not "real" addition here.  What's really happening is that we are
    //  adding the modulo 2 of the N and M values from N + M*sqrt(2)
    //  and then taking the modulo 2 of the individual results
    //  where N is bit 1 and M is bit 0:
    //    Examples:  0 == N=0, M=0
    //               1 == N=0, M=1
    //               2 == N=1, M=0
    //               3 == N=1, M=1
    // Pattern addition properties based on above:
    //  0 + 0,1,2,3 = 0,1,2,3
    //  1 + 1,2,3 = 0,3,2
    //  2 + 2,3 = 0,1
    //  3 + 3 = 0
    // Each step below whittles down the possibilities so each successive if statement handles fewer cases
    // This handles the case where a and b are the same
    // After pulling sqrt(2) out immediately...
    //  0 + 0 = 0,0 or 1,1 (same parity)
    //  1 + 1 = 0,0 or 1,1 (same parity)
    //  2 + 2 = 0,1 or 1,0 (different parity)
    //  3 + 3 = 0,1 or 1,0 (different parity)
    //  2 + 3 = 2,3 or 3,2 (different parity)
    // These you cannot pull the sqrt(2) out immediatley and need another T
    //    Try to avoid these!!!
    //  1 + 2 = 3
    //  1 + 3 = 2
    // Note from Ming
    // (“0”+-”1”)/sqrt2 is “2” and “2” or “3” and “3” ( same parity)
    if (a == b) return 0;
    // This handles all the remaining cases where a or b are 0
    else if (a == 0 || b == 0) return a + b;
    // This handles all the remaining cases where a or b are 1.  All that's left though is 1+2=3 or 1+3=2
    else if (a == 1 || b == 1) return (a + b == 4) ? 2 : 3;
    // Now, all that's left is 2+3=1
    else if (a == 2 || b == 2) return 1;
    return -1;  // This should never happen
}

void patternMatrix::loadCases() {
    // Loading from a file is another way to do this but, with only 8 of them and only being used for the patterns, this is fine
    // Case 0 is the root case and is all 0s
    cases.push_back(caseMatrix(0, "[0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(1, "[1,1,0,0,0,0][1,1,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(2, "[1,1,0,0,0,0][1,1,0,0,0,0][1,1,0,0,0,0][1,1,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(3, "[1,1,1,1,0,0][1,1,1,1,0,0][1,1,1,1,0,0][1,1,1,1,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(4, "[1,1,1,1,0,0][1,1,1,1,0,0][1,1,0,0,0,0][1,1,0,0,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(5, "[1,1,0,0,0,0][1,1,0,0,0,0][0,0,1,1,0,0][0,0,1,1,0,0][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(6, "[1,1,1,1,0,0][1,1,1,1,0,0][1,1,0,0,1,1][1,1,0,0,1,1][0,0,0,0,0,0][0,0,0,0,0,0]"));
    cases.push_back(caseMatrix(7, "[1,1,0,0,0,0][1,1,0,0,0,0][0,0,1,1,0,0][0,0,1,1,0,0][0,0,0,0,1,1][0,0,0,0,1,1]"));
    cases.push_back(caseMatrix(8, "[1,1,1,1,0,0][1,1,1,1,0,0][1,1,0,0,1,1][1,1,0,0,1,1][0,0,1,1,1,1][0,0,1,1,1,1]"));
}

std::string patternMatrix::toString() {
    std::ostringstream os;
    os << p;
    return os.str();
}

struct rearrangedPattern
{
   std::vector<std::vector<int>> rearrangedMatrix;
   bool done;
};

// TODO - This might not need a return value so remove?
bool patternMatrix::rearrangeMatrix() {
    // Clear out the previous rearraangements
    caseRearrangements.clear();

    if (caseMatch == -1) {
        // Attempt to match the pattern to a case
        matchOnCases();
        // If there's still no match, then we can't rearrange the matrix
        if (caseMatch == -1) {
            return false;
        }
    }
    // Case 0 is the base case and doesn't need to be rearranged
    // TODO - we might need to rearrange case 0 at some point but not now
    if (caseMatch == 0) {
        return false;
    }
    
    //  We also need to try both the pattern and the transposed pattern
    if (cV == cases[caseMatch].c) {
        rearrangeColumns(p, cV, 0);
    }
    if (cVT == cases[caseMatch].c) {
        rearrangeColumns(pT, cVT, 0);
    }
    return true;
}

void patternMatrix::rearrangeColumns(zmatrix patternVersion, zmatrix caseVersion, int curCol) {
    if (singleCaseRearrangement && caseRearrangements.size() > 0) return;
    for (int i = curCol; i < cols; i++) {
        // If the column is all 0s, then we don't need to do anything
        if (cases[caseMatch].c.zColCounts[curCol][1] == 0) {
            curCol++;
            continue;
        }
        // swap columns
        if (cases[caseMatch].c.zColCounts[curCol][1] == caseVersion.zColCounts[i][1]) {
            zmatrix myPattern = patternVersion;
            zmatrix myCase = caseVersion;
            if (curCol != i) {
                myPattern.swapColumns(curCol, i);
                myCase.swapColumns(curCol, i);
            }
            rearrangeColumns(myPattern, myCase, curCol+1);
        }
    }
    if (curCol == cols && caseVersion.zColCounts == cases[caseMatch].c.zColCounts) {
        rearrangeRows(patternVersion, caseVersion, 0);
    }
}

void patternMatrix::rearrangeRows(zmatrix patternVersion, zmatrix caseVersion, int curRow) {
    if (singleCaseRearrangement && caseRearrangements.size() > 0) return;
    for (int i = curRow; i < rows; i++) {
        // If the row is all 0s, then we don't need to do anything
        if (cases[caseMatch].c.zRowCounts[curRow][1] == 0) {
            curRow++;
            continue;
        }
        // swap rows
        if (cases[caseMatch].c.zRowCounts[curRow][1] == caseVersion.zRowCounts[i][1]) {
            zmatrix myPattern = patternVersion;
            zmatrix myCase = caseVersion;
            if (curRow != i) {
                myPattern.swapRows(curRow, i);
                myCase.swapRows(curRow, i);
            }
            rearrangeRows(myPattern, myCase, curRow+1);
        }
    }
    if (curRow == rows && caseVersion.strictMatch(cases[caseMatch].c)) {
        std::ostringstream os;
        os << patternVersion;
        caseRearrangements[os.str()] = true;
    }
}

// TODO - Refactor this
/* Rules for orthnormality:
   // definition of mij which is a different encoding
   mij = 2yij + xij
    // These are normality
    i. ∑ mij = 0 (mod4)
    ii. mij = 3 has to be paired if exists
    // These are orthogonality
    iii. ri · rj = 0 (mod2) when i  ̸= j
    iv. the count of (1, 2), (1, 3), and (2, 3) pairs in  ri and  rj should be even

*/
bool patternMatrix::isOrthonormal() {
    bool orthNorm = true;
    // Map of N / M vs letters in the paper:
    //  a = N, b = M;
    //  x = N, y = M;
    // N and M values from N + M*sqrt(2)
    //  and then taking the modulo 2 of the individual results
    //  where N is bit 1 and M is bit 0:
    //    Examples:  0 == N=0, M=0
    //               1 == N=0, M=1
    //               2 == N=1, M=0
    //               3 == N=1, M=1
    // Normality checks
    for (int i = 0; i < p.z.size(); i++) {
        int m4Row = 0;
        int m2Row = 0;
        int m4Col = 0;
        int m2Col = 0;
        for (int j = 0; j < p.z.size(); j++) {
            if (p.z[i][j] > 3) {
                std::ostringstream os;
                os << "Invalid value in pattern matrix: " << p.z[i][j];
                throw std::runtime_error(os.str());
            
            }
            int mij = p.z[i][j] / 2 + 2 * (p.z[i][j] % 2);  //mij = Nij + 2*Mij (row)
            int mji = p.z[j][i] / 2 + 2 * (p.z[j][i] % 2);  //mji = Nji + 2*Mji (column)
            // Rule i. ∑ mij = 0 (mod4); row check
            m4Row += mij;
            // Rule i. ∑ mji = 0 (mod4); column check
            m4Col += mji;
            // Rule ii. mij = 3 has to be paired if exists (row check)
            if (mij == 3) m2Row++;
            // Rule ii. mij = 3 has to be paired if exists (col check)
            if (mji == 3) m2Col++;
        }
        // TODO: refactor this
        if (printDebugInfo) {
            if (m4Row % 4 != 0 || m2Row % 2 != 0) {
                std::cout << "Row " << i+1 << " is not normalized" << std::endl;
                std::cout << "  Rule i. ∑ mij = " << m4Row << "; mod4: " << m4Row % 4 << std::endl;
                std::cout << "  Rule ii. mij = 3 has to be paired if exists; count: " << m2Row << "; mod2: " << m2Row % 2 << std::endl;
                orthNorm = false;
            }
            if (m4Col % 4 != 0 || m2Col % 2 != 0) {
                std::cout << "Column " << i+1 << " is not normalized" << std::endl;
                std::cout << "  Rule i. ∑ mji = " << m4Col << "; mod4: " << m4Col % 4 << std::endl;
                std::cout << "  Rule ii. mij = 3 has to be paired if exists; count: " << m2Col << "; mod2: " << m2Col % 2 << std::endl;
                orthNorm = false;
            }
        }
        if (!printDebugInfo && (m4Row % 4 != 0 || m2Row % 2 != 0 || m4Col % 4 != 0 || m2Col % 2 != 0)) {
            return false;
        }
    }
    // Orthogonality Checks
    for (int i = 0; i < p.z.size(); i++) {
        for (int j = i + 1; j < p.z.size(); j++) {
            int m2RowDotProd = 0;
            int m2RowPairs = 0;
            int m2ColDotProd = 0;
            int m2ColPairs = 0;
            for (int k = 0; k < p.z.size(); k++) {
                /*
                */
                int mik = p.z[i][k] / 2 + 2 * (p.z[i][k] % 2);  //mij = Nij + 2*Mij (row A)
                int mjk = p.z[j][k] / 2 + 2 * (p.z[j][k] % 2);  //mkj = Nkj + 2*Mkj (row B)
                int mki = p.z[k][i] / 2 + 2 * (p.z[k][i] % 2);  //mji = Nji + 2*Mji (column A)
                int mkj = p.z[k][j] / 2 + 2 * (p.z[k][j] % 2);  //mjk = Njk + 2*Mjk (column B)
                // iii. ri · rj = 0 (mod2) when i  ̸= k
                // row A * row B
                m2RowDotProd += mik * mjk;
                // col A * col B
                m2ColDotProd += mki * mkj;
                // iv. the count of (1, 2), (1, 3), and (2, 3) pairs in  ri and rj should be even
                if (mik != 0 && mjk != 0 && mik != mjk) m2RowPairs++;
                if (mki != 0 && mkj != 0 && mki != mkj) m2ColPairs++;
            }
            // TODO: refactor this
            if (printDebugInfo) {
                if (m2RowDotProd % 2 != 0 || m2RowPairs % 2 != 0 ) {
                    std::cout << "Rows " << i+1 << " and " << j+1 << " are not orthogonal" << std::endl;
                    std::cout << "  Dot Product = " << m2RowDotProd << "; mod2 = " << m2RowDotProd % 2 << std::endl;
                    std::cout << "  (1, 2), (1, 3), and (2, 3) pairs should be even; count = " << m2RowPairs << "; mod2 = " << m2RowPairs % 2 << std::endl;
                    orthNorm = false;
                }
                if (m2ColDotProd % 2 != 0 || m2ColPairs % 2 != 0) {
                    std::cout << "Columns " << i+1 << " and " << j+1 << " are not orthogonal" << std::endl;
                    std::cout << "  Dot Product = " << m2ColDotProd << "; mod2 = " << m2ColDotProd % 2 << std::endl;
                    std::cout << "  (1, 2), (1, 3), and (2, 3) pairs should be even; count = " << m2ColPairs << "; mod2 = " << m2ColPairs % 2 << std::endl;
                    orthNorm = false;
                }
            }
            if (!printDebugInfo && (m2RowDotProd % 2 != 0 || m2RowPairs % 2 != 0 || m2ColDotProd % 2 != 0 || m2ColPairs % 2 != 0)) {
                return false;
            }
        }
    }
    return orthNorm;
}