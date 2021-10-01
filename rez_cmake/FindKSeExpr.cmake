#
# Find KSeExpr headers and libraries.
#
# This module defines the following variables:
#
#   KSeExpr_FOUND            True if KSeExpr was found
#   KSeExpr_INCLUDE_DIRS     Where to find KSeExpr header files
#   KSeExpr_LIBRARIES        List of KSeExpr libraries to link against
#

include (FindPackageHandleStandardArgs)

find_path(KSeExpr_INCLUDE_DIR
    NAMES
        Expression.h
    HINTS
        $ENV{REZ_KSEEXPR_ROOT}/include/KSeExpr
)

find_library(KSeExpr_LIBRARY
    NAMES
        KSeExpr
    HINTS
        $ENV{REZ_KSEEXPR_ROOT}/lib64
)

find_library(KSeExprUI_LIBRARY
    NAMES
        KSeExprUI
    HINTS
        $ENV{REZ_KSEEXPR_ROOT}/lib64
)

# Handle the QUIETLY and REQUIRED arguments and set KSeExpr_FOUND.
find_package_handle_standard_args(KSeExpr DEFAULT_MSG
    KSeExpr_INCLUDE_DIR
    KSeExpr_LIBRARY
)

# Set the output variables.
if (KSeExpr_FOUND)
    set (KSeExpr_INCLUDE_DIRS ${KSeExpr_INCLUDE_DIR})
    list (APPEND KSeExpr_LIBRARIES ${KSeExpr_LIBRARY} ${KSeExprUI_LIBRARY})
else ()
    set (KSeExpr_INCLUDE_DIRS)
    set (KSeExpr_LIBRARIES)
endif ()

mark_as_advanced (
    KSeExpr_INCLUDE_DIR
    KSeExpr_LIBRARY
    KSeExprUI_LIBRARY
)
