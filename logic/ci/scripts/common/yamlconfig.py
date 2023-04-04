#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import yaml
import jsonschema


def read_config_with_defaults(path, schema, defaults, *, path_may_be_none=True):
    if path:
        with open(path, 'r') as f:
            config = yaml.safe_load(f) or {}
            jsonschema.validate(config, schema)
    elif path_may_be_none:
        config = {}
    else:
        raise RuntimeError('config must be given')

    return {**defaults, **config}
