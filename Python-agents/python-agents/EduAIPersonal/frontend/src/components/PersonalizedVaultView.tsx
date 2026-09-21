import React, { useState } from 'react';
import {
  FolderLock,
  FileText,
  Bookmark,
  Target,
  Download,
  Upload,
  Plus,
  Trash2,
  Edit2,
  CheckCircle2,
  Sparkles,
  User,
  RotateCcw,
  Calendar,
} from 'lucide-react';
import { PersonalizedData, StudyNote, StudyGoal, Question, Course, User as UserType } from '../types';

interface PersonalizedVaultViewProps {
  currentUser: UserType;
  personalization: PersonalizedData;
  questions: Question[];
  courses: Course[];
  onSavePersonalization: (data: Partial<PersonalizedData>) => Promise<void>;
  onAskQuestionInChat: (text: string) => void;
  onResetDemoData: () => void;
}

export const PersonalizedVaultView: React.FC<PersonalizedVaultViewProps> = ({
  currentUser,
  personalization,
  questions,
  courses,
  onSavePersonalization,
  onAskQuestionInChat,
  onResetDemoData,
}) => {
  const [activeSubTab, setActiveSubTab] = useState<'notes' | 'saved' | 'goals' | 'backup'>('notes');
  const [isEditingNote, setIsEditingNote] = useState(false);
  const [noteTitle, setNoteTitle] = useState('');
  const [noteContent, setNoteContent] = useState('');
  const [noteCourseId, setNoteCourseId] = useState(courses[0]?.id || '');
  const [currentNoteId, setCurrentNoteId] = useState<string | null>(null);

  // New goal state
  const [newGoalTitle, setNewGoalTitle] = useState('');
  const [newGoalDate, setNewGoalDate] = useState('2026-03-30');

  // Notes operations
  const handleOpenNewNote = () => {
    setCurrentNoteId(null);
    setNoteTitle('');
    setNoteContent('');
    setNoteCourseId(courses[0]?.id || '');
    setIsEditingNote(true);
  };

  const handleEditNote = (note: StudyNote) => {
    setCurrentNoteId(note.id);
    setNoteTitle(note.title);
    setNoteContent(note.content);
    setNoteCourseId(note.course_id);
    setIsEditingNote(true);
  };

  const handleSaveNote = async (e: React.FormEvent) => {
    e.preventDefault();
    const existingNotes = [...personalization.notes];
    if (currentNoteId) {
      const idx = existingNotes.findIndex((n) => n.id === currentNoteId);
      if (idx >= 0) {
        existingNotes[idx] = {
          ...existingNotes[idx],
          title: noteTitle,
          content: noteContent,
          course_id: noteCourseId,
          updated_at: new Date().toISOString(),
        };
      }
    } else {
      existingNotes.unshift({
        id: `nt_${Date.now()}`,
        course_id: noteCourseId,
        title: noteTitle || 'Ghi chú mới',
        content: noteContent,
        updated_at: new Date().toISOString(),
      });
    }

    await onSavePersonalization({ notes: existingNotes });
    setIsEditingNote(false);
  };

  const handleDeleteNote = async (noteId: string) => {
    const updated = personalization.notes.filter((n) => n.id !== noteId);
    await onSavePersonalization({ notes: updated });
  };

  // Goals operations
  const handleAddGoal = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!newGoalTitle.trim()) return;
    const newGoal: StudyGoal = {
      id: `gl_${Date.now()}`,
      title: newGoalTitle,
      target_date: newGoalDate,
      completed: false,
    };
    const updated = [...personalization.study_goals, newGoal];
    await onSavePersonalization({ study_goals: updated });
    setNewGoalTitle('');
  };

  const handleToggleGoal = async (goalId: string) => {
    const updated = personalization.study_goals.map((g) =>
      g.id === goalId ? { ...g, completed: !g.completed } : g
    );
    await onSavePersonalization({ study_goals: updated });
  };

  const handleDeleteGoal = async (goalId: string) => {
    const updated = personalization.study_goals.filter((g) => g.id !== goalId);
    await onSavePersonalization({ study_goals: updated });
  };

  // Export & Import
  const handleExportJSON = () => {
    const dataStr = JSON.stringify(
      {
        user: currentUser,
        personalization,
        exported_at: new Date().toISOString(),
      },
      null,
      2
    );
    const blob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `edu_ai_personal_data_${currentUser.name.replace(/\s+/g, '_')}.json`;
    a.click();
    URL.revokeObjectURL(url);
  };

  const handleImportJSON = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = async (event) => {
      try {
        const parsed = JSON.parse(event.target?.result as string);
        if (parsed.personalization) {
          await onSavePersonalization(parsed.personalization);
          alert('Đã nhập dữ liệu cá nhân hóa thành công!');
        }
      } catch (err) {
        alert('Lỗi định dạng tệp JSON!');
      }
    };
    reader.readAsText(file);
  };

  const savedQuestionsList = questions.filter((q) =>
    personalization.saved_question_ids.includes(q.id)
  );

  return (
    <div className="space-y-6">
      {/* Profile & Personal Info Header */}
      <div className="p-6 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm flex flex-col sm:flex-row sm:items-center justify-between gap-6">
        <div className="flex items-center gap-4">
          <img
            src={currentUser.avatar}
            alt={currentUser.name}
            className="w-16 h-16 rounded-2xl object-cover ring-2 ring-blue-600/30"
          />
          <div>
            <div className="flex items-center gap-2">
              <h2 className="text-xl font-extrabold text-slate-900 dark:text-white">
                {currentUser.name}
              </h2>
              <span className="px-2.5 py-0.5 rounded-full text-xs font-bold uppercase bg-blue-100 dark:bg-blue-950 text-blue-800 dark:text-blue-300">
                {currentUser.role === 'student' ? 'Sinh viên' : 'Giảng viên'}
              </span>
            </div>
            <p className="text-xs text-slate-500 mt-0.5">{currentUser.email}</p>
            <p className="text-xs text-slate-600 dark:text-slate-300 mt-1 max-w-xl">
              {currentUser.bio}
            </p>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={handleExportJSON}
            className="inline-flex items-center gap-1.5 px-3.5 py-2 rounded-xl text-xs font-semibold bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 dark:hover:bg-slate-700 text-slate-700 dark:text-slate-200 border border-slate-200 dark:border-slate-700 transition"
          >
            <Download className="w-4 h-4 text-blue-600 dark:text-blue-400" />
            <span>Sao lưu JSON</span>
          </button>
        </div>
      </div>

      {/* Sub Tabs Navigation */}
      <div className="flex items-center space-x-2 border-b border-slate-200 dark:border-slate-800 pb-2">
        <button
          onClick={() => setActiveSubTab('notes')}
          className={`flex items-center gap-2 px-4 py-2 rounded-xl text-xs font-bold transition ${
            activeSubTab === 'notes'
              ? 'bg-blue-900 text-white dark:bg-blue-600'
              : 'text-slate-600 hover:bg-slate-100 dark:text-slate-300 dark:hover:bg-slate-800'
          }`}
        >
          <FileText className="w-4 h-4" />
          <span>Sổ tay Ghi chú ({personalization.notes.length})</span>
        </button>

        <button
          onClick={() => setActiveSubTab('saved')}
          className={`flex items-center gap-2 px-4 py-2 rounded-xl text-xs font-bold transition ${
            activeSubTab === 'saved'
              ? 'bg-blue-900 text-white dark:bg-blue-600'
              : 'text-slate-600 hover:bg-slate-100 dark:text-slate-300 dark:hover:bg-slate-800'
          }`}
        >
          <Bookmark className="w-4 h-4" />
          <span>Câu hỏi đã lưu ({savedQuestionsList.length})</span>
        </button>

        <button
          onClick={() => setActiveSubTab('goals')}
          className={`flex items-center gap-2 px-4 py-2 rounded-xl text-xs font-bold transition ${
            activeSubTab === 'goals'
              ? 'bg-blue-900 text-white dark:bg-blue-600'
              : 'text-slate-600 hover:bg-slate-100 dark:text-slate-300 dark:hover:bg-slate-800'
          }`}
        >
          <Target className="w-4 h-4" />
          <span>Mục tiêu học tập ({personalization.study_goals.length})</span>
        </button>

        <button
          onClick={() => setActiveSubTab('backup')}
          className={`flex items-center gap-2 px-4 py-2 rounded-xl text-xs font-bold transition ${
            activeSubTab === 'backup'
              ? 'bg-blue-900 text-white dark:bg-blue-600'
              : 'text-slate-600 hover:bg-slate-100 dark:text-slate-300 dark:hover:bg-slate-800'
          }`}
        >
          <FolderLock className="w-4 h-4" />
          <span>Quản lý Lưu trữ</span>
        </button>
      </div>

      {/* Tab 1: Personal Notes */}
      {activeSubTab === 'notes' && (
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <h3 className="text-sm font-bold text-slate-900 dark:text-white">
              Ghi chú kiến thức & Tóm tắt nguyên lý
            </h3>
            <button
              onClick={handleOpenNewNote}
              className="inline-flex items-center gap-1.5 px-3.5 py-2 rounded-xl text-xs font-semibold bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white shadow-xs transition"
            >
              <Plus className="w-4 h-4" />
              <span>Thêm ghi chú mới</span>
            </button>
          </div>

          {isEditingNote && (
            <form
              onSubmit={handleSaveNote}
              className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-blue-300 dark:border-blue-700 shadow-md space-y-3"
            >
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-blue-600 dark:text-blue-400">
                  {currentNoteId ? 'Chỉnh sửa ghi chú' : 'Tạo ghi chú mới'}
                </span>
                <button
                  type="button"
                  onClick={() => setIsEditingNote(false)}
                  className="text-xs text-slate-400 hover:text-slate-600"
                >
                  Hủy
                </button>
              </div>

              <div className="grid grid-cols-1 sm:grid-cols-3 gap-3">
                <div className="sm:col-span-2">
                  <input
                    type="text"
                    value={noteTitle}
                    onChange={(e) => setNoteTitle(e.target.value)}
                    placeholder="Tiêu đề ghi chú (Ví dụ: Cơ chế AIMD trong TCP Reno)"
                    className="w-full px-3.5 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white font-semibold"
                    required
                  />
                </div>
                <div>
                  <select
                    value={noteCourseId}
                    onChange={(e) => setNoteCourseId(e.target.value)}
                    className="w-full px-3.5 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300"
                  >
                    {courses.map((c) => (
                      <option key={c.id} value={c.id}>
                        {c.code}
                      </option>
                    ))}
                  </select>
                </div>
              </div>

              <div>
                <textarea
                  rows={4}
                  value={noteContent}
                  onChange={(e) => setNoteContent(e.target.value)}
                  placeholder="Ghi chép công thức, so sánh, hoặc bài học đúc kết được từ Trợ lý Mentor..."
                  className="w-full px-3.5 py-2.5 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                  required
                />
              </div>

              <div className="flex justify-end gap-2">
                <button
                  type="submit"
                  className="px-4 py-2 rounded-xl text-xs font-semibold bg-blue-600 text-white hover:bg-blue-500 transition"
                >
                  Lưu vào sổ tay
                </button>
              </div>
            </form>
          )}

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {personalization.notes.length === 0 ? (
              <div className="col-span-2 p-8 text-center bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800 text-slate-500 text-xs">
                Chưa có ghi chú nào. Hãy tạo ghi chú để ghi lại những phát hiện thú vị khi trao đổi với Mentor!
              </div>
            ) : (
              personalization.notes.map((note) => {
                const course = courses.find((c) => c.id === note.course_id);
                return (
                  <div
                    key={note.id}
                    className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-2.5 flex flex-col justify-between"
                  >
                    <div>
                      <div className="flex items-center justify-between gap-2 mb-1">
                        <span className="px-2 py-0.5 rounded-md text-[10px] font-bold bg-blue-50 dark:bg-blue-950 text-blue-700 dark:text-blue-300">
                          {course?.code || 'Chung'}
                        </span>
                        <span className="text-[10px] text-slate-400">
                          {new Date(note.updated_at).toLocaleDateString()}
                        </span>
                      </div>
                      <h4 className="font-bold text-sm text-slate-900 dark:text-white">
                        {note.title}
                      </h4>
                      <p className="text-xs text-slate-600 dark:text-slate-300 whitespace-pre-wrap mt-1 leading-relaxed line-clamp-4">
                        {note.content}
                      </p>
                    </div>

                    <div className="pt-3 border-t border-slate-100 dark:border-slate-800 flex items-center justify-end gap-2">
                      <button
                        onClick={() => handleEditNote(note)}
                        className="p-1.5 rounded-lg text-slate-400 hover:text-blue-600 hover:bg-slate-100 dark:hover:bg-slate-800"
                        title="Chỉnh sửa"
                      >
                        <Edit2 className="w-3.5 h-3.5" />
                      </button>
                      <button
                        onClick={() => handleDeleteNote(note.id)}
                        className="p-1.5 rounded-lg text-slate-400 hover:text-rose-600 hover:bg-slate-100 dark:hover:bg-slate-800"
                        title="Xóa ghi chú"
                      >
                        <Trash2 className="w-3.5 h-3.5" />
                      </button>
                    </div>
                  </div>
                );
              })
            )}
          </div>
        </div>
      )}

      {/* Tab 2: Saved Questions */}
      {activeSubTab === 'saved' && (
        <div className="space-y-4">
          <h3 className="text-sm font-bold text-slate-900 dark:text-white">
            Danh sách câu hỏi bạn đã đánh dấu từ Ngân hàng
          </h3>

          {savedQuestionsList.length === 0 ? (
            <div className="p-8 text-center bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800 text-slate-500 text-xs">
              Bạn chưa lưu câu hỏi nào. Vào tab "Ngân hàng câu hỏi" và bấm nút biểu tượng Bookmark để lưu lại ôn tập!
            </div>
          ) : (
            <div className="space-y-3">
              {savedQuestionsList.map((q) => (
                <div
                  key={q.id}
                  className="p-4 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm flex items-start justify-between gap-4"
                >
                  <div>
                    <div className="flex items-center gap-2 mb-1">
                      <span className="text-[10px] font-bold px-2 py-0.5 rounded-md bg-blue-100 dark:bg-blue-950 text-blue-800 dark:text-blue-300">
                        {q.difficulty.toUpperCase()}
                      </span>
                      <span className="text-xs text-slate-400">{q.course_name}</span>
                    </div>
                    <h4 className="font-bold text-sm text-slate-900 dark:text-white">
                      {q.question}
                    </h4>
                    <p className="text-xs text-amber-700 dark:text-amber-300 mt-1 italic">
                      💡 Gợi ý: {q.hint}
                    </p>
                  </div>

                  <button
                    onClick={() => onAskQuestionInChat(q.question)}
                    className="px-3 py-1.5 rounded-xl text-xs font-semibold bg-blue-50 text-blue-700 dark:bg-blue-950 dark:text-blue-300 hover:bg-blue-100 dark:hover:bg-blue-900 transition shrink-0"
                  >
                    Thảo luận với AI
                  </button>
                </div>
              ))}
            </div>
          )}
        </div>
      )}

      {/* Tab 3: Study Goals */}
      {activeSubTab === 'goals' && (
        <div className="space-y-4">
          <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm">
            <h3 className="text-sm font-bold text-slate-900 dark:text-white mb-3">
              Thiết lập mục tiêu học tập & Thử thách cá nhân
            </h3>

            <form onSubmit={handleAddGoal} className="flex flex-col sm:flex-row items-center gap-3">
              <input
                type="text"
                value={newGoalTitle}
                onChange={(e) => setNewGoalTitle(e.target.value)}
                placeholder="Nhập mục tiêu mới (Ví dụ: Hoàn thành bài tập kiểm soát tắc nghẽn)"
                className="flex-1 px-3.5 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                required
              />
              <input
                type="date"
                value={newGoalDate}
                onChange={(e) => setNewGoalDate(e.target.value)}
                className="px-3.5 py-2 text-xs rounded-xl bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300"
              />
              <button
                type="submit"
                className="px-4 py-2 rounded-xl text-xs font-semibold bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white shadow-xs transition shrink-0"
              >
                Thêm mục tiêu
              </button>
            </form>
          </div>

          <div className="space-y-2.5">
            {personalization.study_goals.map((g) => (
              <div
                key={g.id}
                className="p-4 rounded-xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm flex items-center justify-between gap-3"
              >
                <div className="flex items-center gap-3">
                  <button
                    onClick={() => handleToggleGoal(g.id)}
                    className={`w-5 h-5 rounded-md flex items-center justify-center border transition ${
                      g.completed
                        ? 'bg-emerald-600 border-emerald-600 text-white'
                        : 'border-slate-300 dark:border-slate-600'
                    }`}
                  >
                    {g.completed && <CheckCircle2 className="w-4 h-4" />}
                  </button>
                  <span
                    className={`text-xs font-medium ${
                      g.completed
                        ? 'line-through text-slate-400 dark:text-slate-500'
                        : 'text-slate-800 dark:text-slate-200'
                    }`}
                  >
                    {g.title}
                  </span>
                </div>

                <div className="flex items-center gap-3">
                  <span className="text-[11px] text-slate-400 flex items-center gap-1">
                    <Calendar className="w-3 h-3" /> Hạn: {g.target_date}
                  </span>
                  <button
                    onClick={() => handleDeleteGoal(g.id)}
                    className="text-slate-400 hover:text-rose-500 p-1"
                  >
                    <Trash2 className="w-3.5 h-3.5" />
                  </button>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Tab 4: Storage Backup & Portability */}
      {activeSubTab === 'backup' && (
        <div className="p-6 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-6">
          <div>
            <h3 className="text-base font-bold text-slate-900 dark:text-white">
              Tính năng Lưu trữ Dữ liệu Cá nhân hóa & Khôi phục
            </h3>
            <p className="text-xs text-slate-500 dark:text-slate-400 mt-1">
              Toàn bộ ghi chú, câu hỏi yêu thích, mục tiêu học tập và lịch sử tư duy được lưu trữ an toàn trong hồ sơ của bạn. Bạn có thể xuất và nhập tệp bất cứ lúc nào.
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {/* Export block */}
            <div className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-700 space-y-2">
              <span className="font-bold text-xs text-slate-800 dark:text-slate-200 flex items-center gap-1.5">
                <Download className="w-4 h-4 text-blue-600" /> Xuất tệp JSON cá nhân
              </span>
              <p className="text-xs text-slate-500">
                Tải xuống toàn bộ nhật ký ghi chú, mục tiêu và danh sách câu hỏi đã lưu thành tệp .json để lưu trữ ngoại tuyến.
              </p>
              <button
                onClick={handleExportJSON}
                className="w-full mt-2 px-4 py-2 rounded-xl text-xs font-semibold bg-blue-600 hover:bg-blue-500 text-white transition"
              >
                Tải xuống bản sao lưu (.json)
              </button>
            </div>

            {/* Import block */}
            <div className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-700 space-y-2">
              <span className="font-bold text-xs text-slate-800 dark:text-slate-200 flex items-center gap-1.5">
                <Upload className="w-4 h-4 text-indigo-600" /> Nhập bản sao lưu JSON
              </span>
              <p className="text-xs text-slate-500">
                Khôi phục lại dữ liệu học tập cá nhân hóa của bạn từ một tệp JSON đã xuất trước đó.
              </p>
              <label className="w-full mt-2 inline-flex items-center justify-center px-4 py-2 rounded-xl text-xs font-semibold bg-indigo-600 hover:bg-indigo-500 text-white cursor-pointer transition">
                <input
                  type="file"
                  accept=".json"
                  onChange={handleImportJSON}
                  className="hidden"
                />
                Chọn tệp JSON để khôi phục
              </label>
            </div>
          </div>

          {/* Reset Demo button */}
          <div className="pt-4 border-t border-slate-100 dark:border-slate-800 flex items-center justify-between">
            <div>
              <p className="text-xs font-bold text-slate-800 dark:text-slate-200">
                Khôi phục dữ liệu mẫu ban đầu
              </p>
              <p className="text-[11px] text-slate-500">
                Làm mới lại toàn bộ các câu hỏi mẫu, tài liệu TCP/IP và các đánh giá tư duy mặc định.
              </p>
            </div>
            <button
              onClick={onResetDemoData}
              className="inline-flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-semibold bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 text-slate-700 dark:text-slate-300 transition"
            >
              <RotateCcw className="w-3.5 h-3.5" />
              <span>Khôi phục mẫu</span>
            </button>
          </div>
        </div>
      )}
    </div>
  );
};
