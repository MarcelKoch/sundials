#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# Programmer(s): Cody Balos and David J. Gardner @ LLNL
# -----------------------------------------------------------------------------
# SUNDIALS Copyright Start
# Copyright (c) 2002-2024, Lawrence Livermore National Security
# and Southern Methodist University.
# All rights reserved.
#
# See the top-level LICENSE and NOTICE files for details.
#
# SPDX-License-Identifier: BSD-3-Clause
# SUNDIALS Copyright End
# -----------------------------------------------------------------------------
# Module of Python functions that may be useful to SUNDIALS developers writing
# scripts to parse logs produced by SUNLogger.
# -----------------------------------------------------------------------------

import re
import numpy as np

def convert_to_num(s):
    """Try to convert a string to an int or float"""

    try:
        return np.longlong(s)
    except ValueError:
        try:
            return np.double(s)
        except ValueError:
            return s

def parse_logfile_payload(payload, line_number, all_lines, array_indicator="(:)"):
    """
    This function parses the payload of in a SUNDIALS log file line
    into a dictionary. The payload of a SUNDIALS log file line
    is the part after all the [ ] brackets.
    """
    kvpstrs = payload.split(",")
    kvp_dict = {}
    for kvpstr in kvpstrs:
        kvp = kvpstr.split("=")
        if len(kvp) == 1:
            kvp_dict[kvp[0].strip()] = ""
        else:
            key, value = kvp
            values = []
            if array_indicator in key:
                for line in all_lines[line_number + 1 :]:
                    if line.startswith("["):
                        break
                    values.append(np.double(line))
                kvp_dict[key.strip()] = values
            else:
                kvp_dict[key.strip()] = value.strip()
    return kvp_dict


def parse_logfile_line(line, line_number, all_lines):
    """
    This function takes a line from a SUNDIALS log file and parses it into a dictionary.
    A log file line has the form:
      [loglvl][rank][scope][label] key1 = value, key2 = value
    Log file payloads can be multiline if they are an array/vector with one value per line.
    I.e.
      [loglvl][rank][scope][label] y(:)
      y_1
      y_2
      ...
    """
    pattern = re.compile(r'\[(\w+)\]\[(rank \d+)\]\[(.*)\]\[(.*)\](.*)')
    matches = pattern.findall(line)
    line_dict = {}
    if matches:
        line_dict["loglvl"] = matches[0][0]
        line_dict["rank"] = convert_to_num(matches[0][1].split()[1])
        line_dict["scope"] = matches[0][2]
        line_dict["label"] = matches[0][3]
        line_dict["payload"] = parse_logfile_payload(
            matches[0][4], line_number, all_lines
        )
    return line_dict


def log_file_to_list(filename):
    """
    This function takes a SUNDIALS log file and creates a list where each list
    element represents an integrator step attempt.

    E.g.,
      [
          {
              "loglvl": "DEBUG",
              "rank": "rank 0",
              "scope": "<step_scope_txt>",
              "label": "enter-step-attempt-loop",
              "payload": {"step": "0", "h": "1e-06", "q": "1", "t_n": "0"},
          }, ...
      ]
    """
    with open(filename, "r") as logfile:

        # List of dictionaries one for each step attempt
        log = []

        # Stack of logs for integrators composed of other integrators e.g., MRI
        # methods or splitting methods
        log_stack = []
        log_stack.append(log)

        # Level for nested integrators e.g., MRI methods
        level = 0

        all_lines = logfile.readlines()
        for line_number, line in enumerate(all_lines):
            line_dict = parse_logfile_line(line.rstrip(), line_number, all_lines)
            if not line_dict:
                continue

            line_dict["level"] = level

            if (line_dict["label"] == "begin-fast-steps"):
                level += 1
                # Possibly multiple fast integrations per step so check if a
                # list already exists
                key = f"time-level-{level}"
                if key not in log_stack[-1][-1]:
                    log_stack[-1][-1][key] = []
                log_stack.append(log_stack[-1][-1][key])
                continue
            elif (line_dict["label"] == "end-fast-steps"):
                level -= 1
                log_stack.pop()
                continue

            if line_dict["label"] == "begin-step-attempt":
                log_stack[-1].append(line_dict)
            elif line_dict["label"] == "end-step-attempt":
                log_stack[-1][-1]["payload"].update(line_dict["payload"])

    return log


def get_history(log, key, step_status = None, time_range = None,
                step_range = None):
    """
    This function extracts the step/time series of the requested value.
    """

    steps = []
    times = []
    values = []

    for l in log:

        step = np.longlong(l["payload"]['step'])
        time = np.double(l["payload"]['t_n'])

        if time_range is not None:
            if time > time_range[1] or time < time_range[0]:
                continue

        if step_range is not None:
            if step > step_range[1] or step < step_range[0]:
                continue

        if step_status is not None:
            if step_status not in l["payload"]['status']:
                continue

        steps.append(step)
        times.append(time)
        values.append(convert_to_num(l["payload"][key]))

        next_level_key = f'time-level-{l["level"] + 1}'

        if next_level_key in l:
            sub_steps, sub_times, sub_values = get_history(l[next_level_key], key)
            steps.extend(sub_steps)
            times.extend(sub_times)
            values.extend(sub_values)

    return steps, times, values
