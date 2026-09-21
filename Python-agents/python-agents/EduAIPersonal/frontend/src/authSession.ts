export const SESSION_USER_ID_KEY = 'edu_ai_user_id';
export const SESSION_TOKEN_KEY = 'edu_ai_token';

type SessionStorage = Pick<Storage, 'getItem' | 'setItem' | 'removeItem'>;

export function hasStoredSession(storage: SessionStorage): boolean {
  return Boolean(storage.getItem(SESSION_USER_ID_KEY)?.trim());
}

export function saveSession(storage: SessionStorage, userId: string, token?: string): void {
  storage.setItem(SESSION_USER_ID_KEY, userId);
  if (token) {
    storage.setItem(SESSION_TOKEN_KEY, token);
  } else {
    storage.removeItem(SESSION_TOKEN_KEY);
  }
}

export function clearSession(storage: SessionStorage): void {
  storage.removeItem(SESSION_USER_ID_KEY);
  storage.removeItem(SESSION_TOKEN_KEY);
}
