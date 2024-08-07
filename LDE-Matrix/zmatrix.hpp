#ifndef Z_MATRIX_HPP
#define Z_MATRIX_HPP

#include <vector>
#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// Attempting to generalize the pattern and case matrices into a single class
class zmatrix {
    public:
        zmatrix();
        zmatrix(int rows, int cols, int maxValue);

        // These are the flags for the zmatrix
        bool multilineOutput = false;

        std::vector<std::vector<int>> z;
        int zSum;
        std::vector<int> zNumCounts; // Count of the number of 0s, 1s, 2s, 3s, ..., n's in the matrix
        std::vector<std::vector<int>> zRowCounts;  // Count of the number of 0s, 1s, 2s, 3s, ..., n's in each row
        std::vector<std::vector<int>> zColCounts;  // Count of the number of 0s, 1s, 2s, 3s, ..., n's in each column
        std::vector<std::vector<int>> zCountRows;  // Number of rows with a count of  0, 1, 2, 3, 4, 5, 6, ..n (based on columns) of 0s, 1s, 2s, 3s, ..., n's
        std::vector<std::vector<int>> zCountCols;  // Number of columns with a count of 0, 1, 2, 3, 4, 5, 6, ..n (based on rows) of 0s, 1s, 2s, 3s, ..., n's

        void updateMetadata();
        void swapRows(int i, int j);
        void swapColumns(int i, int j);

        bool operator==(const zmatrix &) const;  // Read the comments in zmatrix.cpp for more information on what equality means
        bool operator!=(const zmatrix &) const;
        bool strictMatch(const zmatrix &other);

        void printDebug(std::ostream&);
        friend std::ostream& operator<<(std::ostream&,const zmatrix &);
    
    private:
        int rows;
        int cols;
        int maxValue;
};

#endif // Z_MATRIX_HPP