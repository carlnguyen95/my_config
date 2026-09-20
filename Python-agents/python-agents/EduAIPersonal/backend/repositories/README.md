# Repository layout

Each persistent table has one repository pair, keeping SQL ownership explicit:

| Database table | SQLite repository |
| --- | --- |
| `users` | `SqliteUserRepository` |
| `subjects` | `SqliteSubjectRepository` |
| `courses` | `SqliteCourseRepository` |
| `teacher_configurations` | `SqliteTeacherConfigurationRepository` |
| `enrollments` | `SqliteEnrollmentRepository` |
| `learning_sessions` | `SqliteLearningSessionRepository` |
| `messages` | `SqliteMessageRepository` |
| `questions` | `SqliteQuestionRepository` |
| `documents` | `SqliteDocumentRepository` |
| `document_chunks` | `SqliteDocumentChunkRepository` |
| `roadmaps` | `SqliteRoadmapRepository` |
| `learning_progress` | `SqliteProgressRepository` |
| `thinking_assessments` | `SqliteAssessmentRepository` |

`Repositories.hpp` contains the framework-independent contracts. `RepositorySupport.hpp` contains only common prepared-statement, binding, and enum conversion helpers; it has no business rules.
