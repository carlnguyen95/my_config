import React, { useState, useEffect } from 'react';
import { 
  GraduationCap,
  Users,
  BookOpen,
  FileText,
  CheckCircle2,
  Clock,
  AlertCircle,
  Sparkles,
  Plus,
  Search,
  Sliders,
  Cpu,
  Layers,
  Award,
  Send,
  RefreshCw,
  FileDown,
  Copy,
  Check,
  ChevronRight,
  Filter,
  ShieldCheck,
  Eye,
  BarChart3,
  BrainCircuit,
  ArrowUpRight,
  HelpCircle,
  ExternalLink,
  Trash2,
  ChevronDown,
  Database,
  UploadCloud,
  FolderPlus,
} from 'lucide-react';
import {
  User as UserType,
  Course,
  ThinkingAssessment,
  Question,
  Roadmap,
  LearningProgressItem,
  CourseDocument,
  AIProviderState,
  AnswerPolicy,
  StudentRosterItem,
} from '../types';
import { apiFetch, readApiJson } from '../api';
import { CreateCourseModal } from './teacher/CreateCourseModal';
import { UploadDocumentModal } from './teacher/UploadDocumentModal';
import { BatchImportQuestionsModal } from './teacher/BatchImportQuestionsModal';

interface TeacherPortalProps {
  currentUser: UserType;
  courses: Course[];
  selectedCourse: Course;
  onSelectCourse: (course: Course) => void;
  assessments: ThinkingAssessment[];
  questions: Question[];
  roadmap: Roadmap | null;
  progress: LearningProgressItem[];
  courseDocuments: CourseDocument[];
  providerState: AIProviderState | null;
  onReviewAssessment: (
    assessmentId: string,
    teacherScore: number,
    teacherFeedback: string
  ) => Promise<void>;
  onApproveQuestion: (id: string) => Promise<void>;
  onAddQuestion: (data: {
    question: string;
    answer: string;
    hint: string;
    difficulty: 'beginner' | 'intermediate' | 'advanced';
    question_type: 'conceptual' | 'scenario' | 'multiple_choice';
  }) => Promise<void>;
  onGenerateAIQuestion: (topic?: string, difficulty?: string) => Promise<void>;
  isGeneratingQuestion: boolean;
  onUploadDocument: (title: string, content: string, tags: string[], sourcePath?: string) => Promise<void>;
  onDeleteDocument?: (docId: string) => Promise<void>;
  onCreateCourse?: (data: {
    name: string;
    code: string;
    description: string;
    answer_policy: AnswerPolicy;
    enrolled_count: number;
  }) => Promise<void>;
  onBatchImportQuestions?: (questions: any[]) => Promise<void>;
  onBatchGenerateAIQuestions?: (topic: string, count: number, difficulty: string, documentId?: string) => Promise<void>;
  onUpdateCoursePolicy: (policy: AnswerPolicy) => Promise<void>;
  onSwitchProvider: (provider: 'gemini' | 'ollama', cloudFallback: boolean) => Promise<void>;
  onNavigateToChat: (prompt?: string) => void;
  onNavigateToStudentView?: () => void;
}

