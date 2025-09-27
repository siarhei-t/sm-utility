/**
 * @file sm_log.hpp
 *
 * @brief
 *
 * @author
 *
 */

#include "log_config.hpp"

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_DEBUG 2

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(...) LOG_OUTPUT_FN(__VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...) LOG_OUTPUT_FN(__VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif