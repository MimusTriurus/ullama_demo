# ullama_demo — интерактивный голосовой NPC на локальной LLM в Unreal Engine 5

> Задел для статьи. Черновое, но фактологически выверенное описание проекта: что это, из чего состоит, как работает и какие фичи внутри.

## TL;DR

`ullama_demo` — демо-проект на **Unreal Engine 5.7**, в котором с персонажем **MetaHuman можно разговаривать голосом**. Полный цикл «услышал — понял — ответил голосом — пошевелил губами — проявил эмоцию — выполнил игровое действие» работает **полностью локально**, без облачных API и интернета.

Проект — развитие идеи из [статьи на Habr (807561)](https://habr.com/ru/articles/807561/), но на более свежем движке (UE 5.7 вместо 5.3) и с собственной модульной архитектурой из нескольких плагинов. Автор — студия **Stendhal Syndrome Studio**; часть плагинов опубликована на Unreal Marketplace.

Показательный сценарий демо — **разговорный NPC-торговец**: игрок голосом просит товары, LLM понимает запрос и через систему действий управляет инвентарём/интерфейсом (на MVVM).

---

## Конвейер (pipeline)

```
┌─────────────┐   голос    ┌─────────────┐   текст   ┌──────────────────────┐
│   Игрок     │──────────▶│    STT      │──────────▶│  Формирование промпта │
│  (микрофон) │            │ (речь→текст)│            │  system + RAG-контекст│
└─────────────┘            └─────────────┘            └───────────┬──────────┘
                                                                   │
                                                                   ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                        ULlamaPlugin  (llama.cpp)                            │
│  локальный инференс LLM, потоковая выдача токенов, CUDA-ускорение           │
└───────────────┬───────────────────────────────────┬──────────────────────┘
                │ текст ответа                        │ триггеры/эмоция
                ▼                                     ▼
     ┌────────────────────┐              ┌──────────────────────────────┐
     │  SpeechGenSystem   │              │  NpcEmotions + ActionHandler  │
     │      (TTS)         │              │  эмоция + игровое действие    │
     └─────────┬──────────┘              └───────────────┬──────────────┘
               │ аудио                                    │
               ▼                                          ▼
     ┌────────────────────┐              ┌──────────────────────────────┐
     │  ULSS / LipSync    │              │   MVVM UI (инвентарь и т.д.)  │
     │  анимация губ MH   │              │   реакция интерфейса          │
     └────────────────────┘              └──────────────────────────────┘
               │
               ▼
       MetaHuman говорит с синхронизацией губ и мимикой
```

Ключевые этапы:

1. **STT** — распознавание речи игрока в текст (в оригинальной статье — Vosk).
2. **RAG / контекст** — семантический поиск релевантных фактов в базе знаний NPC.
3. **LLM-инференс** — `ULlamaPlugin` поверх `llama.cpp`, потоковая генерация токенов.
4. **Пост-обработка** — из потока токенов вычленяются готовые предложения (для TTS) и **триггеры-действия** / метка эмоции.
5. **TTS** — синтез речи (`SpeechGenerationSystem`), голос выбирается под текущую эмоцию.
6. **LipSync** — генерация анимации губ MetaHuman по аудио (`ULSS`).
7. **Действия и UI** — `ULlamaActionHandler` выполняет игровые действия, интерфейс на MVVM реагирует.

---

## Архитектура: плагины и модули

Каждый плагин подключён как самостоятельный git-репозиторий (вложенные `.git`).

| Компонент | Тип | Назначение |
|---|---|---|
| **ULlamaPlugin** | собственный | Обёртка над **llama.cpp**. Локальный инференс LLM + база знаний (RAG). Тянет DLL из `ThirdParty/ULlamaAPI` (`llama.dll`, `ggml-*.dll`, включая `ggml-cuda.dll` — есть GPU/CUDA). |
| **ULSS** (LipSyncSystem) | собственный, Marketplace | Генерация анимации губ MetaHuman по аудио. Только Win64. |
| **SpeechGenerationSystem** | собственный, Marketplace | Text-to-Speech. Только Win64. |
| **uqsgs** | собственный | Вспомогательный плагин студии. |
| MetaHumanCharacter, RigLogic, LiveLinkControlRig | движковые | Персонаж MetaHuman и лицевая анимация. |
| ModelViewViewModel (MVVM), CommonUI | движковые | Архитектура и построение UI. |
| AppleARKitFaceSupport | движковый | Поддержка ARKit-мимики. |

**Платформа:** только **Win64**.

### Игровой модуль `Source/ullama_demo`

Тонкий C++ слой поверх плагинов, задающий data-driven описание NPC:

- **`NpcData`** (`UNPCDataAsset` / `UNPCDataRegistry`) — описание NPC: `Id`, `Name`, `Description` (личность), массив шаблонов действий `FNpcActionTemplate`, реестр всех NPC и сериализация в JSON (`ToJson`).
- **`FNpcVoices`** — набор голосов (`USoundWave`) под эмоции: Neutral / Happy / Sad / Angry / Surprise.
- **`NpcEmotions`** (`ENpcEmotion`) — перечисление эмоций + хелперы конвертации строка↔enum (модель возвращает эмоцию текстом).
- **`KnowledgeBaseData`** — данные для базы знаний NPC (источник чанков для RAG).
- **`ULlamaLogger`** — логирование.

---

## Ключевые API плагина ULlamaPlugin

Всё вызывается из Blueprint (`BlueprintCallable`), тяжёлые операции — асинхронные (`UBlueprintAsyncActionBase`).

### LLM — `ULlamaManager`
- `CreateWorker()` → хэндл воркера; `LoadModelAsync` / `InitWorkerAsync` — асинхронная загрузка модели.
- `MakeSystemPrompt(...)` — сборка system-промпта из личности NPC, описания игрока, списка действий, примера диалога и эмоций.
- `Ask` / `AskWithContext(prompt, context)` → `Run` → `GetToken` (потоковая выдача) + `IsSpeaking`.
- `ExtractCompleteSentence(...)` — вычленение готового предложения из потока (чтобы отдавать в TTS сразу, не дожидаясь конца генерации).
- `ExtractTrigger(...)` — извлечение триггера-действия из ответа модели.
- `Dispose` / `FreeLlmModel` — освобождение ресурсов.

### RAG — `ULlamaKnowledgeBase`
- `UllamaKnowledgeBaseMake()` / `AddChunk` / `Dispose` — создание и наполнение базы.
- `UpdateKbAsync(chunks)` — асинхронное построение эмбеддингов.
- `SearchInKnowledgeBaseAsync(query, maxResults)` → индексы + score релевантных чанков.

### Конфигурация инференса — `FULlamaInferenceConfig`
Полный контроль над llama.cpp прямо из редактора / JSON:
- модель: `ModelFilePath`, `LoraAdapterFilePath`, `SystemPrompt`, `AntiPrompts`;
- сэмплинг: `Temperature`, `TopK`, `TopP`, `RepeatPenalty`, `RepeatLastN`, `Seed`;
- ресурсы: `GpuLayers` (0 = CPU), `bMlock`, `bNoMmap`, `NCtx`, `NPredict`;
- управление: `bResetHistory`, `Grammar` / `bUseGrammar` (GBNF-грамматика для структурированного вывода).
- `UULlamaConfigLoader` — загрузка конфига из JSON или по имени NPC (`LoadInferenceConfigByNpcName`, отдельный конфиг для эмбеддингов).

### Действия — `ULlamaActionHandler`
- `UULlamaActionHandlerBase::Execute(ViewModel, ActionName, Params)` — расширяемый в Blueprint обработчик: модель называет действие и параметры, C++/BP выполняет его над ViewModel.

---

## Фичи

- 🎙️ **Полностью локальный голосовой NPC** — STT → LLM → TTS → LipSync, без облака и интернета.
- 🧠 **LLM в движке через llama.cpp** — поддержка GGUF-моделей, **CUDA-ускорение** (`GpuLayers`), LoRA-адаптеры.
- 📚 **RAG / база знаний** — NPC отвечает с опорой на факты через семантический поиск по эмбеддингам.
- ⚡ **Потоковая генерация** — токены отдаются по мере генерации; готовые предложения уходят в TTS сразу (низкая воспринимаемая задержка).
- 🎭 **Эмоции** — модель возвращает эмоцию, под неё выбирается голос и мимика (5 состояний).
- 👄 **LipSync MetaHuman** — синхронизация губ по синтезированному аудио.
- 🎬 **Триггеры-действия** — LLM управляет игровой логикой через `ActionHandler` (например, инвентарь торговца).
- 🧩 **Data-driven NPC** — личность, голоса, действия и база знаний описываются в Data Asset'ах и JSON, без правки кода.
- 🖥️ **UI на MVVM** — интерфейс (инвентарь/торговля) построен на ModelViewViewModel + CommonUI.
- 🔧 **Полный контроль сэмплинга** — температура, top-k/p, penalty, seed, grammar (GBNF) из редактора.

---

## Демо-сценарий: NPC-торговец

Основной сценарий (см. `Content/Blueprints/NPCs/Trader`, `Content/Blueprints/MVVM`, карта `Trader_UI_Test`):

- Игрок голосом обращается к торговцу.
- LLM понимает запрос (с учётом базы знаний о товарах) и формирует ответ + триггер-действие.
- `ActionHandler` выполняет действие над ViewModel'ями инвентаря (`InventoryVM`, `ItemVM`, `CategoryVM`, `SelectionVM`).
- UI на MVVM обновляется, торговец озвучивает ответ с нужной эмоцией и липсинком.

---

## Карты (эволюция прототипа)

Расположены в `Content/Maps`:

| Карта | Роль |
|---|---|
| `MH_Animations.umap` | Отладка анимаций MetaHuman. |
| `MH_TTS_LS_LLM.umap` | Базовая связка TTS + LipSync + LLM. |
| `MH_TTS_LS_LLM_MVVM.umap` | **Текущая рабочая** — та же связка + MVVM-интерфейс. |
| `MH_TTS_LS_LLM_MVVM_BACKUP.umap` | Резервная копия. |
| `MH_TTS_LS_LLM_emulation.umap` | Эмуляция без реального LLM — для отладки UI/пайплайна. |
| `Trader_UI_Test.umap` | Тест интерфейса торговца. |
| `Main.umap`, `Test.umap` | Основная / тестовая сцена. |

Видно движение от базовой связки TTS+LipSync+LLM к версии с полноценным MVVM-интерфейсом торговца.

---

## Структура проекта

```
ullama_demo/
├── Source/ullama_demo/          # игровой C++ модуль (NpcData, NpcEmotions, KnowledgeBaseData, Logger)
├── Plugins/
│   ├── ULlamaPlugin/            # llama.cpp обёртка: LLM + RAG (+ ThirdParty/ULlamaAPI DLL)
│   ├── ULSS/                    # LipSyncSystem (Marketplace)
│   └── uqsgs/                   # вспомогательный плагин
│   └── SpeechGenerationSystem/  # TTS (подключён как движковый/маркетплейс плагин)
├── Content/
│   ├── Maps/                    # сцены (эволюция прототипа)
│   ├── Blueprints/              # NPCs/Trader, MVVM, UI, GameMode, контроллеры
│   ├── DataAssets/              # DA_NPC*, реестры NPC
│   ├── MetaHumans/, MH/         # персонажи
│   ├── Audio/, Animations/, Textures/
│   └── InventoryMvvmSubproject/ # ассеты инвентаря/иконок
└── ullama_demo.uproject         # UE 5.7, Win64
```

---

## Технические заметки для статьи

- **UE 5.7**, только **Win64**.
- LLM-бинарники (llama.cpp / ggml) поставляются как DLL через `ThirdParty/ULlamaAPI`; в проекте настроен `PublicDelayLoadDLLs` + `RuntimeDependencies`.
- Есть **CUDA** (`ggml-cuda.dll`, параметр `GpuLayers`).
- В корне есть `.venv` и `Tools/` — вероятно, вспомогательные Python-скрипты (подготовка моделей/эмбеддингов) вне репозитория.
- Оригинальный стек из статьи: STT — Vosk, LLM — Mistral 7B через llama.cpp, TTS — Piper, LipSync — переработанный Oculus LipSync, персонаж — MetaHuman, многопоточность — `FRunnableThread`.

---

## Идеи для наполнения статьи

1. Почему **локальный** стек (приватность, отсутствие латентности сети, отсутствие стоимости токенов, оффлайн).
2. Как устроена **потоковая** отдача: token-streaming → sentence extraction → TTS чанками для минимальной задержки.
3. **RAG внутри движка**: эмбеддинги + семантический поиск без внешних сервисов.
4. **Function calling по-своему**: триггеры-действия из текста модели (`ExtractTrigger` + `ActionHandler`) и GBNF-грамматика для надёжного парсинга.
5. Связка **эмоция → голос → мимика/LipSync** как способ «оживить» NPC.
6. **Data-driven** дизайн NPC: личность/голоса/действия/знания в ассетах и JSON.
7. Интеграция с **MVVM** и CommonUI: как LLM «дёргает» реальный игровой UI.
8. Производительность: CPU vs GPU (`GpuLayers`), `NCtx`/`NPredict`, управление контекстом (`bResetHistory`).
