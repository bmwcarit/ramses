#  -------------------------------------------------------------------------
#  Copyright (C) 2016 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import test_classes
import log
import re
from ramses_test_framework import application

def ramses_process_check(target):
    ramsesProcesses = target.get_process_list("ramses")
    if len(ramsesProcesses) > 0:
        log.warning("RAMSES processes running before test startup on target {}. Process list: {}".format(target.name, ramsesProcesses))


def with_ramses_process_check(setup_func):  # decorator function
    def extended_setup_func(self):
        if isinstance(self, test_classes.OneConnectionTest):
            ramses_process_check(self.target)
        elif isinstance(self, test_classes.MultipleConnectionsTest):
            for target in self.targets:
                ramses_process_check(target)
        # continue with setup function of test
        setup_func(self)
    return extended_setup_func


def ensureSystemCompositorRoundTrip(renderer, ivisurfaceid):
    watchRenderer = renderer.start_watch_stdout()
    renderer.send_ramsh_command("scSetSurfaceOpacity {0} 0.5".format(ivisurfaceid))
    renderer.send_ramsh_command("scSetSurfaceOpacity {0} 1.0".format(ivisurfaceid))
    opacityExecutedInSystemCompositor = renderer.wait_for_msg_in_stdout(watchRenderer, "IVIControllerSurface::HandleOpacityCallBack ivi-id: {0} opacity: 256".format(ivisurfaceid), timeout=application.Application.DEFAULT_WAIT_FOR_MESSAGE_TIMEOUT)
    assert(opacityExecutedInSystemCompositor)  # Could not ensure system compositor roundtrip


class IVI_Control(object):

    def __init__(self, target, xdg_runtime_dir):
        self.target          = target
        self.xdg_runtime_dir = "XDG_RUNTIME_DIR=" + xdg_runtime_dir
        self.command_buffer  = []
        self.start_state     = self.retrieveCurrentState()

    def retrieveCurrentState(self):
        self.flush()
        (stdout, _, _) = self.callIVIControl("scene")

        scene_state  = dict()
        section_name = ""
        section_id   = 0
        for line in stdout:
            try:
                # check whether the line starts with a whitespace or not
                # if it does not start with a whitespace we have a new
                # section ("screen", "layer", etc.)
                if not re.match(r'\s', line):
                    line_tokens = line.strip().split()
                    if len(line_tokens) != 2:
                        log.warning("Unexpected line while parsing output from ivi-control: " + line)
                        log.warning("Expect issues (and errors) while running the test(s)")
                        section_name = ""
                        section_id   = 0
                        continue

                    section_name, section_id = line_tokens

                    if not scene_state.has_key(section_name):
                        scene_state[section_name] = dict()

                    scene_state[section_name][section_id] = dict()

                # otherwise we are inside a section, so we add the info
                # to the section's dictionary
                else:
                    assert(section_name != "" and section_id != 0)
                    section_dict = scene_state[section_name][section_id]
                    object_parameter = [ x.strip() for x in line.split(":") ]
                    section_dict[object_parameter[0]] = object_parameter[1]
            except Exception:
                log.warning("ivi: Failed while processing line: " + line)
                log.warning("Full stdout of command:")
                for l in stdout:
                    log.warning(l)
                raise
        return scene_state

    def cleanup(self):
        current_state = self.retrieveCurrentState()

        def getDifference(section):
            return set(current_state.get(section,dict()).keys()) - set(self.start_state.get(section,dict()).keys())

        for surface_id in getDifference('surface'): self.destroySurface(surface_id)
        for layer_id in getDifference('layer'): self.destroyLayer(layer_id)

        for k,v in self.start_state.get('screen', dict()).items():
            self._setScreenRenderorder(k,v['layers'])

        for k,v in self.start_state.get('layer', dict()).items():
            self.setLayerRenderorder(k,v['surfaces'])
            self.setLayerVisibility(k, v['visibility'])
        self.flush()

    def flush(self):
        if len(self.command_buffer) > 0:
            cmd = " . ".join(self.command_buffer)
            self.command_buffer = []
            (stdout, stderr, returnCode) = self.callIVIControl(cmd)

            if returnCode != 0:
                log.error("Failed to execute ivi control commands. Return code: {}".format(returnCode))
                log.error("Stdout: '{}'".format(stdout))
                log.error("Stderr: '{}'".format(stderr))
                log.error("Expect further failing tests!")

    def callIVIControl(self, cmd):
        return self.target.execute_on_target(self.xdg_runtime_dir + " ivi-control " + cmd, block=True)

    def getScreenRenderorder(self, screen_id):
        state = self.retrieveCurrentState()
        return state['screen'][str(screen_id)]["layers"]

    def _setScreenRenderorder(self, screen_id, layer_ids):
        cmd = "screen {0} renderorder {1}".format(screen_id, layer_ids)
        self.command_buffer.append(cmd)

    def getScreenIds(self):
        return self.start_state['screen'].keys()

    def createLayer(self, layer_id, width, height):
        cmd = "layer {0} create {1} {2}".format(layer_id, width, height)
        self.command_buffer.append(cmd)

    def destroyLayer(self, layer_id):
        cmd = "layer {0} destroy".format(layer_id)
        self.command_buffer.append(cmd)

    def setLayerVisibility(self, layer_id, visible=True):
        cmd = "layer {0} visibility {1}".format(layer_id, "1" if visible else "0")
        self.command_buffer.append(cmd)

    def getLayerDestRect(self, layer_id):
        state = self.retrieveCurrentState()
        return state['layer'][str(layer_id)]["dst_rect"].replace(",","")

    def setLayerDestRect(self, layer_id, x, y, width, height):
        cmd = "layer {0} dstrect {1} {2} {3} {4}".format(layer_id,x,y,width,height)
        self.command_buffer.append(cmd)

    def getLayerRenderorder(self, layer_id):
        state = self.retrieveCurrentState()
        return state['layer'][str(layer_id)]["surfaces"]

    def getSurfaceIds(self):
        state = self.retrieveCurrentState()
        if 'surface' in state.keys():
            return state['surface'].keys()
        return []

    def setLayerRenderorder(self, layer_id, surface_ids):
        cmd = "layer {0} renderorder {1}".format(layer_id, surface_ids)
        self.command_buffer.append(cmd)

    def destroySurface(self, surface_id):
        cmd = "surface {0} destroy".format(surface_id)
        self.command_buffer.append(cmd)

    def appendLayerToScreenRenderorder(self, screen_id, layer_id):
        current_renderorder = self.getScreenRenderorder(screen_id)
        self._setScreenRenderorder(screen_id, "{} {}".format(current_renderorder, layer_id))

    def printCurrentState(self):
        self.flush()
        (stdout, _, _) = self.callIVIControl("scene")
        log.info("--Start IVI Scene--\n" + "".join(stdout) + "\n--End  IVI Scene--")
