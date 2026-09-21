import React, { useState } from 'react';
import { Plus, X, GraduationCap, BookOpen, Sliders, Users, RefreshCw, AlertCircle, CheckCircle2 } from 'lucide-react';
import { AnswerPolicy } from '../../types';

interface CreateCourseModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSubmit: (data: {
    name: string;
    code: string;
    description: string;
    answer_policy: AnswerPolicy;
    enrolled_count: number;
  }) => Promise<void>;
}

export const CreateCourseModal: React.FC<CreateCourseModalProps> = ({
  isOpen,
  onClose,
  onSubmit,
}) => {
  const [name, setName] = useState('');
  const [code, setCode] = useState('');
  const [description, setDescription] = useState('');
  const [policy, setPolicy] = useState<AnswerPolicy>('HINT_ONLY');
  const [enrolledCount, setEnrolledCount] = useState(45);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  if (!isOpen) return null;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!name.trim() || !code.trim()) {
      setErrorMessage('Vui lòng nhập đầy đủ tên môn học và mã học phần');
      return;
    }

    setErrorMessage('');
    setIsSubmitting(true);
    try {
      await onSubmit({
        name: name.trim(),
        code: code.trim().toUpperCase(),
        description: description.trim() || 'Môn học mới được khởi tạo trên hệ thống Edu AI Socratic Mentor.',
        answer_policy: policy,
        enrolled_count: Number(enrolledCount) || 40,
      });
      onClose();
    } catch (err: any) {
      setErrorMessage(err.message || 'Lỗi khi tạo môn học mới');
    } finally {
      setIsSubmitting(false);
    }
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/70 backdrop-blur-xs animate-fadeIn">
      <div className="w-full max-w-xl rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-2xl p-6 space-y-5">
        <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
          <div className="flex items-center gap-2.5">
            <div className="w-9 h-9 rounded-xl bg-emerald-50 dark:bg-emerald-950/60 border border-emerald-200 dark:border-emerald-800 flex items-center justify-center">
              <GraduationCap className="w-5 h-5 text-emerald-600 dark:text-emerald-400" />
            </div>
            <div>
              <h3 className="font-bold text-base text-slate-900 dark:text-white">
                Khởi Tạo Lớp Học / Môn Học Mới
              </h3>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                Thiết lập không gian học tập và cấu hình chính sách sư phạm Socratic cho sinh viên
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="p-1 rounded-lg text-slate-400 hover:text-slate-600 hover:bg-slate-100 dark:hover:bg-slate-800 transition"
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        {errorMessage && (
          <div className="p-3 rounded-xl bg-red-50 dark:bg-red-950/40 border border-red-200 dark:border-red-900 text-xs text-red-600 dark:text-red-400 flex items-center gap-2">
            <AlertCircle className="w-4 h-4 shrink-0" />
            <span>{errorMessage}</span>
          </div>
        )}

        <form onSubmit={handleSubmit} className="space-y-4">
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-3">
            <div className="sm:col-span-2">
              <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                Tên lớp học / môn học <span className="text-red-500">*</span>:
              </label>
              <input
                type="text"
                value={name}
                onChange={(e) => setName(e.target.value)}
                placeholder="VD: CS302: An Toàn Hệ Thống & Mật Mã Học"
                className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-emerald-500"
                required
              />
            </div>

            <div>
              <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                Mã học phần <span className="text-red-500">*</span>:
              </label>
              <input
                type="text"
                value={code}
                onChange={(e) => setCode(e.target.value.toUpperCase())}
                placeholder="VD: CS302"
                className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white uppercase font-mono focus:outline-none focus:ring-2 focus:ring-emerald-500"
                required
              />
            </div>
          </div>

          <div>
            <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
              Mô tả ngắn & mục tiêu học phần:
            </label>
            <textarea
              rows={3}
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              placeholder="Mô tả chuẩn đầu ra, đối tượng sinh viên và các chuyên đề trọng tâm của môn học..."
              className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-emerald-500"
            />
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
            <div>
              <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1 flex items-center gap-1.5">
                <Sliders className="w-3.5 h-3.5 text-blue-500" />
                Chính sách trả lời của AI Mentor:
              </label>
              <select
                aria-label="Chính sách trả lời của AI Mentor"
                value={policy}
                onChange={(e) => setPolicy(e.target.value as AnswerPolicy)}
                className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-emerald-500"
              >
                <option value="HINT_ONLY">HINT_ONLY: Gợi ý Socratic (Không đưa đáp án)</option>
                <option value="GUIDED">GUIDED: Hướng dẫn 2 bước (Gợi ý rồi đối thoại)</option>
                <option value="FULL_ANSWER">FULL_ANSWER: Trả lời toàn diện & Phân tích</option>
              </select>
              <p className="text-[10px] text-slate-400 mt-1">
                {policy === 'HINT_ONLY' && 'AI kiên quyết không làm hộ bài, chỉ đặt câu hỏi dẫn dắt tư duy.'}
                {policy === 'GUIDED' && 'AI chia nhỏ vấn đề thành 2 chặng suy luận từng bước.'}
                {policy === 'FULL_ANSWER' && 'AI giải thích đầy đủ kèm bài tập tương tự để luyện tập.'}
              </p>
            </div>

            <div>
              <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1 flex items-center gap-1.5">
                <Users className="w-3.5 h-3.5 text-indigo-500" />
                Sĩ số lớp dự kiến:
              </label>
              <input
                type="number"
                min={1}
                max={500}
                value={enrolledCount}
                onChange={(e) => setEnrolledCount(parseInt(e.target.value) || 0)}
                className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-emerald-500"
              />
              <p className="text-[10px] text-slate-400 mt-1">
                Hệ thống sẽ tự động cấp phát danh sách theo dõi học tập cho lớp.
              </p>
            </div>
          </div>

          <div className="flex justify-end gap-2.5 pt-3 border-t border-slate-100 dark:border-slate-800">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 rounded-xl border border-slate-200 dark:border-slate-700 text-xs font-semibold text-slate-600 dark:text-slate-400 hover:bg-slate-100 dark:hover:bg-slate-800 transition"
            >
              Hủy bỏ
            </button>
            <button
              type="submit"
              disabled={isSubmitting}
              className="px-5 py-2 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-md shadow-emerald-600/20 disabled:opacity-50 transition"
            >
              {isSubmitting ? (
                <>
                  <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                  <span>Đang Khởi Tạo...</span>
                </>
              ) : (
                <>
                  <Plus className="w-3.5 h-3.5" />
                  <span>Tạo Lớp Học Mới</span>
                </>
              )}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};
