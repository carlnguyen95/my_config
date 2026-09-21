import React, { useState } from 'react';
import {
  GraduationCap,
  Moon,
  Sun,
  Bot,
  UserCheck,
  ShieldCheck,
  BookOpen,
  Cpu,
  Sparkles,
  LogIn,
  LogOut,
  LayoutDashboard,
  Compass,
  ChevronDown,
} from 'lucide-react';
import { User, AIProviderState, Course } from '../types';

interface HeaderProps {
  currentUser: User | null;
  isDark: boolean;
  onToggleTheme: () => void;
  activeTab: string;
  onChangeTab: (tab: string) => void;
  providerState: AIProviderState | null;
  selectedCourse: Course | null;
  onOpenLoginModal?: () => void;
  onLogout?: () => void;
}

// Helper to format First Name + Last Name (omit middle name and academic titles)
const getFirstLastName = (fullName: string): string => {
  if (!fullName) return '';
  const clean = fullName
    .replace(/^(TS\.|ThS\.|PGS\.|GS\.|Quản trị viên|Giảng viên|Sinh viên|Thầy|Cô|Admin|GV|SV)\s*/i, '')
    .trim();
  const parts = clean.split(/\s+/).filter(Boolean);
  if (parts.length === 0) return fullName;
  if (parts.length === 1) return parts[0];
  return `${parts[0]} ${parts[parts.length - 1]}`;
};

