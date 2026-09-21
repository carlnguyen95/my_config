-- Test-only seed data. Run only against a disposable database created from init.sql.
PRAGMA foreign_keys = ON;
BEGIN;

DELETE FROM thinking_assessments;
DELETE FROM teacher_configurations;
DELETE FROM messages;
DELETE FROM learning_sessions;
DELETE FROM document_chunks;
DELETE FROM documents;
DELETE FROM learning_progress;
DELETE FROM roadmaps;
DELETE FROM questions;
DELETE FROM enrollments;
DELETE FROM courses;
DELETE FROM subjects;
DELETE FROM users;

INSERT INTO users (id,name,email,password_hash,role,status,created_at,updated_at) VALUES
(1,'Trần Văn Minh','minh.tran@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','teacher','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(2,'Teacher Two','teacher2@test.local','seed-hash','teacher','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(3,'Teacher Three','teacher3@test.local','seed-hash','teacher','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(4,'Teacher Four','teacher4@test.local','seed-hash','teacher','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(5,'Hoàng Quốc Tuấn','admin@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','admin','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(6,'Nguyễn Hoàng Nam','nam.student@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(7,'Lê Thị Mai Anh','maianh.le@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(8,'Phạm Quốc Bảo','bao.pham@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(9,'Đỗ Minh Quân','quan.do@eduai.vn','pbkdf2-sha256$210000$00112233445566778899aabbccddeeff$9cdf77bbac5a1b456880c0c2067e8cca0a814996ee75dca319fe531c5c407cfd','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(10,'Student 05','student05@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(11,'Student 06','student06@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(12,'Student 07','student07@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(13,'Student 08','student08@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(14,'Student 09','student09@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(15,'Student 10','student10@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(16,'Student 11','student11@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(17,'Student 12','student12@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(18,'Student 13','student13@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(19,'Student 14','student14@test.local','seed-hash','student','inactive','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z'),
(20,'Student 15','student15@test.local','seed-hash','student','active','2026-09-01T08:00:00Z','2026-09-01T08:00:00Z');

INSERT INTO subjects (id,name,description) VALUES
(1,'Subject 01','Seed subject 01'),(2,'Subject 02','Seed subject 02'),(3,'Subject 03','Seed subject 03'),(4,'Subject 04','Seed subject 04'),(5,'Subject 05','Seed subject 05'),
(6,'Subject 06','Seed subject 06'),(7,'Subject 07','Seed subject 07'),(8,'Subject 08','Seed subject 08'),(9,'Subject 09','Seed subject 09'),(10,'Subject 10','Seed subject 10'),
(11,'Subject 11','Seed subject 11'),(12,'Subject 12','Seed subject 12'),(13,'Subject 13','Seed subject 13'),(14,'Subject 14','Seed subject 14'),(15,'Subject 15','Seed subject 15'),
(16,'Subject 16','Seed subject 16'),(17,'Subject 17','Seed subject 17'),(18,'Subject 18','Seed subject 18'),(19,'Subject 19','Seed subject 19'),(20,'Subject 20','Seed subject 20');

INSERT INTO courses (id,subject_id,name,description,teacher_id,answer_policy,created_at,updated_at) VALUES
(1,1,'Course 01','Seed course 01',1,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(2,2,'Course 02','Seed course 02',2,'HINT_ONLY','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(3,3,'Course 03','Seed course 03',3,'FULL_ANSWER','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(4,4,'Course 04','Seed course 04',4,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(5,5,'Course 05','Seed course 05',1,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(6,6,'Course 06','Seed course 06',2,'HINT_ONLY','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(7,7,'Course 07','Seed course 07',3,'FULL_ANSWER','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(8,8,'Course 08','Seed course 08',4,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(9,9,'Course 09','Seed course 09',1,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(10,10,'Course 10','Seed course 10',2,'HINT_ONLY','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(11,11,'Course 11','Seed course 11',3,'FULL_ANSWER','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(12,12,'Course 12','Seed course 12',4,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(13,13,'Course 13','Seed course 13',1,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(14,14,'Course 14','Seed course 14',2,'HINT_ONLY','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(15,15,'Course 15','Seed course 15',3,'FULL_ANSWER','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(16,16,'Course 16','Seed course 16',4,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(17,17,'Course 17','Seed course 17',1,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(18,18,'Course 18','Seed course 18',2,'HINT_ONLY','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),
(19,19,'Course 19','Seed course 19',3,'FULL_ANSWER','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z'),(20,20,'Course 20','Seed course 20',4,'GUIDED','2026-09-02T08:00:00Z','2026-09-02T08:00:00Z');

INSERT INTO teacher_configurations (id,teacher_id,course_id,policy_text,created_at,updated_at)
SELECT id,teacher_id,id,'Seed teacher policy for course ' || id,'2026-09-02T09:00:00Z','2026-09-02T09:00:00Z' FROM courses;

INSERT INTO enrollments (id,user_id,course_id,created_at) VALUES
(1,6,1,'2026-09-03T08:00:00Z'),(2,7,2,'2026-09-03T08:00:00Z'),(3,8,3,'2026-09-03T08:00:00Z'),(4,9,4,'2026-09-03T08:00:00Z'),(5,10,5,'2026-09-03T08:00:00Z'),
(6,11,6,'2026-09-03T08:00:00Z'),(7,12,7,'2026-09-03T08:00:00Z'),(8,13,8,'2026-09-03T08:00:00Z'),(9,14,9,'2026-09-03T08:00:00Z'),(10,15,10,'2026-09-03T08:00:00Z'),
(11,16,11,'2026-09-03T08:00:00Z'),(12,17,12,'2026-09-03T08:00:00Z'),(13,18,13,'2026-09-03T08:00:00Z'),(14,19,14,'2026-09-03T08:00:00Z'),(15,20,15,'2026-09-03T08:00:00Z'),
(16,6,16,'2026-09-03T08:00:00Z'),(17,7,17,'2026-09-03T08:00:00Z'),(18,8,18,'2026-09-03T08:00:00Z'),(19,9,19,'2026-09-03T08:00:00Z'),(20,10,20,'2026-09-03T08:00:00Z');

INSERT INTO learning_sessions (id,user_id,course_id,started_at,ended_at) SELECT id,user_id,course_id,'2026-09-04T08:00:00Z',NULL FROM enrollments;
INSERT INTO messages (id,session_id,role,content,token_input,token_output,model,created_at) SELECT id,id,'user','Seed message content ' || id,20,0,'seed-model','2026-09-04T09:00:00Z' FROM learning_sessions;
INSERT INTO questions (id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status,created_at,updated_at) SELECT id,id,((id-1)%4)+1,'Seed question topic ' || id,'Seed answer ' || id,'Seed hint ' || id,'medium','short_answer','teacher',CASE (id-1)%4 WHEN 0 THEN 'draft' WHEN 1 THEN 'review' WHEN 2 THEN 'approved' ELSE 'archived' END,'2026-09-05T08:00:00Z','2026-09-05T08:00:00Z' FROM courses;
INSERT INTO documents (id,course_id,title,source_path,created_at) SELECT id,id,'Seed document ' || id,'seed/document-' || id || '.txt','2026-09-06T08:00:00Z' FROM courses;
INSERT INTO document_chunks (id,document_id,chunk_index,content,embedding) SELECT id,id,0,'Seed document content topic ' || id,'[]' FROM documents;
INSERT INTO roadmaps (id,user_id,course_id,title,content_json,created_at,updated_at) SELECT id,user_id,course_id,'Seed roadmap ' || id,'{"topics":[{"name":"Topic ' || id || '","status":"learning"}]}','2026-09-07T08:00:00Z','2026-09-07T08:00:00Z' FROM enrollments;
INSERT INTO learning_progress (id,user_id,course_id,topic,status,progress_value,updated_at) SELECT id,user_id,course_id,'Topic ' || id,CASE (id-1)%4 WHEN 0 THEN 'NOT_STARTED' WHEN 1 THEN 'LEARNING' WHEN 2 THEN 'COMPLETED' ELSE 'NEEDS_REVIEW' END,id/20.0,'2026-09-08T08:00:00Z' FROM enrollments;
INSERT INTO thinking_assessments (id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_feedback,review_status,created_at) SELECT id,s.user_id,s.course_id,id,0.5,'{"curiosity":0.5}','Seed reasoning','seed-model',NULL,NULL,'pending','2026-09-09T08:00:00Z' FROM learning_sessions s;

COMMIT;
