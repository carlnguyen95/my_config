import React, { useState } from 'react';
import {
  HelpCircle,
  Search,
  Plus,
  Sparkles,
  CheckCircle2,
  Bookmark,
  BookmarkCheck,
  ChevronDown,
  ChevronUp,
  Filter,
  Lightbulb,
  ShieldCheck,
  BrainCircuit,
  MessageSquare,
} from 'lucide-react';
import { Question, Course, User, QuestionDifficulty } from '../types';

interface QuestionBankViewProps {
  questions: Question[];
  selectedCourse: Course;
  currentUser: User;
  savedQuestionIds: string[];
  onToggleSaveQuestion: (questionId: string) => void;
  onApproveQuestion: (questionId: string) => void;
  onGenerateAIQuestion: (topic: string, difficulty: QuestionDifficulty) => Promise<void>;
  onAskQuestionInChat: (questionText: string) => void;
  isGeneratingAI: boolean;
}

export const QuestionBankView: React.FC<QuestionBankViewProps> = ({
  questions,
  selectedCourse,
  currentUser,
  savedQuestionIds,
  onToggleSaveQuestion,
  onApproveQuestion,
  onGenerateAIQuestion,
  onAskQuestionInChat,
  isGeneratingAI,
}) => {
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedDifficulty, setSelectedDifficulty] = useState<string>('all');
  const [selectedStatus, setSelectedStatus] = useState<string>('all');
  const [expandedQuestionId, setExpandedQuestionId] = useState<string | null>(null);
  const [showAIModal, setShowAIModal] = useState(false);
  const [aiTopic, setAiTopic] = useState('TCP Congestion Control');
  const [aiDifficulty, setAiDifficulty] = useState<QuestionDifficulty>('intermediate');

  const isTeacher = currentUser.role === 'teacher';

  const filteredQuestions = questions.filter((q) => {
    const matchesCourse = q.course_id === selectedCourse.id;
    const matchesSearch =
      !searchQuery ||
      q.question.toLowerCase().includes(searchQuery.toLowerCase()) ||
      q.answer.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesDifficulty = selectedDifficulty === 'all' || q.difficulty === selectedDifficulty;
    const matchesStatus =
      selectedStatus === 'all'
        ? currentUser.role === 'student'
          ? q.status === 'approved'
          : true
        : q.status === selectedStatus;

    return matchesCourse && matchesSearch && matchesDifficulty && matchesStatus;
  });

  const toggleExpand = (id: string) => {
    setExpandedQuestionId(expandedQuestionId === id ? null : id);
  };

  const handleCreateAI = async (e: React.FormEvent) => {
    e.preventDefault();
    await onGenerateAIQuestion(aiTopic, aiDifficulty);
    setShowAIModal(false);
  };

  const difficultyColors = {
    beginner: 'bg-emerald-50 text-emerald-700 dark:bg-emerald-950/50 dark:text-emerald-300 border-emerald-200 dark:border-emerald-800',
    intermediate: 'bg-amber-50 text-amber-700 dark:bg-amber-950/50 dark:text-amber-300 border-amber-200 dark:border-amber-800',
    advanced: 'bg-rose-50 text-rose-700 dark:bg-rose-950/50 dark:text-rose-300 border-rose-200 dark:border-rose-800',
  };

  return (
    <div className="space-y-6">
      {/* Top action bar: Search & Filters */}
      <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm flex flex-col md:flex-row md:items-center justify-between gap-4">
        <div className="flex-1 flex flex-wrap items-center gap-3">
          {/* Search bar */}
          <div className="relative flex-1 min-w-[240px]">
            <Search className="w-4 h-4 text-slate-400 absolute left-3.5 top-1/2 -translate-y-1/2" />
            <input
              type="text"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              placeholder="Tìm kiếm câu hỏi trong ngân hàng môn..."
              className="w-full pl-9 pr-4 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white placeholder-slate-400 focus:outline-hidden focus:ring-2 focus:ring-blue-500"
            />
          </div>

          {/* Difficulty Filter */}
          <select
            value={selectedDifficulty}
            onChange={(e) => setSelectedDifficulty(e.target.value)}
            className="px-3 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300"
          >
            <option value="all">Mọi độ khó</option>
            <option value="beginner">Cơ bản</option>
            <option value="intermediate">Trung bình</option>
            <option value="advanced">Nâng cao</option>
          </select>

          {/* Status Filter for Teachers */}
          {isTeacher && (
            <select
              value={selectedStatus}
              onChange={(e) => setSelectedStatus(e.target.value)}
              className="px-3 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300"
            >
              <option value="all">Tất cả trạng thái</option>
              <option value="approved">Đã phê duyệt</option>
              <option value="review">Cần duyệt (AI tạo)</option>
              <option value="draft">Bản nháp</option>
            </select>
          )}
        </div>

        {/* Generate AI Question Candidate button */}
        {isTeacher && (
          <button
            onClick={() => setShowAIModal(true)}
            className="inline-flex items-center gap-2 px-4 py-2 rounded-xl text-xs font-semibold bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white shadow-md transition shrink-0"
          >
            <Sparkles className="w-4 h-4 text-amber-300" />
            <span>AI Tạo Câu hỏi Mới</span>
          </button>
        )}
      </div>

      {/* Question list */}
      <div className="space-y-3">
        {filteredQuestions.length === 0 ? (
          <div className="p-8 text-center bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800">
            <HelpCircle className="w-10 h-10 text-slate-400 mx-auto mb-2" />
            <p className="text-sm font-semibold text-slate-700 dark:text-slate-300">
              Không tìm thấy câu hỏi phù hợp
            </p>
            <p className="text-xs text-slate-500 mt-1">
              Thử thay đổi từ khóa hoặc bộ lọc độ khó để xem thêm câu hỏi.
            </p>
          </div>
        ) : (
          filteredQuestions.map((q) => {
            const isSaved = savedQuestionIds.includes(q.id);
            const isExpanded = expandedQuestionId === q.id;
            const canViewDirectAnswer =
              isTeacher || selectedCourse.answer_policy === 'FULL_ANSWER';

            return (
              <div
                key={q.id}
                className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-3 transition hover:border-slate-300 dark:hover:border-slate-700"
              >
                <div className="flex items-start justify-between gap-4">
                  <div className="flex-1 min-w-0">
                    <div className="flex flex-wrap items-center gap-2 mb-2">
                      <span
                        className={`text-[10px] font-bold px-2.5 py-0.5 rounded-full border ${
                          difficultyColors[q.difficulty] || difficultyColors.intermediate
                        }`}
                      >
                        {q.difficulty.toUpperCase()}
                      </span>
                      <span className="text-[10px] font-semibold px-2 py-0.5 rounded-md bg-slate-100 dark:bg-slate-800 text-slate-600 dark:text-slate-400">
                        {q.source === 'ai' ? 'AI đề xuất' : 'Giảng viên'}
                      </span>
                      {q.status === 'review' && (
                        <span className="text-[10px] font-bold px-2 py-0.5 rounded-md bg-amber-100 dark:bg-amber-950 text-amber-800 dark:text-amber-300">
                          Chờ giảng viên duyệt
                        </span>
                      )}
                      <span className="text-xs text-slate-400">
                        Tạo bởi: {q.created_by}
                      </span>
                    </div>

                    <h3 className="text-sm font-bold text-slate-900 dark:text-white leading-relaxed">
                      {q.question}
                    </h3>
                  </div>

                  {/* Right Actions: Bookmark & Ask Mentor */}
                  <div className="flex items-center gap-1 shrink-0">
                    <button
                      onClick={() => onToggleSaveQuestion(q.id)}
                      className={`p-2 rounded-xl border transition ${
                        isSaved
                          ? 'bg-blue-50 dark:bg-blue-950 text-blue-600 border-blue-200 dark:border-blue-800'
                          : 'border-slate-200 dark:border-slate-700 text-slate-400 hover:text-slate-600 hover:bg-slate-50 dark:hover:bg-slate-800'
                      }`}
                      title={isSaved ? 'Bỏ lưu câu hỏi' : 'Lưu câu hỏi cá nhân'}
                    >
                      {isSaved ? (
                        <BookmarkCheck className="w-4 h-4 text-blue-600" />
                      ) : (
                        <Bookmark className="w-4 h-4" />
                      )}
                    </button>

                    <button
                      onClick={() => onAskQuestionInChat(q.question)}
                      className="p-2 rounded-xl border border-slate-200 dark:border-slate-700 text-slate-600 dark:text-slate-300 hover:bg-blue-50 dark:hover:bg-blue-950/50 hover:text-blue-600 transition"
                      title="Đưa vào hội thoại Mentor"
                    >
                      <MessageSquare className="w-4 h-4" />
                    </button>
                  </div>
                </div>

                {/* Socratic Hint & Answer Accordion */}
                <div className="pt-2 border-t border-slate-100 dark:border-slate-800/80">
                  <div className="flex items-center justify-between">
                    <button
                      onClick={() => toggleExpand(q.id)}
                      className="inline-flex items-center gap-1.5 text-xs font-semibold text-blue-600 dark:text-blue-400 hover:underline"
                    >
                      <Lightbulb className="w-3.5 h-3.5" />
                      <span>{isExpanded ? 'Ẩn gợi ý & đáp án' : 'Xem gợi ý Socratic & đáp án'}</span>
                      {isExpanded ? (
                        <ChevronUp className="w-3.5 h-3.5" />
                      ) : (
                        <ChevronDown className="w-3.5 h-3.5" />
                      )}
                    </button>

                    {/* Teacher 1-click Approval action */}
                    {isTeacher && q.status === 'review' && (
                      <button
                        onClick={() => onApproveQuestion(q.id)}
                        className="inline-flex items-center gap-1 px-3 py-1 rounded-lg text-xs font-bold bg-emerald-600 hover:bg-emerald-500 text-white shadow-xs transition"
                      >
                        <CheckCircle2 className="w-3.5 h-3.5" />
                        <span>Phê duyệt vào ngân hàng</span>
                      </button>
                    )}
                  </div>

                  {isExpanded && (
                    <div className="mt-3 space-y-2.5 text-xs">
                      {/* Hint */}
                      <div className="p-3 rounded-xl bg-amber-50/70 dark:bg-amber-950/40 border border-amber-200/80 dark:border-amber-800/60 text-amber-900 dark:text-amber-200">
                        <span className="font-bold flex items-center gap-1 mb-1 text-amber-800 dark:text-amber-400">
                          <Lightbulb className="w-3.5 h-3.5" /> Gợi ý tư duy:
                        </span>
                        <p className="leading-relaxed">{q.hint}</p>
                      </div>

                      {/* Direct Answer (controlled by policy or teacher) */}
                      {canViewDirectAnswer ? (
                        <div className="p-3 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-200 dark:border-slate-700 text-slate-800 dark:text-slate-200">
                          <span className="font-bold block mb-1 text-slate-900 dark:text-white">
                            Đáp án học thuật:
                          </span>
                          <p className="leading-relaxed whitespace-pre-wrap">{q.answer}</p>
                        </div>
                      ) : (
                        <div className="p-3 rounded-xl bg-slate-100 dark:bg-slate-800/40 border border-slate-200 dark:border-slate-800 text-slate-500 italic">
                          🔒 Đáp án bị ẩn theo chính sách <strong>{selectedCourse.answer_policy}</strong> của môn học. Hãy sử dụng Trợ lý Mentor để cùng thảo luận và rèn luyện tư duy phản biện!
                        </div>
                      )}
                    </div>
                  )}
                </div>
              </div>
            );
          })
        )}
      </div>

      {/* AI Question Generator Modal */}
      {showAIModal && (
        <div className="fixed inset-0 z-50 bg-black/50 backdrop-blur-xs flex items-center justify-center p-4">
          <div className="w-full max-w-md bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800 p-6 shadow-2xl space-y-4">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-2 text-blue-600 dark:text-blue-400">
                <Sparkles className="w-5 h-5 text-amber-400" />
                <h3 className="font-bold text-base text-slate-900 dark:text-white">
                  Tạo câu hỏi ứng viên bằng AI
                </h3>
              </div>
              <button
                onClick={() => setShowAIModal(false)}
                className="text-slate-400 hover:text-slate-600 text-sm font-bold"
              >
                ✕
              </button>
            </div>

            <p className="text-xs text-slate-500 dark:text-slate-400">
              Mô hình LLM sẽ sinh một câu hỏi mang tính kích thích tư duy phản biện. Câu hỏi sẽ vào trạng thái "Chờ duyệt" để Giảng viên kiểm tra trước khi công khai.
            </p>

            <form onSubmit={handleCreateAI} className="space-y-3">
              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Chủ đề trọng tâm:
                </label>
                <input
                  type="text"
                  value={aiTopic}
                  onChange={(e) => setAiTopic(e.target.value)}
                  className="w-full px-3 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                  placeholder="Ví dụ: Cơ chế AIMD vs TCP BBR"
                  required
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Mức độ khó:
                </label>
                <select
                  value={aiDifficulty}
                  onChange={(e) => setAiDifficulty(e.target.value as QuestionDifficulty)}
                  className="w-full px-3 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                >
                  <option value="beginner">Cơ bản (Beginner)</option>
                  <option value="intermediate">Trung bình (Intermediate)</option>
                  <option value="advanced">Nâng cao & Phản biện (Advanced)</option>
                </select>
              </div>

              <div className="pt-2 flex justify-end gap-2">
                <button
                  type="button"
                  onClick={() => setShowAIModal(false)}
                  className="px-4 py-2 rounded-xl text-xs font-semibold text-slate-600 dark:text-slate-400 hover:bg-slate-100 dark:hover:bg-slate-800"
                >
                  Hủy
                </button>
                <button
                  type="submit"
                  disabled={isGeneratingAI}
                  className="px-4 py-2 rounded-xl text-xs font-semibold bg-blue-600 text-white hover:bg-blue-500 disabled:opacity-50 flex items-center gap-1.5"
                >
                  {isGeneratingAI ? 'Đang tạo...' : 'Khởi tạo câu hỏi'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
};
