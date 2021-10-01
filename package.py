name = "kSeExpr"

authors = [
    "SeExpr (from WDA) for Krita"
]

# NOTE: version = <external_version>.sse.<sse_version>
version = "4.0.3.0.sse.1.0.0"

description = \
    """
    Fork of Disney Animation's SeExpr expression library, that is used in Krita.
    """

with scope("config") as c:
    # Determine location to release: internal (int) vs external (ext)

    # NOTE: Modify this variable to reflect the current package situation
    release_as = "ext"

    # The `c` variable here is actually rezconfig.py
    # `release_packages_path` is a variable defined inside rezconfig.py

    import os
    if release_as == "int":
        c.release_packages_path = os.environ["SSE_REZ_REPO_RELEASE_INT"]
    elif release_as == "ext":
        c.release_packages_path = os.environ["SSE_REZ_REPO_RELEASE_EXT"]

    #c.build_thread_count = "physical_cores"

requires = [
    "python-3",
    "qt-5.15.2",
    "libpng",
]

private_build_requires = [
    "ecm",
]

variants = [
    ["platform-linux", "arch-x86_64", "os-centos-7"],
]

uuid = "repository.kseexpr"

def pre_build_commands():
    command("source /opt/rh/devtoolset-6/enable")

def commands():

    # NOTE: REZ package versions can have ".sse." to separate the external
    # version from the internal modification version.
    split_versions = str(version).split(".sse.")
    external_version = split_versions[0]
    internal_version = None
    if len(split_versions) == 2:
        internal_version = split_versions[1]

    env.KSEEXPR_VERSION = external_version
    env.KSEEXPR_PACKAGE_VERSION = external_version
    if internal_version:
        env.KSEEXPR_PACKAGE_VERSION = internal_version

    env.KSEEXPR_ROOT.append("{root}")
    env.KSEEXPR_LOCATION.append("{root}")
