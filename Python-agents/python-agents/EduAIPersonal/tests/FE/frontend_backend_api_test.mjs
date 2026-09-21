import test from 'node:test';
import assert from 'node:assert/strict';

process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0';

const FE_BASE = process.env.FE_BASE_URL ?? 'http://127.0.0.1:3000';
const BE_BASE = process.env.BE_BASE_URL || null;
const runBeChecks = !!BE_BASE;
const beTest = runBeChecks ? test : test.skip;

async function requestJson(url, options = {}) {
  const res = await fetch(url, options);
  const text = await res.text();
  let json = null;

  if (text) {
    try {
      json = JSON.parse(text);
    } catch {
      json = text;
    }
  }

  return { res, json, text };
}

test('FE health endpoint is accessible', async () => {
  const { res, json } = await requestJson(`${FE_BASE}/api/health`);
  assert.equal(res.status, 200);
  assert.equal(json.status, 'ok');
  assert.equal(json.version, '1.0.0-mvp');
});

test('FE auth login endpoint works for a known user', async () => {
  const { res, json } = await requestJson(`${FE_BASE}/api/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ user_id: 'usr_student_1' }),
  });

  assert.equal(res.status, 200);
  assert.ok(json.user);
  assert.equal(json.user.id, 'usr_student_1');
  assert.equal(json.user.role, 'student');
  assert.equal(typeof json.user.name, 'string');
  assert.ok(json.user.name.length > 0);
  assert.equal(typeof json.user.avatar, 'string');
  assert.ok(json.user.avatar.length > 0);
  assert.ok(json.token);
});

test('FE auth login yields a session token that can be sent as a bearer header', async () => {
  const { res: loginRes, json: loginJson } = await requestJson(`${FE_BASE}/api/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ user_id: 'usr_teacher_1' }),
  });

  assert.equal(loginRes.status, 200);
  assert.equal(loginJson.user.id, 'usr_teacher_1');
  assert.ok(loginJson.token);

  const { res: profileRes, json: profileJson } = await requestJson(
    `${FE_BASE}/api/auth/me?user_id=${loginJson.user.id}`,
    { headers: { Authorization: `Bearer ${loginJson.token}` } },
  );
  assert.equal(profileRes.status, 200);
  assert.equal(profileJson.user.id, loginJson.user.id);
  assert.equal(profileJson.user.avatar, loginJson.user.avatar);
});

test('FE auth logout accepts a bearer token and returns a successful logout response', async () => {
  const { res: loginRes, json: loginJson } = await requestJson(`${FE_BASE}/api/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ user_id: 'usr_student_1' }),
  });
  assert.equal(loginRes.status, 200);

  const { res, json } = await requestJson(`${FE_BASE}/api/auth/logout`, {
    method: 'POST',
    headers: { Authorization: `Bearer ${loginJson.token}` },
  });
  assert.equal(res.status, 200);
  assert.equal(json.success, true);
});

test('FE courses endpoint returns a populated course list', async () => {
  const { res, json } = await requestJson(`${FE_BASE}/api/courses`);
  assert.equal(res.status, 200);
  const courses = Array.isArray(json) ? json : json.courses;
  assert.ok(Array.isArray(courses) && courses.length > 0);
  assert.ok(courses[0].id);
});

test('FE course document list is returned for a selected course', async () => {
  const { res, json } = await requestJson(`${FE_BASE}/api/courses/crs_tcp_201/documents`);
  assert.equal(res.status, 200);
  const documents = Array.isArray(json) ? json : json.documents;
  assert.ok(Array.isArray(documents) && documents.length > 0);
  assert.ok(documents[0].course_id === 'crs_tcp_201');
});

beTest('BE login endpoint accepts seeded teacher account and returns token', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ email: 'minh.tran@eduai.vn', password: 'EduAI@2026' }),
  });

  assert.equal(res.status, 200);
  assert.ok(json.token);
  assert.equal(json.user.email, 'minh.tran@eduai.vn');
});

beTest('BE me endpoint returns the requested user profile', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/auth/me?user_id=1`);
  assert.equal(res.status, 200);
  assert.equal(json.user.id, 1);
  assert.equal(json.user.email, 'minh.tran@eduai.vn');
});

beTest('BE course listing returns seed data', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/courses`);
  assert.equal(res.status, 200);
  assert.ok(Array.isArray(json.courses) && json.courses.length >= 1);
  assert.ok(json.courses[0].id);
});

beTest('BE single course detail is accessible to an enrolled student', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/courses/1?actor_id=6`);
  assert.equal(res.status, 200);
  assert.equal(json.course.id, 1);
  assert.equal(json.course.name, 'Course 01');
});

beTest('BE protects a course from unauthorized access', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/courses/2?actor_id=6`);
  assert.equal(res.status, 200);
  assert.ok(json.error || json.error_json || json.message);
});

beTest('BE document list for a course returns seeded documents', async () => {
  const { res, json } = await requestJson(`${BE_BASE}/api/courses/1/documents`);
  assert.equal(res.status, 200);
  assert.ok(Array.isArray(json.documents) && json.documents.length >= 1);
  assert.ok(json.documents[0].course_id === 1 || json.documents[0].courseId === 1);
});
