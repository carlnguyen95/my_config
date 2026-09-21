/**
 * Small compatibility boundary between the React UI and the API implementation.
 *
 * With no VITE_API_BASE_URL the temporary Express API in `frontend/server.ts`
 * remains the default.  A deployment can point the UI at the Drogon service by
 * setting VITE_API_BASE_URL (for example, https://localhost:8443).
 */
const apiBaseUrl = (import.meta.env?.VITE_API_BASE_URL || '').replace(/\/$/, '');

export class ApiError extends Error {
  constructor(message: string, readonly status: number) {
    super(message);
    this.name = 'ApiError';
  }
}

export function apiUrl(path: string): string {
  return buildApiUrl(apiBaseUrl, path);
}

export function buildApiUrl(baseUrl: string, path: string): string {
  if (!path.startsWith('/')) {
    throw new Error(`API path must start with '/': ${path}`);
  }
  return `${baseUrl.replace(/\/$/, '')}${path}`;
}

export function roadmapTopicStatusPath(courseId: string, topicId: string): string {
  return `/api/roadmap/${encodeURIComponent(courseId)}/topics/${encodeURIComponent(topicId)}/status`;
}

export async function apiFetch(path: string, init: RequestInit = {}): Promise<Response> {
  const headers = new Headers(init.headers);
  const token = window.localStorage.getItem('edu_ai_token');
  if (token && !headers.has('Authorization')) {
    headers.set('Authorization', `Bearer ${token}`);
  }

  return fetch(apiUrl(path), {
    ...init,
    headers,
  });
}

/**
 * Reads both response styles used in this project:
 * - temporary API: the resource itself, such as `{ user: ... }` or `[...]`
 * - C++ adapter: `{ success: true, data: ... }`
 */
export async function readApiJson<T>(response: Response): Promise<T> {
  const contentType = response.headers.get('content-type') || '';
  if (!contentType.includes('application/json')) {
    throw new ApiError('Máy chủ không trả về JSON.', response.status);
  }

  const payload: unknown = await response.json();
  const record = payload && typeof payload === 'object' ? payload as Record<string, unknown> : undefined;
  const error = record?.error && typeof record.error === 'object'
    ? record.error as Record<string, unknown>
    : undefined;

  if (!response.ok || error) {
    const message = typeof error?.message === 'string' ? error.message : 'Yêu cầu tới máy chủ thất bại.';
    throw new ApiError(message, response.status);
  }

  if (record?.success === true && Object.prototype.hasOwnProperty.call(record, 'data')) {
    return record.data as T;
  }
  return payload as T;
}

export function apiResponseValue<T>(payload: unknown, key: string): T | undefined {
  if (!payload || typeof payload !== 'object') return undefined;
  const value = (payload as Record<string, unknown>)[key];
  return value as T | undefined;
}
