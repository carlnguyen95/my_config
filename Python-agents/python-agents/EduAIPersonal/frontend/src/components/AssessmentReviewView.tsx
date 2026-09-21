import React, { useState } from 'react';
import {
  BrainCircuit,
  Award,
  CheckCircle2,
  Clock,
  User,
  Shield,
  MessageSquare,
  Cpu,
  Settings,
  Sparkles,
  Edit3,
  Save,
  TrendingUp,
} from 'lucide-react';
import { ThinkingAssessment, Course, User as UserType, AIProviderState, AnswerPolicy } from '../types';

interface AssessmentReviewViewProps {
  assessments: ThinkingAssessment[];
  selectedCourse: Course;
  currentUser: UserType;
  providerState: AIProviderState | null;
  onReviewAssessment: (
    assessmentId: string,
    teacherScore: number,
    teacherFeedback: string
  ) => Promise<void>;
  onSwitchProvider: (provider: 'gemini' | 'ollama', cloudFallback: boolean) => Promise<void>;
  onUpdateCoursePolicy: (policy: AnswerPolicy) => Promise<void>;
}

export const AssessmentReviewView: React.FC<AssessmentReviewViewProps> = ({
  assessments,
  selectedCourse,
  currentUser,
  providerState,
  onReviewAssessment,
  onSwitchProvider,
  onUpdateCoursePolicy,
}) => {
  const [selectedAssessment, setSelectedAssessment] = useState<ThinkingAssessment | null>(
    assessments[0] || null
  );
  const [teacherScoreInput, setTeacherScoreInput] = useState<number>(0.85);
  const [teacherFeedbackInput, setTeacherFeedbackInput] = useState<string>('');
  const [isSubmittingReview, setIsSubmittingReview] = useState(false);

  const isTeacher = currentUser.role === 'teacher';

  const handleSelectAssessment = (asm: ThinkingAssessment) => {
    setSelectedAssessment(asm);
    setTeacherScoreInput(asm.teacher_score ?? asm.score);
    setTeacherFeedbackInput(asm.teacher_feedback || '');
  };

  const handleSaveReview = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!selectedAssessment) return;
    setIsSubmittingReview(true);
    await onReviewAssessment(selectedAssessment.id, teacherScoreInput, teacherFeedbackInput);
    setIsSubmittingReview(false);
  };

  const dimensionLabels = [
    { key: 'curiosity', label: '1. Tính tò mò & ham học hỏi', desc: 'Sự chủ động đặt câu hỏi thăm dò nguyên lý' },
    { key: 'depth', label: '2. Độ sâu tư duy & phân tích', desc: 'Khả năng đi từ hiện tượng bề mặt vào cấu trúc lõi' },
    { key: 'assumption_awareness', label: '3. Nhận thức giả định ngầm', desc: 'Phát hiện các tiền đề mà hệ thống ngầm thừa nhận' },
    { key: 'causal_reasoning', label: '4. Lập luận nhân quả & logic', desc: 'Mạch liên kết chặt chẽ giữa nguyên nhân và hệ quả' },
    { key: 'alternative_perspectives', label: '5. Góc nhìn đa chiều & phản biện', desc: 'Đặt ra giả thuyết trái chiều hoặc phương án thay thế' },
    { key: 'cross_domain_potential', label: '6. Khả năng liên hệ liên ngành', desc: 'Kết nối kiến thức sang các lĩnh vực tương tự' },
    { key: 'challenge_level', label: '7. Mức độ thử thách & độ khó', desc: 'Độ khó và tính trừu tượng của câu hỏi' },
  ];

  return (
    <div className="space-y-6">
      {/* Teacher Configuration Panel: LLM Switcher & Policy Selector */}
      {isTeacher && (
        <div className="p-5 rounded-2xl bg-gradient-to-r from-blue-900/10 via-indigo-900/10 to-slate-900/10 border border-blue-200 dark:border-blue-900/60 p-5 shadow-sm space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Cpu className="w-5 h-5 text-blue-600 dark:text-blue-400" />
              <h3 className="font-bold text-sm text-slate-900 dark:text-white">
                Bảng điều khiển Giảng viên: Quản trị Mô hình AI & Chính sách Môn học
              </h3>
            </div>
            <span className="text-[10px] font-bold px-2 py-0.5 rounded-full bg-blue-100 text-blue-800 dark:bg-blue-950 dark:text-blue-300">
              Quyền Giảng viên
            </span>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {/* LLM Provider Switcher */}
            <div className="p-4 rounded-xl bg-white dark:bg-[#0b1329] border border-slate-200 dark:border-slate-800 space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-800 dark:text-slate-200 flex items-center gap-1.5">
                  <Cpu className="w-4 h-4 text-blue-500" /> Nhà cung cấp LLM hiện hành
                </span>
                <span className="text-[11px] font-semibold text-emerald-600 dark:text-emerald-400">
                  {providerState?.current_provider === 'gemini' ? 'Gemini 3.8 Flash (Cloud)' : 'Local Ollama (Gemma4)'}
                </span>
              </div>

              <div className="grid grid-cols-2 gap-2">
                <button
                  onClick={() => onSwitchProvider('gemini', true)}
                  className={`px-3 py-2 rounded-xl text-xs font-semibold border transition text-center ${
                    providerState?.current_provider === 'gemini'
                      ? 'bg-blue-900 text-white border-blue-900 dark:bg-blue-600 dark:border-blue-600 shadow-xs'
                      : 'border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 hover:bg-slate-50 dark:hover:bg-slate-800'
                  }`}
                >
                  Cloud: Gemini 3.8 Flash
                </button>
                <button
                  onClick={() => onSwitchProvider('ollama', false)}
                  className={`px-3 py-2 rounded-xl text-xs font-semibold border transition text-center ${
                    providerState?.current_provider === 'ollama'
                      ? 'bg-blue-900 text-white border-blue-900 dark:bg-blue-600 dark:border-blue-600 shadow-xs'
                      : 'border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 hover:bg-slate-50 dark:hover:bg-slate-800'
                  }`}
                >
                  Local: Ollama (Gemma4)
                </button>
              </div>
              <p className="text-[11px] text-slate-500">
                Cho phép chuyển đổi linh hoạt giữa mô hình chạy nội bộ (Ollama) bảo mật dữ liệu và Cloud Adapter (Gemini) dự phòng.
              </p>
            </div>

            {/* Answer Policy Selector for Course */}
            <div className="p-4 rounded-xl bg-white dark:bg-[#0b1329] border border-slate-200 dark:border-slate-800 space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-800 dark:text-slate-200 flex items-center gap-1.5">
                  <Shield className="w-4 h-4 text-amber-500" /> Chính sách phản hồi sinh viên
                </span>
                <span className="text-[11px] font-bold text-blue-600 dark:text-blue-400">
                  {selectedCourse.answer_policy}
                </span>
              </div>

              <div className="grid grid-cols-3 gap-1.5">
                {(['HINT_ONLY', 'GUIDED', 'FULL_ANSWER'] as AnswerPolicy[]).map((pol) => (
                  <button
                    key={pol}
                    onClick={() => onUpdateCoursePolicy(pol)}
                    className={`px-2 py-2 rounded-xl text-[11px] font-semibold border transition text-center ${
                      selectedCourse.answer_policy === pol
                        ? 'bg-blue-900 text-white border-blue-900 dark:bg-blue-600 dark:border-blue-600 shadow-xs'
                        : 'border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 hover:bg-slate-50 dark:hover:bg-slate-800'
                    }`}
                  >
                    {pol === 'HINT_ONLY' ? 'Hint-Only' : pol === 'GUIDED' ? 'Guided' : 'Full Answer'}
                  </button>
                ))}
              </div>
              <p className="text-[11px] text-slate-500">
                HINT_ONLY kích hoạt phương pháp vấn đáp Socratic; GUIDED dẫn dắt từng bước logic.
              </p>
            </div>
          </div>
        </div>
      )}

      {/* Main Split: Left list of Assessments, Right Detailed 7-Dimension Visualizer & Teacher Review */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Col: List of Assessment Records */}
        <div className="space-y-3">
          <div className="flex items-center justify-between">
            <h3 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-2">
              <BrainCircuit className="w-4 h-4 text-blue-600 dark:text-blue-400" />
              Lịch sử Đánh giá ({assessments.length})
            </h3>
            <span className="text-xs text-slate-500 dark:text-slate-400">
              {isTeacher ? 'Tất cả sinh viên' : 'Của bạn'}
            </span>
          </div>

          <div className="space-y-2.5">
            {assessments.map((asm) => {
              const isSelected = selectedAssessment?.id === asm.id;
              const displayScore = asm.teacher_score ?? asm.score;

              return (
                <div
                  key={asm.id}
                  onClick={() => handleSelectAssessment(asm)}
                  className={`p-4 rounded-xl border transition-all duration-150 cursor-pointer ${
                    isSelected
                      ? 'bg-blue-50/70 dark:bg-blue-950/40 border-blue-500 dark:border-blue-500 ring-2 ring-blue-500/20 shadow-md'
                      : 'bg-white dark:bg-[#0b1329] border-slate-200/80 dark:border-slate-800 hover:border-slate-300 dark:hover:border-slate-700'
                  }`}
                >
                  <div className="flex items-center justify-between gap-2 mb-1.5">
                    <span className="font-semibold text-xs text-slate-800 dark:text-slate-200 flex items-center gap-1.5 truncate">
                      <User className="w-3.5 h-3.5 text-blue-500 shrink-0" />
                      {asm.student_name}
                    </span>
                    <span
                      className={`text-[10px] font-bold px-2 py-0.5 rounded-full ${
                        asm.review_status === 'reviewed'
                          ? 'bg-emerald-100 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-300'
                          : 'bg-amber-100 text-amber-800 dark:bg-amber-950 dark:text-amber-300'
                      }`}
                    >
                      {asm.review_status === 'reviewed' ? 'Đã duyệt' : 'Chờ duyệt'}
                    </span>
                  </div>

                  <p className="text-xs text-slate-600 dark:text-slate-300 line-clamp-2 leading-relaxed italic">
                    "{asm.prompt_excerpt}"
                  </p>

                  <div className="mt-2.5 pt-2 border-t border-slate-100 dark:border-slate-800/80 flex items-center justify-between text-xs">
                    <span className="text-slate-500 dark:text-slate-400 font-medium">
                      Điểm: <strong className="text-slate-900 dark:text-white">{(displayScore * 100).toFixed(0)}%</strong>
                    </span>
                    <span className="text-[11px] text-slate-400">
                      {new Date(asm.created_at).toLocaleDateString([], { month: 'numeric', day: 'numeric', hour: '2-digit', minute: '2-digit' })}
                    </span>
                  </div>
                </div>
              );
            })}
          </div>
        </div>

        {/* Right 2 Cols: Detailed 7-Dimension Visualizer & Teacher Review Form */}
        <div className="lg:col-span-2 space-y-5">
          {selectedAssessment ? (
            <div className="p-6 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-6">
              {/* Header Info */}
              <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-4 border-b border-slate-100 dark:border-slate-800">
                <div>
                  <span className="text-xs font-bold text-blue-600 dark:text-blue-400 uppercase tracking-wider">
                    Báo cáo Đánh giá Năng lực Tư duy Phản biện
                  </span>
                  <h3 className="text-lg font-extrabold text-slate-900 dark:text-white mt-1">
                    Sinh viên: {selectedAssessment.student_name}
                  </h3>
                  <p className="text-xs text-slate-500">
                    Khóa học: {selectedAssessment.course_name} • Mô hình: {selectedAssessment.model_name}
                  </p>
                </div>

                <div className="flex items-center gap-3">
                  <div className="text-right">
                    <span className="text-3xl font-black text-slate-900 dark:text-white">
                      {((selectedAssessment.teacher_score ?? selectedAssessment.score) * 100).toFixed(0)}%
                    </span>
                    <p className="text-xs text-slate-500">
                      {selectedAssessment.teacher_score !== undefined ? 'Điểm GV duyệt' : 'Điểm AI đề xuất'}
                    </p>
                  </div>
                  <div className="w-12 h-12 rounded-xl bg-blue-50 dark:bg-blue-950/60 text-blue-600 dark:text-blue-400 flex items-center justify-center">
                    <Award className="w-6 h-6" />
                  </div>
                </div>
              </div>

              {/* Prompt Excerpt Block */}
              <div className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200/60 dark:border-slate-700/60 text-xs">
                <span className="font-semibold text-slate-500 dark:text-slate-400 block mb-1">
                  Câu hỏi học thuật được đánh giá:
                </span>
                <p className="text-sm font-medium text-slate-800 dark:text-slate-200 leading-relaxed italic">
                  "{selectedAssessment.prompt_excerpt}"
                </p>
              </div>

              {/* The 7 Dimensions Visualization */}
              <div className="space-y-3">
                <h4 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-2">
                  <TrendingUp className="w-4 h-4 text-indigo-500" />
                  Phân tích 7 Chiều Tư Duy Cốt Lõi
                </h4>

                <div className="space-y-3">
                  {dimensionLabels.map((dim) => {
                    const score = (selectedAssessment.dimensions as any)[dim.key] || 0.75;
                    const percent = Math.round(score * 100);

                    return (
                      <div key={dim.key} className="space-y-1">
                        <div className="flex justify-between text-xs">
                          <div>
                            <span className="font-semibold text-slate-800 dark:text-slate-200">
                              {dim.label}
                            </span>
                            <span className="text-[11px] text-slate-600 dark:text-slate-300 ml-2 hidden sm:inline">
                              ({dim.desc})
                            </span>
                          </div>
                          <span className="font-bold text-slate-900 dark:text-white">{percent}%</span>
                        </div>

                        <div className="w-full h-2 rounded-full bg-slate-100 dark:bg-slate-800 overflow-hidden">
                          <div
                            className="h-full bg-gradient-to-r from-blue-700 to-blue-500 rounded-full transition-all duration-300"
                            style={{ width: `${percent}%` }}
                          />
                        </div>
                      </div>
                    );
                  })}
                </div>
              </div>

              {/* AI Reasoning Text */}
              <div className="p-4 rounded-xl bg-blue-50/60 dark:bg-blue-950/30 border border-blue-200/60 dark:border-blue-800/60 text-xs space-y-1.5">
                <span className="font-bold text-blue-900 dark:text-blue-300 flex items-center gap-1.5">
                  <BrainCircuit className="w-4 h-4 text-blue-600" /> Nhận xét phân tích của AI:
                </span>
                <p className="text-slate-700 dark:text-slate-300 leading-relaxed">
                  {selectedAssessment.reasoning}
                </p>
              </div>

              {/* Teacher Review Section (Teacher can edit; Student can read) */}
              <div className="pt-4 border-t border-slate-100 dark:border-slate-800 space-y-4">
                <h4 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-2">
                  <Shield className="w-4 h-4 text-emerald-500" />
                  Đánh giá & Phê duyệt của Giảng viên
                </h4>

                {isTeacher ? (
                  <form onSubmit={handleSaveReview} className="space-y-4">
                    <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                      <div>
                        <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                          Điểm Giảng viên phê duyệt (0.00 - 1.00):
                        </label>
                        <div className="flex items-center gap-3">
                          <input
                            type="number"
                            step="0.05"
                            min="0"
                            max="1"
                            value={teacherScoreInput}
                            onChange={(e) => setTeacherScoreInput(parseFloat(e.target.value))}
                            className="w-32 px-3 py-2 text-xs font-bold rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                          />
                          <span className="text-xs font-semibold text-blue-600 dark:text-blue-400">
                            = {(teacherScoreInput * 100).toFixed(0)} điểm
                          </span>
                        </div>
                      </div>
                    </div>

                    <div>
                      <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                        Nhận xét sư phạm & Lời khuyên định hướng:
                      </label>
                      <textarea
                        rows={3}
                        value={teacherFeedbackInput}
                        onChange={(e) => setTeacherFeedbackInput(e.target.value)}
                        placeholder="Ví dụ: Em đã nhận biết được cơ chế tự đồng bộ phản hồi mạng rất tốt. Cần đào sâu thêm về các trường hợp nghẽn do bufferbloat..."
                        className="w-full px-4 py-2.5 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white placeholder-slate-400 focus:outline-hidden focus:ring-2 focus:ring-blue-500"
                      />
                    </div>

                    <div className="flex justify-end">
                      <button
                        type="submit"
                        disabled={isSubmittingReview}
                        className="inline-flex items-center gap-2 px-5 py-2.5 rounded-xl font-semibold text-xs bg-emerald-600 hover:bg-emerald-500 text-white shadow-md transition"
                      >
                        <Save className="w-4 h-4" />
                        <span>{isSubmittingReview ? 'Đang lưu...' : 'Lưu & Hoàn tất duyệt'}</span>
                      </button>
                    </div>
                  </form>
                ) : (
                  <div className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-200 dark:border-slate-700 text-xs">
                    {selectedAssessment.teacher_feedback ? (
                      <div className="space-y-1">
                        <div className="flex items-center justify-between font-bold text-emerald-600 dark:text-emerald-400">
                          <span>Giảng viên chấm: {((selectedAssessment.teacher_score || 0) * 100).toFixed(0)} điểm</span>
                          <span className="text-[10px] uppercase">Đã duyệt</span>
                        </div>
                        <p className="text-slate-700 dark:text-slate-300 leading-relaxed">
                          "{selectedAssessment.teacher_feedback}"
                        </p>
                      </div>
                    ) : (
                      <div className="text-slate-500 italic">
                        ⏳ Giảng viên chưa cập nhật nhận xét chính thức cho phiên này. Điểm số hiện tại đang tạm tính theo thuật toán đánh giá AI.
                      </div>
                    )}
                  </div>
                )}
              </div>
            </div>
          ) : (
            <div className="p-8 text-center bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800 text-slate-500">
              Chọn một bản ghi đánh giá ở danh sách bên trái để kiểm tra chi tiết 7 chiều.
            </div>
          )}
        </div>
      </div>
    </div>
  );
};
