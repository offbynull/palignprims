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


SYSTEM_TYPE_TO_HEADER_MAPPING = {
    'std::ranges::range': 'ranges',
    'std::ranges::forward_range': 'ranges',
    'std::ranges::bidirectional_range': 'ranges',
    'std::ranges::random_access_range': 'ranges',
    'std::ranges::range_reference_t': 'ranges',
    'std::ranges::range_value_t': 'ranges',
    'std::ranges::distance': 'ranges',
    'std::ranges::sort': 'algorithm',
    'std::ranges::max_element': 'ranges',
    'std::ranges::begin': 'ranges',
    'std::ranges::end': 'ranges',
    'std::ranges::single_view': 'ranges',
    'std::ranges::common_view': 'ranges',
    'std::ranges::view_interface': 'ranges',
    'std::views::empty': 'ranges',
    'std::views::single': 'ranges',
    'std::views::transform': 'ranges',
    'std::views::filter': 'ranges',
    'std::views::iota': 'ranges',
    'std::views::cartesian_product': 'ranges',
    'std::views::take': 'ranges',
    'std::views::drop': 'ranges',
    'std::views::drop_while': 'ranges',
    'std::views::split': 'ranges',
    'std::views::common': 'ranges',
    'std::views::reverse': 'ranges',
    'std::views::concat': 'ranges',
    'std::views::enumerate': 'ranges',
    'std::views::all': 'ranges',
    'std::same_as': 'concepts',
    'std::floating_point': 'concepts',
    'std::integral': 'concepts',
    'std::unsigned_integral': 'concepts',
    'std::regular': 'concepts',
    'std::semiregular': 'concepts',
    'std::is_void_v': 'type_traits',
    'std::is_same_v': 'type_traits',
    'std::is_convertible_v': 'type_traits',
    'std::is_rvalue_reference_v': 'type_traits',
    'std::remove_reference_t': 'type_traits',
    'std::invoke_result_t': 'type_traits',
    'std::decay_t': 'type_traits',
    'std::numeric_limits': 'limits',
    'std::begin': 'iterator',
    'std::end': 'iterator',
    'std::input_iterator': 'iterator',
    'std::input_iterator_tag': 'iterator',
    'std::forward_iterator': 'iterator',
    'std::forward_iterator_tag': 'iterator',
    'std::bidirectional_iterator': 'iterator',
    'std::bidirectional_iterator_tag': 'iterator',
    'std::sentinel_for': 'iterator',
    'std::lower_bound': 'algorithm',
    'std::reverse': 'algorithm',
    'std::sort': 'algorithm',
    'std::mt19937_64': 'random',
    'std::uniform_int_distribution': 'random',
    'std::uniform_real_distribution': 'random',
    'std::abs': 'cmath',
    'std::hypot': 'cmath',
    'std::size_t': 'cstddef',
    'std::int8_t': 'cstdint',
    'std::int16_t': 'cstdint',
    'std::int32_t': 'cstdint',
    'std::int64_t': 'cstdint',
    'std::uint8_t': 'cstdint',
    'std::uint16_t': 'cstdint',
    'std::uint32_t': 'cstdint',
    'std::uint64_t': 'cstdint',
    'std::ptrdiff_t': 'cstdint',
    'std::uintmax_t': 'cstdint',
    'std::float16_t': 'stdfloat',
    'std::float32_t': 'stdfloat',
    'std::float64_t': 'stdfloat',
    'std::string': 'string',
    'std::to_string': 'string',
    'std::string_view': 'string_view',
    'std::isalnum': 'cctype',
    'std::formatter': 'format',
    'std::format': 'format',
    'std::format_to': 'format',
    'std::format_context': 'format',
    'std::format_parse_context': 'format',
    'std::format_error': 'format',
    'std::optional': 'optional',
    'std::nullopt': 'optional',
    'std::nullopt_t': 'optional',
    'std::reference_wrapper': 'functional',
    'std::ref': 'functional',
    'std::function': 'functional',
    'std::less': 'functional',
    'std::make_pair': 'utility',
    'std::pair': 'utility',
    'std::declval': 'utility',
    'std::forward': 'utility',
    'std::move': 'utility',
    'std::unreachable': 'utility',
    'std::tuple': 'tuple',
    'std::tuple_cat': 'tuple',
    'std::make_tuple': 'tuple',
    'std::tie': 'tuple',
    'std::get': 'tuple',
    'std::apply': 'tuple',
    'std::variant': 'variant',
    'std::get_if': 'variant',
    'std::holds_alternative': 'variant',
    'std::strong_ordering': 'compare',
    'std::vector': 'vector',
    'std::array': 'array',
    'std::map': 'map',
    'std::set': 'set',
    'std::multiset': 'set',
    'std::runtime_error': 'stdexcept',
    'std::cout': 'iostream',
    'std::ofstream': 'fstream',
    'std::ostream': 'ostream',
    'std::endl': 'ostream',
    'std::istringstream': 'sstream',
    'std::ios::binary': 'ios',
    'boost::container::small_vector': 'boost/container/small_vector.hpp',
    'boost::container::static_vector': 'boost/container/static_vector.hpp',
    'boost::container::static_vector_options': 'boost/container/static_vector.hpp',
    'boost::container::throw_on_overflow': 'boost/container/options.hpp',
    'boost::container::inplace_alignment': 'boost/container/options.hpp',
    'boost::filesystem::path': 'boost/filesystem.hpp',
    'boost::filesystem::temp_directory_path': 'boost/filesystem.hpp',
    'boost::filesystem::unique_path': 'boost/filesystem.hpp',
    'boost::iostreams::mapped_file_source': 'boost/iostreams/device/mapped_file.hpp',
    'boost::iostreams::mapped_file_base::readonly': 'boost/iostreams/device/mapped_file.hpp',
    'boost::safe_numerics': 'boost/safe_numerics/safe_integer.hpp',
    'boost::safe_numerics::safe_numerics_error': 'boost/safe_numerics/safe_integer.hpp',
}


