import assert from 'node:assert/strict';
import test from 'node:test';
import React from 'react';
import { renderToStaticMarkup } from 'react-dom/server';

import { ApiError, apiFetch, apiUrl, buildApiUrl, readApiJson, roadmapTopicStatusPath } from '../src/api';
import { clearSession, hasStoredSession, saveSession, SESSION_TOKEN_KEY, SESSION_USER_ID_KEY } from '../src/authSession';
import { toMentorReply } from '../src/chatResponse';
import { Header } from '../src/components/Header';
import { User } from '../src/types';

const storage = new Map<string, string>();

Object.defineProperty(globalThis, 'window', {
  value: {
    localStorage: {
      getItem: (key: string) => storage.get(key) ?? null,
      setItem: (key: string, value: string) => storage.set(key, value),
      removeItem: (key: string) => storage.delete(key),
    },
  },
  configurable: true,
});

const signedInUser: User = {
  id: 'usr_teacher_1',
  name: 'Trần Văn Minh',
  email: 'teacher@example.test',
  role: 'teacher',
  avatar: 'https://example.test/avatar.png',
};

function renderHeader(currentUser: User | null): string {
  return renderToStaticMarkup(React.createElement(Header, {
    currentUser,
    isDark: false,
    onToggleTheme: () => {},
    activeTab: 'dashboard',
    onChangeTab: () => {},
    providerState: null,
    selectedCourse: null,
    onOpenLoginModal: () => {},
    onLogout: () => {},
  }));
}

test('apiUrl keeps relative calls by default', () => {
  assert.equal(apiUrl('/api/courses'), '/api/courses');
  assert.equal(buildApiUrl('https://backend.example/', '/api/courses'), 'https://backend.example/api/courses');
  assert.throws(() => apiUrl('api/courses'), /must start/);
});

test('apiFetch attaches the stored bearer token without replacing explicit headers', async () => {
  storage.set('edu_ai_token', 'test-token');
  const originalFetch = globalThis.fetch;
  let requestUrl: string | undefined;
  let requestHeaders: Headers | undefined;
  globalThis.fetch = async (input, init) => {
    requestUrl = String(input);
    requestHeaders = new Headers(init?.headers);
    return new Response('{}', { headers: { 'content-type': 'application/json' } });
  };

  try {
    await apiFetch('/api/auth/me', { headers: { 'X-Trace-Id': 'contract-test' } });
  } finally {
    globalThis.fetch = originalFetch;
    storage.delete('edu_ai_token');
  }

  assert.equal(requestUrl, '/api/auth/me');
  assert.equal(requestHeaders?.get('authorization'), 'Bearer test-token');
  assert.equal(requestHeaders?.get('x-trace-id'), 'contract-test');
});

test('apiFetch preserves an explicit authorization header', async () => {
  storage.set('edu_ai_token', 'test-token');
  const originalFetch = globalThis.fetch;
  let requestHeaders: Headers | undefined;
  globalThis.fetch = async (_input, init) => {
    requestHeaders = new Headers(init?.headers);
    return new Response('{}', { headers: { 'content-type': 'application/json' } });
  };

  try {
    await apiFetch('/api/auth/me', { headers: { Authorization: 'Bearer caller-token' } });
  } finally {
    globalThis.fetch = originalFetch;
    storage.delete('edu_ai_token');
  }

  assert.equal(requestHeaders?.get('authorization'), 'Bearer caller-token');
});

test('a successful login persists the active user and optional bearer token', () => {
  clearSession(window.localStorage);
  assert.equal(hasStoredSession(window.localStorage), false);

  saveSession(window.localStorage, 'usr_teacher_1', 'server-token');

  assert.equal(hasStoredSession(window.localStorage), true);
  assert.equal(window.localStorage.getItem(SESSION_USER_ID_KEY), 'usr_teacher_1');
  assert.equal(window.localStorage.getItem(SESSION_TOKEN_KEY), 'server-token');
  clearSession(window.localStorage);
});

