import React, { useState, useRef } from 'react';
import {
  UploadCloud,
  FileText,
  Plus,
  X,
  RefreshCw,
  AlertCircle,
  CheckCircle2,
  Tag,
  Layers,
  Sparkles,
  FileCode,
  FileSpreadsheet,
} from 'lucide-react';

interface UploadDocumentModalProps {
  isOpen: boolean;
  onClose: () => void;
  courseName: string;
  onUpload: (title: string, content: string, tags: string[], sourcePath?: string) => Promise<void>;
  onTriggerBatchAI?: (topic: string, count: number, difficulty: string) => Promise<void>;
}

export const UploadDocumentModal: React.FC<UploadDocumentModalProps> = ({
  isOpen,
  onClose,
  courseName,
  onUpload,
  onTriggerBatchAI,
}) => {
  const [activeMode, setActiveMode] = useState<'upload' | 'manual'>('upload');
  const [docTitle, setDocTitle] = useState('');
  const [docContent, setDocContent] = useState('');
  const [docTags, setDocTags] = useState('giao-trinh, ly-thuyet, thuc-hanh');
  const [fileName, setFileName] = useState('');
  const [fileSize, setFileSize] = useState('');
  const [autoGenQuestions, setAutoGenQuestions] = useState(true);
  const [isUploading, setIsUploading] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [isDragOver, setIsDragOver] = useState(false);
  const fileInputRef = useRef<HTMLInputElement>(null);

  if (!isOpen) return null;

  const handleFileSelect = (file: File) => {
    if (!file) return;
    setFileName(file.name);
    const sizeKb = Math.round(file.size / 1024);
    setFileSize(sizeKb > 1024 ? `${(sizeKb / 1024).toFixed(1)} MB` : `${sizeKb} KB`);
    
    // Auto set title if empty
    if (!docTitle.trim()) {
      const cleanName = file.name.replace(/\.[^/.]+$/, '').replace(/[-_]/g, ' ');
      setDocTitle(cleanName);
    }

    const reader = new FileReader();
    reader.onload = (e) => {
      const text = e.target?.result as string;
      if (text) {
        setDocContent(text);
      }
    };
    reader.onerror = () => {
      setErrorMessage('Không thể đọc nội dung tệp. Vui lòng thử lại hoặc dán văn bản.');
    };
    reader.readAsText(file);
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragOver(false);
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      handleFileSelect(e.dataTransfer.files[0]);
    }
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!docTitle.trim() || !docContent.trim()) {
      setErrorMessage('Vui lòng cung cấp tiêu đề và nội dung tài liệu học tập.');
      return;
    }

    setIsUploading(true);
    setErrorMessage('');
    try {
      const tagsArray = docTags
        .split(',')
        .map((t) => t.trim())
        .filter(Boolean);

      const sourcePath = fileName ? `/uploads/${fileName}` : `/materials/${Date.now()}.txt`;
      await onUpload(docTitle.trim(), docContent.trim(), tagsArray, sourcePath);

      // If teacher checked auto-generate questions, trigger batch generation
      if (autoGenQuestions && onTriggerBatchAI) {
        try {
          await onTriggerBatchAI(docTitle.trim(), 3, 'intermediate');
        } catch (genErr) {
          console.warn('Auto batch question generation non-blocking error:', genErr);
        }
      }

      onClose();
    } catch (err: any) {
      setErrorMessage(err.message || 'Lỗi khi nạp tài liệu vào hệ thống RAG');
    } finally {
      setIsUploading(false);
    }
  };

  // Estimate chunks count
  const estimatedChunks = Math.max(
    1,
    docContent.trim().split(/\n\s*\n/).filter((p) => p.trim().length > 0).length
  );

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/70 backdrop-blur-xs animate-fadeIn">
      <div className="w-full max-w-2xl rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-2xl p-6 space-y-5 max-h-[90vh] overflow-y-auto">
        <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
          <div className="flex items-center gap-2.5">
            <div className="w-9 h-9 rounded-xl bg-blue-50 dark:bg-blue-950/60 border border-blue-200 dark:border-blue-800 flex items-center justify-center">
              <UploadCloud className="w-5 h-5 text-blue-600 dark:text-blue-400" />
            </div>
            <div>
              <h3 className="font-bold text-base text-slate-900 dark:text-white">
                Nạp Tài Liệu Giáo Trình & Tri Thức RAG
              </h3>
              <p className="text-xs text-slate-500 dark:text-slate-400">
                Môn học: <strong className="text-slate-700 dark:text-slate-300">{courseName}</strong>
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

        {/* Mode Switcher */}
        <div className="flex items-center gap-2 p-1 rounded-xl bg-slate-100 dark:bg-slate-800/80 text-xs">
          <button
            type="button"
            onClick={() => setActiveMode('upload')}
            className={`flex-1 py-1.5 rounded-lg font-semibold flex items-center justify-center gap-1.5 transition ${
              activeMode === 'upload'
                ? 'bg-white dark:bg-slate-700 text-blue-600 dark:text-blue-400 shadow-xs'
                : 'text-slate-600 dark:text-slate-400'
            }`}
          >
            <UploadCloud className="w-4 h-4" />
            <span>Tải Lên Tệp Tài Liệu (.txt, .md, .pdf, .json)</span>
          </button>
          <button
            type="button"
            onClick={() => setActiveMode('manual')}
            className={`flex-1 py-1.5 rounded-lg font-semibold flex items-center justify-center gap-1.5 transition ${
              activeMode === 'manual'
                ? 'bg-white dark:bg-slate-700 text-blue-600 dark:text-blue-400 shadow-xs'
                : 'text-slate-600 dark:text-slate-400'
            }`}
          >
            <FileText className="w-4 h-4" />
            <span>Dán / Soạn Trực Tiếp Văn Bản</span>
          </button>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4">
          {activeMode === 'upload' && (
            <div
              onDragOver={(e) => {
                e.preventDefault();
                setIsDragOver(true);
              }}
              onDragLeave={() => setIsDragOver(false)}
              onDrop={handleDrop}
              onClick={() => fileInputRef.current?.click()}
              className={`p-6 rounded-2xl border-2 border-dashed text-center cursor-pointer transition ${
                isDragOver
                  ? 'border-blue-500 bg-blue-50/50 dark:bg-blue-950/30'
                  : fileName
                  ? 'border-emerald-400 bg-emerald-50/30 dark:bg-emerald-950/20'
                  : 'border-slate-300 dark:border-slate-700 hover:border-blue-400 dark:hover:border-blue-600 bg-slate-50/50 dark:bg-slate-900/30'
              }`}
            >
              <input
                ref={fileInputRef}
                type="file"
                accept=".txt,.md,.markdown,.json,.csv,.doc,.docx,.pdf"
                className="hidden"
                onChange={(e) => {
                  if (e.target.files && e.target.files[0]) {
                    handleFileSelect(e.target.files[0]);
                  }
                }}
              />
              <div className="flex flex-col items-center justify-center gap-2">
                <div className="w-12 h-12 rounded-2xl bg-blue-100 dark:bg-blue-900/50 flex items-center justify-center text-blue-600 dark:text-blue-400">
                  <UploadCloud className="w-6 h-6" />
                </div>
                {fileName ? (
                  <div>
                    <p className="text-xs font-bold text-emerald-600 dark:text-emerald-400 flex items-center justify-center gap-1.5">
                      <CheckCircle2 className="w-4 h-4" />
                      Đã chọn tệp: {fileName}
                    </p>
                    <p className="text-[11px] text-slate-400 mt-0.5">
                      Dung lượng: {fileSize} • Đã đọc {docContent.length.toLocaleString()} ký tự
                    </p>
                    <span className="inline-block mt-2 text-[10px] text-blue-600 dark:text-blue-400 underline font-medium">
                      Nhấp để chọn tệp khác
                    </span>
                  </div>
                ) : (
                  <div>
                    <p className="text-xs font-bold text-slate-800 dark:text-slate-200">
                      Kéo thả tệp tài liệu vào đây hoặc <span className="text-blue-600 dark:text-blue-400 underline">duyệt từ máy tính</span>
                    </p>
                    <p className="text-[11px] text-slate-400 mt-1">
                      Hỗ trợ định dạng văn bản .txt, .md, .markdown, .json, .csv (tự động phân mảnh cho RAG)
                    </p>
                  </div>
                )}
              </div>
            </div>
          )}

          <div>
            <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
              Tiêu đề tài liệu / Chương bài giảng <span className="text-red-500">*</span>:
            </label>
            <input
              type="text"
              value={docTitle}
              onChange={(e) => setDocTitle(e.target.value)}
              placeholder="VD: Chương 4 - Cơ chế Kiểm soát Tắc nghẽn TCP & Thuật toán BBR"
              className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
              required
            />
          </div>

          <div>
            <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1 flex items-center justify-between">
              <span className="flex items-center gap-1.5">
                <Tag className="w-3.5 h-3.5 text-blue-500" />
                Thẻ chủ đề / Tags (phân cách bằng dấu phẩy):
              </span>
              <span className="text-[10px] text-slate-400 font-normal">Giúp AI Mentor gắn nhãn truy vấn</span>
            </label>
            <input
              type="text"
              value={docTags}
              onChange={(e) => setDocTags(e.target.value)}
              placeholder="tcp, congestion-control, bbr, quic, slow-start"
              className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>

          <div>
            <div className="flex items-center justify-between mb-1">
              <label className="text-xs font-semibold text-slate-700 dark:text-slate-300 flex items-center gap-1.5">
                <Layers className="w-3.5 h-3.5 text-indigo-500" />
                Nội dung tài liệu học thuật <span className="text-red-500">*</span>:
              </label>
              <div className="text-[10px] text-slate-400 font-mono">
                {docContent.length.toLocaleString()} ký tự • Ước tính: ~{estimatedChunks} chunks
              </div>
            </div>
            <textarea
              rows={6}
              value={docContent}
              onChange={(e) => setDocContent(e.target.value)}
              placeholder="Dán nội dung giáo trình, bài giảng hoặc ghi chú học thuật vào đây. Hệ thống sẽ tự động phân mảnh (chunking) theo đoạn văn bản để làm căn cứ trích dẫn chính xác cho AI Mentor..."
              className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white font-mono focus:outline-none focus:ring-2 focus:ring-blue-500"
              required
            />
          </div>

          {/* AI Auto-generate Questions Checkbox */}
          <div className="p-3 rounded-xl bg-blue-50/60 dark:bg-blue-950/40 border border-blue-200/60 dark:border-blue-900/50 flex items-start gap-2.5">
            <input
              type="checkbox"
              id="autoGen"
              checked={autoGenQuestions}
              onChange={(e) => setAutoGenQuestions(e.target.checked)}
              className="mt-0.5 rounded text-blue-600 focus:ring-blue-500"
            />
            <label htmlFor="autoGen" className="text-xs text-slate-700 dark:text-slate-300 cursor-pointer">
              <strong className="text-blue-900 dark:text-blue-300 flex items-center gap-1">
                <Sparkles className="w-3.5 h-3.5 text-blue-600" />
                Tự động sinh 3 câu hỏi Socratic từ tài liệu này nạp vào Ngân Hàng Câu Hỏi
              </strong>
              <p className="text-[11px] text-slate-500 dark:text-slate-400 mt-0.5">
                AI sẽ phân tích các đoạn văn bản trong tài liệu để thiết kế các câu hỏi phản biện, gợi mở tư duy và đáp án mẫu cho lớp học.
              </p>
            </label>
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
              disabled={isUploading}
              className="px-5 py-2 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-md shadow-blue-900/20 disabled:opacity-50 transition"
            >
              {isUploading ? (
                <>
                  <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                  <span>Đang Phân Đoạn & Nạp RAG...</span>
                </>
              ) : (
                <>
                  <UploadCloud className="w-3.5 h-3.5" />
                  <span>Nạp Vào Hệ Thống Tri Thức</span>
                </>
              )}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};
