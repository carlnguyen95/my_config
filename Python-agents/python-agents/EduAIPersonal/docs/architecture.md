# Architecture boundaries

```
HTTP controller → application service → repository
                         ↓
                       AIEngine
                  ┌──────┼──────┐
               policy  prompts  tool registry
                         ↓
                     AIProvider
                  Ollama / Cloud
```

Controllers never select a role from request JSON, call an AI provider directly, or execute a tool. `ToolRegistry` is the single registration and authorization point for model-requested operations. Assessment processing is separate from the learning response path.

