#!/usr/bin/env python3
"""Convert BuyDisplay gslX680_311_5_F.h into the module's firmware source.

Usage: python3 tools/gsl1680_fw_convert.py ~/Downloads/gslX680_311_5_F.h
"""
from pathlib import Path
import argparse
import re


def convert(source: str) -> tuple[list[tuple[int, int]], str]:
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    match = re.search(r"\bGSLX680_FW\s*\[\s*\]\s*=\s*\{", source)
    if not match:
        raise ValueError("Could not locate GSLX680_FW[] in the header")
    tail = source[match.end():]
    end = tail.find("};")
    if end == -1:
        raise ValueError("Missing end of GSLX680_FW[]")
    body = tail[:end]
    entries = re.findall(r"\{\s*(0[xX][0-9a-fA-F]+|\d+)\s*,\s*(0[xX][0-9a-fA-F]+|\d+)\s*\}", body)
    if len(entries) < 128:
        raise ValueError(f"Only {len(entries)} entries found; expected a full firmware image")

    parsed = []
    for address, value in entries:
        reg, word = int(address, 0), int(value, 0)
        if reg != 0xF0 and (reg > 0x7C or reg % 4):
            raise ValueError(f"Unexpected GSL register offset 0x{reg:X}")
        if not 0 <= word <= 0xFFFFFFFF:
            raise ValueError("Firmware word exceeds uint32_t")
        parsed.append((reg, word))
    if not any(reg == 0xF0 for reg, _ in parsed):
        raise ValueError("Firmware does not contain page selectors")

    lines = ['#include "touch_gsl1680.h"', '',
             '/* Generated from a vendor-provided, panel-specific firmware file. */',
             'const gsl1680_fw_entry_t gsl1680_firmware[] = {']
    for reg, value in parsed:
        lines.append(f'    {{0x{reg:02X}U, 0x{value:08X}U}},')
    lines.extend(['};', '',
                  'const size_t gsl1680_firmware_count =',
                  '    sizeof(gsl1680_firmware) / sizeof(gsl1680_firmware[0]);', ''])
    return parsed, '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--output', type=Path,
                        default=Path('firmware/touch_gsl1680/src/touch_gsl1680_firmware.c'))
    args = parser.parse_args()
    entries, generated = convert(args.source.read_text(encoding='utf-8', errors='replace'))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding='utf-8')
    print(f'Wrote {len(entries)} firmware words to {args.output}')


if __name__ == '__main__':
    main()
