# 3. Expressive Agent

Эмоция NPC — это **результат оценки ситуации**, в которой он (NPC) оказался. LLM взвешивает тон запроса игрока, состояние игрока и NPC и реагирует соответственно.

```mermaid
flowchart LR
    subgraph in["Input request"]
        TONE["тон и суть запроса игрока:<br/>Дай живо! / Продай по братски, пожалуйста"]
        US["состояние игрока:<br/>ранен, в кошельке 0$"]
        NS["состояние NPC:<br/>товара в избытке"]
    end

    LLM{{"LLM<br/>взвешивает всё сразу"}}

    TAG(["тэг эмоции:<br/>Neutral / Happy / Sad /<br/>Angry / Surprise"])

    TONE --> LLM
    US --> LLM
    NS --> LLM
    LLM --> TAG

    classDef state fill:#fff3e0,stroke:#d9822b,stroke-width:2px,color:#3b2a12
    classDef engine fill:#e8f0fe,stroke:#3b6fb6,stroke-width:2px,color:#12243b
    classDef edge fill:#f3f4f6,stroke:#8b8f96,stroke-width:1.5px,color:#22262b

    class TONE,US,NS state
    class LLM engine
    class TAG edge
```

Игрок хамит, но принёс редкий трофей да ещё и ранен — LLM взвешивает всё вместе и может выдать `Surprise` вместо ожидаемого `Angry`. Один и тот же вопрос в разном состоянии даёт разную реакцию.

И тэг приходит **вместе с ответом**, одним структурированным объектом:

```json
{
  "emotion": "Happy",
  "answer":  "Конечно, вот лучшие стволы в городе!",
  "action":  { "name": "ShowItemsByCategory", "parameters": { "category": "weapons" } }
}
```

* `emotion` — тэг, одна из пяти эмоций (`Neutral`, `Happy`, `Sad`, `Angry`, `Surprise`).

* `answer` — текст, который уйдёт в озвучку.

* `action` — то, что изменит игру.

***

## Тэг эмоции -> голос и мимика

Дальше тэг эмоции расходится по двум каналам и "оживляет" MetaHuman:

```mermaid
flowchart TD
    TAG(["тэг: Happy"])

    SAMPLE["сэмпл голоса под эмоцию<br/>— референс для клонирования<br/>(из DataAsset NPC)"]
    FACE["мимика MetaHuman<br/>изменяется"]

    TTS["синтез речи в реальном времени<br/>(клонирование голоса)"]
    LS["LipSync"]
    OUT(["MetaHuman говорит —<br/>голос, лицо и губы синхронно"])

    TAG -- голос --> SAMPLE
    TAG -- лицо --> FACE
    SAMPLE --> TTS
    ANSWER(["текст 'answer' из ответа модели"]) --> TTS
    TTS -- аудио --> LS
    LS --> OUT
    FACE --> OUT

    classDef state fill:#fff3e0,stroke:#d9822b,stroke-width:2px,color:#3b2a12
    classDef engine fill:#e8f0fe,stroke:#3b6fb6,stroke-width:2px,color:#12243b
    classDef edge fill:#f3f4f6,stroke:#8b8f96,stroke-width:1.5px,color:#22262b

    class SAMPLE,FACE state
    class TTS,LS engine
    class TAG,ANSWER,OUT edge
```

В DataAsset персонажа, в поле `NpcVoices`, лежат **сэмплы голоса с разными эмоциями** — по одному на каждый тэг: `Neutral`, `Happy`, `Sad`, `Angry`, `Surprise`.

И это **референсы для клонирования**, а не готовые реплики.

Пришёл тэг `Happy` — берём соответствующий сэмпл как образец тембра и настроения и **в реальном времени синтезируем этим голосом сам текст ответа** из поля `answer`.

Так NPC произносит любую сгенерированную фразу своим голосом и с нужной эмоцией. Параллельно тэг включает мимику, а lipsync подгоняет движение губ под получившееся аудио.

Как это выглядит вживую — на видео ниже.

> 🎬 **Видео:** NPC последовательно произносит пять реплик — по одной на `Neutral`, `Happy`, `Sad`, `Angry` и `Surprise`. Тэг задаётся напрямую, LLM в этом ролике не участвует.

***

Теперь NPC может проявлять характер и *реагировать на игрока*. Осталось сделать так, чтобы он ещё и *взаимодействовал с игроком*.
