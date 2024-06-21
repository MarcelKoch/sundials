/* -----------------------------------------------------------------
 * Programmer(s): Cody J. Balos @ LLNL
 * -----------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2024, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -----------------------------------------------------------------
 * SUNDIALS logging class implementation.
 * ----------------------------------------------------------------*/

#ifndef _SUNDIALS_LOGGER_IMPL_H
#define _SUNDIALS_LOGGER_IMPL_H

#include <stdarg.h>
#include <sundials/sundials_logger.h>
#include <sundials/sundials_types.h>

#include "sundials_hashmap_impl.h"
#include "sundials_utils.h"

#define SUNDIALS_LOGGING_ERROR   1
#define SUNDIALS_LOGGING_WARNING 2
#define SUNDIALS_LOGGING_INFO    3
#define SUNDIALS_LOGGING_DEBUG   4
#if SUNDIALS_LOGGING_LEVEL > SUNDIALS_LOGGING_DEBUG
#define SUNDIALS_LOGGING_EXTRA_DEBUG
#endif

#if SUNDIALS_LOGGING_LEVEL >= SUNDIALS_LOGGING_ERROR
#define SUNLogError(logger, scope, label, msg_txt, ...)                 \
  SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_ERROR, scope, label, msg_txt, \
                     __VA_ARGS__)
#define SUNLogErrorIf(condition, logger, scope, label, msg_txt, ...)        \
  do {                                                                      \
    if ((condition))                                                        \
    {                                                                       \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_ERROR, scope, label, msg_txt, \
                         __VA_ARGS__);                                      \
    }                                                                       \
  }                                                                         \
  while (0)
#else
#define SUNLogError(logger, scope, label, msg_txt, ...)
#define SUNLogErrorIf(condition, logger, scope, label, msg_txt, ...)
#endif

#if SUNDIALS_LOGGING_LEVEL >= SUNDIALS_LOGGING_WARNING
#define SUNLogWarning(logger, scope, label, msg_txt, ...)                 \
  SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_WARNING, scope, label, msg_txt, \
                     __VA_ARGS__)
#define SUNLogWarningIf(condition, logger, scope, label, msg_txt, ...)        \
  do {                                                                        \
    if ((condition))                                                          \
    {                                                                         \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_WARNING, scope, label, msg_txt, \
                         __VA_ARGS__);                                        \
    }                                                                         \
  }                                                                           \
  while (0)
#else
#define SUNLogWarning(logger, scope, label, msg_txt, ...)
#define SUNLogWarningIf(condition, logger, scope, label, msg_txt, ...)
#endif

#if SUNDIALS_LOGGING_LEVEL >= SUNDIALS_LOGGING_INFO
#define SUNLogInfo(logger, scope, label, msg_txt, ...)                 \
  SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_INFO, scope, label, msg_txt, \
                     __VA_ARGS__)
#define SUNLogInfoIf(condition, logger, scope, label, msg_txt, ...)        \
  do {                                                                     \
    if ((condition))                                                       \
    {                                                                      \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_INFO, scope, label, msg_txt, \
                         __VA_ARGS__);                                     \
    }                                                                      \
  }                                                                        \
  while (0)
#else
#define SUNLogInfo(logger, scope, label, msg_txt, ...)
#define SUNLogInfoIf(condition, logger, scope, label, msg_txt, ...)
#endif

#if SUNDIALS_LOGGING_LEVEL >= SUNDIALS_LOGGING_DEBUG
#define SUNLogDebug(logger, scope, label, msg_txt, ...)                 \
  SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, \
                     __VA_ARGS__)
#define SUNLogDebugIf(condition, logger, scope, label, msg_txt, ...)        \
  do {                                                                      \
    if ((condition))                                                        \
    {                                                                       \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, \
                         __VA_ARGS__);                                      \
    }                                                                       \
  }                                                                         \
  while (0)
