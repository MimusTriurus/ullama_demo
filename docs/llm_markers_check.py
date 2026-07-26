#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Проверка черновиков статьи на стилистические маркеры LLM-текста.

Чек-листы, по которым идёт проверка:
  * docs/llm-style-markers.md    — формальные приметы (клише, хеджи, триады)
  * docs/llm-style-markers_2.md  — интонация («AI-слоп»: тире, антитезы, афоризмы)

Реализация своя (не код из статьи-источника): здесь нет обращения к LLM,
только регулярки и простая статистика. Семантику всё равно проверяет человек —
скрипт лишь показывает, где смотреть.

Запуск:
    py docs/llm_markers_check.py
    py docs/llm_markers_check.py docs/parts/section-03-emotions.md
    py docs/llm_markers_check.py --verbose
"""

from __future__ import annotations

import argparse
import re
import statistics
import sys
from dataclasses import dataclass, field
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):          # без этого Windows-консоль давится кириллицей
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

REPO = Path(__file__).resolve().parent.parent
DEFAULT_GLOB = "docs/parts/section-*.md"

EM_DASH = "—"

# ── словари маркеров ────────────────────────────────────────────────────────

FILLERS = [
    "по сути", "казалось бы", "грубо говоря", "как известно", "не секрет",
    "в эпоху", "в наше время", "выходит, что", "в этой связи", "в данном контексте",
]

CONNECTIVE_OPENERS = [
    "таким образом", "итак", "отсюда", "значит", "поэтому", "следовательно",
    "подводя итог", "в итоге", "именно поэтому",
]

DIRECTIVES = [
    "обратите внимание", "стоит отметить", "важно понимать", "следует признать",
    "главное свойство", "ключевое свойство", "ключевой момент", "важный момент",
    "заметьте", "безусловно",
]

HEDGES = [
    "потенциально", "как правило", "в целом", "скорее всего", "вероятно",
    "в некотором смысле", "можно сказать",
]

PSEUDO_SINCERITY = [
    "честно говоря", "если честно", "признаться", "без воды", "положа руку",
    "а теперь честно", "буду откровенен",
]

VAGUE_ATTRIBUTION = [
    "исследования показывают", "эксперты считают", "принято считать",
    "многие считают", "играет ключевую роль", "открывает новые возможности",
    "выходит на новый уровень",
]

# ── регулярки ───────────────────────────────────────────────────────────────

RE_ANTITHESIS = [
    re.compile(r",\s*а\s+не\s+", re.I),                       # «X, а не Y»
    re.compile(r"\bне\s+[^,.;:!?]{2,60},\s*а\s+", re.I),      # «не X, а Y»
    re.compile(r"\bэто\s+не\s+[^,.;:!?]{2,60},\s*это\s+", re.I),
]
RE_TRIAD = re.compile(r"\b[\wа-яё-]+,\s+[\wа-яё-]+\s+и\s+[\wа-яё-]+\b", re.I)
RE_CONCRETE = re.compile(r"`[^`]+`|\b\d+[\w%]*\b|\b[A-Z][a-zA-Z]{2,}\b")
RE_SENT_SPLIT = re.compile(r"(?<=[.!?…])\s+")
RE_BOLD_TAIL = re.compile(r"\*\*[^*]+\*\*[\s.!?»)]*$")


@dataclass
class Hit:
    line: int
    marker: str
    text: str


@dataclass
class Report:
    path: Path
    words: int = 0
    dashes: int = 0
    paragraphs: list[str] = field(default_factory=list)
    hits: list[Hit] = field(default_factory=list)
    triad_lists: int = 0
    concrete_first: int = 0
    concrete_second: int = 0

    @property
    def dash_density(self) -> float:
        """Тире на 100 слов. Ориентир: >5 — плотно, >7 — бросается в глаза."""
        return 100.0 * self.dashes / self.words if self.words else 0.0

    @property
    def para_spread(self) -> float:
        """Разброс длин абзацев. Низкий = подозрительно ровный текст."""
        lens = [len(p.split()) for p in self.paragraphs if len(p.split()) > 3]
        return statistics.pstdev(lens) if len(lens) > 1 else 0.0

    def count(self, marker: str) -> int:
        return sum(1 for h in self.hits if h.marker == marker)


def strip_markdown(raw: str) -> tuple[str, dict[int, int]]:
    """Выкидывает всё, что не идёт в публикацию: листинги, mermaid, таблицы,
    заметки автора, TODO для вёрстки. Возвращает текст и карту «новая строка → исходная»."""
    kept: list[str] = []
    line_map: dict[int, int] = {}
    in_fence = False

    for idx, line in enumerate(raw.splitlines(), start=1):
        stripped = line.strip()

        if stripped.startswith("```"):
            in_fence = not in_fence
            continue
        if in_fence:
            continue
        if stripped.startswith("## Заметки автора"):
            break                                   # дальше только служебное
        if stripped and (stripped.startswith("|") or set(stripped) <= set("-| ")):
            continue                                # таблицы и их разделители
        if "TODO" in stripped:
            continue
        if stripped in {"***", "---", "___"}:
            continue

        kept.append(line)
        line_map[len(kept)] = idx

    return "\n".join(kept), line_map


def iter_paragraphs(lines: list[str], line_map: dict[int, int]):
    """Отдаёт (текст абзаца, номер строки в исходном файле)."""
    buf: list[str] = []
    start = 0
    for i, line in enumerate(lines, start=1):
        if line.strip():
            if not buf:
                start = line_map.get(i, i)
            buf.append(line.strip())
        elif buf:
            yield " ".join(buf), start
            buf = []
    if buf:
        yield " ".join(buf), start


def scan(path: Path, *, verbose: bool = False) -> Report:
    raw = path.read_text(encoding="utf-8")
    text, line_map = strip_markdown(raw)
    rep = Report(path=path)

    lines = text.splitlines()
    rep.words = len(text.split())
    rep.dashes = text.count(EM_DASH)
    rep.paragraphs = [p.strip() for p in re.split(r"\n\s*\n", text) if p.strip()]

    phrase_groups = [
        ("филлер", FILLERS),
        ("связка-зачин", CONNECTIVE_OPENERS),
        ("директива", DIRECTIVES),
        ("хедж", HEDGES),
        ("псевдоискренность", PSEUDO_SINCERITY),
        ("размытая атрибуция", VAGUE_ATTRIBUTION),
    ]

    for i, line in enumerate(lines, start=1):
        src = line_map.get(i, i)
        low = line.lower()

        for marker, phrases in phrase_groups:
            for phrase in phrases:
                # связки ловим только в начале предложения — там они и вредят
                if marker == "связка-зачин":
                    found = low.startswith(phrase)
                else:
                    # \b, иначе «вероятно» ловится внутри «вероятностей»
                    found = re.search(rf"\b{re.escape(phrase)}\b", low) is not None
                if found:
                    rep.hits.append(Hit(src, marker, phrase))

        for rx in RE_ANTITHESIS:
            for m in rx.finditer(line):
                rep.hits.append(Hit(src, "антитеза «не X, а Y»", m.group(0).strip()))

    # абзацы: обрывы и афористичные концовки
    for para, src in iter_paragraphs(lines, line_map):
        if para.lstrip().startswith((">", "#", "*", "-", "1.")):
            continue
        words = para.split()
        if len(words) <= 7:
            rep.hits.append(Hit(src, "абзац-обрыв", para[:60]))
        elif RE_BOLD_TAIL.search(para):
            rep.hits.append(Hit(src, "афористичная концовка", para[-60:].strip()))
        else:
            sentences = [s for s in RE_SENT_SPLIT.split(para) if s.strip()]
            if len(sentences) > 1 and len(sentences[-1].split()) <= 8:
                rep.hits.append(Hit(src, "афористичная концовка", sentences[-1][:60]))

    # триады: списки ровно из трёх пунктов + перечисления «X, Y и Z»
    bullets = 0
    for line in lines + [""]:
        if re.match(r"\s*[*\-+]\s+\S", line) or re.match(r"\s*\d+\.\s+\S", line):
            bullets += 1
        else:
            if bullets == 3:
                rep.triad_lists += 1
            bullets = 0
    for i, line in enumerate(lines, start=1):
        for m in RE_TRIAD.finditer(line):
            rep.hits.append(Hit(line_map.get(i, i), "триада «X, Y и Z»", m.group(0)))

    # затухание конкретики: цифры, `код` и Имена в первой половине против второй
    half = len(text) // 2
    rep.concrete_first = len(RE_CONCRETE.findall(text[:half]))
    rep.concrete_second = len(RE_CONCRETE.findall(text[half:]))

    if verbose:
        for h in sorted(rep.hits, key=lambda x: x.line):
            print(f"    {path.name}:{h.line}  [{h.marker}] {h.text}")

    return rep


def print_report(reports: list[Report], verbose: bool) -> None:
    print("=" * 78)
    print("МАРКЕРЫ LLM-ТЕКСТА — сводка")
    print("=" * 78)
    print(f"{'файл':<34}{'слов':>6}{'тире/100':>10}{'разброс':>9}{'находок':>9}")
    print("-" * 78)

    for r in reports:
        flag = " !" if r.dash_density > 5 else ""
        print(f"{r.path.name:<34}{r.words:>6}{r.dash_density:>9.1f}{flag:<1}"
              f"{r.para_spread:>9.1f}{len(r.hits):>9}")

    print("-" * 78)
    total = sum(len(r.hits) for r in reports)
    all_words = sum(r.words for r in reports)
    all_dashes = sum(r.dashes for r in reports)
    print(f"{'ИТОГО':<34}{all_words:>6}"
          f"{100.0 * all_dashes / all_words if all_words else 0:>9.1f} "
          f"{'':>8}{total:>9}")

    print("\nПо маркерам:")
    markers: dict[str, int] = {}
    for r in reports:
        for h in r.hits:
            markers[h.marker] = markers.get(h.marker, 0) + 1
    for marker, n in sorted(markers.items(), key=lambda kv: -kv[1]):
        print(f"  {n:>3}  {marker}")

    triads = sum(r.triad_lists for r in reports)
    if triads:
        print(f"  {triads:>3}  списки ровно из трёх пунктов")

    print("\nЗатухание конкретики (цифры, `код`, Имена — 1-я половина → 2-я):")
    for r in reports:
        a, b = r.concrete_first, r.concrete_second
        warn = "  <- проседает" if a and b < a * 0.5 else ""
        print(f"  {r.path.name:<34}{a:>4} → {b:<4}{warn}")

    if not verbose:
        print("\nЗапустите с --verbose, чтобы увидеть каждую находку с номером строки.")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("paths", nargs="*", help=f"файлы .md (по умолчанию {DEFAULT_GLOB})")
    ap.add_argument("--verbose", "-v", action="store_true", help="показать каждую находку")
    args = ap.parse_args()

    if args.paths:
        files = [Path(p) for p in args.paths]
    else:
        files = sorted(REPO.glob(DEFAULT_GLOB))

    files = [f for f in files if f.is_file()]
    if not files:
        print("Нечего проверять: файлы не найдены.", file=sys.stderr)
        return 1

    reports = []
    for f in files:
        if args.verbose:
            print(f"\n  {f}")
        reports.append(scan(f, verbose=args.verbose))

    print()
    print_report(reports, args.verbose)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
