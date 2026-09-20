PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS teacher_configurations (
    id INTEGER PRIMARY KEY,
    teacher_id INTEGER NOT NULL REFERENCES users(id),
    course_id INTEGER NOT NULL REFERENCES courses(id),
    policy_text TEXT NOT NULL DEFAULT '' CHECK(length(policy_text) <= 12000),
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    UNIQUE(teacher_id, course_id)
);
