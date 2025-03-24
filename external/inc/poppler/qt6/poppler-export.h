
#ifndef POPPLER_QT6_EXPORT_H
#define POPPLER_QT6_EXPORT_H

#ifdef POPPLER_QT6_STATIC_DEFINE
#  define POPPLER_QT6_EXPORT
#  define POPPLER_QT6_NO_EXPORT
#else
#  ifndef POPPLER_QT6_EXPORT
#    ifdef poppler_qt6_EXPORTS
        /* We are building this library */
#      define POPPLER_QT6_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define POPPLER_QT6_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef POPPLER_QT6_NO_EXPORT
#    define POPPLER_QT6_NO_EXPORT 
#  endif
#endif

#ifndef POPPLER_QT6_DEPRECATED
#  define POPPLER_QT6_DEPRECATED __declspec(deprecated)
#endif

#ifndef POPPLER_QT6_DEPRECATED_EXPORT
#  define POPPLER_QT6_DEPRECATED_EXPORT POPPLER_QT6_EXPORT POPPLER_QT6_DEPRECATED
#endif

#ifndef POPPLER_QT6_DEPRECATED_NO_EXPORT
#  define POPPLER_QT6_DEPRECATED_NO_EXPORT POPPLER_QT6_NO_EXPORT POPPLER_QT6_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef POPPLER_QT6_NO_DEPRECATED
#    define POPPLER_QT6_NO_DEPRECATED
#  endif
#endif

#endif /* POPPLER_QT6_EXPORT_H */
