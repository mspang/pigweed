#!/usr/bin/env python3
# Copyright 2020 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
"""Generates a C macro for the PW tokenizer 65599 fixed length hash."""

from __future__ import print_function

import datetime
import os

HASH_CONSTANT = 65599
HASH_NAME = 'pw_tokenizer_65599_fixed_length'
HASH_LENGTHS = 80, 96, 128

FILE_HEADER = """\
// Copyright {year} The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

// AUTOGENERATED - DO NOT EDIT
//
// This file was generated by {script}.
// To make changes, update the script and run it to regenerate the files.
#pragma once

#include <stdint.h>

// {hash_length}-character version of the tokenizer hash function.
//
// The argument must be a string literal. It is concatenated with "" to ensure
// that this is the case.
//
// clang-format off

"""


def generate_pw_tokenizer_65599_fixed_length_hash_macro(hash_length):
    """Generate macro that hashes a string literal using a modified x65599 hash.

    The macros generated by this function only operate on string literals.

    Since macros can only operate on fixed-length strings, the hash macro only
    hashes up to a fixed length, and characters beyond that length are ignored.
    To eliminate some collisions, the length of the string is hashed as if it
    were the first character.

    This hash is calculated with the following equation, where s is the string
    and k is the maximum hash length:

       H(s, k) = len(s) + 65599 * s[0] + 65599^2 * s[1] + ... + 65599^k * s[k-1]

    The hash algorithm is a modified version of the x65599 hash used by the SDBM
    open source project. This hash has the following differences from x65599:
      - Characters are only hashed up to a fixed maximum string length.
      - Characters are hashed in reverse order.
      - The string length is hashed as the first character in the string.

    The code generated by this function is intentionally sparse. Each character
    appears hash_length times per log message, so using fewer characters results
    in faster compilation times.

    Args:
      hash_length: maximum string size to hash; extra characters are ignored

    Returns:
      the macro header file as a string
  """

    first_hash_term = ('(uint32_t)(sizeof(str "") - 1 + '
                       '/* The argument must be a string literal. */ \\\n')

    # Use this to add the aligned backslash at the end of the macro lines.
    line_format = '{{:<{}}}\\\n'.format(len(first_hash_term))

    lines = [
        FILE_HEADER.format(script=os.path.basename(__file__),
                           hash_length=hash_length,
                           year=datetime.date.today().year)
    ]

    lines.append(
        line_format.format('#define {}_{}_HASH(str)'.format(
            HASH_NAME.upper(), hash_length)))
    lines.append('  ' + first_hash_term)  # add indendation and the macro line

    indent = ' ' * len('  (uint32_t)(')
    coefficient_format = '0x{coefficient:0>8x}u'

    # The string will have at least a null terminator
    lines.append(
        line_format.format('{}0x{:0>8x}u * (uint8_t)str[0] +'.format(
            indent, HASH_CONSTANT)))

    # Format string to use for the remaining terms.
    term_format = (
        '{indent}{coefficient} * '
        '(uint8_t)({index} < sizeof(str) ? str[{index}] : 0) +').format(
            indent=indent,
            coefficient=coefficient_format,
            index='{{index:>{}}}'.format(len(str(hash_length - 1))))

    for i in range(1, hash_length):
        coefficient = HASH_CONSTANT**(i + 1) % 2**32
        term = term_format.format(index=i, coefficient=coefficient)
        lines.append(line_format.format(term))

    # Remove the extra + and \ and add the closing )
    lines[-1] = lines[-1].rstrip(' +\\\n') + ')'

    lines.append('\n\n// clang-format on\n')

    return ''.join(lines)


def _main():
    base = os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..', 'public', 'pw_tokenizer',
                     'internal'))

    # Generate macros for hashes of the specified lengths.
    for hash_length in HASH_LENGTHS:
        path = os.path.join(
            base, '{}_{}_hash_macro.h'.format(HASH_NAME, hash_length))

        with open(path, 'w') as header_file:
            header_file.write(
                generate_pw_tokenizer_65599_fixed_length_hash_macro(
                    hash_length))

        print('Generated {}-character hash macro at {}'.format(
            hash_length, path))


if __name__ == '__main__':
    _main()