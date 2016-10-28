#pragma once

#include "logging/Logger.hpp"
#include <Eigen/Core>
#include <string>
#include <fstream>

// ----------------------------------------------------------- CLASS DEFINITION

namespace precice {
namespace io {

/**
 * @brief File reader for matrix/vector in Matlab V7 ASCII format.
 */
class TXTReader
{
public:

  /// Constructor, opens file and sets format.
  explicit TXTReader(const std::string& filename);

  /// Destructor, closes file.
  ~TXTReader();

  /// Reads the Eigen::Matrix from the file.
  template<typename Scalar, int Rows, int Cols>
  void read(Eigen::Matrix<Scalar, Rows, Cols>& matrix)
  {
    for (long i = 0; i < matrix.rows(); i++) {
      for (long j = 0; j < matrix.cols(); j++) {
        double scalar;
        _file >> scalar;
        matrix(i,j) = scalar;
      }
    }
  }


private:

  static logging::Logger _log;

  /// @brief Filestream.
  std::ifstream _file;
};

}} // namespace precice, io
