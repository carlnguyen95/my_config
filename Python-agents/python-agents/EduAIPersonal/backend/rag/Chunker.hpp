#pragma once

#include <string>
#include <vector>

namespace edu_ai::rag {
class Chunker {
 public:
  /// Splits text into overlapping chunks suitable for retrieval indexing.
  std::vector<std::string> chunk(const std::string& text, std::size_t chunk_size, std::size_t overlap) const;
};
}  // namespace edu_ai::rag
