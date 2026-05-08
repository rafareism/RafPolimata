#!/usr/bin/env python3
"""Valida o YAML canônico de coerência sem depender de PyYAML.

O objetivo é garantir que o protocolo permaneça operacionalmente auditável:
- 50 fórmulas registradas, uma para cada equação/semente fornecida;
- 7 dimensões do vetor toroidal;
- 42 atratores;
- gates mínimos de pipeline presentes;
- arquivos referenciados existem no repositório.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PROTOCOL = ROOT / "configs" / "semantic_coherence.yml"


def _read(path: Path) -> str:
    if not path.exists():
        raise SystemExit(f"protocol_not_found: {path}")
    return path.read_text(encoding="utf-8")


def _scalar_int(text: str, key: str) -> int:
    match = re.search(rf"^\s*{re.escape(key)}:\s*(\d+)\b", text, re.MULTILINE)
    if not match:
        raise SystemExit(f"missing_integer_key: {key}")
    return int(match.group(1))


def _section_lines(text: str, section: str) -> list[str]:
    lines = text.splitlines()
    start = None
    for index, line in enumerate(lines):
        if line == f"{section}:":
            start = index + 1
            break
    if start is None:
        raise SystemExit(f"missing_section: {section}")

    body: list[str] = []
    for line in lines[start:]:
        if line and not line.startswith((" ", "\t")):
            break
        if line.strip():
            body.append(line)
    return body


def _validate_state_vector(text: str) -> list[str]:
    lines = _section_lines(text, "state_vector")
    values = [line.strip()[2:].strip() for line in lines if line.strip().startswith("- ")]
    expected = ["u", "v", "psi", "chi", "rho", "delta", "sigma"]
    if values != expected:
        raise SystemExit(f"invalid_state_vector: {values!r}")
    return values


def _validate_formulas(text: str) -> list[tuple[str, str]]:
    ids = re.findall(r"- id:\s*(F\d{2});", text)
    expected = [f"F{i:02d}" for i in range(1, 51)]
    if ids != expected:
        raise SystemExit(f"invalid_formula_registry: expected {expected!r}, got {ids!r}")

    refs = re.findall(r"implementation:\s*([^;\n]+)", text)
    missing = []
    for ref in refs:
        candidate = ROOT / ref.strip()
        if not candidate.exists():
            missing.append(ref.strip())
    if missing:
        raise SystemExit("missing_implementation_refs: " + ", ".join(sorted(set(missing))))
    return list(zip(ids, refs))


def _validate_gates(text: str) -> list[str]:
    gates = re.findall(r"- id:\s*(G\d+);\s*command:\s*([^;\n]+);", text)
    if len(gates) < 4:
        raise SystemExit(f"insufficient_pipeline_gates: {len(gates)}")
    required = {
        "python3 scripts/validate_coherence_protocol.py",
        "bash RAF_host_syntax_check.sh",
        "python3 scripts/first_test_pk.py --output results/first_test_report.json",
    }
    commands = {command.strip() for _, command in gates}
    missing = required - commands
    if missing:
        raise SystemExit("missing_pipeline_commands: " + ", ".join(sorted(missing)))
    return [gate_id for gate_id, _ in gates]


def validate(path: Path = DEFAULT_PROTOCOL) -> dict[str, object]:
    text = _read(path)
    torus_dimensions = _scalar_int(text, "torus_dimensions")
    attractor_count = _scalar_int(text, "attractor_count")
    alpha_q16 = _scalar_int(text, "alpha_q16")
    entropy_milli_max = _scalar_int(text, "entropy_milli_max")

    if torus_dimensions != 7:
        raise SystemExit(f"invalid_torus_dimensions: {torus_dimensions}")
    if attractor_count != 42:
        raise SystemExit(f"invalid_attractor_count: {attractor_count}")
    if alpha_q16 != 16384:
        raise SystemExit(f"invalid_alpha_q16: {alpha_q16}")
    if entropy_milli_max > 9000:
        raise SystemExit(f"invalid_entropy_milli_max: {entropy_milli_max}")

    state_vector = _validate_state_vector(text)
    formulas = _validate_formulas(text)
    gates = _validate_gates(text)
    return {
        "protocol": str(path.relative_to(ROOT)),
        "state_dimensions": len(state_vector),
        "formula_count": len(formulas),
        "pipeline_gate_count": len(gates),
        "verdict": "PASS",
    }


def main() -> None:
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PROTOCOL
    result = validate(path if path.is_absolute() else ROOT / path)
    print(result)


if __name__ == "__main__":
    main()
