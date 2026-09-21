import React, { useState } from 'react';
import { FileText, Plus, Upload, Tag, X, BookOpen, Layers } from 'lucide-react';
import { CourseDocument, Course, User } from '../types';

interface CourseDocumentModalProps {
  isOpen: boolean;
  onClose: () => void;
  course: Course;
  currentUser: User;
  documents: CourseDocument[];
  onUploadDocument: (title: string, content: string, tags: string[]) => Promise<void>;
}

export const CourseDocumentModal: React.FC<CourseDocumentModalProps> = ({
  isOpen,
  onClose,
  course,
  currentUser,
  documents,
  onUploadDocument,
}) => {
  const [isAdding, setIsAdding] = useState(false);
  const [title, setTitle] = useState('');
  const [content, setContent] = useState('');
  const [tagsInput, setTagsInput] = useState('tcp, congestion, aimd');
  const [isSubmitting, setIsSubmitting] = useState(false);

  if (!isOpen) return null;

  const isTeacher = currentUser.role === 'teacher';

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!title.trim() || !content.trim()) return;
    setIsSubmitting(true);
    const tags = tagsInput
      .split(',')
      .map((t) => t.trim().toLowerCase())
      .filter(Boolean);
    await onUploadDocument(title, content, tags);
    setIsSubmitting(false);
    setIsAdding(false);
    setTitle('');
    setContent('');
  };

  return (
    <div className="fixed inset-0 z-50 bg-black/60 backdrop-blur-xs flex items-center justify-center p-4">
      <div className="w-full max-w-2xl bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800 p-6 shadow-2xl space-y-5 max-h-[85vh] flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
          <div className="flex items-center gap-2.5">
            <div className="w-9 h-9 rounded-xl bg-blue-50 dark:bg-blue-950 text-blue-600 dark:text-blue-400 flex items-center justify-center">
              <BookOpen className="w-5 h-5" />
            </div>
            <div>
              <h3 className="text-base font-bold text-slate-900 dark:text-white">
                Tài liệu Môn học & Dữ liệu RAG
              </h3>
              <p className="text-xs text-slate-500">
                {course.name} ({documents.length} tài liệu được chỉ mục)
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="p-1.5 rounded-lg text-slate-400 hover:text-slate-600 dark:hover:text-slate-200"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Action button for teacher */}
        {isTeacher && !isAdding && (
          <div className="flex justify-end">
            <button
              onClick={() => setIsAdding(true)}
              className="inline-flex items-center gap-1.5 px-3.5 py-1.5 rounded-xl text-xs font-semibold bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white shadow-xs transition"
            >
              <Plus className="w-4 h-4" />
              <span>Thêm tài liệu RAG</span>
            </button>
          </div>
        )}

        {/* Upload form for teacher */}
        {isAdding && (
          <form
            onSubmit={handleSubmit}
            className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-200 dark:border-slate-700 space-y-3 text-xs"
          >
            <div className="flex justify-between items-center">
              <span className="font-bold text-slate-800 dark:text-slate-200">
                Thêm tài liệu mới vào chỉ mục RAG
              </span>
              <button
                type="button"
                onClick={() => setIsAdding(false)}
                className="text-slate-400 hover:text-slate-600"
              >
                Hủy
              </button>
            </div>

            <div>
              <label className="block text-[11px] font-semibold text-slate-600 dark:text-slate-400 mb-1">
                Tiêu đề tài liệu:
              </label>
              <input
                type="text"
                value={title}
                onChange={(e) => setTitle(e.target.value)}
                placeholder="Ví dụ: Cơ chế điều khiển luồng Sliding Window"
                className="w-full px-3 py-2 rounded-lg bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                required
              />
            </div>

            <div>
              <label className="block text-[11px] font-semibold text-slate-600 dark:text-slate-400 mb-1">
                Nội dung học thuật (sẽ được tự động băm thành các chunk):
              </label>
              <textarea
                rows={4}
                value={content}
                onChange={(e) => setContent(e.target.value)}
                placeholder="Dán nội dung lý thuyết, công thức hoặc đoạn trích giáo trình tại đây..."
                className="w-full px-3 py-2 rounded-lg bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
                required
              />
            </div>

            <div>
              <label className="block text-[11px] font-semibold text-slate-600 dark:text-slate-400 mb-1">
                Tags từ khóa (cách nhau bằng dấu phẩy):
              </label>
              <input
                type="text"
                value={tagsInput}
                onChange={(e) => setTagsInput(e.target.value)}
                placeholder="tcp, congestion, sliding window"
                className="w-full px-3 py-2 rounded-lg bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white"
              />
            </div>

            <div className="flex justify-end gap-2 pt-1">
              <button
                type="submit"
                disabled={isSubmitting}
                className="px-4 py-2 rounded-lg bg-blue-600 text-white font-semibold hover:bg-blue-500 disabled:opacity-50"
              >
                {isSubmitting ? 'Đang lưu...' : 'Lưu tài liệu'}
              </button>
            </div>
          </form>
        )}

        {/* Documents list */}
        <div className="flex-1 overflow-y-auto space-y-4 pr-1">
          {documents.map((doc) => (
            <div
              key={doc.id}
              className="p-4 rounded-xl bg-slate-50 dark:bg-slate-800/40 border border-slate-200/80 dark:border-slate-800 space-y-2.5"
            >
              <div className="flex items-center justify-between">
                <h4 className="font-bold text-sm text-slate-900 dark:text-white flex items-center gap-2">
                  <FileText className="w-4 h-4 text-blue-500" />
                  {doc.title}
                </h4>
                <span className="text-[11px] text-slate-400">
                  {doc.chunks.length} đoạn RAG
                </span>
              </div>

              {/* Chunks */}
              <div className="space-y-2">
                {doc.chunks.map((chunk) => (
                  <div
                    key={chunk.id}
                    className="p-3 rounded-lg bg-white dark:bg-[#0b1329] border border-slate-200/60 dark:border-slate-800 text-xs text-slate-700 dark:text-slate-300 leading-relaxed"
                  >
                    <p>{chunk.content}</p>
                    <div className="mt-2 flex flex-wrap gap-1">
                      {chunk.tags.map((tag, i) => (
                        <span
                          key={i}
                          className="px-2 py-0.5 rounded-md text-[10px] font-medium bg-slate-100 text-slate-600 dark:bg-slate-800 dark:text-slate-300 flex items-center gap-1"
                        >
                          <Tag className="w-2.5 h-2.5" /> {tag}
                        </span>
                      ))}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};