export const TeacherPortal: React.FC<TeacherPortalProps> = ({
  currentUser,
  courses,
  selectedCourse,
  onSelectCourse,
  assessments,
  questions,
  roadmap,
  progress,
  courseDocuments,
  providerState,
  onReviewAssessment,
  onApproveQuestion,
  onAddQuestion,
  onGenerateAIQuestion,
  isGeneratingQuestion,
  onUploadDocument,
  onDeleteDocument,
  onCreateCourse,
  onBatchImportQuestions,
  onBatchGenerateAIQuestions,
  onUpdateCoursePolicy,
  onSwitchProvider,
  onNavigateToChat,
  onNavigateToStudentView,
}) => {
  // Active sub-tab inside Teacher Portal
  const [activeSubTab, setActiveSubTab] = useState<
    'overview' | 'grading' | 'question_studio' | 'curriculum_rag' | 'student_roster' | 'ai_policy'
  >('overview');

  // Grading desk state
  const [selectedAssessment, setSelectedAssessment] = useState<ThinkingAssessment | null>(
    assessments[0] || null
  );
  const [gradingScore, setGradingScore] = useState<number>(0.85);
  const [gradingFeedback, setGradingFeedback] = useState<string>('');
  const [isSubmittingGrade, setIsSubmittingGrade] = useState(false);
  const [gradingFilter, setGradingFilter] = useState<'all' | 'pending' | 'reviewed'>('pending');

  // Question Studio state
  const [questionSearch, setQuestionSearch] = useState('');
  const [questionStatusFilter, setQuestionStatusFilter] = useState<'all' | 'review' | 'approved'>('all');
  const [aiTopicInput, setAiTopicInput] = useState('TCP Congestion Control & Bufferbloat');
  const [aiDifficultyInput, setAiDifficultyInput] = useState<'beginner' | 'intermediate' | 'advanced'>('intermediate');
  const [isCreatingQuestion, setIsCreatingQuestion] = useState(false);
  const [newQText, setNewQText] = useState('');
  const [newQAnswer, setNewQAnswer] = useState('');
  const [newQHint, setNewQHint] = useState('');
  const [newQDifficulty, setNewQDifficulty] = useState<'beginner' | 'intermediate' | 'advanced'>('intermediate');

  // Exam generator modal
  const [showExamModal, setShowExamModal] = useState(false);
  const [examTitle, setExamTitle] = useState(`Đề Đánh Giá Năng Lực Tư Duy: ${selectedCourse.name}`);
  const [examDuration, setExamDuration] = useState(45);
  const [generatedExamData, setGeneratedExamData] = useState<any | null>(null);
  const [isGeneratingExam, setIsGeneratingExam] = useState(false);
  const [copiedExam, setCopiedExam] = useState(false);

  // Student Roster state
  const [roster, setRoster] = useState<StudentRosterItem[]>([]);
  const [isLoadingRoster, setIsLoadingRoster] = useState(false);
  const [rosterFilter, setRosterFilter] = useState<'all' | 'needs_help' | 'excellent'>('all');

  // RAG Inspector state
  const [ragQuery, setRagQuery] = useState('Khác biệt giữa Flow Control và Congestion Control là gì?');
  const [ragResults, setRagResults] = useState<any[]>([]);
  const [isTestingRag, setIsTestingRag] = useState(false);

  // Modals for Teacher actions
  const [showUploadModal, setShowUploadModal] = useState(false);
  const [showCreateCourseModal, setShowCreateCourseModal] = useState(false);
  const [showBatchImportModal, setShowBatchImportModal] = useState(false);

  // Delete Document Handler
  const handleDeleteDoc = async (docId: string, title: string) => {
    if (!onDeleteDocument) return;
    if (!confirm(`Bạn có chắc chắn muốn xóa tài liệu "${title}" khỏi hệ thống tri thức RAG của môn học?`)) {
      return;
    }
    try {
      await onDeleteDocument(docId);
    } catch (err: any) {
      alert(err.message || 'Không thể xóa tài liệu');
    }
  };

  // Sync selected assessment when assessments change
  useEffect(() => {
    if (assessments.length > 0 && !selectedAssessment) {
      setSelectedAssessment(assessments[0]);
      setGradingScore(assessments[0].teacher_score ?? assessments[0].score);
      setGradingFeedback(assessments[0].teacher_feedback || '');
    }
  }, [assessments]);

  // Load student roster for selected course
  useEffect(() => {
    loadClassRoster();
  }, [selectedCourse.id]);

  const loadClassRoster = async () => {
    setIsLoadingRoster(true);
    try {
      const res = await apiFetch(`/api/teacher/class-roster/${selectedCourse.id}`);
      const data = await readApiJson<any>(res);
      if (data.roster) {
        setRoster(data.roster);
      }
    } catch (err) {
      console.error('Error loading roster:', err);
    } finally {
      setIsLoadingRoster(false);
    }
  };

  // Submit assessment grade
  const handleSubmitGrade = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!selectedAssessment) return;
    setIsSubmittingGrade(true);
    try {
      await onReviewAssessment(selectedAssessment.id, gradingScore, gradingFeedback);
      // Update local item
      setSelectedAssessment((prev) =>
        prev
          ? {
              ...prev,
              teacher_score: gradingScore,
              teacher_feedback: gradingFeedback,
              review_status: 'reviewed',
            }
          : null
      );
    } catch (err) {
      console.error('Error grading assessment:', err);
    } finally {
      setIsSubmittingGrade(false);
    }
  };

  // Run RAG Inspection Test
  const handleTestRag = async () => {
    if (!ragQuery.trim()) return;
    setIsTestingRag(true);
    try {
      const res = await apiFetch('/api/teacher/test-rag', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          course_id: selectedCourse.id,
          query: ragQuery,
          top_k: 3,
        }),
      });
      const data = await readApiJson<any>(res);
      setRagResults(data.results || []);
    } catch (err) {
      console.error('Error testing RAG:', err);
    } finally {
      setIsTestingRag(false);
    }
  };

  // Create new teacher question
  const handleCreateQuestion = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!newQText.trim() || !newQAnswer.trim()) return;
    await onAddQuestion({
      question: newQText,
      answer: newQAnswer,
      hint: newQHint || 'Phân tích các thuộc tính và nguyên lý cốt lõi liên quan.',
      difficulty: newQDifficulty,
      question_type: 'conceptual',
    });
    setNewQText('');
    setNewQAnswer('');
    setNewQHint('');
    setIsCreatingQuestion(false);
  };

  // Generate Exam
  const handleGenerateExam = async () => {
    setIsGeneratingExam(true);
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/generate-exam`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          title: examTitle,
          exam_duration_minutes: examDuration,
          user_id: currentUser.id,
        }),
      });
      const data = await readApiJson<any>(res);
      if (data.exam) {
        setGeneratedExamData(data.exam);
      }
    } catch (err) {
      console.error('Error generating exam:', err);
    } finally {
      setIsGeneratingExam(false);
    }
  };

  const handleCopyExamMarkdown = () => {
    if (!generatedExamData) return;
    const md = `# ${generatedExamData.title}
Môn học: ${generatedExamData.course_name} (${generatedExamData.course_code})
Thời gian làm bài: ${generatedExamData.duration_minutes} phút | Người biên soạn: ${generatedExamData.created_by}

---

## Danh Sách Câu Hỏi Kiểm Tra Đánh Giá Năng Lực Tư Duy:
${generatedExamData.questions
  .map(
    (q: any) => `### Câu ${q.number} [${q.difficulty.toUpperCase()}]:
**Câu hỏi:** ${q.question}

*Gợi mở Socratic:* ${q.hint_socratic}

*Đáp án học thuật chuẩn mực:*
${q.ideal_answer}

*Barem chấm tư duy:*
- ${q.grading_rubric.understanding}
- ${q.grading_rubric.causal_analysis}
- ${q.grading_rubric.critical_thinking}
`
  )
  .join('\n\n')}
`;
    navigator.clipboard.writeText(md);
    setCopiedExam(true);
    setTimeout(() => setCopiedExam(false), 2000);
  };

  // Calculations for KPI Cards
  const pendingAssessments = assessments.filter((a) => a.review_status === 'pending');
  const pendingQuestions = questions.filter(
    (q) => q.course_id === selectedCourse.id && q.status === 'review'
  );
  const approvedQuestions = questions.filter(
    (q) => q.course_id === selectedCourse.id && q.status === 'approved'
  );
  const avgClassThinkingScore =
    assessments.length > 0
      ? (
          assessments.reduce((acc, a) => acc + (a.teacher_score ?? a.score), 0) /
          assessments.length
        ).toFixed(2)
      : '0.80';

  const filteredAssessments = assessments.filter((a) => {
    if (gradingFilter === 'all') return true;
    return a.review_status === gradingFilter;
  });

  const filteredQuestions = questions.filter((q) => {
    if (q.course_id !== selectedCourse.id) return false;
    if (questionStatusFilter !== 'all' && q.status !== questionStatusFilter) return false;
    if (questionSearch.trim()) {
      const s = questionSearch.toLowerCase();
      return q.question.toLowerCase().includes(s) || q.answer.toLowerCase().includes(s);
    }
    return true;
  });

  const filteredRoster = roster.filter((item) => {
    if (rosterFilter === 'all') return true;
    return item.status_alert === rosterFilter;
  });

  return (
    <div className="space-y-6 animate-fadeIn pb-12">
      {/* 1. TEACHER HERO BANNER & COURSE SELECTOR */}
      <div className="relative overflow-hidden rounded-2xl bg-gradient-to-br from-slate-900 via-blue-950 to-slate-900 border border-blue-900/60 p-6 text-white shadow-xl">
        <div className="absolute top-0 right-0 w-96 h-96 bg-blue-500/10 rounded-full blur-3xl pointer-events-none" />
        <div className="relative z-10 flex flex-col lg:flex-row lg:items-center justify-between gap-6">
          <div className="space-y-2">
            <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-blue-500/20 border border-blue-400/30 text-blue-300 text-xs font-semibold">
              <ShieldCheck className="w-3.5 h-3.5 text-blue-400" />
              Teacher Studio & Quản Trị Sư Phạm AI
            </div>
            <h1 className="text-2xl sm:text-3xl font-extrabold tracking-tight text-white flex items-center gap-3">
              <span>Xin chào, {currentUser.name}</span>
            </h1>
            <p className="text-sm text-slate-300 max-w-2xl">
              Không gian làm việc dành riêng cho Giảng viên: Điều phối chính sách phản xạ Socratic,
              chấm điểm tư duy phản biện 7 chiều, kiểm duyệt học liệu và tạo đề kiểm tra thông minh.
            </p>
          </div>

          {/* Quick Actions & Course Switcher */}
          <div className="flex flex-wrap items-center gap-2.5">
            {/* Course Switcher */}
            <div className="flex items-center gap-2 px-3.5 py-2 rounded-xl bg-slate-800/80 border border-slate-700 text-sm">
              <BookOpen className="w-4 h-4 text-blue-400" />
              <select
                aria-label="Chọn môn học giảng dạy"
                value={selectedCourse.id}
                onChange={(e) => {
                  const found = courses.find((c) => c.id === e.target.value);
                  if (found) onSelectCourse(found);
                }}
                className="bg-transparent text-white font-semibold focus:outline-none cursor-pointer"
              >
                {courses.map((c) => (
                  <option key={c.id} value={c.id} className="bg-slate-900 text-white">
                    {c.code} — {c.name.split(':')[0]}
                  </option>
                ))}
              </select>
            </div>

            {/* Add Course Button */}
            {onCreateCourse && (
              <button
                onClick={() => setShowCreateCourseModal(true)}
                className="flex items-center gap-1.5 px-3.5 py-2 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white text-xs font-bold shadow-md shadow-emerald-600/20 transition"
                title="Tạo thêm môn học / lớp học mới"
              >
                <Plus className="w-4 h-4" />
                <span>Thêm Lớp Mới</span>
              </button>
            )}

            {/* Quick Upload Document */}
            <button
              onClick={() => setShowUploadModal(true)}
              className="flex items-center gap-1.5 px-3.5 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-slate-200 text-xs font-semibold transition"
              title="Nạp tài liệu giáo trình RAG mới cho môn học này"
            >
              <UploadCloud className="w-4 h-4 text-blue-400" />
              <span>Nạp Tài Liệu</span>
            </button>

            {/* Quick Deposit Question Bank */}
            <button
              onClick={() => setShowBatchImportModal(true)}
              className="flex items-center gap-1.5 px-3.5 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-slate-200 text-xs font-semibold transition"
              title="Nạp hàng loạt câu hỏi hoặc sinh đề AI vào Ngân Hàng Câu Hỏi"
            >
              <Database className="w-4 h-4 text-indigo-400" />
              <span>Nạp Question Bank</span>
            </button>

            {/* Switch to Student View */}
            {onNavigateToStudentView && (
              <button
                onClick={onNavigateToStudentView}
                className="flex items-center gap-2 px-3.5 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 border border-slate-700 text-slate-200 text-xs font-medium transition"
                title="Chuyển sang góc nhìn Sinh viên để kiểm thử bài giảng"
              >
                <Eye className="w-4 h-4 text-slate-400" />
                <span>Góc nhìn Học viên</span>
              </button>
            )}

            {/* Test AI Mentor in Chat */}
            <button
              onClick={() => onNavigateToChat('Chào Mentor, hãy giải thích cơ chế Flow Control theo góc độ giảng viên!')}
              className="flex items-center gap-2 px-4 py-2 rounded-xl bg-blue-600 hover:bg-blue-500 text-white text-xs font-semibold shadow-md shadow-blue-600/30 transition"
            >
              <Sparkles className="w-4 h-4" />
              <span>Thử Nghiệm Mentor</span>
            </button>
          </div>
        </div>

        {/* 4 CORE KPI METRICS FOR TEACHER */}
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 mt-6 pt-6 border-t border-slate-800/80">
          <div className="p-3.5 rounded-xl bg-slate-800/50 border border-slate-700/60">
            <div className="flex items-center justify-between text-xs text-slate-400">
              <span>Sĩ số lớp</span>
              <Users className="w-4 h-4 text-blue-400" />
            </div>
            <p className="text-2xl font-bold text-white mt-1">{selectedCourse.enrolled_count}</p>
            <span className="text-[11px] text-emerald-400 font-medium">100% tài khoản đã kích hoạt</span>
          </div>

          <div
            onClick={() => setActiveSubTab('grading')}
            className="p-3.5 rounded-xl bg-slate-800/50 border border-slate-700/60 cursor-pointer hover:border-blue-500/50 transition"
          >
            <div className="flex items-center justify-between text-xs text-slate-400">
              <span>Chờ chấm tư duy</span>
              <Award className="w-4 h-4 text-amber-400" />
            </div>
            <p className="text-2xl font-bold text-amber-400 mt-1">{pendingAssessments.length}</p>
            <span className="text-[11px] text-slate-400">Yêu cầu phản hồi sư phạm</span>
          </div>

          <div
            onClick={() => setActiveSubTab('question_studio')}
            className="p-3.5 rounded-xl bg-slate-800/50 border border-slate-700/60 cursor-pointer hover:border-blue-500/50 transition"
          >
            <div className="flex items-center justify-between text-xs text-slate-400">
              <span>Câu hỏi AI chờ duyệt</span>
              <HelpCircle className="w-4 h-4 text-indigo-400" />
            </div>
            <p className="text-2xl font-bold text-indigo-300 mt-1">{pendingQuestions.length}</p>
            <span className="text-[11px] text-slate-400">Đã chuẩn hóa: {approvedQuestions.length} câu</span>
          </div>

          <div
            onClick={() => setActiveSubTab('ai_policy')}
            className="p-3.5 rounded-xl bg-slate-800/50 border border-slate-700/60 cursor-pointer hover:border-blue-500/50 transition"
          >
            <div className="flex items-center justify-between text-xs text-slate-400">
              <span>Chính sách AI hiện tại</span>
              <Sliders className="w-4 h-4 text-emerald-400" />
            </div>
            <p className="text-lg font-bold text-emerald-400 mt-1 truncate">
              {selectedCourse.answer_policy}
            </p>
            <span className="text-[11px] text-slate-300">
              {selectedCourse.answer_policy === 'HINT_ONLY'
                ? 'Gợi ý Socratic nghiêm ngặt'
                : selectedCourse.answer_policy === 'GUIDED'
                ? 'Dẫn dắt 2 bước'
                : 'Giải thích chi tiết'}
            </span>
          </div>
        </div>
      </div>

      {/* 2. TEACHER NAVIGATION SUB-TABS */}
      <div className="flex overflow-x-auto p-1.5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm no-scrollbar">
        {[
          { id: 'overview', label: 'Tổng quan & Phân tích', icon: BarChart3 },
          {
            id: 'grading',
            label: `Bàn Chấm Điểm Tư Duy (${pendingAssessments.length})`,
            icon: Award,
            badge: pendingAssessments.length > 0,
          },
          {
            id: 'question_studio',
            label: 'Xưởng Câu Hỏi & Sinh Đề AI',
            icon: BrainCircuit,
          },
          { id: 'curriculum_rag', label: 'Giáo Trình & Tri Thức RAG', icon: Layers },
          { id: 'student_roster', label: 'Sổ Sinh Viên & Cảnh Báo', icon: Users },
          { id: 'ai_policy', label: 'Chính Sách & Hạ Tầng AI', icon: Sliders },
        ].map((tab) => {
          const Icon = tab.icon;
          const isActive = activeSubTab === tab.id;
          return (
            <button
              key={tab.id}
              onClick={() => setActiveSubTab(tab.id as any)}
              className={`flex items-center gap-2 px-4 py-2.5 rounded-xl text-xs sm:text-sm font-semibold whitespace-nowrap transition-all duration-150 ${
                isActive
                  ? 'bg-blue-900 text-white shadow-sm dark:bg-blue-600 dark:text-white'
                  : 'text-slate-600 hover:text-slate-900 hover:bg-slate-100 dark:text-slate-300 dark:hover:text-white dark:hover:bg-slate-800/60'
              }`}
            >
              <Icon className={`w-4 h-4 ${isActive ? 'text-white' : 'text-slate-400'}`} />
              <span>{tab.label}</span>
              {tab.badge && (
                <span className="w-2 h-2 rounded-full bg-amber-400 animate-pulse" />
              )}
            </button>
          );
        })}
      </div>

      {/* 3. TAB CONTENT VIEWS */}

      {/* --- SUB-TAB 1: OVERVIEW & ANALYTICS --- */}
      {activeSubTab === 'overview' && (
        <div className="space-y-6">
          <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
            {/* 7 Critical Thinking Dimensions Radar/Cohort Matrix */}
            <div className="lg:col-span-2 p-5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
              <div className="flex items-center justify-between">
                <div>
                  <h2 className="text-base font-bold text-slate-900 dark:text-white flex items-center gap-2">
                    <BarChart3 className="w-5 h-5 text-blue-600 dark:text-blue-400" />
                    Phổ Năng Lực Tư Duy Phản Biện Toàn Lớp (7 Chiều)
                  </h2>
                  <p className="text-xs text-slate-500 dark:text-slate-400">
                    Phân bổ chỉ số trung bình từ các phiên tương tác học tập với Socratic Mentor
                  </p>
                </div>
                <span className="px-3 py-1 rounded-full text-xs font-bold bg-blue-50 text-blue-700 dark:bg-blue-950 dark:text-blue-300 border border-blue-200 dark:border-blue-800">
                  Điểm TB Lớp: {avgClassThinkingScore}/1.00
                </span>
              </div>

              {/* 7 Dimensions Bar breakdown */}
              <div className="space-y-3 pt-2">
                {[
                  { key: 'curiosity', label: '1. Tính tò mò & ham học hỏi', score: 0.88, desc: 'Chủ động khám phá cơ chế ngầm định' },
                  { key: 'depth', label: '2. Độ sâu tư duy & cấu trúc', score: 0.82, desc: 'Đi từ bề mặt vào kiến trúc tầng lõi' },
                  { key: 'assumption_awareness', label: '3. Nhận thức giả định ngầm', score: 0.77, desc: 'Kiểm chứng các điều kiện tiền đề' },
                  { key: 'causal_reasoning', label: '4. Lập luận nhân quả & logic', score: 0.85, desc: 'Mạch liên kết nguyên nhân - hệ quả' },
                  { key: 'alternative_perspectives', label: '5. Góc nhìn phản biện đa chiều', score: 0.76, desc: 'Phản biện các phương án đối nghịch' },
                  { key: 'cross_domain_potential', label: '6. Liên hệ liên ngành', score: 0.74, desc: 'Vận dụng sang lý thuyết trò chơi/vật lý' },
                  { key: 'challenge_level', label: '7. Mức độ thử thách & độ khó', score: 0.83, desc: 'Độ phức tạp của bài toán sinh viên đặt ra' },
                ].map((item) => {
                  const pct = Math.round(item.score * 100);
                  const isHigh = item.score >= 0.8;
                  return (
                    <div key={item.key} className="space-y-1">
                      <div className="flex justify-between items-center text-xs">
                        <span className="font-semibold text-slate-800 dark:text-slate-200">
                          {item.label}
                        </span>
                        <div className="flex items-center gap-2">
                          <span className="text-[11px] text-slate-400 hidden sm:inline">{item.desc}</span>
                          <span
                            className={`font-mono font-bold ${
                              isHigh ? 'text-blue-600 dark:text-blue-400' : 'text-amber-600 dark:text-amber-400'
                            }`}
                          >
                            {item.score.toFixed(2)}
                          </span>
                        </div>
                      </div>
                      <div className="w-full h-2 rounded-full bg-slate-100 dark:bg-slate-800 overflow-hidden">
                        <div
                          className={`h-full rounded-full transition-all duration-500 ${
                            isHigh ? 'bg-blue-600 dark:bg-blue-500' : 'bg-amber-500'
                          }`}
                          style={{ width: `${pct}%` }}
                        />
                      </div>
                    </div>
                  );
                })}
              </div>

              <div className="p-3.5 rounded-xl bg-blue-50/60 dark:bg-blue-950/40 border border-blue-200/60 dark:border-blue-900/60 text-xs text-blue-900 dark:text-blue-200 flex items-start gap-2.5">
                <Sparkles className="w-4 h-4 text-blue-600 shrink-0 mt-0.5" />
                <div>
                  <strong className="font-semibold">Nhận định sư phạm AI:</strong> Chiều{' '}
                  <em>"Tính tò mò" (0.88)</em> và <em>"Lập luận nhân quả" (0.85)</em> đang phát triển rất tốt.
                  Giảng viên nên thiết kế thêm bài tập tình huống thực tế để nâng cao{' '}
                  <em>"Góc nhìn phản biện đa chiều" (0.76)</em>.
                </div>
              </div>
            </div>

            {/* Quick Action Tasks for Teacher */}
            <div className="space-y-4">
              <div className="p-5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
                <h3 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-2">
                  <Clock className="w-4 h-4 text-amber-500" />
                  Nhiệm Vụ Cần Xử Lý Ngay
                </h3>

                <div className="space-y-2.5">
                  <div
                    onClick={() => setActiveSubTab('grading')}
                    className="p-3 rounded-xl border border-amber-200 dark:border-amber-900/40 bg-amber-50/50 dark:bg-amber-950/20 hover:bg-amber-100/50 transition cursor-pointer flex items-center justify-between"
                  >
                    <div>
                      <p className="text-xs font-bold text-amber-900 dark:text-amber-300">
                        {pendingAssessments.length} tương tác cần chấm duyệt
                      </p>
                      <p className="text-[11px] text-amber-700/80 dark:text-amber-400">
                        Từ học viên Nguyễn Hoàng Nam, Lê Thị Mai Anh...
                      </p>
                    </div>
                    <ChevronRight className="w-4 h-4 text-amber-600" />
                  </div>

                  <div
                    onClick={() => setActiveSubTab('question_studio')}
                    className="p-3 rounded-xl border border-indigo-200 dark:border-indigo-900/40 bg-indigo-50/50 dark:bg-indigo-950/20 hover:bg-indigo-100/50 transition cursor-pointer flex items-center justify-between"
                  >
                    <div>
                      <p className="text-xs font-bold text-indigo-900 dark:text-indigo-300">
                        {pendingQuestions.length} câu hỏi AI đề xuất chờ duyệt
                      </p>
                      <p className="text-[11px] text-indigo-700/80 dark:text-indigo-400">
                        Chủ đề: Tắc nghẽn mạng & Bắt tay 3 bước
                      </p>
                    </div>
                    <ChevronRight className="w-4 h-4 text-indigo-600" />
                  </div>

                  <div
                    onClick={() => setActiveSubTab('curriculum_rag')}
                    className="p-3 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-900 hover:bg-slate-100 dark:hover:bg-slate-800/80 transition cursor-pointer flex items-center justify-between"
                  >
                    <div>
                      <p className="text-xs font-bold text-slate-800 dark:text-slate-200">
                        {courseDocuments.length} tài liệu giáo trình RAG
                      </p>
                      <p className="text-[11px] text-slate-500">
                        Đang làm cơ sở tri thức trích dẫn cho Mentor
                      </p>
                    </div>
                    <ChevronRight className="w-4 h-4 text-slate-400" />
                  </div>
                </div>

                <div className="pt-2 border-t border-slate-100 dark:border-slate-800 grid grid-cols-2 gap-2">
                  {onCreateCourse && (
                    <button
                      onClick={() => setShowCreateCourseModal(true)}
                      className="py-2 px-3 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white text-xs font-bold flex items-center justify-center gap-1.5 shadow-xs transition"
                    >
                      <Plus className="w-3.5 h-3.5" />
                      <span>Thêm Lớp Mới</span>
                    </button>
                  )}
                  <button
                    onClick={() => setShowUploadModal(true)}
                    className="py-2 px-3 rounded-xl border border-blue-200 dark:border-blue-800 bg-blue-50 dark:bg-blue-950/40 text-blue-700 dark:text-blue-300 hover:bg-blue-100 dark:hover:bg-blue-900/60 text-xs font-semibold flex items-center justify-center gap-1.5 transition"
                  >
                    <UploadCloud className="w-3.5 h-3.5 text-blue-600 dark:text-blue-400" />
                    <span>Nạp Tài Liệu</span>
                  </button>
                  <button
                    onClick={() => setShowBatchImportModal(true)}
                    className="py-2 px-3 rounded-xl border border-indigo-200 dark:border-indigo-800 bg-indigo-50 dark:bg-indigo-950/40 text-indigo-700 dark:text-indigo-300 hover:bg-indigo-100 dark:hover:bg-indigo-900/60 text-xs font-semibold flex items-center justify-center gap-1.5 transition"
                  >
                    <Database className="w-3.5 h-3.5 text-indigo-600 dark:text-indigo-400" />
                    <span>Nạp Question Bank</span>
                  </button>
                  <button
                    onClick={() => setShowExamModal(true)}
                    className="py-2 px-3 rounded-xl bg-slate-800 hover:bg-slate-700 text-white text-xs font-semibold flex items-center justify-center gap-1.5 shadow-xs transition"
                  >
                    <FileDown className="w-3.5 h-3.5 text-blue-400" />
                    <span>Xuất Đề Thi</span>
                  </button>
                </div>
              </div>

              {/* Class Progress summary */}
              <div className="p-4 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-2">
                <span className="text-xs font-bold text-slate-900 dark:text-white flex items-center gap-1.5">
                  <CheckCircle2 className="w-4 h-4 text-emerald-500" /> Tiến Độ Lộ Trình Cả Lớp
                </span>
                <p className="text-xs text-slate-500 dark:text-slate-400">
                  {roadmap ? `${roadmap.topics.length} chủ đề chính khóa học` : 'Đang đồng bộ lộ trình...'}
                </p>
                <div className="flex items-center gap-2 pt-1">
                  <div className="flex-1 h-2 rounded-full bg-slate-100 dark:bg-slate-800 overflow-hidden">
                    <div className="h-full bg-emerald-500 rounded-full" style={{ width: '68%' }} />
                  </div>
                  <span className="text-xs font-bold font-mono text-emerald-600 dark:text-emerald-400">
                    68%
                  </span>
                </div>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* --- SUB-TAB 2: GRADING & ASSESSMENT DESK --- */}
      {activeSubTab === 'grading' && (
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
          {/* Left: Assessments Queue List */}
          <div className="lg:col-span-5 space-y-3">
            <div className="flex items-center justify-between">
              <h2 className="text-sm font-bold text-slate-900 dark:text-white flex items-center gap-2">
                <Award className="w-4 h-4 text-blue-600 dark:text-blue-400" />
                Hàng Đợi Chấm Điểm ({filteredAssessments.length})
              </h2>

              <div className="flex items-center gap-1 p-0.5 rounded-lg bg-slate-100 dark:bg-slate-800 text-xs">
                {(['pending', 'reviewed', 'all'] as const).map((status) => (
                  <button
                    key={status}
                    onClick={() => setGradingFilter(status)}
                    className={`px-2.5 py-1 rounded-md capitalize font-medium transition ${
                      gradingFilter === status
                        ? 'bg-white dark:bg-slate-700 text-blue-600 dark:text-blue-400 shadow-xs'
                        : 'text-slate-500 hover:text-slate-900 dark:hover:text-white'
                    }`}
                  >
                    {status === 'pending' ? 'Chờ duyệt' : status === 'reviewed' ? 'Đã duyệt' : 'Tất cả'}
                  </button>
                ))}
              </div>
            </div>

            <div className="space-y-2 max-h-[640px] overflow-y-auto pr-1">
              {filteredAssessments.length === 0 ? (
                <div className="p-8 rounded-xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 text-center text-slate-400 text-xs">
                  Không có đánh giá nào thuộc trạng thái này.
                </div>
              ) : (
                filteredAssessments.map((asm) => {
                  const isSelected = selectedAssessment?.id === asm.id;
                  const isReviewed = asm.review_status === 'reviewed';
                  return (
                    <div
                      key={asm.id}
                      onClick={() => {
                        setSelectedAssessment(asm);
                        setGradingScore(asm.teacher_score ?? asm.score);
                        setGradingFeedback(asm.teacher_feedback || '');
                      }}
                      className={`p-4 rounded-xl border text-left cursor-pointer transition-all duration-150 ${
                        isSelected
                          ? 'border-blue-500 bg-blue-50/60 dark:bg-blue-950/40 shadow-sm ring-1 ring-blue-500/30'
                          : 'border-slate-200 dark:border-slate-800 bg-white dark:bg-[#070e1b] hover:border-slate-300 dark:hover:border-slate-700'
                      }`}
                    >
                      <div className="flex items-center justify-between text-xs mb-1.5">
                        <span className="font-bold text-slate-900 dark:text-white flex items-center gap-1.5">
                          <Users className="w-3.5 h-3.5 text-slate-400" />
                          {asm.student_name}
                        </span>
                        <span
                          className={`px-2 py-0.5 rounded-full text-[10px] font-bold ${
                            isReviewed
                              ? 'bg-emerald-50 text-emerald-700 dark:bg-emerald-950/60 dark:text-emerald-400 border border-emerald-200 dark:border-emerald-800'
                              : 'bg-amber-50 text-amber-700 dark:bg-amber-950/60 dark:text-amber-400 border border-amber-200 dark:border-amber-800'
                          }`}
                        >
                          {isReviewed ? 'Đã duyệt' : 'Chờ chấm'}
                        </span>
                      </div>

                      <p className="text-xs text-slate-700 dark:text-slate-300 line-clamp-2 italic mb-2">
                        "{asm.prompt_excerpt}"
                      </p>

                      <div className="flex items-center justify-between text-[11px] pt-2 border-t border-slate-100 dark:border-slate-800/80">
                        <span className="text-slate-400">
                          AI Chấm: <strong className="text-blue-600 font-mono">{(asm.score * 10).toFixed(1)}/10</strong>
                        </span>
                        {asm.teacher_score !== undefined && (
                          <span className="font-semibold text-emerald-600 dark:text-emerald-400">
                            GV: {(asm.teacher_score * 10).toFixed(1)}/10
                          </span>
                        )}
                        <span className="text-slate-400 text-[10px]">
                          {new Date(asm.created_at).toLocaleDateString('vi-VN')}
                        </span>
                      </div>
                    </div>
                  );
                })
              )}
            </div>
          </div>

          {/* Right: Detailed Assessment Inspector & Grading Form */}
          <div className="lg:col-span-7">
            {selectedAssessment ? (
              <div className="p-5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-5">
                <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
                  <div>
                    <h3 className="font-bold text-base text-slate-900 dark:text-white flex items-center gap-2">
                      <span>Đánh Giá Năng Lực Tư Duy Của:</span>
                      <span className="text-blue-600 dark:text-blue-400">
                        {selectedAssessment.student_name}
                      </span>
                    </h3>
                    <p className="text-xs text-slate-400">
                      Môn học: {selectedAssessment.course_name} • Mô hình: {selectedAssessment.model_name}
                    </p>
                  </div>

                  <span
                    className={`px-3 py-1 rounded-full text-xs font-bold ${
                      selectedAssessment.review_status === 'reviewed'
                        ? 'bg-emerald-100 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-300'
                        : 'bg-amber-100 text-amber-800 dark:bg-amber-950 dark:text-amber-300'
                    }`}
                  >
                    {selectedAssessment.review_status === 'reviewed'
                      ? 'Đã Phê Duyệt'
                      : 'Đang Chờ Giảng Viên'}
                  </span>
                </div>

                {/* Prompt Excerpt */}
                <div className="p-3.5 rounded-xl bg-slate-50 dark:bg-slate-900 border border-slate-200 dark:border-slate-800">
                  <div className="text-[11px] font-bold text-slate-400 uppercase tracking-wider mb-1">
                    Câu hỏi / Thắc mắc sinh viên đặt ra cho Mentor:
                  </div>
                  <p className="text-sm font-medium text-slate-900 dark:text-slate-100 italic">
                    "{selectedAssessment.prompt_excerpt}"
                  </p>
                </div>

                {/* 7 Critical Thinking Dimensions Breakdown */}
                <div className="space-y-2.5">
                  <h4 className="text-xs font-bold text-slate-800 dark:text-slate-200 uppercase tracking-wider">
                    Điểm Chi Tiết 7 Chiều Tư Duy Phản Biện (Do AI Đo Lường)
                  </h4>
                  <div className="grid grid-cols-2 sm:grid-cols-3 gap-2.5">
                    {Object.entries(selectedAssessment.dimensions).map(([k, val]) => (
                      <div
                        key={k}
                        className="p-2.5 rounded-lg bg-slate-50 dark:bg-slate-900/60 border border-slate-200 dark:border-slate-800 text-center"
                      >
                        <p className="text-[11px] text-slate-500 capitalize truncate">
                          {k.replace('_', ' ')}
                        </p>
                        <p className="text-base font-bold font-mono text-blue-600 dark:text-blue-400 mt-0.5">
                          {(val * 10).toFixed(1)}/10
                        </p>
                      </div>
                    ))}
                  </div>
                </div>

                {/* AI Reasoning */}
                <div className="p-3 rounded-xl bg-blue-50/50 dark:bg-blue-950/30 border border-blue-100 dark:border-blue-900/50 text-xs text-slate-700 dark:text-slate-300">
                  <strong className="font-semibold text-blue-900 dark:text-blue-300">
                    Phân tích từ mô hình:
                  </strong>{' '}
                  {selectedAssessment.reasoning}
                </div>

                {/* TEACHER REVIEW & SCORING FORM */}
                <form
                  onSubmit={handleSubmitGrade}
                  className="p-4 rounded-xl bg-gradient-to-br from-slate-50 to-blue-50/30 dark:from-slate-900 dark:to-blue-950/20 border border-blue-200 dark:border-blue-900/60 space-y-4"
                >
                  <h4 className="text-xs font-bold text-blue-950 dark:text-blue-300 uppercase tracking-wider flex items-center gap-1.5">
                    <ShieldCheck className="w-4 h-4 text-blue-600" />
                    Đánh Giá & Phản Hồi Sư Phạm Của Giảng Viên
                  </h4>

                  {/* Score adjustment */}
                  <div>
                    <div className="flex justify-between items-center text-xs mb-1.5">
                      <label className="font-semibold text-slate-800 dark:text-slate-200">
                        Điểm Giảng Viên Chấm (Thang 10):
                      </label>
                      <span className="font-mono font-bold text-blue-600 dark:text-blue-400 text-sm">
                        {(gradingScore * 10).toFixed(1)} / 10 (Hệ số quy đổi: {gradingScore.toFixed(2)})
                      </span>
                    </div>
                    <input
                      type="range"
                      min="0.1"
                      max="1.0"
                      step="0.05"
                      value={gradingScore}
                      onChange={(e) => setGradingScore(parseFloat(e.target.value))}
                      className="w-full accent-blue-600 cursor-pointer"
                    />
                  </div>

                  {/* Teacher Feedback textarea */}
                  <div>
                    <label className="block text-xs font-semibold text-slate-800 dark:text-slate-200 mb-1">
                      Nhận xét sư phạm gửi trực tiếp cho sinh viên:
                    </label>
                    <textarea
                      rows={3}
                      value={gradingFeedback}
                      onChange={(e) => setGradingFeedback(e.target.value)}
                      placeholder="Ví dụ: Lập luận rất chặt chẽ, em đã chỉ ra được mối quan hệ giữa ACK và cơ chế tự điều tốc (Self-Clocking). Cần phát huy thêm ở câu hỏi tiếp theo..."
                      className="w-full px-3 py-2 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:ring-2 focus:ring-blue-500 focus:outline-none"
                    />
                  </div>

                  <div className="flex items-center justify-end gap-3 pt-2">
                    <button
                      type="submit"
                      disabled={isSubmittingGrade}
                      className="px-5 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white text-xs font-bold shadow-md shadow-blue-900/20 flex items-center gap-2 disabled:opacity-50 transition"
                    >
                      {isSubmittingGrade ? (
                        <>
                          <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                          <span>Đang Lưu Phê Duyệt...</span>
                        </>
                      ) : (
                        <>
                          <CheckCircle2 className="w-4 h-4 text-emerald-400" />
                          <span>Lưu Chấm Điểm & Gửi Nhận Xét</span>
                        </>
                      )}
                    </button>
                  </div>
                </form>
              </div>
            ) : (
              <div className="p-12 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 text-center text-slate-400 text-xs">
                Vui lòng chọn một đánh giá từ hàng đợi bên trái để bắt đầu chấm điểm.
              </div>
            )}
          </div>
        </div>
      )}

      {/* --- SUB-TAB 3: QUESTION STUDIO & AI GENERATOR --- */}
      {activeSubTab === 'question_studio' && (
        <div className="space-y-6">
          {/* Top Control Bar: Search, Filter, Generate AI, Add Question */}
          <div className="p-4 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm flex flex-col md:flex-row md:items-center justify-between gap-4">
            <div className="flex flex-1 items-center gap-3">
              <div className="relative flex-1 max-w-md">
                <Search className="w-4 h-4 text-slate-400 absolute left-3 top-2.5" />
                <input
                  type="text"
                  value={questionSearch}
                  onChange={(e) => setQuestionSearch(e.target.value)}
                  placeholder="Tìm kiếm câu hỏi trong ngân hàng môn học..."
                  className="w-full pl-9 pr-3 py-1.5 text-xs rounded-xl border border-slate-200 dark:border-slate-700 bg-slate-50 dark:bg-slate-900 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>

              <select
                aria-label="Lọc trạng thái câu hỏi"
                value={questionStatusFilter}
                onChange={(e) => setQuestionStatusFilter(e.target.value as any)}
                className="px-3 py-1.5 text-xs rounded-xl border border-slate-200 dark:border-slate-700 bg-slate-50 dark:bg-slate-900 text-slate-700 dark:text-slate-300 focus:outline-none"
              >
                <option value="all">Tất cả trạng thái</option>
                <option value="review">Chờ duyệt (AI đề xuất)</option>
                <option value="approved">Đã duyệt chính thức</option>
              </select>
            </div>

            <div className="flex flex-wrap items-center gap-2">
              <button
                onClick={() => setShowBatchImportModal(true)}
                className="px-3.5 py-1.5 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-xs transition"
              >
                <UploadCloud className="w-3.5 h-3.5" />
                <span>Nạp Question Bank</span>
              </button>

              <button
                onClick={() => setIsCreatingQuestion(true)}
                className="px-3.5 py-1.5 rounded-xl border border-slate-300 dark:border-slate-700 hover:bg-slate-100 dark:hover:bg-slate-800 text-slate-800 dark:text-slate-200 text-xs font-semibold flex items-center gap-1.5 transition"
              >
                <Plus className="w-3.5 h-3.5" />
                <span>Soạn 1 Câu Hỏi</span>
              </button>

              <button
                onClick={() => setShowExamModal(true)}
                className="px-3.5 py-1.5 rounded-xl bg-slate-800 hover:bg-slate-700 text-white text-xs font-semibold flex items-center gap-1.5 shadow-sm transition"
              >
                <FileDown className="w-3.5 h-3.5 text-blue-400" />
                <span>Tạo Đề Thi</span>
              </button>
            </div>
          </div>

          {/* AI Generator Box for Teacher */}
          <div className="p-4 rounded-2xl bg-gradient-to-r from-blue-900/10 via-indigo-900/10 to-slate-900/10 border border-blue-200 dark:border-blue-900/60 shadow-sm space-y-3">
            <div className="flex items-center justify-between">
              <span className="text-xs font-bold text-blue-900 dark:text-blue-300 flex items-center gap-1.5">
                <BrainCircuit className="w-4 h-4 text-blue-600" />
                Bộ Tạo Câu Hỏi Tự Động Bằng Gemini AI & Khung Socratic
              </span>
              <span className="text-[10px] text-slate-500 font-mono">Model: gemini-3.8-flash</span>
            </div>

            <div className="flex flex-col sm:flex-row items-center gap-3">
              <input
                type="text"
                value={aiTopicInput}
                onChange={(e) => setAiTopicInput(e.target.value)}
                placeholder="Nhập chủ đề giáo trình (ví dụ: TCP Congestion Control, QUIC, BBR...)"
                className="flex-1 w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
              />

              <select
                aria-label="Chọn độ khó câu hỏi AI"
                value={aiDifficultyInput}
                onChange={(e) => setAiDifficultyInput(e.target.value as any)}
                className="px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-200 focus:outline-none"
              >
                <option value="beginner">Cơ bản (Beginner)</option>
                <option value="intermediate">Trung bình (Intermediate)</option>
                <option value="advanced">Nâng cao (Advanced)</option>
              </select>

              <button
                onClick={() => onGenerateAIQuestion(aiTopicInput, aiDifficultyInput)}
                disabled={isGeneratingQuestion}
                className="w-full sm:w-auto px-4 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white text-xs font-bold flex items-center justify-center gap-1.5 shadow-sm disabled:opacity-50 transition"
              >
                {isGeneratingQuestion ? (
                  <>
                    <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                    <span>Đang Tạo...</span>
                  </>
                ) : (
                  <>
                    <Sparkles className="w-3.5 h-3.5" />
                    <span>Sinh 1 Câu AI</span>
                  </>
                )}
              </button>

              <button
                onClick={() => setShowBatchImportModal(true)}
                className="w-full sm:w-auto px-4 py-2 rounded-xl border border-indigo-300 dark:border-indigo-700 hover:bg-indigo-50 dark:hover:bg-indigo-950/40 text-indigo-700 dark:text-indigo-300 text-xs font-bold flex items-center justify-center gap-1.5 transition"
              >
                <Sparkles className="w-3.5 h-3.5 text-indigo-500" />
                <span>Sinh Hàng Loạt (Batch)</span>
              </button>
            </div>
          </div>

          {/* Form Create Question Modal/Inline */}
          {isCreatingQuestion && (
            <form
              onSubmit={handleCreateQuestion}
              className="p-5 rounded-2xl bg-white dark:bg-[#070e1b] border-2 border-blue-500 shadow-md space-y-3"
            >
              <div className="flex items-center justify-between pb-2 border-b border-slate-100 dark:border-slate-800">
                <h4 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-1.5">
                  <Plus className="w-4 h-4 text-blue-600" />
                  Soạn Câu Hỏi Mới Cho Môn Học: {selectedCourse.name}
                </h4>
                <button
                  type="button"
                  onClick={() => setIsCreatingQuestion(false)}
                  className="text-slate-400 hover:text-slate-600 text-xs font-bold"
                >
                  Đóng
                </button>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Nội dung câu hỏi:
                </label>
                <textarea
                  rows={2}
                  value={newQText}
                  onChange={(e) => setNewQText(e.target.value)}
                  placeholder="Ví dụ: Trong cơ chế AIMD của TCP Reno, tại sao tốc độ tăng là tuyến tính nhưng giảm lại là cấp số nhân?"
                  className="w-full px-3 py-2 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white"
                  required
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Gợi ý Socratic (Dẫn dắt sinh viên suy nghĩ, không lộ đáp án):
                </label>
                <input
                  type="text"
                  value={newQHint}
                  onChange={(e) => setNewQHint(e.target.value)}
                  placeholder="Ví dụ: Nếu cả hai luồng cùng tăng cấp số nhân khi mạng sắp đầy, điều gì sẽ xảy ra với độ ổn định?"
                  className="w-full px-3 py-2 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white"
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Đáp án phân tích chuẩn mực (Dành cho Giảng viên & Đề thi):
                </label>
                <textarea
                  rows={2}
                  value={newQAnswer}
                  onChange={(e) => setNewQAnswer(e.target.value)}
                  placeholder="Phân tích khoa học và công thức toán học/logic liên quan..."
                  className="w-full px-3 py-2 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white"
                  required
                />
              </div>

              <div className="flex items-center justify-between pt-2">
                <div className="flex items-center gap-2">
                  <span className="text-xs text-slate-500 font-medium">Độ khó:</span>
                  <select
                    aria-label="Chọn độ khó câu hỏi mới"
                    value={newQDifficulty}
                    onChange={(e) => setNewQDifficulty(e.target.value as any)}
                    className="px-2.5 py-1 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-800 dark:text-slate-200"
                  >
                    <option value="beginner">Cơ bản</option>
                    <option value="intermediate">Trung bình</option>
                    <option value="advanced">Nâng cao</option>
                  </select>
                </div>

                <div className="flex gap-2">
                  <button
                    type="button"
                    onClick={() => setIsCreatingQuestion(false)}
                    className="px-3 py-1.5 rounded-lg border border-slate-300 text-slate-600 text-xs font-medium"
                  >
                    Hủy
                  </button>
                  <button
                    type="submit"
                    className="px-4 py-1.5 rounded-lg bg-blue-900 text-white text-xs font-bold"
                  >
                    Lưu Vào Ngân Hàng
                  </button>
                </div>
              </div>
            </form>
          )}

          {/* Question List */}
          <div className="space-y-3">
            <div className="flex justify-between items-center text-xs text-slate-500">
              <span>Hiển thị {filteredQuestions.length} câu hỏi phù hợp</span>
              <span>Đã duyệt: {approvedQuestions.length} • Chờ duyệt: {pendingQuestions.length}</span>
            </div>

            <div className="grid grid-cols-1 gap-3">
              {filteredQuestions.map((q) => {
                const isPendingReview = q.status === 'review';
                return (
                  <div
                    key={q.id}
                    className={`p-4 rounded-xl border transition ${
                      isPendingReview
                        ? 'border-indigo-300 bg-indigo-50/40 dark:border-indigo-900/60 dark:bg-indigo-950/20'
                        : 'border-slate-200 dark:border-slate-800 bg-white dark:bg-[#070e1b]'
                    }`}
                  >
                    <div className="flex flex-col sm:flex-row sm:items-start justify-between gap-3">
                      <div className="space-y-1.5 flex-1">
                        <div className="flex items-center gap-2">
                          <span
                            className={`px-2 py-0.5 rounded-full text-[10px] font-bold uppercase tracking-wider ${
                              q.difficulty === 'beginner'
                                ? 'bg-emerald-100 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-300'
                                : q.difficulty === 'intermediate'
                                ? 'bg-blue-100 text-blue-800 dark:bg-blue-950 dark:text-blue-300'
                                : 'bg-rose-100 text-rose-800 dark:bg-rose-950 dark:text-rose-300'
                            }`}
                          >
                            {q.difficulty}
                          </span>
                          <span className="text-[11px] text-slate-400">
                            Nguồn: {q.source === 'ai' ? '🤖 AI Generator' : '👨‍🏫 Giảng viên'}
                          </span>
                          <span
                            className={`px-2 py-0.5 rounded-full text-[10px] font-bold ${
                              isPendingReview
                                ? 'bg-amber-100 text-amber-800 dark:bg-amber-950 dark:text-amber-300'
                                : 'bg-emerald-100 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-300'
                            }`}
                          >
                            {isPendingReview ? 'Chờ GV Duyệt' : 'Đã Phê Duyệt'}
                          </span>
                        </div>

                        <h4 className="text-sm font-bold text-slate-900 dark:text-white">
                          {q.question}
                        </h4>

                        <div className="p-2.5 rounded-lg bg-slate-50 dark:bg-slate-900/80 border border-slate-200/80 dark:border-slate-800/80 text-xs space-y-1">
                          <p className="text-slate-600 dark:text-slate-300">
                            <strong className="text-slate-800 dark:text-slate-200">Gợi ý Socratic:</strong>{' '}
                            {q.hint}
                          </p>
                          <p className="text-slate-600 dark:text-slate-300 border-t border-slate-200 dark:border-slate-800 pt-1">
                            <strong className="text-slate-800 dark:text-slate-200">Đáp án chuẩn:</strong>{' '}
                            {q.answer}
                          </p>
                        </div>
                      </div>

                      {/* Action buttons */}
                      <div className="flex sm:flex-col items-center sm:items-end gap-2 shrink-0">
                        {isPendingReview && (
                          <button
                            onClick={() => onApproveQuestion(q.id)}
                            className="px-3 py-1.5 rounded-lg bg-emerald-600 hover:bg-emerald-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-sm transition"
                          >
                            <CheckCircle2 className="w-3.5 h-3.5" />
                            <span>Phê Duyệt Câu Này</span>
                          </button>
                        )}
                        <button
                          onClick={() =>
                            onNavigateToChat(
                              `Tôi muốn giải bài toán sau: "${q.question}". Hãy cho tôi gợi ý tư duy Socratic!`
                            )
                          }
                          className="px-2.5 py-1 text-xs text-blue-600 dark:text-blue-400 hover:underline flex items-center gap-1"
                        >
                          <Sparkles className="w-3 h-3" />
                          <span>Thử nghiệm với AI</span>
                        </button>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        </div>
      )}

      {/* --- SUB-TAB 4: CURRICULUM & RAG INGESTION --- */}
      {activeSubTab === 'curriculum_rag' && (
        <div className="space-y-6">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 p-4 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm">
            <div>
              <h3 className="font-bold text-base text-slate-900 dark:text-white flex items-center gap-2">
                <Layers className="w-5 h-5 text-blue-600 dark:text-blue-400" />
                Kho Tài Liệu Giáo Trình & Phân Đoạn RAG Chunks
              </h3>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                Các đoạn văn bản này được AI Mentor trích xuất tự động làm căn cứ trích dẫn học thuật cho sinh viên.
              </p>
            </div>

            <button
              onClick={() => setShowUploadModal(true)}
              className="px-4 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-sm transition"
            >
              <Plus className="w-4 h-4" />
              <span>Thêm Tài Liệu Giáo Trình</span>
            </button>
          </div>

          {/* RAG Query Inspector */}
          <div className="p-4 rounded-2xl bg-slate-50 dark:bg-slate-900/60 border border-slate-200 dark:border-slate-800 space-y-3">
            <span className="text-xs font-bold text-slate-900 dark:text-white flex items-center gap-1.5">
              <Search className="w-4 h-4 text-blue-500" />
              RAG Inspector: Kiểm Tra Thử Khả Năng Trích Xuất Tri Thức Của AI
            </span>
            <div className="flex items-center gap-2">
              <input
                type="text"
                value={ragQuery}
                onChange={(e) => setRagQuery(e.target.value)}
                placeholder="Nhập câu hỏi kiểm thử để xem các đoạn tài liệu AI sẽ tìm được..."
                className="flex-1 px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none"
              />
              <button
                onClick={handleTestRag}
                disabled={isTestingRag}
                className="px-4 py-2 rounded-xl bg-slate-800 hover:bg-slate-700 text-white text-xs font-semibold flex items-center gap-1.5 disabled:opacity-50"
              >
                {isTestingRag ? <RefreshCw className="w-3.5 h-3.5 animate-spin" /> : <Eye className="w-3.5 h-3.5" />}
                <span>Truy Xuất Thử</span>
              </button>
            </div>

            {ragResults.length > 0 && (
              <div className="space-y-2 pt-2 border-t border-slate-200 dark:border-slate-800">
                <span className="text-[11px] font-bold text-slate-400">
                  Kết quả trích xuất ({ragResults.length} đoạn chunk phù hợp nhất):
                </span>
                {ragResults.map((res, i) => (
                  <div
                    key={i}
                    className="p-3 rounded-lg bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 text-xs space-y-1"
                  >
                    <div className="flex justify-between text-[10px] text-blue-600 dark:text-blue-400 font-semibold">
                      <span>Nguồn: {res.source}</span>
                      <span>Trùng khớp: {Math.round(res.score * 100)}%</span>
                    </div>
                    <p className="text-slate-700 dark:text-slate-300">{res.chunk}</p>
                  </div>
                ))}
              </div>
            )}
          </div>

          {/* List of Documents in Course */}
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {courseDocuments.map((doc) => (
              <div
                key={doc.id}
                className="p-4 rounded-xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-3"
              >
                <div className="flex items-start justify-between gap-2">
                  <div>
                    <h4 className="text-sm font-bold text-slate-900 dark:text-white flex items-center gap-1.5">
                      <FileText className="w-4 h-4 text-blue-600" />
                      {doc.title}
                    </h4>
                    <p className="text-[11px] text-slate-400 font-mono mt-0.5">
                      {doc.source_path} • {doc.chunks.length} phân đoạn (chunks)
                    </p>
                  </div>
                  <div className="flex items-center gap-1.5">
                    <span className="px-2 py-0.5 rounded-full text-[10px] font-bold bg-blue-50 text-blue-700 dark:bg-blue-950 dark:text-blue-300">
                      Sẵn sàng nạp RAG
                    </span>
                    {onDeleteDocument && (
                      <button
                        onClick={() => handleDeleteDoc(doc.id, doc.title)}
                        className="p-1.5 rounded-lg text-slate-400 hover:text-red-500 hover:bg-red-50 dark:hover:bg-red-950/30 transition"
                        title="Xóa tài liệu này khỏi kho RAG"
                      >
                        <Trash2 className="w-3.5 h-3.5" />
                      </button>
                    )}
                  </div>
                </div>

                {/* Chunks preview */}
                <div className="space-y-1.5 max-h-48 overflow-y-auto pr-1">
                  {doc.chunks.map((chk, idx) => (
                    <div
                      key={chk.id || idx}
                      className="p-2.5 rounded-lg bg-slate-50 dark:bg-slate-900 border border-slate-100 dark:border-slate-800 text-xs text-slate-700 dark:text-slate-300"
                    >
                      <div className="flex items-center justify-between text-[10px] text-slate-400 mb-1">
                        <span>Phân đoạn #{idx + 1}</span>
                        <span className="font-mono">Tags: {chk.tags?.join(', ') || 'none'}</span>
                      </div>
                      <p className="line-clamp-3">{chk.content}</p>
                    </div>
                  ))}
                </div>
              </div>
            ))}
          </div>

          {courseDocuments.length === 0 && (
            <div className="p-10 rounded-2xl bg-white dark:bg-[#070e1b] border border-dashed border-slate-300 dark:border-slate-800 text-center space-y-3">
              <div className="w-12 h-12 mx-auto rounded-2xl bg-blue-50 dark:bg-blue-950/50 flex items-center justify-center text-blue-600 dark:text-blue-400">
                <UploadCloud className="w-6 h-6" />
              </div>
              <h4 className="text-sm font-bold text-slate-900 dark:text-white">
                Chưa có tài liệu giáo trình nào cho môn học này
              </h4>
              <p className="text-xs text-slate-500 max-w-md mx-auto">
                Hãy nạp giáo trình, bài giảng hoặc ghi chú học thuật để AI Socratic Mentor có thể trích dẫn chính xác và phản biện bài tập cùng sinh viên.
              </p>
              <button
                onClick={() => setShowUploadModal(true)}
                className="px-4 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white text-xs font-bold inline-flex items-center gap-1.5 shadow-sm transition"
              >
                <Plus className="w-4 h-4" />
                <span>Nạp Tài Liệu Đầu Tiên</span>
              </button>
            </div>
          )}
        </div>
      )}

      {/* --- SUB-TAB 5: STUDENT ROSTER & MONITORING --- */}
      {activeSubTab === 'student_roster' && (
        <div className="space-y-4">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 p-4 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm">
            <div>
              <h3 className="font-bold text-base text-slate-900 dark:text-white flex items-center gap-2">
                <Users className="w-5 h-5 text-blue-600 dark:text-blue-400" />
                Sổ Theo Dõi Sinh Viên & Cảnh Báo Sớm
              </h3>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                Theo dõi mức độ tiến bộ, điểm tư duy phản biện và phát hiện sớm các sinh viên cần bồi dưỡng thêm.
              </p>
            </div>

            <div className="flex items-center gap-1.5 p-1 rounded-xl bg-slate-100 dark:bg-slate-800 text-xs">
              {(['all', 'needs_help', 'excellent'] as const).map((filter) => (
                <button
                  key={filter}
                  onClick={() => setRosterFilter(filter)}
                  className={`px-3 py-1 rounded-lg font-medium transition ${
                    rosterFilter === filter
                      ? 'bg-white dark:bg-slate-700 text-blue-600 dark:text-blue-400 font-bold shadow-xs'
                      : 'text-slate-600 dark:text-slate-300'
                  }`}
                >
                  {filter === 'all'
                    ? 'Tất cả sinh viên'
                    : filter === 'needs_help'
                    ? 'Cần hỗ trợ'
                    : 'Xuất sắc'}
                </button>
              ))}
            </div>
          </div>

          {/* Student Roster Table/Cards */}
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filteredRoster.map((std) => (
              <div
                key={std.id}
                className={`p-4 rounded-xl border transition space-y-3 ${
                  std.status_alert === 'needs_help'
                    ? 'border-amber-300 bg-amber-50/30 dark:border-amber-900/60 dark:bg-amber-950/20'
                    : std.status_alert === 'excellent'
                    ? 'border-blue-300 bg-blue-50/30 dark:border-blue-900/60 dark:bg-blue-950/20'
                    : 'border-slate-200 dark:border-slate-800 bg-white dark:bg-[#070e1b]'
                }`}
              >
                <div className="flex items-center gap-3">
                  <img
                    src={std.avatar || 'https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=150'}
                    alt={std.name}
                    className="w-10 h-10 rounded-full object-cover ring-2 ring-blue-500/20"
                  />
                  <div className="flex-1 min-w-0">
                    <h4 className="font-bold text-sm text-slate-900 dark:text-white truncate">
                      {std.name}
                    </h4>
                    <p className="text-xs text-slate-400 truncate">{std.email}</p>
                  </div>
                  <span
                    className={`px-2 py-0.5 rounded-full text-[10px] font-bold shrink-0 ${
                      std.status_alert === 'excellent'
                        ? 'bg-blue-100 text-blue-800 dark:bg-blue-950 dark:text-blue-300'
                        : std.status_alert === 'needs_help'
                        ? 'bg-amber-100 text-amber-800 dark:bg-amber-950 dark:text-amber-300'
                        : 'bg-emerald-100 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-300'
                    }`}
                  >
                    {std.status_alert === 'excellent'
                      ? 'Xuất sắc'
                      : std.status_alert === 'needs_help'
                      ? 'Cần hỗ trợ'
                      : 'Đạt chuẩn'}
                  </span>
                </div>

                <div className="grid grid-cols-2 gap-2 pt-2 border-t border-slate-100 dark:border-slate-800 text-xs">
                  <div>
                    <span className="text-slate-400 text-[11px]">Tiến độ lộ trình:</span>
                    <p className="font-bold font-mono text-slate-900 dark:text-white">
                      {std.progress_percent}%
                    </p>
                  </div>
                  <div>
                    <span className="text-slate-400 text-[11px]">Điểm tư duy TB:</span>
                    <p className="font-bold font-mono text-blue-600 dark:text-blue-400">
                      {(std.average_thinking_score * 10).toFixed(1)} / 10
                    </p>
                  </div>
                </div>

                {std.attention_reason && (
                  <p className="text-[11px] text-slate-500 italic bg-slate-50 dark:bg-slate-900/60 p-2 rounded-lg">
                    {std.attention_reason}
                  </p>
                )}

                <div className="flex justify-between items-center text-[11px] text-slate-400 pt-1">
                  <span>{std.questions_count} câu hỏi đã đặt</span>
                  <button
                    onClick={() => setActiveSubTab('grading')}
                    className="text-blue-600 dark:text-blue-400 hover:underline font-semibold"
                  >
                    Xem lịch sử chấm
                  </button>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* --- SUB-TAB 6: AI POLICY & SOCRATIC TUNING --- */}
      {activeSubTab === 'ai_policy' && (
        <div className="space-y-6">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            {/* Socratic Response Policy */}
            <div className="p-5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
              <div className="flex items-center justify-between">
                <span className="text-sm font-bold text-slate-900 dark:text-white flex items-center gap-2">
                  <Sliders className="w-4 h-4 text-blue-600" />
                  Chính Sách Phản Hồi Của Trợ Lý Mentor
                </span>
                <span className="text-xs px-2 py-0.5 rounded-full bg-blue-100 text-blue-800 dark:bg-blue-950 dark:text-blue-300 font-bold">
                  {selectedCourse.answer_policy}
                </span>
              </div>

              <p className="text-xs text-slate-500 dark:text-slate-400">
                Quy định mức độ nghiêm ngặt khi sinh viên hỏi bài. Được áp dụng tức thời cho toàn bộ người học trong môn{' '}
                <strong>{selectedCourse.name}</strong>.
              </p>

              <div className="space-y-3">
                {[
                  {
                    policy: 'HINT_ONLY' as AnswerPolicy,
                    title: '1. Gợi ý Socratic Nghiêm ngặt (HINT_ONLY)',
                    desc: 'Mô hình tuyệt đối KHÔNG đưa ra lời giải trực tiếp. Chia nhỏ câu hỏi, đặt câu hỏi phản biện dẫn dắt sinh viên tự khám phá.',
                    badge: 'Khuyến nghị cho học kỳ chính',
                  },
                  {
                    policy: 'GUIDED' as AnswerPolicy,
                    title: '2. Dẫn dắt 2 bước (GUIDED)',
                    desc: 'Mô hình tóm tắt bản chất hiện tượng và hướng dẫn bước tiếp theo, khuyến khích sinh viên tự giải phần còn lại.',
                    badge: 'Cân bằng',
                  },
                  {
                    policy: 'FULL_ANSWER' as AnswerPolicy,
                    title: '3. Phân tích trọn vẹn (FULL_ANSWER)',
                    desc: 'Mô hình giải thích toàn diện lý thuyết, cung cấp công thức và bài tập tương tự để sinh viên tự luyện.',
                    badge: 'Ôn tập cuối kỳ',
                  },
                ].map((opt) => {
                  const isSelected = selectedCourse.answer_policy === opt.policy;
                  return (
                    <div
                      key={opt.policy}
                      onClick={() => onUpdateCoursePolicy(opt.policy)}
                      className={`p-3.5 rounded-xl border text-left cursor-pointer transition ${
                        isSelected
                          ? 'border-blue-600 bg-blue-50/80 dark:bg-blue-950/40 ring-1 ring-blue-600'
                          : 'border-slate-200 dark:border-slate-800 hover:border-slate-300 dark:hover:border-slate-700'
                      }`}
                    >
                      <div className="flex items-center justify-between">
                        <span className="font-bold text-xs text-slate-900 dark:text-white">
                          {opt.title}
                        </span>
                        <span className="text-[10px] font-semibold text-slate-400">{opt.badge}</span>
                      </div>
                      <p className="text-xs text-slate-600 dark:text-slate-300 mt-1">{opt.desc}</p>
                    </div>
                  );
                })}
              </div>
            </div>

            {/* AI Provider & Infrastructure Switcher */}
            <div className="p-5 rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
              <span className="text-sm font-bold text-slate-900 dark:text-white flex items-center gap-2">
                <Cpu className="w-4 h-4 text-blue-600" />
                Hạ Tầng Mô Hình AI & Token Tiêu Thụ
              </span>

              <p className="text-xs text-slate-500 dark:text-slate-400">
                Cho phép chuyển đổi linh hoạt giữa đám mây Google Gemini 3.8 Flash và mô hình Local Ollama trên máy trạm trường học.
              </p>

              <div className="p-3 rounded-xl bg-slate-50 dark:bg-slate-900 border border-slate-200 dark:border-slate-800 space-y-2">
                <div className="flex justify-between items-center text-xs">
                  <span className="text-slate-500">Mô hình hiện hành:</span>
                  <span className="font-bold text-slate-900 dark:text-white">
                    {providerState?.current_provider === 'gemini'
                      ? 'Google Gemini 3.8 Flash'
                      : 'Local Ollama (Gemma4:edu-mentor)'}
                  </span>
                </div>
                <div className="flex justify-between items-center text-xs">
                  <span className="text-slate-500">Tổng tokens tiêu thụ:</span>
                  <span className="font-mono font-bold text-blue-600 dark:text-blue-400">
                    {providerState?.total_tokens_used?.toLocaleString() || 12450} tokens
                  </span>
                </div>
              </div>

              <div className="space-y-2">
                <button
                  onClick={() => onSwitchProvider('gemini', true)}
                  className={`w-full p-3 rounded-xl border text-left text-xs transition flex items-center justify-between ${
                    providerState?.current_provider === 'gemini'
                      ? 'border-blue-600 bg-blue-50/60 dark:bg-blue-950/40 text-blue-900 dark:text-blue-300 font-bold'
                      : 'border-slate-200 dark:border-slate-800 text-slate-700 dark:text-slate-300 hover:bg-slate-50'
                  }`}
                >
                  <div>
                    <p className="font-bold">Google Gemini 3.8 Flash (Cloud)</p>
                    <p className="text-[11px] text-slate-400">
                      Tốc độ cực nhanh, khả năng suy luận 7 chiều tư duy phản biện sắc bén nhất.
                    </p>
                  </div>
                  {providerState?.current_provider === 'gemini' && (
                    <span className="w-2 h-2 rounded-full bg-blue-600 animate-pulse" />
                  )}
                </button>

                <button
                  onClick={() => onSwitchProvider('ollama', true)}
                  className={`w-full p-3 rounded-xl border text-left text-xs transition flex items-center justify-between ${
                    providerState?.current_provider === 'ollama'
                      ? 'border-blue-600 bg-blue-50/60 dark:bg-blue-950/40 text-blue-900 dark:text-blue-300 font-bold'
                      : 'border-slate-200 dark:border-slate-800 text-slate-700 dark:text-slate-300 hover:bg-slate-50'
                  }`}
                >
                  <div>
                    <p className="font-bold">Local Ollama / Gemma4 (On-Premises)</p>
                    <p className="text-[11px] text-slate-400">
                      Hoạt động độc lập không cần internet, bảo mật dữ liệu nội bộ trường học.
                    </p>
                  </div>
                  {providerState?.current_provider === 'ollama' && (
                    <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse" />
                  )}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* --- MODAL: EXAM GENERATOR & EXPORT --- */}
      {showExamModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/70 backdrop-blur-xs animate-fadeIn">
          <div className="w-full max-w-3xl max-h-[90vh] overflow-y-auto rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-2xl p-6 space-y-4">
            <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
              <div className="flex items-center gap-2">
                <FileDown className="w-5 h-5 text-blue-600" />
                <h3 className="font-bold text-base text-slate-900 dark:text-white">
                  Bộ Tạo & Xuất Đề Kiểm Tra Tư Duy Socratic
                </h3>
              </div>
              <button
                onClick={() => setShowExamModal(false)}
                className="text-slate-400 hover:text-slate-600 font-bold text-sm"
              >
                Đóng
              </button>
            </div>

            {!generatedExamData ? (
              <div className="space-y-4">
                <div>
                  <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                    Tiêu đề đề thi:
                  </label>
                  <input
                    type="text"
                    value={examTitle}
                    onChange={(e) => setExamTitle(e.target.value)}
                    className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white"
                  />
                </div>

                <div>
                  <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                    Thời gian làm bài (Phút):
                  </label>
                  <input
                    type="number"
                    value={examDuration}
                    onChange={(e) => setExamDuration(parseInt(e.target.value) || 45)}
                    className="w-32 px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white"
                  />
                </div>

                <div className="p-3.5 rounded-xl bg-blue-50/60 dark:bg-blue-950/30 text-xs text-blue-900 dark:text-blue-200 space-y-1">
                  <p className="font-bold">Cơ chế đóng gói tự động:</p>
                  <p>
                    Hệ thống sẽ tổng hợp các câu hỏi đã được Giảng viên phê duyệt trong ngân hàng môn{' '}
                    <strong>{selectedCourse.name}</strong>, tự động gắn barem chấm điểm phân loại theo 3 mức:
                    Hiểu định nghĩa (40%), Lập luận nhân quả (35%), Phản biện sáng tạo (25%).
                  </p>
                </div>

                <div className="flex justify-end gap-2 pt-2">
                  <button
                    onClick={() => setShowExamModal(false)}
                    className="px-4 py-2 rounded-xl border border-slate-200 text-xs font-semibold text-slate-600"
                  >
                    Hủy
                  </button>
                  <button
                    onClick={handleGenerateExam}
                    disabled={isGeneratingExam}
                    className="px-5 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 text-white text-xs font-bold flex items-center gap-1.5 shadow-md disabled:opacity-50"
                  >
                    {isGeneratingExam ? (
                      <>
                        <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                        <span>Đang Khởi Tạo...</span>
                      </>
                    ) : (
                      <>
                        <Sparkles className="w-3.5 h-3.5" />
                        <span>Tạo Đề Thi Ngay</span>
                      </>
                    )}
                  </button>
                </div>
              </div>
            ) : (
              <div className="space-y-4">
                <div className="flex items-center justify-between">
                  <div className="text-xs text-slate-500">
                    Đã tạo thành công: <strong>{generatedExamData.total_questions} câu hỏi</strong> • Thời gian:{' '}
                    <strong>{generatedExamData.duration_minutes} phút</strong>
                  </div>

                  <div className="flex items-center gap-2">
                    <button
                      onClick={handleCopyExamMarkdown}
                      className="px-3.5 py-1.5 rounded-lg bg-blue-600 hover:bg-blue-500 text-white text-xs font-semibold flex items-center gap-1.5 shadow-xs transition"
                    >
                      {copiedExam ? <Check className="w-3.5 h-3.5" /> : <Copy className="w-3.5 h-3.5" />}
                      <span>{copiedExam ? 'Đã Sao Chép Markdown!' : 'Sao Chép Markdown'}</span>
                    </button>
                    <button
                      onClick={() => setGeneratedExamData(null)}
                      className="px-3 py-1.5 rounded-lg border border-slate-200 text-xs text-slate-600"
                    >
                      Tạo Lại
                    </button>
                  </div>
                </div>

                <div className="p-4 rounded-xl bg-slate-50 dark:bg-slate-900 border border-slate-200 dark:border-slate-800 space-y-4 max-h-96 overflow-y-auto font-mono text-xs">
                  {generatedExamData.questions.map((q: any) => (
                    <div key={q.number} className="space-y-1.5 border-b border-slate-200 dark:border-slate-800 pb-3">
                      <p className="font-bold text-slate-900 dark:text-white">
                        Câu {q.number} [{q.difficulty.toUpperCase()}]: {q.question}
                      </p>
                      <p className="text-slate-500 dark:text-slate-400 italic">
                        💡 Gợi mở Socratic: {q.hint_socratic}
                      </p>
                      <p className="text-emerald-700 dark:text-emerald-400">
                        📘 Đáp án chuẩn: {q.ideal_answer}
                      </p>
                    </div>
                  ))}
                </div>
              </div>
            )}
          </div>
        </div>
      )}

      {/* --- MODAL: CREATE COURSE / CLASS --- */}
      {onCreateCourse && (
        <CreateCourseModal
          isOpen={showCreateCourseModal}
          onClose={() => setShowCreateCourseModal(false)}
          onSubmit={onCreateCourse}
        />
      )}

      {/* --- MODAL: UPLOAD CURRICULUM DOCUMENT (RAG INGESTION) --- */}
      <UploadDocumentModal
        isOpen={showUploadModal}
        onClose={() => setShowUploadModal(false)}
        courseName={selectedCourse.name}
        onUpload={onUploadDocument}
        onTriggerBatchAI={
          onBatchGenerateAIQuestions
            ? (topic, count, diff) => onBatchGenerateAIQuestions(topic, count, diff)
            : undefined
        }
      />

      {/* --- MODAL: BATCH IMPORT QUESTION BANK --- */}
      <BatchImportQuestionsModal
        isOpen={showBatchImportModal}
        onClose={() => setShowBatchImportModal(false)}
        courseName={selectedCourse.name}
        courseDocuments={courseDocuments}
        onBatchImport={onBatchImportQuestions || (async () => {})}
        onBatchGenerateAI={onBatchGenerateAIQuestions || (async () => {})}
      />
    </div>
  );
};
