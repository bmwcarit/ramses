#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os
import pytest
import tempfile
import jsonschema

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/../common"))
import yamlconfig  # noqa E402 module level import not at top of file

SCHEMA_DICT = {
    'type': 'object',
    'properties': {
        'include': {
            'type': 'array',
            'items': {
                'type': 'string',
            },
        },
        'exclude': {
            'type': 'array',
            'items': {
                'type': 'string',
            },
        },
    },
    'additionalProperties': False
}

DEFAULTS = {
    'include': ['a', 'b'],
    'exclude': ['c', 'd'],
}


def test_with_empty_config():
    with tempfile.NamedTemporaryFile(mode='w') as f:
        assert {} == yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, {})
        assert DEFAULTS == yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, DEFAULTS)


def test_with_full_config():
    with tempfile.NamedTemporaryFile(mode='w') as f:
        f.write("""
include:
  - xx
  - ^yy
exclude:
  - foo
""")
        f.flush()
        assert {'include': ['xx', '^yy'], 'exclude': ['foo']} == yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, {})


def test_with_partial_config():
    with tempfile.NamedTemporaryFile(mode='w') as f:
        f.write("""
exclude:
  - foo
""")
        f.flush()
        assert {'exclude': ['foo']} == yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, {})
        assert {'include': ['a', 'b'], 'exclude': ['foo']} == yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, DEFAULTS)


def test_validation_fail():
    with tempfile.NamedTemporaryFile(mode='w') as f:
        f.write('foobar:\n  - value\n')
        f.flush()
        with pytest.raises(jsonschema.exceptions.ValidationError):
            yamlconfig.read_config_with_defaults(f.name, SCHEMA_DICT, {})


def test_path_may_be_none():
    assert {} == yamlconfig.read_config_with_defaults(None, SCHEMA_DICT, {})
    assert DEFAULTS == yamlconfig.read_config_with_defaults(None, SCHEMA_DICT, DEFAULTS)
    with pytest.raises(RuntimeError):
        yamlconfig.read_config_with_defaults(None, SCHEMA_DICT, {}, path_may_be_none=False)


def test_fails_on_non_existing_config():
    with pytest.raises(FileNotFoundError):
        yamlconfig.read_config_with_defaults('/tmp/yamlconfig_test_file_should_not_exist', SCHEMA_DICT, {})