test('logout clears both user identity and token even when a token was absent', () => {
  saveSession(window.localStorage, 'usr_student_1');
  assert.equal(hasStoredSession(window.localStorage), true);
  assert.equal(window.localStorage.getItem(SESSION_TOKEN_KEY), null);

  clearSession(window.localStorage);
  assert.equal(hasStoredSession(window.localStorage), false);
  assert.equal(window.localStorage.getItem(SESSION_USER_ID_KEY), null);
  assert.equal(window.localStorage.getItem(SESSION_TOKEN_KEY), null);
});

test('unauthenticated header exposes only the login CTA, not an account avatar or menu', () => {
  const markup = renderHeader(null);
  assert.match(markup, />Đăng nhập</);
  assert.doesNotMatch(markup, /Tài khoản:/);
  assert.doesNotMatch(markup, /avatar\.png/);
});

test('authenticated header renders the active account identity and replaces the login CTA', () => {
  const markup = renderHeader(signedInUser);
  assert.match(markup, /Tài khoản: Trần Văn Minh/);
  assert.match(markup, /https:\/\/example\.test\/avatar\.png/);
  assert.match(markup, /Trần Minh/);
  assert.doesNotMatch(markup, />Đăng nhập</);
  assert.doesNotMatch(markup, /Tài khoản học tập/);
});

test('roadmap topic updates are scoped by course id', () => {
  assert.equal(
    roadmapTopicStatusPath('course 1', 'topic/2'),
    '/api/roadmap/course%201/topics/topic%2F2/status',
  );
});

test('readApiJson unwraps the C++ success/data envelope', async () => {
  const response = new Response(JSON.stringify({ success: true, data: { id: 7 } }), {
    headers: { 'content-type': 'application/json' },
  });
  assert.deepEqual(await readApiJson<{ id: number }>(response), { id: 7 });
});

test('readApiJson preserves temporary API resource payloads', async () => {
  const response = new Response(JSON.stringify({ courses: [{ id: 'crs_1' }] }), {
    headers: { 'content-type': 'application/json' },
  });
  assert.deepEqual(await readApiJson(response), { courses: [{ id: 'crs_1' }] });
});

test('readApiJson surfaces backend error messages and status', async () => {
  const response = new Response(JSON.stringify({ error: { message: 'Course not found.' } }), {
    status: 404,
    headers: { 'content-type': 'application/json' },
  });
  await assert.rejects(
    () => readApiJson(response),
    (error: unknown) => error instanceof ApiError && error.status === 404 && error.message === 'Course not found.',
  );
});

test('Drogon chat answer is rendered as an assistant message', () => {
  const reply = toMentorReply({
    answer: 'A structured backend answer.',
    message_id: 42,
    model: 'qwen3:8b',
    policy_applied: 'GUIDED',
    assessment: { id: 'assessment-1' },
  }, {
    userId: 'user-1',
    courseId: 'course-1',
    now: () => 123,
    createdAt: () => '2026-09-21T00:00:00.000Z',
  });

  assert.deepEqual(reply, {
    id: '42',
    session_id: 'default',
    user_id: 'user-1',
    course_id: 'course-1',
    role: 'assistant',
    content: 'A structured backend answer.',
    model: 'qwen3:8b',
    created_at: '2026-09-21T00:00:00.000Z',
    policy_applied: 'GUIDED',
    tool_calls: undefined,
    rag_sources: undefined,
    assessment_id: 'assessment-1',
  });
});

test('temporary chat reply is retained and an empty backend answer creates no message', () => {
  const mockReply = {
    id: 'reply-1',
    session_id: 'default',
    role: 'assistant' as const,
    content: 'mock reply',
    created_at: '2026-09-21T00:00:00.000Z',
  };
  const context = { userId: 'user-1', courseId: 'course-1', now: () => 123, createdAt: () => 'now' };
  assert.equal(toMentorReply({ reply: mockReply }, context), mockReply);
  assert.equal(toMentorReply({}, context), undefined);
});
