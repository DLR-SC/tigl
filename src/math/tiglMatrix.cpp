#include "tiglMatrix.h"

#include <fstream>

void tigl::writeMatrix(const tigl::tiglMatrix &x, const std::string &filename)
{

    // NOTE: we actually should also write what data type we are writing!!!
    std::ofstream fout(filename, std::ios::out | std::ios::binary);

    if (!fout.is_open()) {
        throw std::runtime_error("Cannot write matrix to file: " + filename);
    }

    uint32_t m_min = static_cast<uint32_t>(x.LowerRow());
    uint32_t m_max = static_cast<uint32_t>(x.UpperRow());
    uint32_t n_min = static_cast<uint32_t>(x.LowerCol());
    uint32_t n_max = static_cast<uint32_t>(x.UpperCol());
    uint32_t sizes[4] = {m_min, m_max, n_min, n_max};
    fout.write((char*)&sizes[0], sizeof (uint32_t)*4);
    fout.write((char*)&x.Value(x.LowerRow(), x.LowerCol()), sizeof(Standard_Real)*x.RowNumber()*x.ColNumber());
    fout.close();
}

tigl::tiglMatrix tigl::readMatrix(const std::string &filename)
{
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open matrix from file: " + filename);
    }

    uint32_t sizes[4];
    uint32_t n_min, n_max, m_min, m_max;
    fin.read((char*)&sizes[0], sizeof(uint32_t)*4);
    m_min = sizes[0];
    m_max = sizes[1];
    n_min = sizes[2];
    n_max = sizes[3];

    tiglMatrix result(m_min, m_max, n_min, n_max);
    fin.read((char*)&result(m_min, n_min), sizeof(Standard_Real)*result.RowNumber()*result.ColNumber());
    fin.close();
    return result;
}
