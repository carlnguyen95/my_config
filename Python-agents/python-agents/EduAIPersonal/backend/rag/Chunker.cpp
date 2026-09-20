#include "backend/rag/Chunker.hpp"
#include <algorithm>

namespace edu_ai::rag {
/**
 * @brief Splits source text into overlapping chunks for retrieval indexing.
 * @param text Source document text to split.
 * @param chunk_size Maximum characters in one chunk.
 * @param overlap Number of characters shared by adjacent chunks.
 * @return Ordered text chunks, or an empty list for invalid input.
 */
std::vector<std::string> Chunker::chunk(const std::string& text, std::size_t chunk_size, std::size_t overlap) const {
  if (text.empty() || chunk_size == 0)
    return {};
  overlap = std::min(overlap, chunk_size - 1);
  std::vector<std::string> chunks;
  for (std::size_t start = 0; start < text.size();) {
    const auto length = std::min(chunk_size, text.size() - start);
    chunks.push_back(text.substr(start, length));
    if (start + length == text.size())
      break;
    start += chunk_size - overlap;
  }
  return chunks;
}
}  // namespace edu_ai::rag
