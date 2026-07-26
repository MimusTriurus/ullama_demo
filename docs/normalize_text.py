#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Нормализация markdown-черновиков статьи.

Зачем: внешние редакторы (в т.ч. веб-редактор Хабра и всякие WYSIWYG) любят
подсовывать HTML-сущности, неразрывные пробелы и невидимые символы. Часть из
этого не видно глазом, но она ломает рендер и портит diff.

По умолчанию скрипт НИЧЕГО не пишет — показывает, что собирается сделать.
Применить: --write.

    py docs/normalize_text.py                 # что не так (dry-run)
    py docs/normalize_text.py --write         # починить безопасное
    py docs/normalize_text.py --quotes --write # плюс «ёлочки» -> "

Безопасные починки (всегда):
  * HTML-сущности: &#x20; &#x43A; &amp; &nbsp; -> реальные символы
  * битые ударения после декода: `**текст **слово` -> `**текст** слово`
  * NBSP (U+00A0) и узкий NBSP (U+202F) -> обычный пробел
  * невидимки: BOM, zero-width space, мягкий перенос, LRM/RLM
  * нормализация Unicode в NFC (одна и та же буква — один код)

Типографика (по флагам, только вне блоков кода):
  --quotes    « » “ ” „ ‟ -> "
  --arrows    → -> ->,  ← -> <-
  --ellipsis  … -> ...
  --dashes    – (en dash) -> — (em dash);  em dash НЕ трогаем, он осмысленный
  --emoji     выкинуть эмодзи

