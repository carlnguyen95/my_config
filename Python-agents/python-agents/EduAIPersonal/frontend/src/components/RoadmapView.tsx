import React, { useState } from 'react';
import {
  Layers,
  CheckCircle2,
  Clock,
  CircleDot,
  AlertCircle,
  ArrowRight,
  BookOpen,
  Calendar,
  Sparkles,
  HelpCircle,
  TrendingUp,
} from 'lucide-react';
import { Roadmap, RoadmapTopic, Course, User } from '../types';

interface RoadmapViewProps {
  roadmap: Roadmap | null;
  selectedCourse: Course;
  currentUser: User;
  onUpdateTopicStatus: (topicId: string, status: RoadmapTopic['status']) => void;
  onAskTopicInChat: (topicName: string) => void;
}

export const RoadmapView: React.FC<RoadmapViewProps> = ({
  roadmap,
  selectedCourse,
  currentUser,
  onUpdateTopicStatus,
  onAskTopicInChat,
}) => {
  const [selectedTopic, setSelectedTopic] = useState<RoadmapTopic | null>(
    roadmap?.topics[0] || null
  );

  if (!roadmap) {
    return (
      <div className="p-8 text-center bg-white dark:bg-[#0b1329] rounded-2xl border border-slate-200 dark:border-slate-800">
        <Layers className="w-12 h-12 text-slate-400 mx-auto mb-3" />
        <h3 className="font-bold text-lg text-slate-800 dark:text-slate-200">
          Chưa có lộ trình cho khóa học này
        </h3>
        <p className="text-sm text-slate-500 mt-1">
          Hệ thống đang tự động khởi tạo lộ trình chuẩn mực dựa trên tài liệu môn học.
        </p>
      </div>
    );
  }

  const topics = roadmap.topics;
  const completedCount = topics.filter((t) => t.status === 'completed').length;
  const inProgressCount = topics.filter((t) => t.status === 'learning').length;
  const totalHours = topics.reduce((acc, t) => acc + t.hours_est, 0);
  const percent = Math.round((completedCount / topics.length) * 100);

  const statusConfig: Record<
    RoadmapTopic['status'],
    { label: string; bg: string; text: string; icon: any }
  > = {
    completed: {
      label: 'Đã hoàn thành',
      bg: 'bg-emerald-50 text-emerald-700 dark:bg-emerald-950/60 dark:text-emerald-300 border-emerald-200 dark:border-emerald-800',
      text: 'text-emerald-600',
      icon: CheckCircle2,
    },
    learning: {
      label: 'Đang học',
      bg: 'bg-blue-50 text-blue-700 dark:bg-blue-950/60 dark:text-blue-300 border-blue-200 dark:border-blue-800',
      text: 'text-blue-600',
      icon: Clock,
    },
    pending: {
      label: 'Chưa học',
      bg: 'bg-slate-100 text-slate-600 dark:bg-slate-800 dark:text-slate-400 border-slate-200 dark:border-slate-700',
      text: 'text-slate-400',
      icon: CircleDot,
    },
    needs_review: {
      label: 'Cần ôn tập',
      bg: 'bg-rose-50 text-rose-700 dark:bg-rose-950/60 dark:text-rose-300 border-rose-200 dark:border-rose-800',
      text: 'text-rose-600',
      icon: AlertCircle,
    },
  };

  const difficultyConfig = {
    easy: { label: 'Cơ bản', color: 'text-emerald-600 bg-emerald-50 dark:bg-emerald-950/40' },
    medium: { label: 'Trung bình', color: 'text-amber-600 bg-amber-50 dark:bg-amber-950/40' },
    hard: { label: 'Nâng cao', color: 'text-rose-600 bg-rose-50 dark:bg-rose-950/40' },
  };

  return (
    <div className="space-y-6">
      {/* Header card with progress statistics */}
      <div className="p-6 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm flex flex-col md:flex-row md:items-center justify-between gap-6">
        <div>
          <div className="flex items-center gap-2 mb-1">
            <span className="px-2.5 py-0.5 rounded-md text-xs font-bold bg-blue-100 dark:bg-blue-950 text-blue-800 dark:text-blue-300">
              {selectedCourse.code}
            </span>
            <span className="text-xs text-slate-500 dark:text-slate-400">
              Lộ trình học tập chuẩn hóa
            </span>
          </div>
          <h2 className="text-xl font-extrabold text-slate-900 dark:text-white">
            {roadmap.title}
          </h2>
          <p className="text-xs text-slate-500 dark:text-slate-400 mt-1">
            Tổng cộng {topics.length} chủ đề chính • Ước tính {totalHours} giờ học lý thuyết & bài tập
          </p>
        </div>

        <div className="flex items-center gap-6 shrink-0">
          <div className="text-right">
            <span className="text-2xl font-black text-slate-900 dark:text-white">
              {percent}%
            </span>
            <p className="text-xs text-slate-500 dark:text-slate-400">
              {completedCount}/{topics.length} chủ đề
            </p>
          </div>
          <div className="w-24 h-2.5 rounded-full bg-slate-100 dark:bg-slate-800 overflow-hidden">
            <div
              className="h-full bg-blue-600 rounded-full transition-all duration-500"
              style={{ width: `${percent}%` }}
            />
          </div>
        </div>
      </div>

      {/* Main Roadmap Split: Timeline on Left, Details & Actions on Right */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Topic Timeline Nodes */}
        <div className="lg:col-span-2 space-y-3">
          {topics.map((topic, index) => {
            const StatusIcon = statusConfig[topic.status].icon;
            const isSelected = selectedTopic?.id === topic.id;

            return (
              <div
                key={topic.id}
                onClick={() => setSelectedTopic(topic)}
                className={`p-4 rounded-xl border transition-all duration-150 cursor-pointer flex items-start gap-4 ${
                  isSelected
                    ? 'bg-blue-50/60 dark:bg-blue-950/40 border-blue-500 dark:border-blue-500 ring-2 ring-blue-500/20 shadow-md'
                    : 'bg-white dark:bg-[#0b1329] border-slate-200/80 dark:border-slate-800 hover:border-slate-300 dark:hover:border-slate-700'
                }`}
              >
                {/* Node Number & Status Icon */}
                <div className="flex flex-col items-center">
                  <div
                    className={`w-9 h-9 rounded-xl flex items-center justify-center font-bold text-xs ${
                      topic.status === 'completed'
                        ? 'bg-emerald-500 text-white'
                        : topic.status === 'learning'
                        ? 'bg-blue-600 text-white'
                        : 'bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400'
                    }`}
                  >
                    <StatusIcon className="w-4 h-4" />
                  </div>
                  {index < topics.length - 1 && (
                    <div className="w-0.5 h-6 bg-slate-200 dark:bg-slate-800 my-1" />
                  )}
                </div>

                {/* Topic Info */}
                <div className="flex-1 min-w-0">
                  <div className="flex items-center justify-between gap-2">
                    <h3 className="font-bold text-sm text-slate-900 dark:text-white truncate">
                      {index + 1}. {topic.name}
                    </h3>
                    <div className="flex items-center gap-1.5 shrink-0">
                      <span
                        className={`text-[10px] font-bold px-2 py-0.5 rounded-full border ${statusConfig[topic.status].bg}`}
                      >
                        {statusConfig[topic.status].label}
                      </span>
                      <span
                        className={`text-[10px] font-semibold px-2 py-0.5 rounded-md ${difficultyConfig[topic.difficulty].color}`}
                      >
                        {difficultyConfig[topic.difficulty].label}
                      </span>
                    </div>
                  </div>

                  <p className="mt-1 text-xs text-slate-600 dark:text-slate-300 line-clamp-2">
                    {topic.description}
                  </p>

                  <div className="mt-2 text-[11px] text-slate-600 dark:text-slate-300 flex items-center gap-3">
                    <span className="flex items-center gap-1">
                      <Clock className="w-3 h-3 text-slate-500 dark:text-slate-400" /> ~{topic.hours_est} giờ
                    </span>
                    {topic.prerequisite && (
                      <span>Tiền đề: Chủ đề #{topics.findIndex((t) => t.id === topic.prerequisite) + 1}</span>
                    )}
                  </div>
                </div>
              </div>
            );
          })}
        </div>

        {/* Selected Topic Inspector & Actions */}
        <div className="space-y-4">
          {selectedTopic ? (
            <div className="p-5 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200/80 dark:border-slate-800 shadow-sm space-y-4">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-blue-600 dark:text-blue-400 uppercase tracking-wider">
                  Chi tiết chủ đề
                </span>
                <span
                  className={`text-[10px] font-bold px-2 py-0.5 rounded-md ${difficultyConfig[selectedTopic.difficulty].color}`}
                >
                  Độ khó: {difficultyConfig[selectedTopic.difficulty].label}
                </span>
              </div>

              <h3 className="text-base font-bold text-slate-900 dark:text-white">
                {selectedTopic.name}
              </h3>

              <p className="text-xs text-slate-600 dark:text-slate-300 leading-relaxed">
                {selectedTopic.description}
              </p>

              <div className="p-3 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-100 dark:border-slate-800 text-xs space-y-2">
                <div className="flex justify-between">
                  <span className="text-slate-500 dark:text-slate-400">Thời lượng ước tính:</span>
                  <span className="font-semibold text-slate-800 dark:text-slate-200">
                    {selectedTopic.hours_est} giờ học
                  </span>
                </div>
                <div className="flex justify-between">
                  <span className="text-slate-500 dark:text-slate-400">Trạng thái hiện tại:</span>
                  <span className="font-semibold capitalize text-blue-600 dark:text-blue-400">
                    {statusConfig[selectedTopic.status].label}
                  </span>
                </div>
              </div>

              {/* Status Updater Buttons */}
              <div className="space-y-2">
                <span className="text-xs font-semibold text-slate-700 dark:text-slate-300 block">
                  Cập nhật tiến độ của bạn:
                </span>
                <div className="grid grid-cols-2 gap-2">
                  <button
                    onClick={() => onUpdateTopicStatus(selectedTopic.id, 'learning')}
                    className={`px-3 py-2 rounded-lg text-xs font-semibold border transition ${
                      selectedTopic.status === 'learning'
                        ? 'bg-blue-600 text-white border-blue-600 shadow-xs'
                        : 'border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 hover:bg-slate-50 dark:hover:bg-slate-800'
                    }`}
                  >
                    Đang học
                  </button>
                  <button
                    onClick={() => onUpdateTopicStatus(selectedTopic.id, 'completed')}
                    className={`px-3 py-2 rounded-lg text-xs font-semibold border transition ${
                      selectedTopic.status === 'completed'
                        ? 'bg-emerald-600 text-white border-emerald-600 shadow-xs'
                        : 'border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 hover:bg-slate-50 dark:hover:bg-slate-800'
                    }`}
                  >
                    Hoàn thành
                  </button>
                </div>
              </div>

              {/* Ask Mentor about this topic button */}
              <button
                onClick={() => onAskTopicInChat(selectedTopic.name)}
                className="w-full flex items-center justify-center gap-2 px-4 py-2.5 rounded-xl font-semibold text-xs bg-blue-900 hover:bg-blue-800 dark:bg-blue-600 dark:hover:bg-blue-500 text-white shadow-md transition"
              >
                <Sparkles className="w-3.5 h-3.5" />
                <span>Hỏi Mentor về chủ đề này</span>
              </button>
            </div>
          ) : (
            <div className="p-6 rounded-2xl bg-white dark:bg-[#0b1329] border border-slate-200 dark:border-slate-800 text-center text-xs text-slate-500">
              Chọn một chủ đề để xem chi tiết và cập nhật tiến độ
            </div>
          )}
        </div>
      </div>
    </div>
  );
};
