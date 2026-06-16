#ifndef DEMO_TO_CAT_VISIBILITY_CONTROL_H_
#define DEMO_TO_CAT_VISIBILITY_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
// https://gcc.gnu.org/wiki/Visibility

#define DEMO_TO_CAT_EXPORT __attribute__ ((visibility("default")))
#define DEMO_TO_CAT_IMPORT

#if __GNUC__ >= 4
    #define DEMO_TO_CAT_PUBLIC __attribute__ ((visibility("default")))
    #define DEMO_TO_CAT_LOCAL  __attribute__ ((visibility("hidden")))
#else
    #define DEMO_TO_CAT_PUBLIC
    #define DEMO_TO_CAT_LOCAL
#endif

#define DEMO_TO_CAT_PUBLIC_TYPE

#ifdef __cplusplus
}
#endif

#endif  // DEMO_TO_CAT_VISIBILITY_CONTROL_H_
