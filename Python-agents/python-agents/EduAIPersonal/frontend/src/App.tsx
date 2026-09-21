import React, { useState, useEffect } from 'react';
import { Header } from './components/Header';
import { DashboardOverview } from './components/DashboardOverview';
import { MentorChat } from './components/MentorChat';
import { RoadmapView } from './components/RoadmapView';
import { QuestionBankView } from './components/QuestionBankView';
import { AssessmentReviewView } from './components/AssessmentReviewView';
import { PersonalizedVaultView } from './components/PersonalizedVaultView';
import { TeacherPortal } from './components/TeacherPortal';
import { CourseDocumentModal } from './components/CourseDocumentModal';
import { LoginModal } from './components/LoginModal';
import {
  User,
  Course,
  ChatMessage,
  ThinkingAssessment,
  Roadmap,
  Question,
  PersonalizedData,
  AIProviderState,
  LearningProgressItem,
  CourseDocument,
  AnswerPolicy,
  QuestionDifficulty,
} from './types';
import { apiFetch, apiResponseValue, readApiJson, roadmapTopicStatusPath } from './api';
import { clearSession, hasStoredSession, saveSession, SESSION_USER_ID_KEY } from './authSession';
import { toMentorReply } from './chatResponse';

export default function App() {
  // Theme state
  const [darkMode, setDarkMode] = useState<boolean>(() => {
    const saved = localStorage.getItem('edu_ai_theme');
    if (saved) return saved === 'dark';
    return window.matchMedia('(prefers-color-scheme: dark)').matches;
  });

  // Current User (Role: student or teacher)
  const [users, setUsers] = useState<User[]>([]);
  const [currentUser, setCurrentUser] = useState<User>({
    id: 'usr_student_1',
    name: 'Nguyễn Văn Minh',
    role: 'student',
    email: 'minh.nv@edu.vn',
    avatar: 'https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=150&auto=format&fit=crop&q=80',
    bio: 'Sinh viên K21 Khoa Mạng Máy tính. Đam mê giao thức mạng lõi và lập trình phân tán.',
  });
  const [isAuthenticated, setIsAuthenticated] = useState<boolean>(() => hasStoredSession(window.localStorage));

  // Navigation tab
  const [activeTab, setActiveTab] = useState<string>('dashboard');

  // Courses & Active Course
  const [courses, setCourses] = useState<Course[]>([]);
  const [selectedCourse, setSelectedCourse] = useState<Course>({
    id: 'crs_network_101',
    subject_id: 'subj_cs_1',
    code: 'CS301',
    name: 'Mạng Máy tính & Kiến trúc TCP/IP',
    description: 'Nghiên cứu cấu trúc tầng giao thức Internet, TCP/UDP, thuật toán kiểm soát tắc nghẽn AIMD, cơ chế định tuyến và an ninh mạng.',
    teacher_id: 'usr_teacher_1',
    teacher_name: 'TS. Trần Hoàng Long',
    enrolled_count: 128,
    answer_policy: 'HINT_ONLY',
    topics_count: 5,
    documents_count: 3,
    questions_count: 14,
    created_at: '2026-09-01',
  });

  // Core Data states
  const [messages, setMessages] = useState<ChatMessage[]>([]);
  const [assessments, setAssessments] = useState<ThinkingAssessment[]>([]);
  const [roadmap, setRoadmap] = useState<Roadmap | null>(null);
  const [progress, setProgress] = useState<LearningProgressItem[]>([]);
  const [questions, setQuestions] = useState<Question[]>([]);
  const [personalization, setPersonalization] = useState<PersonalizedData>({
    notes: [],
    saved_question_ids: [],
    study_goals: [],
    theme: 'light',
  });
  const [providerState, setProviderState] = useState<AIProviderState | null>(null);
  const [courseDocuments, setCourseDocuments] = useState<CourseDocument[]>([]);
  const [isDocModalOpen, setIsDocModalOpen] = useState(false);
  const [isLoginModalOpen, setIsLoginModalOpen] = useState(false);
  const [isSendingMessage, setIsSendingMessage] = useState(false);
  const [isGeneratingQuestion, setIsGeneratingQuestion] = useState(false);
  const [isLoadingInitial, setIsLoadingInitial] = useState(true);

  // Sync dark class on documentElement
  useEffect(() => {
    if (darkMode) {
      document.documentElement.classList.add('dark');
      localStorage.setItem('edu_ai_theme', 'dark');
    } else {
      document.documentElement.classList.remove('dark');
      localStorage.setItem('edu_ai_theme', 'light');
    }
  }, [darkMode]);

  // Safe helper to fetch JSON with fallback and avoid HTML parse errors
  const safeFetchJson = async <T,>(url: string, fallback: T): Promise<T> => {
    try {
      const res = await apiFetch(url);
      if (!res.ok) return fallback;
      return await readApiJson<T>(res);
    } catch {
      return fallback;
    }
  };

  // Load initial application data
  const loadAppData = async () => {
    try {
      // 1. Users
      const usersData = await safeFetchJson<any>('/api/users', []);
      const usersList: User[] = Array.isArray(usersData) ? usersData : (usersData?.users || []);
      if (usersList.length > 0) {
        setUsers(usersList);
        const savedUserId = localStorage.getItem(SESSION_USER_ID_KEY);
        if (isAuthenticated && savedUserId) {
          const found = usersList.find((u: User) => u.id === savedUserId);
          if (found) {
            setCurrentUser(found);
            if (found.role === 'teacher') setActiveTab('teacher_portal');
          }
        }
      }

      // 2. Courses
      const coursesData = await safeFetchJson<any>('/api/courses', []);
      const coursesList: Course[] = Array.isArray(coursesData) ? coursesData : (coursesData?.courses || []);
      if (coursesList.length > 0) {
        setCourses(coursesList);
        if (!coursesList.some((c: Course) => c.id === selectedCourse.id)) {
          setSelectedCourse(coursesList[0]);
        }
      }

      // 3. Provider State
      const provData = await safeFetchJson<any>('/api/provider-state', null);
      if (provData) {
        setProviderState(provData?.provider || provData);
      }

      // 4. Assessments
      const asmData = await safeFetchJson<any>('/api/assessments', []);
      const asmList = Array.isArray(asmData) ? asmData : (asmData?.assessments || []);
      setAssessments(asmList);

      // 5. Questions
      const qData = await safeFetchJson<any>('/api/questions', []);
      const qList = Array.isArray(qData) ? qData : (qData?.questions || []);
      setQuestions(qList);
    } catch (err) {
      console.error('Error fetching initial data:', err);
    } finally {
      setIsLoadingInitial(false);
    }
  };

  useEffect(() => {
    loadAppData();
  }, []);

  // When selectedCourse or currentUser changes, load course-specific data
  useEffect(() => {
    if (!selectedCourse.id) return;

    // Load Chat messages for this course
    safeFetchJson<any>(`/api/learning/history?course_id=${selectedCourse.id}&user_id=${currentUser.id}`, [])
      .then((data) => {
        const list = Array.isArray(data) ? data : (data?.messages || []);
        setMessages(list);
      });

    // Load Roadmap
    safeFetchJson<any>(`/api/roadmap/${selectedCourse.id}`, null)
      .then((data) => {
        if (data) {
          setRoadmap(data?.roadmap || data);
        }
      });

    // Load Progress
    safeFetchJson<any>(`/api/progress/${selectedCourse.id}?user_id=${currentUser.id}`, [])
      .then((data) => {
        const list = Array.isArray(data) ? data : (data?.progress || []);
        setProgress(list);
      });

    // Load Course Documents
    safeFetchJson<any>(`/api/courses/${selectedCourse.id}/documents`, [])
      .then((data) => {
        const list = Array.isArray(data) ? data : (data?.documents || []);
        setCourseDocuments(list);
      });

    // Load Personalization
    safeFetchJson<any>(`/api/personalization?user_id=${currentUser.id}`, null)
      .then((data) => {
        if (data) {
          setPersonalization(data?.personalization || data);
        }
      });
  }, [selectedCourse.id, currentUser.id]);

  // Handle User switch (Student <-> Teacher)
  const handleSwitchUser = (newUser: User) => {
    setCurrentUser(newUser);
    localStorage.setItem('edu_ai_user_id', newUser.id);
    if (newUser.role === 'teacher') {
      setActiveTab('teacher_portal');
    } else if (activeTab === 'teacher_portal') {
      setActiveTab('dashboard');
    }
  };

  // Handle Login Success from Pop-up Modal
  const handleLoginSuccess = (user: User, token?: string) => {
    setCurrentUser(user);
    saveSession(localStorage, user.id, token);
    setIsAuthenticated(true);
    setUsers((prev) => {
      if (prev.some((u) => u.id === user.id)) return prev;
      return [...prev, user];
    });
    if (user.role === 'teacher') {
      setActiveTab('teacher_portal');
    } else if (activeTab === 'teacher_portal') {
      setActiveTab('dashboard');
    }
  };

  // Handle Logout
  const handleLogout = async () => {
    try {
      await apiFetch('/api/auth/logout', { method: 'POST' });
    } catch {
      // Local logout still needs to clear a stale client session when the
      // temporary API or the remote backend is unavailable.
    }
    clearSession(localStorage);
    setIsAuthenticated(false);
    setMessages([]);
    setAssessments([]);
    setProgress([]);
    setRoadmap(null);
    setPersonalization({ notes: [], saved_question_ids: [], study_goals: [], theme: 'light' });
    setActiveTab('dashboard');
    setIsLoginModalOpen(false);
  };

  // Chat message send handler
  const handleSendMessage = async (text: string): Promise<ThinkingAssessment | null> => {
    setIsSendingMessage(true);
    // Optimistic user message
    const userMsg: ChatMessage = {
      id: `usr_${Date.now()}`,
      session_id: 'default',
      user_id: currentUser.id,
      course_id: selectedCourse.id,
      role: 'user',
      content: text,
      created_at: new Date().toISOString(),
    };
    setMessages((prev) => [...prev, userMsg]);

    try {
      const res = await apiFetch('/api/learning/chat', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: currentUser.id,
          course_id: selectedCourse.id,
          message: text,
        }),
      });
      const data = await readApiJson<any>(res);

      // The demo service returns `reply`; Drogon returns an `answer` string.
      // Convert the latter to the UI's message shape so the mentor response is
      // rendered for either implementation.
      const reply = toMentorReply(data, {
        userId: currentUser.id,
        courseId: selectedCourse.id,
        now: Date.now,
        createdAt: () => new Date().toISOString(),
      });
      if (reply) {
        setMessages((prev) => [...prev, reply]);
      }

      if (data.assessment) {
        setAssessments((prev) => [data.assessment, ...prev]);
        return data.assessment;
      }
    } catch (err) {
      console.error('Error sending message to mentor:', err);
    } finally {
      setIsSendingMessage(false);
    }
    return null;
  };

  // Toggle bookmark question
  const handleToggleSaveQuestion = async (questionId: string) => {
    const isAlreadySaved = personalization.saved_question_ids.includes(questionId);
    const updated = isAlreadySaved
      ? personalization.saved_question_ids.filter((id) => id !== questionId)
      : [...personalization.saved_question_ids, questionId];

    setPersonalization((prev) => ({ ...prev, saved_question_ids: updated }));

    try {
      await apiFetch('/api/personalization', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: currentUser.id,
          saved_question_ids: updated,
        }),
      });
    } catch (err) {
      console.error('Error saving question bookmark:', err);
    }
  };

  // Approve question by teacher
  const handleApproveQuestion = async (questionId: string) => {
    try {
      const res = await apiFetch(`/api/questions/${questionId}/approve`, {
        method: 'PATCH',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ teacher_name: currentUser.name }),
      });
      const updatedQ = await readApiJson<Question>(res);
      setQuestions((prev) => prev.map((q) => (q.id === questionId ? updatedQ : q)));
    } catch (err) {
      console.error('Error approving question:', err);
    }
  };

  // AI generate question
  const handleGenerateAIQuestion = async (topic?: string, difficulty?: QuestionDifficulty | string) => {
    setIsGeneratingQuestion(true);
    try {
      const res = await apiFetch('/api/questions/generate-ai', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          course_id: selectedCourse.id,
          topic: topic || selectedCourse.name,
          difficulty: difficulty || 'intermediate',
        }),
      });
      const newQ = await readApiJson<Question>(res);
      setQuestions((prev) => [newQ, ...prev]);
    } catch (err) {
      console.error('Error generating AI question:', err);
    } finally {
      setIsGeneratingQuestion(false);
    }
  };

  // Add Question by Teacher
  const handleAddQuestion = async (data: {
    question: string;
    answer: string;
    hint: string;
    difficulty: 'beginner' | 'intermediate' | 'advanced';
    question_type: 'conceptual' | 'scenario' | 'multiple_choice';
  }) => {
    try {
      const res = await apiFetch('/api/questions', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          course_id: selectedCourse.id,
          created_by: currentUser.name,
          source: 'teacher',
          status: 'approved',
          ...data,
        }),
      });
      const newQ = await readApiJson<Question>(res);
      setQuestions((prev) => [newQ, ...prev]);
    } catch (err) {
      console.error('Error adding question:', err);
    }
  };

  // Teacher Review Assessment
  const handleReviewAssessment = async (
    assessmentId: string,
    teacherScore: number,
    teacherFeedback: string
  ) => {
    try {
      const res = await apiFetch(`/api/assessments/${assessmentId}/review`, {
        method: 'PATCH',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          teacher_score: teacherScore,
          teacher_feedback: teacherFeedback,
        }),
      });
      const updated = await readApiJson<ThinkingAssessment>(res);
      setAssessments((prev) => prev.map((a) => (a.id === assessmentId ? updated : a)));
    } catch (err) {
      console.error('Error saving teacher review:', err);
    }
  };

  // Teacher Switch LLM Provider
  const handleSwitchProvider = async (provider: 'gemini' | 'ollama', cloudFallback: boolean) => {
    try {
      const res = await apiFetch('/api/admin/llm-provider', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          provider,
          cloud_fallback: cloudFallback,
        }),
      });
      const updated = await readApiJson<AIProviderState>(res);
      setProviderState(updated);
    } catch (err) {
      console.error('Error switching provider:', err);
    }
  };

  // Update Course Answer Policy
  const handleUpdateCoursePolicy = async (policy: AnswerPolicy) => {
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/policy`, {
        method: 'PATCH',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ answer_policy: policy }),
      });
      const payload = await readApiJson<Course | { course: Course }>(res);
      const updatedCourse = apiResponseValue<Course>(payload, 'course') || payload as Course;
      setSelectedCourse(updatedCourse);
      setCourses((prev) => prev.map((c) => (c.id === updatedCourse.id ? updatedCourse : c)));
    } catch (err) {
      console.error('Error updating course policy:', err);
    }
  };

  // Update Roadmap topic status
  const handleUpdateTopicStatus = async (topicId: string, status: any) => {
    if (!roadmap) return;
    try {
      const res = await apiFetch(roadmapTopicStatusPath(selectedCourse.id, topicId), {
        method: 'PATCH',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ status }),
      });
      const updatedRoadmap = await readApiJson<Roadmap>(res);
      setRoadmap(updatedRoadmap);

      // Also refresh progress
      const pData = await safeFetchJson<any>(`/api/progress/${selectedCourse.id}?user_id=${currentUser.id}`, []);
      const pList = Array.isArray(pData) ? pData : (pData?.progress || []);
      setProgress(pList);
    } catch (err) {
      console.error('Error updating roadmap topic:', err);
    }
  };

  // Save Personalized Data (notes, goals, etc.)
  const handleSavePersonalization = async (partialData: Partial<PersonalizedData>) => {
    const updated: PersonalizedData = { ...personalization, ...partialData };
    setPersonalization(updated);
    try {
      await apiFetch('/api/personalization', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ...updated, user_id: currentUser.id }),
      });
    } catch (err) {
      console.error('Error saving personalization:', err);
    }
  };

  // Upload new Course Document
  const handleUploadDocument = async (title: string, content: string, tags: string[], sourcePath?: string) => {
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/documents`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ title, content, tags, source_path: sourcePath }),
      });
      const payload = await readApiJson<CourseDocument | { document: CourseDocument }>(res);
      const newDoc = apiResponseValue<CourseDocument>(payload, 'document') || payload as CourseDocument;
      setCourseDocuments((prev) => [newDoc, ...prev]);
      setSelectedCourse((prev) => ({
        ...prev,
        documents_count: prev.documents_count + 1,
      }));
    } catch (err) {
      console.error('Error uploading document:', err);
      throw err;
    }
  };

  // Delete Course Document
  const handleDeleteDocument = async (docId: string) => {
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/documents/${docId}`, {
        method: 'DELETE',
      });
      if (!res.ok) throw new Error('Không thể xóa tài liệu');
      setCourseDocuments((prev) => prev.filter((d) => d.id !== docId));
      setSelectedCourse((prev) => ({
        ...prev,
        documents_count: Math.max(0, prev.documents_count - 1),
      }));
    } catch (err) {
      console.error('Error deleting document:', err);
      throw err;
    }
  };

  // Create New Course / Class
  const handleCreateCourse = async (data: {
    name: string;
    code: string;
    description: string;
    answer_policy: AnswerPolicy;
    enrolled_count: number;
  }) => {
    try {
      const res = await apiFetch('/api/courses', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          ...data,
          teacher_id: currentUser.id,
          teacher_name: currentUser.name,
        }),
      });
      if (!res.ok) {
        throw new Error('Không thể tạo môn học mới');
      }
      const newCourse: Course = await readApiJson<Course>(res);
      setCourses((prev) => [newCourse, ...prev]);
      setSelectedCourse(newCourse);
      setCourseDocuments([]);
      setQuestions((prev) => prev.filter((q) => q.course_id === newCourse.id));
    } catch (err) {
      console.error('Error creating course:', err);
      throw err;
    }
  };

  // Batch Import Questions to Question Bank
  const handleBatchImportQuestions = async (items: any[]) => {
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/questions/batch`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          questions: items,
          teacher_name: currentUser.name,
        }),
      });
      if (!res.ok) {
        throw new Error('Lỗi khi nạp câu hỏi');
      }
      const data = await readApiJson<{ questions?: Question[] }>(res);
      const importedQuestions = data.questions;
      if (Array.isArray(importedQuestions)) {
        setQuestions((prev) => [...importedQuestions, ...prev]);
        setSelectedCourse((prev) => ({
          ...prev,
          questions_count: prev.questions_count + importedQuestions.length,
        }));
      }
    } catch (err) {
      console.error('Error batch importing questions:', err);
      throw err;
    }
  };

  // Batch Generate AI Questions
  const handleBatchGenerateAIQuestions = async (topic: string, count: number, difficulty: string, documentId?: string) => {
    try {
      const res = await apiFetch(`/api/courses/${selectedCourse.id}/questions/generate-batch`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          topic,
          count,
          difficulty,
          document_id: documentId,
        }),
      });
      if (!res.ok) throw new Error('Lỗi khi sinh hàng loạt câu hỏi AI');
      const data = await readApiJson<{ questions?: Question[] }>(res);
      const generatedQuestions = data.questions;
      if (Array.isArray(generatedQuestions)) {
        setQuestions((prev) => [...generatedQuestions, ...prev]);
        setSelectedCourse((prev) => ({
          ...prev,
          questions_count: prev.questions_count + generatedQuestions.length,
        }));
      }
    } catch (err) {
      console.error('Error generating batch AI questions:', err);
      throw err;
    }
  };

  // Reset Demo Data
  const handleResetDemoData = async () => {
    if (!confirm('Bạn có chắc muốn khôi phục lại dữ liệu học tập mẫu ban đầu?')) return;
    try {
      await apiFetch('/api/demo/reset', { method: 'POST' });
      await loadAppData();
      alert('Đã khôi phục dữ liệu mẫu thành công!');
    } catch (err) {
      console.error('Error resetting demo:', err);
    }
  };

  // Quick navigation helpers
  const handleAskInChat = (text: string) => {
    setActiveTab('chat');
    handleSendMessage(text);
  };

  return (
    <div className="min-h-screen bg-slate-100 dark:bg-[#050b18] text-slate-900 dark:text-slate-100 flex flex-col transition-colors duration-200">
      {/* Top Universal App Header */}
      <Header
        currentUser={isAuthenticated ? currentUser : null}
        isDark={darkMode}
        onToggleTheme={() => setDarkMode(!darkMode)}
        activeTab={activeTab}
        onChangeTab={setActiveTab}
        providerState={providerState}
        selectedCourse={selectedCourse}
        onOpenLoginModal={() => setIsLoginModalOpen(true)}
        onLogout={handleLogout}
      />

      {/* Main View Container */}
      <main className="flex-1 w-full max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        {isLoadingInitial ? (
          <div className="flex flex-col items-center justify-center h-96 space-y-4">
            <div className="w-10 h-10 rounded-full border-4 border-blue-600 border-t-transparent animate-spin" />
            <p className="text-sm font-semibold text-slate-500 dark:text-slate-400">
              Đang khởi tạo hệ thống Edu AI Mentor & Bảng điều khiển...
            </p>
          </div>
        ) : (
          <>
            {activeTab === 'dashboard' && (
              <DashboardOverview
                currentUser={currentUser}
                courses={courses}
                selectedCourse={selectedCourse}
                onSelectCourse={setSelectedCourse}
                assessments={assessments}
                progress={progress}
                onNavigateTab={setActiveTab}
                onOpenDocuments={() => setIsDocModalOpen(true)}
                onOpenLoginModal={() => setIsLoginModalOpen(true)}
              />
            )}

            {activeTab === 'chat' && (
              <MentorChat
                currentUser={currentUser}
                selectedCourse={selectedCourse}
                messages={messages}
                onSendMessage={handleSendMessage}
                isSending={isSendingMessage}
                onClearSession={() => {
                  setMessages([]);
                }}
              />
            )}

            {activeTab === 'roadmap' && (
              <RoadmapView
                roadmap={roadmap}
                selectedCourse={selectedCourse}
                currentUser={currentUser}
                onUpdateTopicStatus={handleUpdateTopicStatus}
                onAskTopicInChat={(topicName) => handleAskInChat(`Giải thích chủ đề "${topicName}" và các ứng dụng thực tế của nó`)}
              />
            )}

            {activeTab === 'questions' && (
              <QuestionBankView
                questions={questions}
                selectedCourse={selectedCourse}
                currentUser={currentUser}
                savedQuestionIds={personalization.saved_question_ids}
                onToggleSaveQuestion={handleToggleSaveQuestion}
                onApproveQuestion={handleApproveQuestion}
                onGenerateAIQuestion={handleGenerateAIQuestion}
                onAskQuestionInChat={(qText) => handleAskInChat(`Tôi muốn giải bài toán sau: "${qText}". Hãy cho tôi gợi ý tư duy Socratic!`)}
                isGeneratingAI={isGeneratingQuestion}
              />
            )}

            {activeTab === 'teacher_portal' && (
              <TeacherPortal
                currentUser={currentUser}
                courses={courses}
                selectedCourse={selectedCourse}
                onSelectCourse={setSelectedCourse}
                assessments={assessments}
                questions={questions}
                roadmap={roadmap}
                progress={progress}
                courseDocuments={courseDocuments}
                providerState={providerState}
                onReviewAssessment={handleReviewAssessment}
                onApproveQuestion={handleApproveQuestion}
                onAddQuestion={handleAddQuestion}
                onGenerateAIQuestion={handleGenerateAIQuestion}
                isGeneratingQuestion={isGeneratingQuestion}
                onUploadDocument={handleUploadDocument}
                onDeleteDocument={handleDeleteDocument}
                onCreateCourse={handleCreateCourse}
                onBatchImportQuestions={handleBatchImportQuestions}
                onBatchGenerateAIQuestions={handleBatchGenerateAIQuestions}
                onUpdateCoursePolicy={handleUpdateCoursePolicy}
                onSwitchProvider={handleSwitchProvider}
                onNavigateToChat={(prompt) => (prompt ? handleAskInChat(prompt) : setActiveTab('chat'))}
                onNavigateToStudentView={() => setActiveTab('dashboard')}
              />
            )}

            {activeTab === 'assessment' && (
              currentUser.role === 'teacher' ? (
                <TeacherPortal
                  currentUser={currentUser}
                  courses={courses}
                  selectedCourse={selectedCourse}
                  onSelectCourse={setSelectedCourse}
                  assessments={assessments}
                  questions={questions}
                  roadmap={roadmap}
                  progress={progress}
                  courseDocuments={courseDocuments}
                  providerState={providerState}
                  onReviewAssessment={handleReviewAssessment}
                  onApproveQuestion={handleApproveQuestion}
                  onAddQuestion={handleAddQuestion}
                  onGenerateAIQuestion={handleGenerateAIQuestion}
                  isGeneratingQuestion={isGeneratingQuestion}
                  onUploadDocument={handleUploadDocument}
                  onDeleteDocument={handleDeleteDocument}
                  onCreateCourse={handleCreateCourse}
                  onBatchImportQuestions={handleBatchImportQuestions}
                  onBatchGenerateAIQuestions={handleBatchGenerateAIQuestions}
                  onUpdateCoursePolicy={handleUpdateCoursePolicy}
                  onSwitchProvider={handleSwitchProvider}
                  onNavigateToChat={(prompt) => (prompt ? handleAskInChat(prompt) : setActiveTab('chat'))}
                  onNavigateToStudentView={() => setActiveTab('dashboard')}
                />
              ) : (
                <AssessmentReviewView
                  assessments={assessments}
                  selectedCourse={selectedCourse}
                  currentUser={currentUser}
                  providerState={providerState}
                  onReviewAssessment={handleReviewAssessment}
                  onSwitchProvider={handleSwitchProvider}
                  onUpdateCoursePolicy={handleUpdateCoursePolicy}
                />
              )
            )}

            {(activeTab === 'vault' || activeTab === 'personal') && (
              <PersonalizedVaultView
                currentUser={currentUser}
                personalization={personalization}
                questions={questions}
                courses={courses}
                onSavePersonalization={handleSavePersonalization}
                onAskQuestionInChat={(text) => handleAskInChat(text)}
                onResetDemoData={handleResetDemoData}
              />
            )}
          </>
        )}
      </main>

      {/* RAG Documents & Material Inspection Modal */}
      <CourseDocumentModal
        isOpen={isDocModalOpen}
        onClose={() => setIsDocModalOpen(false)}
        course={selectedCourse}
        currentUser={currentUser}
        documents={courseDocuments}
        onUploadDocument={handleUploadDocument}
      />

      {/* Authentication & User Login Pop-up Card */}
      <LoginModal
        isOpen={isLoginModalOpen}
        onClose={() => setIsLoginModalOpen(false)}
        onLoginSuccess={handleLoginSuccess}
      />
    </div>
  );
}
