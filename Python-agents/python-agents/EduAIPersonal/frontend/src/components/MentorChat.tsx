import React, { useState, useRef, useEffect } from 'react';
import {
  Send,
  Bot,
  User as UserIcon,
  Sparkles,
  Shield,
  FileText,
  Wrench,
  CheckCircle2,
  Award,
  ChevronDown,
  ChevronUp,
  RotateCcw,
  Lightbulb,
  Cpu,
} from 'lucide-react';
import { User, Course, ChatMessage, ThinkingAssessment, ToolCallLog } from '../types';

interface MentorChatProps {
  currentUser: User;
  selectedCourse: Course;
  messages: ChatMessage[];
  onSendMessage: (text: string) => Promise<ThinkingAssessment | null>;
  isSending: boolean;
  onSelectAssessment?: (assessment: ThinkingAssessment) => void;
  onClearSession?: () => void;
}

export const MentorChat: React.FC<MentorChatProps> = ({
  currentUser,
  selectedCourse,
  messages,
  onSendMessage,
  isSending,
  onSelectAssessment,
  onClearSession,
}) => {
  const [inputText, setInputText] = useState('');
  const [expandedTools, setExpandedTools] = useState<Record<string, boolean>>({});
  const messagesEndRef = useRef<HTMLDivElement>(null);

  const isStudent = currentUser.role === 'student';

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages, isSending]);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!inputText.trim() || isSending) return;
    const text = inputText;
    setInputText('');
    await onSendMessage(text);
  };

  const toggleToolExpand = (msgId: string) => {
    setExpandedTools((prev) => ({ ...prev, [msgId]: !prev[msgId] }));
  };

  const samplePrompts = [
    'Tại sao TCP cần cả Flow Control và Congestion Control? Chúng có thể gộp làm một không?',
    'Trong AIMD, vì sao tăng là tuyến tính (Additive) mà giảm lại là cấp số nhân (Multiplicative)?',
    'Kiểm tra tiến độ học tập hiện tại của tôi trong môn học này',
    'Tìm các câu hỏi thực hành liên quan đến cơ chế Slow Start trong ngân hàng',
  ];

  return (
    <div className="flex flex-col h-[calc(100vh-140px)] min-h-[550px] max-w-5xl mx-auto rounded-2xl bg-white dark:bg-[#081021] border border-slate-200/80 dark:border-slate-800 shadow-xl overflow-hidden">
      {/* Top Chat Bar */}
      <div className="px-5 py-3.5 border-b border-slate-200 dark:border-slate-800/80 bg-slate-50/70 dark:bg-[#0b1429]/80 flex flex-wrap items-center justify-between gap-3">
        <div className="flex items-center gap-3">
          <div className="w-9 h-9 rounded-xl bg-blue-900 dark:bg-blue-600 text-white flex items-center justify-center shadow-sm">
            <Bot className="w-5 h-5" />
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h3 className="font-bold text-sm text-slate-900 dark:text-white">
                Trợ lý Mentor: {selectedCourse.code}
              </h3>
              <span className="px-2 py-0.5 rounded-full text-[10px] font-bold bg-blue-100 text-blue-800 dark:bg-blue-950 dark:text-blue-300">
                {currentUser.role === 'student' ? 'Học sinh' : 'Giảng viên'}
              </span>
            </div>
            <p className="text-xs text-slate-500 dark:text-slate-400 truncate max-w-sm sm:max-w-md">
              {selectedCourse.name}
            </p>
          </div>
        </div>

        {/* Policy Badge & Actions */}
        <div className="flex items-center gap-2">
          <div className="flex items-center gap-1.5 px-3 py-1 rounded-lg text-xs font-semibold bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 shadow-xs">
            <Shield className="w-3.5 h-3.5 text-blue-600 dark:text-blue-400" />
            <span>Chính sách:</span>
            <span className="text-blue-600 dark:text-blue-400 font-bold">
              {currentUser.role === 'teacher' ? 'FULL_ANSWER' : selectedCourse.answer_policy}
            </span>
          </div>

          {onClearSession && (
            <button
              onClick={onClearSession}
              className="p-1.5 rounded-lg border border-slate-200 dark:border-slate-700 text-slate-500 hover:text-slate-800 dark:hover:text-slate-200 hover:bg-slate-100 dark:hover:bg-slate-800 transition"
              title="Đặt lại phiên học"
            >
              <RotateCcw className="w-4 h-4" />
            </button>
          )}
        </div>
      </div>

      {/* Policy Notification Callout for Students */}
      {isStudent && selectedCourse.answer_policy === 'HINT_ONLY' && (
        <div className="px-5 py-2 bg-amber-50 dark:bg-amber-950/40 border-b border-amber-200/80 dark:border-amber-900/60 flex items-center justify-between text-xs text-amber-800 dark:text-amber-300">
          <div className="flex items-center gap-2">
            <Lightbulb className="w-4 h-4 text-amber-600 dark:text-amber-400 shrink-0" />
            <span>
              <strong>Chế độ Socratic bật:</strong> Mentor sẽ chỉ cung cấp gợi ý, câu hỏi định hướng và kiểm tra lập luận thay vì giải sẵn bài tập.
            </span>
          </div>
        </div>
      )}

      {/* Messages List Area */}
      <div className="flex-1 overflow-y-auto p-4 sm:p-6 space-y-5">
        {messages.map((msg) => {
          const isUser = msg.role === 'user';
          return (
            <div
              key={msg.id}
              className={`flex gap-3 max-w-3xl ${isUser ? 'ml-auto flex-row-reverse' : 'mr-auto'}`}
            >
              {/* Avatar */}
              <div
                className={`w-8 h-8 rounded-xl flex items-center justify-center shrink-0 text-white font-bold text-xs ${
                  isUser
                    ? 'bg-blue-600 ring-2 ring-blue-500/20'
                    : 'bg-[#0f172a] dark:bg-blue-800 border border-slate-700'
                }`}
              >
                {isUser ? <UserIcon className="w-4 h-4" /> : <Bot className="w-4 h-4 text-blue-300" />}
              </div>

              {/* Bubble */}
              <div className={`space-y-2 max-w-[85%] ${isUser ? 'items-end' : 'items-start'}`}>
                <div
                  className={`p-4 rounded-2xl text-sm leading-relaxed ${
                    isUser
                      ? 'bg-blue-900 text-white rounded-tr-none shadow-sm'
                      : 'bg-slate-50 dark:bg-[#0e172e] text-slate-800 dark:text-slate-100 border border-slate-200/80 dark:border-slate-800/80 rounded-tl-none shadow-xs'
                  }`}
                >
                  {/* Content with simple formatted paragraphs */}
                  <div className="whitespace-pre-wrap font-normal">
                    {msg.content}
                  </div>

                  {/* RAG Source Citations */}
                  {msg.rag_sources && msg.rag_sources.length > 0 && (
                    <div className="mt-3 pt-2.5 border-t border-slate-200 dark:border-slate-700/80 flex flex-wrap gap-1.5">
                      <span className="text-[11px] font-semibold text-slate-500 dark:text-slate-400 flex items-center gap-1">
                        <FileText className="w-3 h-3 text-blue-500" /> Nguồn RAG:
                      </span>
                      {msg.rag_sources.map((src, i) => (
                        <span
                          key={i}
                          className="px-2 py-0.5 rounded-md text-[10px] font-medium bg-blue-50 text-blue-800 dark:bg-blue-950/70 dark:text-blue-300 border border-blue-200/60 dark:border-blue-800/60"
                        >
                          {src}
                        </span>
                      ))}
                    </div>
                  )}

                  {/* Backend Tool Calls Log Dropdown */}
                  {msg.tool_calls && msg.tool_calls.length > 0 && (
                    <div className="mt-2 pt-2 border-t border-slate-200 dark:border-slate-700/80">
                      <button
                        onClick={() => toggleToolExpand(msg.id)}
                        className="flex items-center gap-1.5 text-[11px] font-semibold text-indigo-600 dark:text-indigo-400 hover:underline"
                      >
                        <Wrench className="w-3 h-3" />
                        <span>Đã kích hoạt {msg.tool_calls.length} công cụ hệ thống</span>
                        {expandedTools[msg.id] ? (
                          <ChevronUp className="w-3 h-3" />
                        ) : (
                          <ChevronDown className="w-3 h-3" />
                        )}
                      </button>

                      {expandedTools[msg.id] && (
                        <div className="mt-2 space-y-1.5 text-[11px]">
                          {msg.tool_calls.map((tc) => (
                            <div
                              key={tc.id}
                              className="p-2 rounded-lg bg-slate-100 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 font-mono text-[10px]"
                            >
                              <div className="flex items-center justify-between text-indigo-700 dark:text-indigo-300 font-bold mb-1">
                                <span>Tool: {tc.tool_name}</span>
                                <span className="text-emerald-500 flex items-center gap-1">
                                  <CheckCircle2 className="w-3 h-3" /> {tc.status}
                                </span>
                              </div>
                              <div className="text-slate-600 dark:text-slate-300 truncate">
                                Args: {JSON.stringify(tc.arguments)}
                              </div>
                            </div>
                          ))}
                        </div>
                      )}
                    </div>
                  )}
                </div>

                {/* Footer metadata: Token count & Timestamp */}
                <div
                  className={`flex items-center gap-2 text-[10px] text-slate-600 dark:text-slate-300 ${
                    isUser ? 'justify-end' : 'justify-start'
                  }`}
                >
                  <span>{new Date(msg.created_at).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</span>
                  {msg.token_input && (
                    <span>• {msg.token_input + (msg.token_output || 0)} tokens</span>
                  )}
                  {msg.model && (
                    <span className="flex items-center gap-0.5 text-blue-700 dark:text-blue-300 font-medium">
                      <Cpu className="w-3 h-3" /> {msg.model}
                    </span>
                  )}
                </div>
              </div>
            </div>
          );
        })}

        {isSending && (
          <div className="flex gap-3 max-w-3xl mr-auto">
            <div className="w-8 h-8 rounded-xl bg-[#0f172a] dark:bg-blue-800 flex items-center justify-center text-white shrink-0">
              <Bot className="w-4 h-4 text-blue-300" />
            </div>
            <div className="p-4 rounded-2xl rounded-tl-none bg-slate-50 dark:bg-[#0e172e] border border-slate-200 dark:border-slate-800 text-sm flex items-center gap-2 text-slate-500 dark:text-slate-400">
              <span className="w-2 h-2 rounded-full bg-blue-600 animate-ping" />
              <span>Mentor đang phân tích câu hỏi & đối chiếu tài liệu RAG...</span>
            </div>
          </div>
        )}

        <div ref={messagesEndRef} />
      </div>

      {/* Suggestion Chips */}
      <div className="px-4 py-2 border-t border-slate-100 dark:border-slate-800/60 bg-slate-50/50 dark:bg-slate-900/30 overflow-x-auto no-scrollbar flex items-center gap-2">
        <span className="text-[11px] font-semibold text-slate-400 uppercase tracking-wider shrink-0 flex items-center gap-1">
          <Sparkles className="w-3 h-3 text-amber-500" /> Gợi ý:
        </span>
        {samplePrompts.map((prompt, idx) => (
          <button
            key={idx}
            onClick={() => setInputText(prompt)}
            className="text-xs px-3 py-1 rounded-full whitespace-nowrap bg-white dark:bg-slate-800 hover:bg-blue-50 dark:hover:bg-blue-950/60 hover:text-blue-600 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 transition"
          >
            {prompt}
          </button>
        ))}
      </div>

      {/* Input Form */}
      <form
        onSubmit={handleSubmit}
        className="p-3 sm:p-4 border-t border-slate-200 dark:border-slate-800 bg-white dark:bg-[#0b1329] flex items-center gap-2"
      >
        <input
          type="text"
          value={inputText}
          onChange={(e) => setInputText(e.target.value)}
          placeholder={
            isStudent
              ? `Hỏi Mentor môn ${selectedCourse.code} (Socratic mode: Đặt câu hỏi vì sao/như thế nào)...`
              : `Nhập câu hỏi hoặc chỉ đạo học thuật cho AI...`
          }
          className="flex-1 px-4 py-3 rounded-xl text-sm bg-slate-50 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 text-slate-900 dark:text-white placeholder-slate-400 focus:outline-hidden focus:ring-2 focus:ring-blue-600 dark:focus:ring-blue-500 transition"
          disabled={isSending}
        />
        <button
          type="submit"
          disabled={!inputText.trim() || isSending}
          className="px-4 py-3 rounded-xl bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white font-semibold text-sm shadow-md disabled:opacity-50 disabled:cursor-not-allowed transition flex items-center gap-1.5"
        >
          <Send className="w-4 h-4" />
          <span className="hidden sm:inline">Gửi</span>
        </button>
      </form>
    </div>
  );
};
