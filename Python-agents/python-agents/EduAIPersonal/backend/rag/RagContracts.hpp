#pragma once

#include <string>
#include <vector>

#include "backend/models/Domain.hpp"

namespace edu_ai::rag {
class DocumentService { public: virtual ~DocumentService() = default; virtual std::string extract_text(const models::Document& document) = 0; };
class EmbeddingProvider { public: virtual ~EmbeddingProvider() = default; virtual std::vector<float> embed(const std::string& text) = 0; };
struct RetrievedChunk { models::DocumentChunk chunk; double similarity{}; };
class Retriever { public: virtual ~Retriever() = default; virtual std::vector<RetrievedChunk> retrieve(models::Id course_id, const std::string& query, int top_k) = 0; };
}  // namespace edu_ai::rag