#else
#define SUNLogDebug(logger, scope, label, msg_txt, ...)
#define SUNLogDebugIf(condition, logger, scope, label, msg_txt, ...)
#endif

#ifdef SUNDIALS_LOGGING_EXTRA_DEBUG
#define SUNLogExtraDebug(logger, scope, label, msg_txt, ...)            \
  SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, \
                     __VA_ARGS__)
#define SUNLogExtraDebugIf(condition, logger, scope, label, msg_txt, ...)   \
  do {                                                                      \
    if ((condition))                                                        \
    {                                                                       \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, \
                         __VA_ARGS__);                                      \
    }                                                                       \
  }                                                                         \
  while (0)
#define SUNLogExtraDebugVec(logger, scope, label, msg_txt, vec, ...)      \
  do {                                                                    \
    SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, \
                       __VA_ARGS__);                                      \
    N_VPrintFile(vec, logger->debug_fp);                                  \
  }                                                                       \
  while (0)
#define SUNLogExtraDebugVecIf(condition, logger, scope, label, msg_txt, vec, ...) \
  do {                                                                            \
    if ((condition))                                                              \
    {                                                                             \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt,       \
                         __VA_ARGS__);                                            \
      N_VPrintFile(vec, logger->debug_fp);                                        \
    }                                                                             \
  }                                                                               \
  while (0)
#define SUNLogExtraDebugVecArray(logger, scope, label, msg_txt, vecs, nvecs)     \
  do {                                                                           \
    for (int vi = 0; vi < (nvecs); ++vi)                                         \
    {                                                                            \
      SUNLogger_QueueMsg(logger, SUN_LOGLEVEL_DEBUG, scope, label, msg_txt, vi); \
      N_VPrintFile(vecs[vi], logger->debug_fp);                                  \
    }                                                                            \
  }                                                                              \
  while (0)
#else
#define SUNLogExtraDebug(logger, scope, label, msg_txt, ...)
#define SUNLogExtraDebugIf(condition, logger, scope, label, msg_txt, ...)
#define SUNLogExtraDebugVec(logger, scope, label, msg_txt, vec, ...)
#define SUNLogExtraDebugVecIf(condition, logger, scope, label, msg_txt, vec, ...)
#define SUNLogExtraDebugVecArray(logger, scope, label, msg_txt, vecs, nvecs)
#endif

struct SUNLogger_
{
  /* MPI information */
  SUNComm comm;
  int output_rank;

  /* Ouput files */
  FILE* debug_fp;
  FILE* warning_fp;
  FILE* info_fp;
  FILE* error_fp;

  /* Hashmap used to store filename, FILE* pairs */
  SUNHashMap filenames;

  /* Slic-style format string */
  const char* format;

  /* Content for custom implementations */
  void* content;

  /* Overridable operations */
  SUNErrCode (*queuemsg)(SUNLogger logger, SUNLogLevel lvl, const char* scope,
                         const char* label, const char* msg_txt, va_list args);
  SUNErrCode (*flush)(SUNLogger logger, SUNLogLevel lvl);
  SUNErrCode (*destroy)(SUNLogger* logger);
};

/*
  This function creates a log message string in the correct format.
  It allocates the log_msg parameter, which must be freed by the caller.
  The format of the log message is:

    [ERROR][rank <rank>][<scope>][<label>] <formatted txt>

  :param lvl: the logging level (ERROR, WARNING, INFO, DEBUG)
  :param rank: the MPI rank of the caller
  :param scope: the scope part of the log message (see above)
  :param label: the label part of the log message (see above)
  :param txt: descriptive text for the log message in the form of a format string
  :param args: format string substitutions
  :param log_msg: on output this is an allocated string containing the log message

  :return: void
*/
void sunCreateLogMessage(SUNLogLevel lvl, int rank, const char* scope,
                         const char* label, const char* txt, va_list args,
                         char** log_msg);

#endif /* _SUNDIALS_LOGGER_IMPL_H */