def check_system_includes(path: Path, content: str):
    assert(path.suffix in {'.cpp', '.h'})
    content = content.strip()
    content_lines = [line for line in content.splitlines(keepends=False) if line]
    base_path = Path(__file__).resolve().parent
    path = path.relative_to(base_path)
    result = CheckResult.SUCCESS
    found_includes = set()
    expected_includes = set()
    usages = set()
    for line in content_lines:
        for include in re.findall(r'#include <([\w/\.]+)>', line):
            found_includes.add(include)
        for usage in re.findall(r'((?:std|boost)::[\w:]+)', line):
            if usage in usages:
                continue
            usages.add(usage)
            expected_include = SYSTEM_TYPE_TO_HEADER_MAPPING.get(usage, None)
            expected_includes.add(expected_include)
            if expected_include is None:
                print(f'{path}: No include mapping for {usage}')
                result = CheckResult.FAIL
            elif expected_include not in found_includes:
                print(f'{path}: {usage} requires include {expected_include}')
                result = CheckResult.FAIL
    excess_includes = found_includes - expected_includes
    if excess_includes:
        for excess_include in excess_includes:
            print(f'{path}: {excess_include} included but not required')
            result = CheckResult.FAIL
    return result


def check_line_length(path: Path, content: str):
    assert(path.suffix in {'.cpp', '.h'})
    content = content.strip()
    content_lines = [line for line in content.splitlines(keepends=False) if line]
    result = CheckResult.SUCCESS
    for line in content_lines:
        if len(line) > 140:
            print(f'{path}: line exceeds 140 chars')
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
        if source_path.suffix in {'.cpp', '.h'}:
            if check_system_includes(source_path, source_path.read_text()) == CheckResult.FAIL:
                result = CheckResult.FAIL
            if check_line_length(source_path, source_path.read_text()) == CheckResult.FAIL:
                result = CheckResult.FAIL
    return result


if __name__ == '__main__':
    result = main()
    exit(0 if result == CheckResult.SUCCESS else 1)