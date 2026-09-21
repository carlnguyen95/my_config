import React, { useState, useRef } from 'react';
import {
  UploadCloud,
  FileSpreadsheet,
  FileCode,
  FileText,
  Sparkles,
  Plus,
  X,
  RefreshCw,
  AlertCircle,
  CheckCircle2,
  Download,
  HelpCircle,
  Check,
  Filter,
  Eye,
} from 'lucide-react';
import { CourseDocument } from '../../types';

interface BatchImportQuestionsModalProps {
  isOpen: boolean;
  onClose: () => void;
  courseName: string;
  courseDocuments: CourseDocument[];
  onBatchImport: (questions: any[]) => Promise<void>;
  onBatchGenerateAI: (topic: string, count: number, difficulty: string, documentId?: string) => Promise<void>;
}

export const BatchImportQuestionsModal: React.FC<BatchImportQuestionsModalProps> = ({
  isOpen,
  onClose,
  courseName,
  courseDocuments,
  onBatchImport,
  onBatchGenerateAI,
}) => {
  const [activeTab, setActiveTab] = useState<'file' | 'text' | 'ai'>('ai');
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [successMessage, setSuccessMessage] = useState('');

  // Tab 1: File import
  const [selectedFileName, setSelectedFileName] = useState('');
  const [fileParsedCount, setFileParsedCount] = useState(0);
  const [fileParsedQuestions, setFileParsedQuestions] = useState<any[]>([]);
  const fileInputRef = useRef<HTMLInputElement>(null);

  // Tab 2: Raw Text / Markdown paste
  const [rawText, setRawText] = useState(`Câu hỏi: Cơ chế BBR phát hiện băng thông khả dụng như thế nào?
Đáp án: BBR tạo các chu kỳ ProbeBW ngắn để đẩy nhịp truyền lên nhằm đo lường max delivery rate mà không làm tràn hàng đợi.
Gợi ý: Hãy quan sát chu kỳ dao động pacing rate trong mô hình BBR.
Độ khó: intermediate
Chủ đề: Kiểm soát tắc nghẽn BBR

---

Câu hỏi: Tại sao giao thức UDP không cần cơ chế bắt tay 3 bước?
Đáp án: UDP được thiết kế phi kết nối (connectionless), tối ưu tốc độ và độ trễ tối thiểu, chấp nhận ứng dụng tự kiểm soát mất mát nếu cần.
Gợi ý: Xem xét mục tiêu thiết kế của tầng giao vận cho các ứng dụng thời gian thực như DNS và Live Streaming.
Độ khó: beginner
Chủ đề: Giao thức UDP`);

  // Tab 3: Batch AI Generation
  const [aiTopic, setAiTopic] = useState(courseName.split(':')[1]?.trim() || 'TCP & Giao thức mạng');
  const [aiCount, setAiCount] = useState(3);
  const [aiDifficulty, setAiDifficulty] = useState<'beginner' | 'intermediate' | 'advanced'>('intermediate');
  const [aiDocId, setAiDocId] = useState('');

  // Question status
  const [defaultStatus, setDefaultStatus] = useState<'approved' | 'review'>('approved');

  if (!isOpen) return null;

  // Download Sample CSV
  const handleDownloadCsvTemplate = () => {
    const csvContent =
      'question,answer,hint,difficulty,topic,question_type\n' +
      '"Cơ chế Flow Control trong TCP hoạt động thế nào?","Bên nhận quảng bá trường Receive Window (rwnd) để báo dung lượng bộ đệm còn trống, giúp bên gửi không truyền quá tải.","Hãy nhớ đến trường thông tin trong TCP Header được bên nhận gửi về.","intermediate","Giao thức TCP","conceptual"\n' +
      '"So sánh HTTP/2 và HTTP/3 về mặt tầng giao vận?","HTTP/2 chạy trên TCP nên chịu rủi ro Head-of-Line Blocking, còn HTTP/3 chạy trên QUIC/UDP với các stream độc lập hoàn toàn.","Quan sát sự cố khi một gói tin bị rớt giữa các luồng.","advanced","Giao thức Web","conceptual"\n';

    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.setAttribute('download', `mau_ngan_hang_cau_hoi_${Date.now()}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  // Download Sample JSON
  const handleDownloadJsonTemplate = () => {
    const jsonContent = JSON.stringify(
      [
        {
          question: 'Phân tích bản chất của cuộc tấn công SYN Flood và giải pháp SYN Cookies?',
          answer:
            'Kẻ tấn công gửi hàng loạt gói SYN giả mạo IP khiến server dành tài nguyên hàng đợi nửa mở (half-open backlog). SYN Cookies mã hóa thông tin trạng thái vào Sequence Number ban đầu, không cấp phát bộ nhớ cho tới khi nhận được ACK hợp lệ.',
          hint: 'Tại sao máy chủ không lưu trạng thái kết nối ngay lập tức khi nhận SYN?',
          difficulty: 'advanced',
          topic: 'An toàn mạng',
          question_type: 'scenario',
        },
      ],
      null,
      2
    );

    const blob = new Blob([jsonContent], { type: 'application/json;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.setAttribute('download', `mau_ngan_hang_cau_hoi_${Date.now()}.json`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  // Handle File Parse (JSON or CSV)
  const handleFileChange = (file: File) => {
    if (!file) return;
    setSelectedFileName(file.name);
    setErrorMessage('');
    const reader = new FileReader();

    reader.onload = (e) => {
      try {
        const content = e.target?.result as string;
        if (file.name.endsWith('.json')) {
          const parsed = JSON.parse(content);
          if (Array.isArray(parsed) && parsed.length > 0) {
            setFileParsedQuestions(parsed);
            setFileParsedCount(parsed.length);
          } else {
            setErrorMessage('Tệp JSON không hợp lệ hoặc rỗng. Cần là một danh sách các câu hỏi.');
          }
        } else {
          // Parse CSV
          const lines = content.split('\n').filter((l) => l.trim().length > 0);
          if (lines.length <= 1) {
            setErrorMessage('Tệp CSV rỗng hoặc chỉ có dòng tiêu đề.');
            return;
          }
          const headers = lines[0].split(',').map((h) => h.trim().replace(/^["']|["']$/g, '').toLowerCase());
          const qIndex = headers.indexOf('question');
          const aIndex = headers.indexOf('answer');
          const hIndex = headers.indexOf('hint');
          const dIndex = headers.indexOf('difficulty');
          const tIndex = headers.indexOf('topic');

          const parsedList: any[] = [];
          for (let i = 1; i < lines.length; i++) {
            // Regex parse csv line respecting quotes
            const row = lines[i].match(/(".*?"|[^",\s]+)(?=\s*,|\s*$)/g) || lines[i].split(',');
            if (row.length > 1) {
              const clean = (val?: string) => (val || '').trim().replace(/^["']|["']$/g, '');
              parsedList.push({
                question: clean(row[qIndex >= 0 ? qIndex : 0]),
                answer: clean(row[aIndex >= 0 ? aIndex : 1]),
                hint: clean(row[hIndex >= 0 ? hIndex : 2]),
                difficulty: clean(row[dIndex >= 0 ? dIndex : 3]) || 'intermediate',
                topic: clean(row[tIndex >= 0 ? tIndex : 4]) || 'Tổng quan',
                question_type: 'conceptual',
              });
            }
          }
          if (parsedList.length > 0) {
            setFileParsedQuestions(parsedList);
            setFileParsedCount(parsedList.length);
          } else {
            setErrorMessage('Không thể đọc cấu trúc câu hỏi từ tệp CSV này. Vui lòng kiểm tra mẫu.');
          }
        }
      } catch (err: any) {
        setErrorMessage(`Lỗi đọc tệp: ${err.message}`);
      }
    };
    reader.readAsText(file);
  };

  // Parse Text / Markdown
  const parseRawTextQuestions = (): any[] => {
    const blocks = rawText.split(/---+|\n\s*\n\s*\n/).filter((b) => b.trim().length > 0);
    const results: any[] = [];

    for (const block of blocks) {
      const lines = block.split('\n').map((l) => l.trim()).filter(Boolean);
      let q = '',
        a = '',
        h = '',
        d = 'intermediate',
        t = 'Tổng quan';

      for (const line of lines) {
        if (/^(câu hỏi|question|q):/i.test(line)) {
          q = line.replace(/^(câu hỏi|question|q):/i, '').trim();
        } else if (/^(đáp án|answer|a):/i.test(line)) {
          a = line.replace(/^(đáp án|answer|a):/i, '').trim();
        } else if (/^(gợi ý|hint|h):/i.test(line)) {
          h = line.replace(/^(gợi ý|hint|h):/i, '').trim();
        } else if (/^(độ khó|difficulty|d):/i.test(line)) {
          const val = line.replace(/^(độ khó|difficulty|d):/i, '').trim().toLowerCase();
          if (['beginner', 'intermediate', 'advanced'].includes(val)) d = val;
        } else if (/^(chủ đề|topic|t):/i.test(line)) {
          t = line.replace(/^(chủ đề|topic|t):/i, '').trim();
        } else if (!q) {
          q = line;
        } else if (!a) {
          a = line;
        }
      }

      if (q && a) {
        results.push({
          question: q,
          answer: a,
          hint: h || 'Hãy phân tích từ bản chất nguyên lý cốt lõi của bài học.',
          difficulty: d,
          topic: t,
          question_type: 'conceptual',
          status: defaultStatus,
        });
      }
    }

    return results;
  };

  // Submit Handler
  const handleExecuteImport = async () => {
    setIsSubmitting(true);
    setErrorMessage('');
    setSuccessMessage('');

    try {
      if (activeTab === 'ai') {
        // AI batch generation
        await onBatchGenerateAI(aiTopic, aiCount, aiDifficulty, aiDocId || undefined);
        setSuccessMessage(`Đã sinh thành công ${aiCount} câu hỏi Socratic và nạp vào Ngân Hàng!`);
        setTimeout(() => onClose(), 1200);
      } else if (activeTab === 'file') {
        if (fileParsedQuestions.length === 0) {
          throw new Error('Chưa có câu hỏi nào được trích xuất từ tệp đã chọn');
        }
        const formatted = fileParsedQuestions.map((item) => ({
          ...item,
          status: defaultStatus,
        }));
        await onBatchImport(formatted);
        setSuccessMessage(`Đã nạp thành công ${formatted.length} câu hỏi vào Ngân Hàng!`);
        setTimeout(() => onClose(), 1200);
      } else if (activeTab === 'text') {
        const parsed = parseRawTextQuestions();
        if (parsed.length === 0) {
          throw new Error('Không phân tích được câu hỏi nào từ văn bản. Vui lòng định dạng dạng "Câu hỏi: ... / Đáp án: ..."');
        }
        await onBatchImport(parsed);
        setSuccessMessage(`Đã nạp thành công ${parsed.length} câu hỏi vào Ngân Hàng!`);
        setTimeout(() => onClose(), 1200);
      }
    } catch (err: any) {
      setErrorMessage(err.message || 'Lỗi khi nạp ngân hàng câu hỏi');
    } finally {
      setIsSubmitting(false);
    }
  };

  const parsedTextCount = activeTab === 'text' ? parseRawTextQuestions().length : 0;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/70 backdrop-blur-xs animate-fadeIn">
      <div className="w-full max-w-3xl rounded-2xl bg-white dark:bg-[#070e1b] border border-slate-200 dark:border-slate-800 shadow-2xl p-6 space-y-5 max-h-[92vh] overflow-y-auto">
        {/* Header */}
        <div className="flex items-center justify-between pb-3 border-b border-slate-100 dark:border-slate-800">
          <div className="flex items-center gap-2.5">
            <div className="w-9 h-9 rounded-xl bg-indigo-50 dark:bg-indigo-950/60 border border-indigo-200 dark:border-indigo-800 flex items-center justify-center">
              <UploadCloud className="w-5 h-5 text-indigo-600 dark:text-indigo-400" />
            </div>
            <div>
              <h3 className="font-bold text-base text-slate-900 dark:text-white">
                Nạp Ngân Hàng Câu Hỏi Hàng Loạt (Question Bank Deposit)
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

        {/* Notifications */}
        {errorMessage && (
          <div className="p-3 rounded-xl bg-red-50 dark:bg-red-950/40 border border-red-200 dark:border-red-900 text-xs text-red-600 dark:text-red-400 flex items-center gap-2">
            <AlertCircle className="w-4 h-4 shrink-0" />
            <span>{errorMessage}</span>
          </div>
        )}

        {successMessage && (
          <div className="p-3 rounded-xl bg-emerald-50 dark:bg-emerald-950/40 border border-emerald-200 dark:border-emerald-900 text-xs text-emerald-600 dark:text-emerald-400 flex items-center gap-2">
            <CheckCircle2 className="w-4 h-4 shrink-0" />
            <span>{successMessage}</span>
          </div>
        )}

        {/* Tab Selection */}
        <div className="flex items-center gap-2 p-1 rounded-xl bg-slate-100 dark:bg-slate-800/80 text-xs">
          <button
            type="button"
            onClick={() => setActiveTab('ai')}
            className={`flex-1 py-2 rounded-lg font-semibold flex items-center justify-center gap-1.5 transition ${
              activeTab === 'ai'
                ? 'bg-white dark:bg-slate-700 text-indigo-600 dark:text-indigo-400 shadow-xs'
                : 'text-slate-600 dark:text-slate-400'
            }`}
          >
            <Sparkles className="w-4 h-4 text-indigo-500" />
            <span>Sinh Tự Động Bằng AI (Batch AI)</span>
          </button>

          <button
            type="button"
            onClick={() => setActiveTab('file')}
            className={`flex-1 py-2 rounded-lg font-semibold flex items-center justify-center gap-1.5 transition ${
              activeTab === 'file'
                ? 'bg-white dark:bg-slate-700 text-indigo-600 dark:text-indigo-400 shadow-xs'
                : 'text-slate-600 dark:text-slate-400'
            }`}
          >
            <FileSpreadsheet className="w-4 h-4 text-emerald-500" />
            <span>Nạp Qua Tệp (CSV / JSON)</span>
          </button>

          <button
            type="button"
            onClick={() => setActiveTab('text')}
            className={`flex-1 py-2 rounded-lg font-semibold flex items-center justify-center gap-1.5 transition ${
              activeTab === 'text'
                ? 'bg-white dark:bg-slate-700 text-indigo-600 dark:text-indigo-400 shadow-xs'
                : 'text-slate-600 dark:text-slate-400'
            }`}
          >
            <FileText className="w-4 h-4 text-blue-500" />
            <span>Dán Văn Bản Nhanh</span>
          </button>
        </div>

        {/* Tab 1: AI Batch Generator */}
        {activeTab === 'ai' && (
          <div className="space-y-4 p-4 rounded-2xl bg-indigo-50/40 dark:bg-indigo-950/20 border border-indigo-100 dark:border-indigo-900/50">
            <div className="flex items-center justify-between">
              <span className="text-xs font-bold text-indigo-900 dark:text-indigo-300 flex items-center gap-1.5">
                <Sparkles className="w-4 h-4 text-indigo-600" />
                Cấu hình sinh hàng loạt câu hỏi tư duy Socratic
              </span>
              <span className="text-[10px] text-slate-500 font-mono">Gemini AI Socratic Generator</span>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Chủ đề / Chuyên đề bài giảng:
                </label>
                <input
                  type="text"
                  value={aiTopic}
                  onChange={(e) => setAiTopic(e.target.value)}
                  placeholder="VD: Kiểm soát tắc nghẽn, Bắt tay TCP, Giao thức QUIC..."
                  className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-indigo-500"
                />
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Căn cứ trên tài liệu giáo trình (tùy chọn):
                </label>
                <select
                  aria-label="Căn cứ trên tài liệu giáo trình"
                  value={aiDocId}
                  onChange={(e) => setAiDocId(e.target.value)}
                  className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none"
                >
                  <option value="">Toàn bộ giáo trình môn học</option>
                  {courseDocuments.map((doc) => (
                    <option key={doc.id} value={doc.id}>
                      {doc.title} ({doc.chunks.length} chunks)
                    </option>
                  ))}
                </select>
              </div>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Số lượng câu hỏi muốn sinh:
                </label>
                <div className="flex gap-2">
                  {[3, 5, 8, 10].map((num) => (
                    <button
                      key={num}
                      type="button"
                      onClick={() => setAiCount(num)}
                      className={`flex-1 py-1.5 rounded-lg text-xs font-bold border transition ${
                        aiCount === num
                          ? 'border-indigo-600 bg-indigo-600 text-white shadow-xs'
                          : 'border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-700 dark:text-slate-300'
                      }`}
                    >
                      {num} câu
                    </button>
                  ))}
                </div>
              </div>

              <div>
                <label className="block text-xs font-semibold text-slate-700 dark:text-slate-300 mb-1">
                  Độ khó mong muốn:
                </label>
                <select
                  aria-label="Độ khó mong muốn"
                  value={aiDifficulty}
                  onChange={(e) => setAiDifficulty(e.target.value as any)}
                  className="w-full px-3 py-1.5 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-900 dark:text-white focus:outline-none"
                >
                  <option value="beginner">Cơ bản (Beginner) - Nhận thức nguyên lý</option>
                  <option value="intermediate">Trung bình (Intermediate) - Phân tích & Lập luận</option>
                  <option value="advanced">Nâng cao (Advanced) - Tư duy phản biện & Thiết kế hệ thống</option>
                </select>
              </div>
            </div>
          </div>
        )}

        {/* Tab 2: File Import (CSV / JSON) */}
        {activeTab === 'file' && (
          <div className="space-y-4">
            <div className="flex items-center justify-between">
              <span className="text-xs font-semibold text-slate-600 dark:text-slate-400">
                Chọn tệp .csv hoặc .json theo cấu trúc ngân hàng câu hỏi
              </span>
              <div className="flex gap-2">
                <button
                  type="button"
                  onClick={handleDownloadCsvTemplate}
                  className="px-2.5 py-1 rounded-lg border border-slate-200 dark:border-slate-700 hover:bg-slate-100 dark:hover:bg-slate-800 text-[11px] font-semibold text-slate-700 dark:text-slate-300 flex items-center gap-1 transition"
                >
                  <Download className="w-3.5 h-3.5 text-emerald-500" />
                  <span>Tải mẫu CSV</span>
                </button>
                <button
                  type="button"
                  onClick={handleDownloadJsonTemplate}
                  className="px-2.5 py-1 rounded-lg border border-slate-200 dark:border-slate-700 hover:bg-slate-100 dark:hover:bg-slate-800 text-[11px] font-semibold text-slate-700 dark:text-slate-300 flex items-center gap-1 transition"
                >
                  <Download className="w-3.5 h-3.5 text-blue-500" />
                  <span>Tải mẫu JSON</span>
                </button>
              </div>
            </div>

            <div
              onClick={() => fileInputRef.current?.click()}
              className={`p-6 rounded-2xl border-2 border-dashed text-center cursor-pointer transition ${
                selectedFileName
                  ? 'border-emerald-400 bg-emerald-50/30 dark:bg-emerald-950/20'
                  : 'border-slate-300 dark:border-slate-700 hover:border-indigo-400 bg-slate-50/50 dark:bg-slate-900/30'
              }`}
            >
              <input
                ref={fileInputRef}
                type="file"
                accept=".csv,.json"
                className="hidden"
                onChange={(e) => {
                  if (e.target.files && e.target.files[0]) {
                    handleFileChange(e.target.files[0]);
                  }
                }}
              />
              <div className="flex flex-col items-center justify-center gap-2">
                <div className="w-10 h-10 rounded-xl bg-emerald-100 dark:bg-emerald-950/60 flex items-center justify-center text-emerald-600 dark:text-emerald-400">
                  <FileSpreadsheet className="w-5 h-5" />
                </div>
                {selectedFileName ? (
                  <div>
                    <p className="text-xs font-bold text-emerald-600 dark:text-emerald-400 flex items-center justify-center gap-1">
                      <CheckCircle2 className="w-4 h-4" />
                      Đã đọc tệp: {selectedFileName}
                    </p>
                    <p className="text-[11px] text-slate-500 mt-0.5 font-semibold">
                      Trích xuất thành công: {fileParsedCount} câu hỏi
                    </p>
                  </div>
                ) : (
                  <div>
                    <p className="text-xs font-bold text-slate-800 dark:text-slate-200">
                      Bấm để tải lên hoặc kéo thả tệp CSV / JSON vào đây
                    </p>
                    <p className="text-[11px] text-slate-400 mt-0.5">
                      Cột yêu cầu: question, answer, hint, difficulty, topic
                    </p>
                  </div>
                )}
              </div>
            </div>

            {/* Preview of file items */}
            {fileParsedQuestions.length > 0 && (
              <div className="space-y-2">
                <span className="text-xs font-bold text-slate-700 dark:text-slate-300">
                  Xem trước {Math.min(fileParsedQuestions.length, 3)} câu hỏi mẫu từ tệp:
                </span>
                <div className="space-y-2 max-h-40 overflow-y-auto pr-1">
                  {fileParsedQuestions.slice(0, 3).map((q, idx) => (
                    <div
                      key={idx}
                      className="p-2.5 rounded-xl bg-slate-50 dark:bg-slate-900 border border-slate-200 dark:border-slate-800 text-xs space-y-1"
                    >
                      <div className="flex items-center justify-between font-semibold text-slate-800 dark:text-slate-200">
                        <span>Câu #{idx + 1}: {q.question}</span>
                        <span className="px-2 py-0.5 rounded-full text-[10px] bg-slate-200 dark:bg-slate-800 font-mono">
                          {q.difficulty}
                        </span>
                      </div>
                      <p className="text-[11px] text-slate-500 line-clamp-1">Gợi ý: {q.hint}</p>
                    </div>
                  ))}
                </div>
              </div>
            )}
          </div>
        )}

        {/* Tab 3: Raw Text Paste */}
        {activeTab === 'text' && (
          <div className="space-y-3">
            <div className="flex items-center justify-between">
              <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">
                Dán danh sách câu hỏi (Phân cách giữa các câu bằng dấu gạch ngang <code className="text-blue-600">---</code>):
              </label>
              <span className="text-[11px] font-bold text-indigo-600 dark:text-indigo-400 font-mono">
                Đã phát hiện: {parsedTextCount} câu hỏi
              </span>
            </div>
            <textarea
              rows={8}
              value={rawText}
              onChange={(e) => setRawText(e.target.value)}
              placeholder="Câu hỏi: ...&#10;Đáp án: ...&#10;Gợi ý: ...&#10;Độ khó: intermediate&#10;---&#10;Câu hỏi tiếp theo..."
              className="w-full px-3 py-2 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-white font-mono focus:outline-none focus:ring-2 focus:ring-indigo-500"
            />
          </div>
        )}

        {/* Status Option */}
        <div className="pt-2 border-t border-slate-100 dark:border-slate-800 flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          <div className="flex items-center gap-2">
            <span className="text-xs font-semibold text-slate-700 dark:text-slate-300">
              Trạng thái sau khi nạp:
            </span>
            <select
              aria-label="Trạng thái sau khi nạp"
              value={defaultStatus}
              onChange={(e) => setDefaultStatus(e.target.value as any)}
              className="px-2.5 py-1 text-xs rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-800 dark:text-slate-200"
            >
              <option value="approved">Duyệt chính thức ngay (Sinh viên thấy ngay)</option>
              <option value="review">Lưu chờ duyệt (Giảng viên rà soát lại trước)</option>
            </select>
          </div>

          <div className="flex items-center gap-2">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 rounded-xl border border-slate-200 dark:border-slate-700 text-xs font-semibold text-slate-600 dark:text-slate-400 hover:bg-slate-100 dark:hover:bg-slate-800 transition"
            >
              Hủy bỏ
            </button>
            <button
              type="button"
              onClick={handleExecuteImport}
              disabled={isSubmitting}
              className="px-5 py-2 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white text-xs font-bold flex items-center gap-1.5 shadow-md shadow-indigo-600/20 disabled:opacity-50 transition"
            >
              {isSubmitting ? (
                <>
                  <RefreshCw className="w-3.5 h-3.5 animate-spin" />
                  <span>Đang Xử Lý & Nạp...</span>
                </>
              ) : (
                <>
                  <UploadCloud className="w-3.5 h-3.5" />
                  <span>
                    {activeTab === 'ai'
                      ? `Sinh & Nạp ${aiCount} Câu Hỏi`
                      : activeTab === 'file'
                      ? `Nạp ${fileParsedCount || 0} Câu Hỏi Từ Tệp`
                      : `Nạp ${parsedTextCount} Câu Hỏi`}
                  </span>
                </>
              )}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
