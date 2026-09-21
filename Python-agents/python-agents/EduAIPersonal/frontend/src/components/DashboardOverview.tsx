import React from 'react';
import {
  BrainCircuit,
  BookOpen,
  HelpCircle,
  TrendingUp,
  Award,
  Sparkles,
  ArrowRight,
  Shield,
  ShieldCheck,
  Layers,
  FileText,
  CheckCircle2,
  Clock,
  Flame,
  LogIn,
} from 'lucide-react';
import { Course, User, ThinkingAssessment, RoadmapTopic, LearningProgressItem } from '../types';

interface DashboardOverviewProps {
  currentUser: User;
  courses: Course[];
  selectedCourse: Course;
  onSelectCourse: (course: Course) => void;
  assessments: ThinkingAssessment[];
  progress: LearningProgressItem[];
  onNavigateTab: (tab: string) => void;
  onOpenDocuments: () => void;
  onOpenLoginModal?: () => void;
}

export const DashboardOverview: React.FC<DashboardOverviewProps> = ({
  currentUser,
  courses,
  selectedCourse,
  onSelectCourse,
  assessments,
  progress,
  onNavigateTab,
  onOpenDocuments,
  onOpenLoginModal,
}) => {
  const isStudent = currentUser.role === 'student';
  const isTeacher = currentUser.role === 'teacher';

  // Calculate average critical thinking score
  const studentAssessments = assessments.filter((a) => a.user_id === currentUser.id || isTeacher);
  const avgScore =
    studentAssessments.length > 0
      ? studentAssessments.reduce((acc, a) => acc + (a.teacher_score ?? a.score), 0) / studentAssessments.length
      : 0.81;

  // Calculate overall course progress
  const completedTopics = progress.filter((p) => p.status === 'COMPLETED').length;
  const totalTopics = progress.length || 5;
  const progressPercent = Math.round((completedTopics / Math.max(1, totalTopics)) * 100);

  const policyLabels: Record<string, { label: string; desc: string; bg: string; text: string }> = {
    HINT_ONLY: {
      label: 'Gợi ý Socratic (Hint-Only)',
      desc: 'AI không lộ đáp án trực tiếp, dẫn dắt bằng câu hỏi phản xạ',
      bg: 'bg-amber-50 dark:bg-amber-950/40 border-amber-200 dark:border-amber-800',
      text: 'text-amber-700 dark:text-amber-400',
    },
    GUIDED: {
      label: 'Hướng dẫn từng bước (Guided)',
      desc: 'AI chia nhỏ vấn đề thành các mốc logic kiểm tra tư duy',
      bg: 'bg-blue-50 dark:bg-blue-950/40 border-blue-200 dark:border-blue-800',
      text: 'text-blue-700 dark:text-blue-400',
    },
    FULL_ANSWER: {
      label: 'Phân tích toàn diện (Full Answer)',
      desc: 'Cung cấp đáp án học thuật chi tiết và chứng minh hoàn chỉnh',
      bg: 'bg-emerald-50 dark:bg-emerald-950/40 border-emerald-200 dark:border-emerald-800',
      text: 'text-emerald-700 dark:text-emerald-400',
    },
  };

  const currentPolicyInfo = policyLabels[selectedCourse.answer_policy] || policyLabels.HINT_ONLY;

  return (
    <div className="space-y-6">
      {/* Top Banner / Welcome card */}
      <div className="relative overflow-hidden rounded-2xl bg-gradient-to-r from-[#0a192f] via-[#112240] to-[#1e3a8a] text-white p-6 sm:p-8 shadow-xl border border-blue-900/40">
        <div className="absolute -right-12 -bottom-12 w-64 h-64 rounded-full bg-blue-500/10 blur-3xl pointer-events-none" />
        <div className="relative z-10 flex flex-col md:flex-row md:items-center justify-between gap-6">
          <div className="space-y-2 max-w-2xl">
            <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full text-xs font-semibold bg-blue-500/20 text-blue-300 border border-blue-400/30">
              <Sparkles className="w-3.5 h-3.5 text-blue-300" />
              <span>Hệ thống Trợ lý Mentor Socratic</span>
            </div>
            <h1 className="text-2xl sm:text-3xl font-extrabold tracking-tight">
              Xin chào, {currentUser.name}! 👋
            </h1>
            <p className="text-slate-300 text-sm leading-relaxed">
              {isStudent
                ? `Bạn đang học môn "${selectedCourse.name}". Chính sách phản hồi hiện tại là ${currentPolicyInfo.label}. Hãy đặt câu hỏi sâu sắc để rèn luyện tư duy phản biện!`
                : `Chào Thầy! Thầy đang quản trị môn "${selectedCourse.name}". Thầy có thể phê duyệt câu hỏi, xem đánh giá tư duy của sinh viên và tinh chỉnh chính sách LLM.`}
            </p>
          </div>

          <div className="flex flex-wrap items-center gap-3">
            {!isStudent && (
              <button
                onClick={() => onNavigateTab('teacher_portal')}
                className="inline-flex items-center gap-2 px-4 py-2.5 rounded-xl font-bold text-sm bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-500 hover:to-indigo-500 text-white shadow-lg shadow-blue-600/30 ring-1 ring-white/20 transition"
              >
                <ShieldCheck className="w-4 h-4" />
                <span>Cổng Quản Trị Giảng Viên</span>
                <ArrowRight className="w-4 h-4" />
              </button>
            )}
            <button
              onClick={() => onNavigateTab('chat')}
              className="inline-flex items-center gap-2 px-4 py-2.5 rounded-xl font-semibold text-sm bg-blue-500 hover:bg-blue-400 text-white shadow-md shadow-blue-500/25 transition"
            >
              <BrainCircuit className="w-4 h-4" />
              <span>{isStudent ? 'Hỏi Trợ lý Mentor' : 'Trò chuyện cùng AI'}</span>
              <ArrowRight className="w-4 h-4" />
            </button>
            <button
              onClick={onOpenDocuments}
              className="inline-flex items-center gap-2 px-4 py-2.5 rounded-xl font-semibold text-sm bg-white/10 hover:bg-white/15 text-white border border-white/20 transition"
            >
              <FileText className="w-4 h-4" />
              <span>Tài liệu & RAG ({selectedCourse.documents_count})</span>
            </button>
            {onOpenLoginModal && (
              <button
                onClick={onOpenLoginModal}
                className="inline-flex items-center gap-2 px-3.5 py-2.5 rounded-xl font-semibold text-sm bg-white/10 hover:bg-white/20 text-white/90 border border-white/20 transition"
                title="Đăng nhập tài khoản khác"
              >
                <LogIn className="w-4 h-4" />
                <span className="hidden sm:inline">Đổi tài khoản</span>
              </button>
            )}
          </div>
        </div>
      </div>

      {/* 4 Stat Cards */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {/* Metric 1: Critical Thinking Score */}
        <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm transition hover:shadow-md">
          <div className="flex items-center justify-between">
            <span className="text-xs font-semibold text-slate-500 dark:text-slate-400">
              Điểm Tư duy Phản biện
            </span>
            <div className="w-9 h-9 rounded-xl bg-blue-50 dark:bg-blue-950/60 text-blue-600 dark:text-blue-400 flex items-center justify-center">
              <BrainCircuit className="w-5 h-5" />
            </div>
          </div>
          <div className="mt-3 flex items-baseline gap-2">
            <span className="text-2xl font-black text-slate-900 dark:text-white">
              {(avgScore * 100).toFixed(0)}%
            </span>
            <span className="text-xs font-semibold text-emerald-600 dark:text-emerald-400">
              +{Math.round(avgScore * 10)}% so với tuần trước
            </span>
          </div>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
            Dựa trên {studentAssessments.length} phiên đánh giá 7 chiều
          </p>
        </div>

        {/* Metric 2: Progress completion */}
        <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm transition hover:shadow-md">
          <div className="flex items-center justify-between">
            <span className="text-xs font-semibold text-slate-500 dark:text-slate-400">
              Tiến độ Lộ trình Môn học
            </span>
            <div className="w-9 h-9 rounded-xl bg-indigo-50 dark:bg-indigo-950/60 text-indigo-600 dark:text-indigo-400 flex items-center justify-center">
              <TrendingUp className="w-5 h-5" />
            </div>
          </div>
          <div className="mt-3 flex items-baseline gap-2">
            <span className="text-2xl font-black text-slate-900 dark:text-white">
              {progressPercent}%
            </span>
            <span className="text-xs text-slate-500 dark:text-slate-400">
              ({completedTopics}/{totalTopics} chủ đề)
            </span>
          </div>
          <div className="mt-2 w-full h-1.5 rounded-full bg-slate-100 dark:bg-slate-800 overflow-hidden">
            <div
              className="h-full bg-indigo-600 dark:bg-indigo-500 rounded-full transition-all duration-500"
              style={{ width: `${progressPercent}%` }}
            />
          </div>
        </div>

        {/* Metric 3: Question Bank */}
        <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm transition hover:shadow-md">
          <div className="flex items-center justify-between">
            <span className="text-xs font-semibold text-slate-500 dark:text-slate-400">
              Ngân hàng Câu hỏi
            </span>
            <div className="w-9 h-9 rounded-xl bg-sky-50 dark:bg-sky-950/60 text-sky-600 dark:text-sky-400 flex items-center justify-center">
              <HelpCircle className="w-5 h-5" />
            </div>
          </div>
          <div className="mt-3 flex items-baseline gap-2">
            <span className="text-2xl font-black text-slate-900 dark:text-white">
              {selectedCourse.questions_count}
            </span>
            <span className="text-xs font-medium text-slate-500 dark:text-slate-400">câu hỏi thực hành</span>
          </div>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
            Bao gồm câu hỏi của Thầy & AI tạo
          </p>
        </div>

        {/* Metric 4: Active Policy */}
        <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm transition hover:shadow-md">
          <div className="flex items-center justify-between">
            <span className="text-xs font-semibold text-slate-500 dark:text-slate-400">
              Chính sách Phản hồi
            </span>
            <div className="w-9 h-9 rounded-xl bg-amber-50 dark:bg-amber-950/60 text-amber-600 dark:text-amber-400 flex items-center justify-center">
              <Shield className="w-5 h-5" />
            </div>
          </div>
          <div className="mt-3">
            <span className={`inline-block px-2.5 py-0.5 rounded-md text-xs font-bold ${currentPolicyInfo.bg} ${currentPolicyInfo.text}`}>
              {selectedCourse.answer_policy}
            </span>
          </div>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400 line-clamp-1">
            {currentPolicyInfo.desc}
          </p>
        </div>
      </div>

      {/* Main Grid: Course Switcher & Critical Thinking Dimension Highlight */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left 2 Cols: Course Selection & Status */}
        <div className="lg:col-span-2 space-y-4">
          <div className="flex items-center justify-between">
            <h2 className="text-lg font-bold text-slate-900 dark:text-white flex items-center gap-2">
              <BookOpen className="w-5 h-5 text-blue-600 dark:text-blue-400" />
              Khóa học & Môn học sẵn có
            </h2>
            <span className="text-xs text-slate-500 dark:text-slate-400">
              Chọn khóa học để đổi ngữ cảnh học tập
            </span>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {(Array.isArray(courses) ? courses : []).map((course) => {
              const isSelected = course.id === selectedCourse.id;
              return (
                <div
                  key={course.id}
                  onClick={() => onSelectCourse(course)}
                  className={`p-5 rounded-2xl cursor-pointer border transition-all duration-200 ${
                    isSelected
                      ? 'border-blue-600 dark:border-blue-500 bg-blue-50/40 dark:bg-blue-950/30 ring-2 ring-blue-500/20 shadow-md'
                      : 'border-slate-200 dark:border-slate-800 bg-white dark:bg-[#0b1329] hover:border-slate-300 dark:hover:border-slate-700'
                  }`}
                >
                  <div className="flex items-start justify-between gap-3">
                    <span className="px-2.5 py-0.5 rounded-md text-[11px] font-bold tracking-wide uppercase bg-slate-100 text-slate-700 dark:bg-slate-800 dark:text-slate-300">
                      {course.code}
                    </span>
                    <span className="text-xs font-medium text-slate-500 dark:text-slate-400 flex items-center gap-1">
                      {course.enrolled_count} sinh viên
                    </span>
                  </div>

                  <h3 className="mt-2 text-base font-bold text-slate-900 dark:text-white leading-snug line-clamp-1">
                    {course.name}
                  </h3>

                  <p className="mt-1.5 text-xs text-slate-600 dark:text-slate-300 line-clamp-2 leading-relaxed">
                    {course.description}
                  </p>

                  <div className="mt-4 pt-3 border-t border-slate-100 dark:border-slate-800/80 flex items-center justify-between text-xs">
                    <span className="text-slate-500 dark:text-slate-400 font-medium">
                      GV: {course.teacher_name}
                    </span>
                    <span className="font-semibold text-blue-600 dark:text-blue-400 flex items-center gap-1">
                      {isSelected ? 'Đang kích hoạt' : 'Chọn học'}
                    </span>
                  </div>
                </div>
              );
            })}
          </div>

          {/* Quick Learning Roadmap Snippet */}
          <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm">
            <div className="flex items-center justify-between mb-4">
              <div className="flex items-center gap-2">
                <Layers className="w-5 h-5 text-indigo-600 dark:text-indigo-400" />
                <h3 className="font-bold text-sm text-slate-900 dark:text-white">
                  Tiến độ các mốc chính môn {selectedCourse.code}
                </h3>
              </div>
              <button
                onClick={() => onNavigateTab('roadmap')}
                className="text-xs font-semibold text-blue-600 dark:text-blue-400 hover:underline flex items-center gap-1"
              >
                Xem toàn bộ lộ trình <ArrowRight className="w-3.5 h-3.5" />
              </button>
            </div>

            <div className="space-y-2.5">
              {progress.slice(0, 3).map((item) => {
                const isDone = item.status === 'COMPLETED';
                const isLearning = item.status === 'LEARNING';
                return (
                  <div
                    key={item.id}
                    className="flex items-center justify-between p-3 rounded-xl bg-slate-50 dark:bg-slate-800/40 border border-slate-100 dark:border-slate-800"
                  >
                    <div className="flex items-center gap-3">
                      {isDone ? (
                        <CheckCircle2 className="w-4 h-4 text-emerald-500 shrink-0" />
                      ) : isLearning ? (
                        <Clock className="w-4 h-4 text-blue-500 shrink-0" />
                      ) : (
                        <div className="w-4 h-4 rounded-full border-2 border-slate-300 dark:border-slate-600 shrink-0" />
                      )}
                      <span className="text-xs font-medium text-slate-800 dark:text-slate-200 line-clamp-1">
                        {item.topic}
                      </span>
                    </div>
                    <div className="flex items-center gap-2 shrink-0">
                      <span className="text-xs font-bold text-slate-700 dark:text-slate-300">
                        {item.progress_value}%
                      </span>
                      <span
                        className={`text-[10px] font-semibold px-2 py-0.5 rounded-full ${
                          isDone
                            ? 'bg-emerald-50 text-emerald-700 dark:bg-emerald-950/50 dark:text-emerald-300'
                            : isLearning
                            ? 'bg-blue-50 text-blue-700 dark:bg-blue-950/50 dark:text-blue-300'
                            : 'bg-slate-200 text-slate-600 dark:bg-slate-800 dark:text-slate-400'
                        }`}
                      >
                        {isDone ? 'Hoàn thành' : isLearning ? 'Đang học' : 'Chưa bắt đầu'}
                      </span>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        </div>

        {/* Right 1 Col: Critical Thinking Radar & Recent Assessment */}
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <h2 className="text-lg font-bold text-slate-900 dark:text-white flex items-center gap-2">
              <Award className="w-5 h-5 text-amber-500" />
              Tư duy Phản biện (7 Chiều)
            </h2>
            <button
              onClick={() => onNavigateTab('assessment')}
              className="text-xs font-semibold text-blue-600 dark:text-blue-400 hover:underline"
            >
              Chi tiết
            </button>
          </div>

          {/* 7 Dimension visual breakdown */}
          <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-3.5">
            <p className="text-xs text-slate-500 dark:text-slate-400">
              Đo lường năng lực tư duy phản biện qua các tương tác học tập gần nhất:
            </p>

            {assessments[0] && (
              <div className="space-y-2.5">
                {[
                  { key: 'curiosity', label: 'Tính tò mò & ham học hỏi', val: assessments[0].dimensions.curiosity, color: 'bg-blue-600' },
                  { key: 'depth', label: 'Độ sâu tư duy & phân tích', val: assessments[0].dimensions.depth, color: 'bg-indigo-600' },
                  { key: 'assumption_awareness', label: 'Nhận thức giả định ngầm', val: assessments[0].dimensions.assumption_awareness, color: 'bg-purple-600' },
                  { key: 'causal_reasoning', label: 'Lập luận nhân quả & logic', val: assessments[0].dimensions.causal_reasoning, color: 'bg-emerald-600' },
                  { key: 'alternative_perspectives', label: 'Góc nhìn đa chiều & phản biện', val: assessments[0].dimensions.alternative_perspectives, color: 'bg-amber-500' },
                  { key: 'cross_domain_potential', label: 'Liên hệ liên ngành', val: assessments[0].dimensions.cross_domain_potential, color: 'bg-rose-500' },
                  { key: 'challenge_level', label: 'Mức độ thử thách câu hỏi', val: assessments[0].dimensions.challenge_level, color: 'bg-sky-500' },
                ].map((d) => (
                  <div key={d.key} className="space-y-1">
                    <div className="flex justify-between text-[11px] font-medium text-slate-700 dark:text-slate-300">
                      <span>{d.label}</span>
                      <span className="font-bold">{(d.val * 100).toFixed(0)}%</span>
                    </div>
                    <div className="w-full h-1.5 bg-slate-100 dark:bg-slate-800 rounded-full overflow-hidden">
                      <div
                        className={`h-full ${d.color} rounded-full transition-all duration-300`}
                        style={{ width: `${d.val * 100}%` }}
                      />
                    </div>
                  </div>
                ))}
              </div>
            )}

            {/* Latest assessment reasoning box */}
            {assessments[0] && (
              <div className="mt-4 p-3 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-100 dark:border-slate-800 text-xs">
                <p className="font-semibold text-slate-800 dark:text-slate-200 mb-1">
                  Nhận xét AI gần nhất:
                </p>
                <p className="text-slate-600 dark:text-slate-300 italic leading-relaxed">
                  "{assessments[0].reasoning}"
                </p>
                {assessments[0].teacher_feedback && (
                  <div className="mt-2 pt-2 border-t border-slate-200/60 dark:border-slate-700/60 text-emerald-600 dark:text-emerald-400">
                    <span className="font-bold">Đánh giá của Thầy: </span>
                    {assessments[0].teacher_feedback}
                  </div>
                )}
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
