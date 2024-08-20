from enum import Enum
from pathlib import Path
import re


class CheckResult(Enum):
    FAIL = False
    SUCCESS = True


def check_guards(path: Path, content: str):
    assert(path.suffix == '.h')
    content = content.strip()
    content_lines = [line for line in content.splitlines(keepends=False) if line]
    base_path = Path(__file__).resolve().parent
    path = path.relative_to(base_path)
    guard = '_'.join(re.sub(r'[^A-Z0-9]', '_', part.upper()) for part in path.parts)
    if content_lines[0] != f'#ifndef {guard}' or content_lines[1] != f'#define {guard}' or content_lines[-1] != f'#endif //{guard}':
        print(f'{path}: Bad or missing guard. Expected {guard}.')
        return CheckResult.FAIL
    return CheckResult.SUCCESS


def check_test_group_name(path: Path, content: str):
    assert(path.name.endswith('_test.cpp'))
    content = content.strip()
    content_lines = [line for line in content.splitlines(keepends=False) if line]
    base_path = Path(__file__).resolve().parent
    path = path.relative_to(base_path)
    def snake_to_pascal(text):
        return ''.join(word.capitalize() for word in text.split('_'))
    group_parts = []
    for part in path.parts[:-1]:
        group_parts.append(re.sub(r'[^a-zA-Z0-9]', '_', part)[0])
    group_parts.append(re.sub(r'[^a-zA-Z0-9]', '_', path.stem))
    group = '_'.join(group_parts)
    group = snake_to_pascal(group)
    group_found_set = set()
    result = CheckResult.SUCCESS
    for line in content_lines:
        for group_found, _ in re.findall(r'TEST\((\w*?), (\w*?)\)', line):
            if group_found != group and group_found not in group_found_set:
                group_found_set.add(group_found)
                print(f'{path}: Bad or missing test group. Expected {group}.')
                result = CheckResult.FAIL
    return result


def main():
    result = CheckResult.SUCCESS
    base_path = Path(__file__).resolve().parent / 'offbynull'
    for source_path in base_path.rglob('*'):
        if source_path.suffix == '.h':
            if check_guards(source_path, source_path.read_text()) == CheckResult.FAIL:
                result = CheckResult.FAIL
        if source_path.name.endswith('_test.cpp'):
            if check_test_group_name(source_path, source_path.read_text()) == CheckResult.FAIL:
                result = CheckResult.FAIL
    return result


if __name__ == '__main__':
    result = main()
    exit(0 if result == CheckResult.SUCCESS else 1)