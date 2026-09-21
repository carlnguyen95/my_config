import { ChatMessage } from './types';

export type MentorChatPayload = {
  reply?: ChatMessage;
  answer?: string;
  message_id?: string | number;
  model?: string;
  policy_applied?: ChatMessage['policy_applied'];
  tool_calls?: ChatMessage['tool_calls'];
  rag_sources?: ChatMessage['rag_sources'];
  assessment?: { id?: string };
};

export type MentorReplyContext = {
  userId: string;
  courseId: string;
  now: () => number;
  createdAt: () => string;
};

/**
 * Normalizes the temporary Express `reply` payload and the Drogon `answer`
 * payload into the message shape rendered by the mentor chat.
 */
export function toMentorReply(
  payload: MentorChatPayload,
  context: MentorReplyContext,
): ChatMessage | undefined {
  if (payload.reply) return payload.reply;
  if (!payload.answer) return undefined;

  return {
    id: String(payload.message_id || `assistant_${context.now()}`),
    session_id: 'default',
    user_id: context.userId,
    course_id: context.courseId,
    role: 'assistant',
    content: payload.answer,
    model: payload.model,
    created_at: context.createdAt(),
    policy_applied: payload.policy_applied,
    tool_calls: payload.tool_calls,
    rag_sources: payload.rag_sources,
    assessment_id: payload.assessment?.id,
  };
}