Мойбейк (кракозябры вида â€") чинится через ftfy, если он установлен: --mojibake
"""

from __future__ import annotations

import argparse
import html
import re
import sys
import unicodedata
from dataclasses import dataclass, field
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

REPO = Path(__file__).resolve().parent.parent
DEFAULT_GLOB = "docs/parts/*.md"

# ── невидимое и пробельное ──────────────────────────────────────────────────

INVISIBLE = {
    "﻿": "BOM",
    "​": "zero-width space",
    "‌": "zero-width non-joiner",
    "­": "мягкий перенос",
    "‎": "LRM",
    "‏": "RLM",
}
SPACES = {" ": "NBSP", " ": "узкий NBSP", " ": "figure space"}

# ── типографика ─────────────────────────────────────────────────────────────

QUOTES = {"«": '"', "»": '"', "“": '"', "”": '"', "„": '"', "‟": '"'}
ARROWS = {"→": "->", "←": "<-", "⇒": "=>"}
ELLIPSIS = {"…": "..."}
DASHES = {"–": "—"}                     # en dash -> em dash
RE_EMOJI = re.compile(
    "[\U0001F300-\U0001FAFF\U00002600-\U000027BF\U0001F1E6-\U0001F1FF]️?"
)

RE_ENTITY = re.compile(r"&(#x[0-9A-Fa-f]+|#\d+|[A-Za-z][A-Za-z0-9]{1,31});")
RE_FENCE = re.compile(r"^\s*```")
RE_LIST_ITEM = re.compile(r"^(\s*(?:[*+-]|\d+\.)\s+)")
RE_HR = re.compile(r"^\s*([*_-])(?:\s*\1){2,}\s*$")
RE_ASTERISKS = re.compile(r"\*+")


@dataclass
class Change:
    line: int
    kind: str
    before: str
    after: str


@dataclass
class FileResult:
    path: Path
    text: str
    changes: list[Change] = field(default_factory=list)

    @property
    def dirty(self) -> bool:
        return bool(self.changes)


def fix_emphasis_spacing(line: str) -> str:
    """Чинит пробел, прилипший к маркеру выделения.

    После декода `**знает&#x20;**&#x43E;б` превращается в `**знает **об` —
    закрывающий `**` с пробелом перед ним markdown не считает закрывающим.
    Правило безопасное: срабатывает только на НЕвалидном выделении,
    корректную разметку тронуть не может.
    """
    if RE_HR.match(line):
        return line

    prefix = ""
    m = RE_LIST_ITEM.match(line)               # маркер списка — не выделение
    if m:
        prefix, line = m.group(1), line[m.end():]

    out: list[str] = []
    pos = 0
    open_runs: dict[int, bool] = {}

    for run in RE_ASTERISKS.finditer(line):
        marker = run.group(0)
        n = len(marker)
        chunk = line[pos:run.start()]
        closing = open_runs.get(n, False)
        open_runs[n] = not closing

        # Одиночную `*` не трогаем: вложенный курсив внутри курсива markdown
        # разбирает неоднозначно, и «закрывающая по чётности» звёздочка вполне
        # может оказаться открывающей по замыслу автора.
        if n < 2:
            out.append(chunk)
            out.append(marker)
            pos = run.end()
            continue

        if closing and chunk.endswith(" ") and chunk.strip():
            out.append(chunk.rstrip(" "))       # пробел уезжает за маркер
            out.append(marker)
            out.append(" ")
        elif not closing and line[run.end():run.end() + 1] == " ":
            out.append(chunk)
            out.append(" " if chunk and not chunk.endswith(" ") else "")
            out.append(marker)
            pos = run.end() + 1                 # проглотили пробел после маркера
            continue
        else:
            out.append(chunk)
            out.append(marker)

        pos = run.end()

    out.append(line[pos:])
    return prefix + "".join(out)


def process(path: Path, opts: argparse.Namespace) -> FileResult:
    original = path.read_text(encoding="utf-8")
    res = FileResult(path=path, text=original)

    if opts.mojibake:
        try:
            import ftfy
        except ImportError:
            print("  ftfy не установлен: pip install ftfy", file=sys.stderr)
        else:
            fixed = ftfy.fix_text(res.text)
            if fixed != res.text:
                res.changes.append(Change(0, "mojibake", "ftfy.fix_text", "применён"))
                res.text = fixed

    lines = res.text.splitlines(keepends=True)
    in_fence = False
    result: list[str] = []

    for num, line in enumerate(lines, start=1):
        if RE_FENCE.match(line):
            in_fence = not in_fence
            result.append(line)
            continue

        new = line

        # 1. HTML-сущности + починка разметки после них (только вне кода)
        if not in_fence and RE_ENTITY.search(new):
            decoded = html.unescape(new)
            repaired = fix_emphasis_spacing(decoded)
            res.changes.append(Change(num, "HTML-сущность", new.strip()[:70],
                                      repaired.strip()[:70]))
            new = repaired

        # 2. невидимки — везде, включая код
        for ch, name in INVISIBLE.items():
            if ch in new:
                res.changes.append(Change(num, f"невидимка ({name})", repr(ch), "удалён"))
                new = new.replace(ch, "")

        # 3. экзотические пробелы -> обычный
        for ch, name in SPACES.items():
            if ch in new:
                res.changes.append(Change(num, f"пробел ({name})", repr(ch), "' '"))
                new = new.replace(ch, " ")

        # 4. типографика — по флагам; в блоки кода лезем только с --in-code
        if not in_fence or opts.in_code:
            table: dict[str, str] = {}
            # Кавычки внутри блоков кода не трогаем даже с --in-code: подписи нод
            # в mermaid сами обёрнуты в ", и «текст» -> "текст" даёт "" — синтаксис
            # рушится. Кавычки в схемах убираются руками.
            if opts.quotes and not in_fence:
                table.update(QUOTES)
            if opts.arrows:
                table.update(ARROWS)
            if opts.ellipsis:
                table.update(ELLIPSIS)
            if opts.dashes:
                table.update(DASHES)
            for ch, repl in table.items():
                if ch in new:
                    res.changes.append(Change(num, f"типографика ({ch} -> {repl})",
                                              new.strip()[:50], ""))
                    new = new.replace(ch, repl)
            if opts.emoji and RE_EMOJI.search(new):
                res.changes.append(Change(num, "эмодзи", new.strip()[:50], "удалены"))
                new = RE_EMOJI.sub("", new)
                new = re.sub(r"  +", " ", new)

        result.append(new)

    text = "".join(result)

    # 5. NFC — одна буква = один код (иначе «й» бывает двумя символами)
    nfc = unicodedata.normalize("NFC", text)
    if nfc != text:
        res.changes.append(Change(0, "нормализация", "NFD/составные", "NFC"))
        text = nfc

    res.text = text
    return res


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("paths", nargs="*", help=f"файлы .md (по умолчанию {DEFAULT_GLOB})")
    ap.add_argument("--write", action="store_true", help="записать изменения на диск")
    ap.add_argument("--quotes", action="store_true", help="«ёлочки» и “лапки” -> \"")
    ap.add_argument("--arrows", action="store_true", help="→ -> ->")
    ap.add_argument("--ellipsis", action="store_true", help="… -> ...")
    ap.add_argument("--dashes", action="store_true", help="– (en dash) -> — (em dash)")
    ap.add_argument("--emoji", action="store_true", help="выкинуть эмодзи")
    ap.add_argument("--in-code", action="store_true",
                    help="применять типографику и внутри блоков кода "
                         "(нужно, чтобы подписи на mermaid-схемах совпадали с прозой)")
    ap.add_argument("--mojibake", action="store_true", help="прогнать через ftfy")
    opts = ap.parse_args()

    files = [Path(p) for p in opts.paths] if opts.paths else sorted(REPO.glob(DEFAULT_GLOB))
    files = [f for f in files if f.is_file()]
    if not files:
        print("Нечего обрабатывать: файлы не найдены.", file=sys.stderr)
        return 1

    total = 0
    touched = 0

    for f in files:
        res = process(f, opts)
        if not res.dirty:
            continue

        touched += 1
        total += len(res.changes)
        print(f"\n{f.relative_to(REPO) if REPO in f.parents else f}")
        for c in res.changes:
            where = f":{c.line}" if c.line else ""
            print(f"  {where:<6} [{c.kind}]")
            if c.before and c.after:
                print(f"         было:  {c.before}")
                print(f"         стало: {c.after}")

        if opts.write:
            f.write_text(res.text, encoding="utf-8", newline="")

    print()
    if not total:
        print("Чисто: править нечего.")
    elif opts.write:
        print(f"Записано: {total} правок в {touched} файл(ах).")
    else:
        print(f"Найдено {total} правок в {touched} файл(ах). "
              f"Это dry-run — примените с --write.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