export const Header: React.FC<HeaderProps> = ({
  currentUser,
  isDark,
  onToggleTheme,
  activeTab,
  onChangeTab,
  providerState,
  selectedCourse,
  onOpenLoginModal,
  onLogout,
}) => {
  const [isAccountMenuOpen, setIsAccountMenuOpen] = useState(false);
  const isTeacher = currentUser?.role === 'teacher';

  const navItems = isTeacher
    ? [
        { id: 'teacher_portal', label: 'Cổng Giảng Viên', icon: ShieldCheck },
        { id: 'dashboard', label: 'Bảng điều khiển', icon: LayoutDashboard },
        { id: 'chat', label: 'Trợ lý Mentor', icon: Bot },
        { id: 'roadmap', label: 'Lộ trình học', icon: Compass },
        { id: 'questions', label: 'Ngân hàng câu hỏi', icon: BookOpen },
        { id: 'personal', label: 'Dữ liệu cá nhân', icon: UserCheck },
      ]
    : [
        { id: 'dashboard', label: 'Bảng điều khiển', icon: LayoutDashboard },
        { id: 'chat', label: 'Trợ lý Mentor', icon: Bot },
        { id: 'roadmap', label: 'Lộ trình học', icon: Compass },
        { id: 'questions', label: 'Ngân hàng câu hỏi', icon: BookOpen },
        { id: 'assessment', label: 'Đánh giá tư duy', icon: Sparkles },
        { id: 'personal', label: 'Dữ liệu cá nhân', icon: UserCheck },
      ];

  return (
    <header className="sticky top-0 z-40 w-full border-b border-slate-200/80 bg-white/95 backdrop-blur-md dark:border-slate-800/80 dark:bg-[#070e1b]/95 transition-colors">
      <div className="w-full max-w-[1536px] mx-auto px-2 sm:px-4 lg:px-6">
        <div className="flex items-center justify-between h-14 sm:h-15 gap-2 sm:gap-3">
          
          {/* 1. Left Brand & Logo (Ultra-compact, never wraps) */}
          <div className="flex items-center gap-2 shrink-0">
            <div className="w-8 h-8 sm:w-9 sm:h-9 rounded-xl bg-gradient-to-tr from-blue-900 via-blue-800 to-blue-600 flex items-center justify-center text-white shadow-sm ring-1 ring-white/20 shrink-0">
              <GraduationCap className="w-4 h-4 sm:w-5 sm:h-5" />
            </div>
            <div className="flex items-center gap-1.5">
              <span className="font-extrabold text-base sm:text-lg tracking-tight text-slate-900 dark:text-white whitespace-nowrap">
                Edu<span className="text-blue-600 dark:text-blue-400">AI</span>
              </span>
              <span className="hidden xl:inline-block px-1.5 py-0.5 text-[9px] font-bold tracking-wider uppercase rounded-md bg-blue-50 text-blue-700 dark:bg-blue-950/70 dark:text-blue-300 border border-blue-200/60 dark:border-blue-800/60 whitespace-nowrap">
                MVP
              </span>
            </div>
          </div>

          {/* 2. Middle Navigation (Strictly on 1 row, fluid horizontal scrolling without scrollbar) */}
          <nav className="flex-1 min-w-0 flex items-center justify-start md:justify-center overflow-x-auto no-scrollbar py-1 px-1">
            <div className="flex items-center gap-1 sm:gap-1.5 whitespace-nowrap">
              {navItems.map((item) => {
                const isActive = activeTab === item.id;
                const IconComponent = item.icon;
                return (
                  <button
                    key={item.id}
                    onClick={() => onChangeTab(item.id)}
                    className={`inline-flex items-center gap-1.5 px-2.5 py-1.5 sm:px-3 sm:py-1.5 rounded-lg text-xs font-semibold whitespace-nowrap shrink-0 transition-all duration-150 ${
                      isActive
                        ? 'bg-blue-600 text-white shadow-sm shadow-blue-500/20 dark:bg-blue-600 dark:text-white'
                        : 'text-slate-600 hover:text-slate-900 hover:bg-slate-100 dark:text-slate-300 dark:hover:text-white dark:hover:bg-slate-800/70'
                    }`}
                  >
                    <IconComponent className={`w-3.5 h-3.5 shrink-0 ${isActive ? 'text-white' : 'text-slate-400 dark:text-slate-400'}`} />
                    <span>{item.label}</span>
                  </button>
                );
              })}
            </div>
          </nav>

          {/* 3. Right Action Tools (Compact, shrink-0, perfectly aligned on the same row) */}
          <div className="flex items-center gap-1.5 sm:gap-2 shrink-0">
            {/* AI Provider Status Pill */}
            <div
              className="hidden 2xl:flex items-center gap-1.5 px-2.5 py-1 rounded-full text-[11px] font-medium bg-slate-100 dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 shrink-0"
              title={
                providerState?.current_provider === 'gemini'
                  ? 'Đang kết nối Gemini 3.8 Flash Cloud'
                  : 'Đang dùng Local Ollama Model'
              }
            >
              <Cpu className="w-3.5 h-3.5 text-blue-600 dark:text-blue-400" />
              <span className="capitalize">{providerState?.current_provider || 'Gemini'}</span>
              <span className="w-1.5 h-1.5 rounded-full bg-emerald-500 animate-pulse" />
            </div>

            {/* A login CTA is shown only when there is no stored session. */}
            {!currentUser && onOpenLoginModal && (
              <button
                onClick={onOpenLoginModal}
                className="hidden sm:inline-flex items-center gap-1 px-2.5 py-1.5 rounded-lg border border-blue-200 dark:border-blue-800/80 bg-blue-50/80 hover:bg-blue-100 dark:bg-blue-950/60 dark:hover:bg-blue-900/60 text-blue-700 dark:text-blue-300 text-xs font-bold transition shadow-xs shrink-0 whitespace-nowrap"
                title="Đăng nhập"
              >
                <LogIn className="w-3.5 h-3.5 text-blue-600 dark:text-blue-400 shrink-0" />
                <span>Đăng nhập</span>
              </button>
            )}

            {/* Session account menu. It deliberately shows only the active
                account; account selection remains inside the login flow. */}
            {currentUser && (
            <div className="relative shrink-0">
              <button
                onClick={() => setIsAccountMenuOpen((open) => !open)}
                className="flex items-center gap-1.5 p-1 sm:pl-1.5 sm:pr-2.5 sm:py-1 rounded-xl border border-slate-200 dark:border-slate-700/80 bg-slate-50 dark:bg-slate-800/80 hover:border-blue-300 dark:hover:border-blue-600 transition shrink-0 cursor-pointer"
                aria-label={`Tài khoản: ${currentUser.name}`}
                aria-expanded={isAccountMenuOpen}
                aria-haspopup="menu"
              >
                <img
                  src={currentUser.avatar}
                  alt={currentUser.name}
                  className="w-6 h-6 rounded-full object-cover ring-1 ring-blue-500/30 shrink-0"
                />
                <div className="text-left max-w-[85px] sm:max-w-[130px] truncate">
                  <p className="text-xs font-semibold leading-none text-slate-800 dark:text-slate-200 truncate">
                    {getFirstLastName(currentUser.name)}
                  </p>
                </div>
                <ChevronDown className="w-3 h-3 text-slate-400 shrink-0" />
              </button>

              {isAccountMenuOpen && (
              <div
                className="absolute right-0 mt-1.5 w-64 p-1.5 rounded-2xl bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-700 shadow-2xl z-50"
                role="menu"
              >
                <div className="px-2.5 py-2.5 flex items-center gap-2.5 border-b border-slate-100 dark:border-slate-800 mb-1">
                  <img src={currentUser.avatar} alt="" className="w-8 h-8 rounded-full object-cover shrink-0" />
                  <div className="min-w-0">
                    <p className="truncate text-xs font-bold text-slate-800 dark:text-slate-200">{currentUser.name}</p>
                    <p className="truncate text-[10px] text-slate-400 dark:text-slate-500">{currentUser.email}</p>
                  </div>
                </div>
                {onLogout && (
                    <button
                      onClick={() => {
                        setIsAccountMenuOpen(false);
                        onLogout();
                      }}
                      className="w-full flex items-center gap-2 px-2.5 py-1.5 rounded-lg text-xs font-semibold text-red-600 dark:text-red-400 hover:bg-red-50 dark:hover:bg-red-950/40 transition"
                      role="menuitem"
                    >
                      <LogOut className="w-3.5 h-3.5" />
                      <span>Đăng xuất tài khoản</span>
                    </button>
                )}
              </div>
              )}
            </div>
            )}

            {/* Theme Toggle Button */}
            <button
              onClick={onToggleTheme}
              className="p-1.5 sm:p-2 rounded-xl border border-slate-200 dark:border-slate-700 text-slate-600 hover:text-slate-900 bg-slate-50 hover:bg-slate-100 dark:text-slate-300 dark:bg-slate-800/80 dark:hover:bg-slate-800 transition shrink-0"
              title={isDark ? 'Chuyển sang chế độ sáng' : 'Chuyển sang chế độ tối'}
              aria-label="Toggle theme"
            >
              {isDark ? <Sun className="w-4 h-4 text-amber-400" /> : <Moon className="w-4 h-4 text-blue-900" />}
            </button>
          </div>

        </div>
      </div>
    </header>
  );
};
