export type UserRole = 'student' | 'teacher' | 'admin';

export interface User {
  id: string;
  name: string;
  email: string;
  role: UserRole;
  avatar?: string;
  bio?: string;
}

export type AnswerPolicy = 'FULL_ANSWER' | 'HINT_ONLY' | 'GUIDED';

export interface Subject {
  id: string;
  name: string;
  code: string;
  description: string;
}

export interface Course {
  id: string;
  subject_id: string;
  name: string;
  code: string;
  description: string;
  teacher_id: string;
  teacher_name: string;
  answer_policy: AnswerPolicy;
  enrolled_count: number;
  created_at: string;
  topics_count: number;
  documents_count: number;
  questions_count: number;
}

export interface DocumentChunk {
  id: string;
  document_id: string;
  chunk_index: number;
  content: string;
  tags: string[];
}

export interface CourseDocument {
  id: string;
  course_id: string;
  title: string;
  source_path: string;
  created_at: string;
  chunks: DocumentChunk[];
}

export type QuestionDifficulty = 'beginner' | 'intermediate' | 'advanced';
export type QuestionType = 'conceptual' | 'multiple_choice' | 'coding' | 'scenario';
export type QuestionSource = 'teacher' | 'ai' | 'imported';
export type QuestionStatus = 'draft' | 'review' | 'approved' | 'archived';

export interface Question {
  id: string;
  course_id: string;
  course_name?: string;
  created_by: string;
  question: string;
  answer: string;
  hint: string;
  difficulty: QuestionDifficulty;
  question_type: QuestionType;
  source: QuestionSource;
  status: QuestionStatus;
  created_at: string;
  updated_at?: string;
}

export interface ToolCallLog {
  id: string;
  tool_name: string;
  arguments: Record<string, any>;
  result: any;
  status: 'success' | 'error';
  timestamp: string;
}

export interface ChatMessage {
  id: string;
  session_id: string;
  user_id?: string;
  course_id?: string;
  role: 'user' | 'assistant' | 'system';
  content: string;
  token_input?: number;
  token_output?: number;
  model?: string;
  created_at: string;
  policy_applied?: AnswerPolicy;
  tool_calls?: ToolCallLog[];
  rag_sources?: string[];
  assessment_id?: string;
}

export interface CriticalThinkingDimensions {
  curiosity: number; // 0.00 to 1.00
  depth: number;
  assumption_awareness: number;
  causal_reasoning: number;
  alternative_perspectives: number;
  cross_domain_potential: number;
  challenge_level: number;
}

export interface ThinkingAssessment {
  id: string;
  user_id: string;
  student_name: string;
  course_id: string;
  course_name: string;
  message_id: string;
  prompt_excerpt: string;
  score: number; // 0.00 to 1.00
  dimensions: CriticalThinkingDimensions;
  reasoning: string;
  model_name: string;
  teacher_score?: number; // 0.00 to 1.00
  teacher_feedback?: string;
  review_status: 'pending' | 'reviewed';
  created_at: string;
}

export interface RoadmapTopic {
  id: string;
  name: string;
  description: string;
  status: 'completed' | 'learning' | 'pending' | 'needs_review';
  difficulty: 'easy' | 'medium' | 'hard';
  hours_est: number;
  prerequisite?: string;
}

export interface Roadmap {
  id: string;
  user_id: string;
  course_id: string;
  title: string;
  topics: RoadmapTopic[];
  updated_at: string;
}

export interface LearningProgressItem {
  id: string;
  user_id: string;
  course_id: string;
  topic: string;
  status: 'NOT_STARTED' | 'LEARNING' | 'COMPLETED' | 'NEEDS_REVIEW';
  progress_value: number; // 0 to 100
  updated_at: string;
}

export interface StudyNote {
  id: string;
  course_id: string;
  title: string;
  content: string;
  updated_at: string;
}

export interface StudyGoal {
  id: string;
  title: string;
  target_date: string;
  completed: boolean;
}

export interface PersonalizedData {
  notes: StudyNote[];
  saved_question_ids: string[];
  study_goals: StudyGoal[];
  theme: 'light' | 'dark';
  learning_focus?: string;
}

export interface AIProviderState {
  current_provider: 'gemini' | 'ollama';
  local_model: string;
  local_endpoint: string;
  cloud_model: string;
  cloud_fallback_enabled: boolean;
  is_local_online: boolean;
  total_tokens_used: number;
}

export interface StudentRosterItem {
  id: string;
  name: string;
  email: string;
  avatar?: string;
  progress_percent: number;
  average_thinking_score: number;
  questions_count: number;
  last_active: string;
  status_alert?: 'needs_help' | 'good' | 'excellent';
  attention_reason?: string;
}
