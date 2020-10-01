#ifndef GTLCOREDYNLIB_GLOBAL_H
#define GTLCOREDYNLIB_GLOBAL_H

#if defined(GTLCOREDYNLIB_LIBRARY)
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# define GTLCOREDYNLIBSHARED_EXPORT __declspec(dllexport)
#else
# define GTLCOREDYNLIBSHARED_EXPORT
#endif
#else
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# define GTLCOREDYNLIBSHARED_EXPORT __declspec(dllimport)
#else
# define GTLCOREDYNLIBSHARED_EXPORT
#endif
#endif

#endif // GTLCOREDYNLIB_GLOBAL_H

