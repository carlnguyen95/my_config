PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY, name TEXT NOT NULL, email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL, role TEXT NOT NULL CHECK(role IN ('student','teacher','admin')),
    status TEXT NOT NULL DEFAULT 'active', created_at TEXT NOT NULL, updated_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS subjects (
    id INTEGER PRIMARY KEY, name TEXT NOT NULL, description TEXT NOT NULL DEFAULT ''
);
CREATE TABLE IF NOT EXISTS courses (
    id INTEGER PRIMARY KEY, subject_id INTEGER NOT NULL REFERENCES subjects(id), name TEXT NOT NULL,
    description TEXT NOT NULL DEFAULT '', teacher_id INTEGER NOT NULL REFERENCES users(id),
    answer_policy TEXT NOT NULL DEFAULT 'GUIDED' CHECK(answer_policy IN ('FULL_ANSWER','HINT_ONLY','GUIDED')),
    created_at TEXT NOT NULL, updated_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS enrollments (
    id INTEGER PRIMARY KEY, user_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id),
    created_at TEXT NOT NULL, UNIQUE(user_id, course_id)
);
CREATE TABLE IF NOT EXISTS learning_sessions (
    id INTEGER PRIMARY KEY, user_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id),
    started_at TEXT NOT NULL, ended_at TEXT
);
CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY, session_id INTEGER NOT NULL REFERENCES learning_sessions(id), role TEXT NOT NULL,
    content TEXT NOT NULL, token_input INTEGER, token_output INTEGER, model TEXT, created_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS questions (
    id INTEGER PRIMARY KEY, course_id INTEGER NOT NULL REFERENCES courses(id), created_by INTEGER NOT NULL REFERENCES users(id),
    question TEXT NOT NULL, answer TEXT NOT NULL, hint TEXT NOT NULL DEFAULT '', difficulty TEXT NOT NULL DEFAULT 'medium',
    question_type TEXT NOT NULL DEFAULT 'short_answer', source TEXT NOT NULL CHECK(source IN ('teacher','ai','imported')),
    status TEXT NOT NULL CHECK(status IN ('draft','review','approved','archived')), created_at TEXT NOT NULL, updated_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS documents (
    id INTEGER PRIMARY KEY, course_id INTEGER NOT NULL REFERENCES courses(id), title TEXT NOT NULL,
    source_path TEXT NOT NULL, created_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS document_chunks (
    id INTEGER PRIMARY KEY, document_id INTEGER NOT NULL REFERENCES documents(id), chunk_index INTEGER NOT NULL,
    content TEXT NOT NULL, embedding TEXT, UNIQUE(document_id, chunk_index)
);
CREATE TABLE IF NOT EXISTS roadmaps (
    id INTEGER PRIMARY KEY, user_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id),
    title TEXT NOT NULL, content_json TEXT NOT NULL, created_at TEXT NOT NULL, updated_at TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS learning_progress (
    id INTEGER PRIMARY KEY, user_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id),
    topic TEXT NOT NULL, status TEXT NOT NULL CHECK(status IN ('NOT_STARTED','LEARNING','COMPLETED','NEEDS_REVIEW')),
    progress_value REAL NOT NULL DEFAULT 0, updated_at TEXT NOT NULL, UNIQUE(user_id, course_id, topic)
);
CREATE TABLE IF NOT EXISTS thinking_assessments (
    id INTEGER PRIMARY KEY, user_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id),
    message_id INTEGER NOT NULL REFERENCES messages(id), score REAL NOT NULL, dimensions_json TEXT NOT NULL,
    reasoning TEXT NOT NULL, model_name TEXT NOT NULL, teacher_score REAL, teacher_feedback TEXT,
    review_status TEXT NOT NULL DEFAULT 'pending', created_at TEXT NOT NULL
);

