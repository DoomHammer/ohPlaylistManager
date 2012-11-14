# Defines the build behaviour for continuous integration builds.
#
# Invoke with "go hudson_build"


import os
import sys
import shutil
import json
import platform
from glob import glob

try:
    from ci import (
        require_version, add_option, add_bool_option, modify_optional_steps,
        specify_optional_steps, default_platform, get_dependency_args,
        build_step, build_condition, userlock, python, rsync, SshSession,
        fetch_dependencies, get_vsvars_environment, scp, fail, shell, cli)
except ImportError:
    print "You need to update ohDevTools."
    sys.exit(1)

require_version(14)

#todo: add test projects in a platform specific manner
solutions = [
    {
     "sln":"src/ohPlaylistManager/ohPlaylistManager.csproj",
     "mdtool":False
    }
]

# Command-line options. See documentation for Python's optparse module.
add_option("-t", "--target", help="Target platform. One of Windows-x86, Windows-x64, Linux-x86, Linux-x64, Linux-ARM.")
add_option("-p", "--publish-version", help="Specify a version to publish.")
add_option("-a", "--artifacts", help="Build artifacts directory. Used to fetch dependencies.")
add_bool_option("-f", "--fetch-only", help="Fetch dependencies, skip building.")
add_bool_option("-F", "--no-fetch", help="Skip fetch dependencies.")
add_option("--steps", default="default", help="Steps to run, comma separated. (all,default,fetch,configure,clean,build,tests,publish)")
add_option('--release', action="store_const", const="release", dest="debugmode", default="release", help="")
add_option('--debug', action="store_const", const="debug", dest="debugmode", default="release", help="")

@build_step()
def choose_optional_steps(context):
    specify_optional_steps(context.options.steps)
    if context.options.publish_version or context.env.get("PUBLISH_RELEASE","false").lower()=="true":
        modify_optional_steps("+publish")
    if context.options.fetch_only:
        specify_optional_steps("fetch")

# Unconditional build step. Choose a platform and set the
# appropriate environment variable.
@build_step()
def choose_platform(context):
    context.env["OH_PLATFORM"] = (
            context.options.target or
            context.env.get("slave", None) or
            default_platform())
    context.env.update(MSBUILDCONFIGURATION="Release" if context.options.debugmode=="release" else "Debug")

@build_step()    
def get_buildinfo(context):
    with open("projectdata/buildinfo.json") as buildinfofile:
        context.env.update(BUILDINFO=json.load(buildinfofile))
        

# Extra Windows build configuration.
@build_step()
@build_condition(OH_PLATFORM="Windows-x86")
@build_condition(OH_PLATFORM="Windows-x64")
def setup_windows(context):
    context.env.update(MSBUILDCMD="msbuild /nologo /p:Configuration=%s" % (context.env["MSBUILDCONFIGURATION"]))
    context.env.update(MSBUILDTARGETSWITCH="/t:")
    context.env.update(MSBUILDSOLUTIONSUFFIX="Windows")

@build_condition(OH_PLATFORM="Windows-x86")
def setup_windows_x86(context):
    context.env.update(get_vsvars_environment("x86"))

@build_condition(OH_PLATFORM="Windows-x64")
def setup_windows_x64(context):
    context.env.update(get_vsvars_environment("amd64"))

# Extra Linux build configuration.
@build_step()
@build_condition(OH_PLATFORM="Linux-x86")
@build_condition(OH_PLATFORM="Linux-x64")
@build_condition(OH_PLATFORM="Linux-ARM")
def setup_linux(context):
    context.env.update(MDTOOLBUILDCMD="mdtool build -c:%s|Any CPU" % context.env["MSBUILDCONFIGURATION"])
    context.env.update(MSBUILDCMD="xbuild /nologo /p:Configuration=%s" % (context.env["MSBUILDCONFIGURATION"]))

# Extra Mac build configuration.
@build_step()
@build_condition(OH_PLATFORM="Mac-x86")
@build_condition(OH_PLATFORM="Mac-x64")
@build_condition(OH_PLATFORM="Mac-ARM")
def setup_linux(context):
    context.env.update(MDTOOLBUILDCMD="/Applications/MonoDevelop.app/Contents/MacOS/mdtool build -c:%s|Any CPU" % context.env["MSBUILDCONFIGURATION"])
    context.env.update(MSBUILDCMD="xbuild /nologo /p:Configuration=%s" % (context.env["MSBUILDCONFIGURATION"]))
    
# Principal build steps.
@build_step("fetch", optional=True)
def fetch(context):
    fetch_dependencies(env={'debugmode':context.options.debugmode,
                     'titlecase-debugmode':context.options.debugmode.title()})

@build_step("clean", optional=True)
def clean(context):
    for solution in solutions:
        do_build(context, solution, "Clean")
    
@build_step("build", optional=True)
def build(context):
    for solution in solutions:
        do_build(context, solution, "Build")
    
#@build_step("publish", optional=True, default=False)
#def publish(context):
#    platform = context.env["OH_PLATFORM"]
#    version = context.options.publish_version or context.env.get("RELEASE_VERSION", "UNKNOWN")
#    publishdir = context.env["OHOS_PUBLISH"]
#    builddir = context.env["BUILDDIR"]

def do_build(context, solution, target):
    mdtool = solution["mdtool"]
    solutionfile = solution["sln"]
    msbuild = context.env['MDTOOLBUILDCMD'] if mdtool else context.env['MSBUILDCMD']
    targetswitch =  "-t:" if mdtool else "/t:"
    buildshell = "%(msbuild)s %(sln)s %(targetswitch)sBuild" % {'sln':solutionfile, 'msbuild':msbuild, 'targetswitch':targetswitch}
    shell(buildshell)
    
