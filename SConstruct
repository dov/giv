import re, os, glob

if ARGUMENTS.get('debug', 0):
    cppflags = ['-g', '-Wall']
    variant = 'Debug'
else:
    cppflags = ['-O2']
    variant = 'Release'

commit_id = os.popen('git rev-parse HEAD').read().replace('\n','')

env = Environment(LIBPATH=[],
                  CPPFLAGS = cppflags + ['-Wno-deprecated-declarations',
                                         '-Wno-reorder',
                                         '-Wno-unused-but-set-variable',
                                         '-Wno-unused-function'],
                  CXXFLAGS=['-std=c++1y']
                  )

env['SBOX'] = False
env['COMMITIDSHORT'] = commit_id[0:6]

# Get version from configure.in
inp = open("configure.in")
for line in inp.readlines():
    m = re.search(r"AM_INIT_AUTOMAKE\(.*,\s*\"?(.*?)\"?\)", line)
    if m:
        env['VER'] = m.group(1)
        break

# All purpose template filling routine
def create_version(env, target, source):
    out = open(str(target[0]), "wb")
    out.write("#define VERSION \"" + env['VER'] + "\"\n")
    out.write('#define GIT_COMMIT_ID \"' + commit_id + '"\n')
    out.close()

def create_dist(env, target, source):
    # Skip if this is not a git repo
    if os.path.exists(".git"):
        vdir = "giv-%s"%env['VER']
        os.mkdir(vdir, 0755)
        os.system("tar -cf - `git ls-files` | (cd %s; tar -xf -); "%vdir)
        os.system("tar -zcf %s.tar.gz %s"%(vdir,vdir))
        os.system("rm -rf %s"%vdir)

if ARGUMENTS.get('mingw', 0) or ARGUMENTS.get('mingw64', 0):
    if ARGUMENTS.get('mingw', 0):
        env['HOST']='w32'
        env['HOSTBITS']='32'
        env['ARCH']='i686-w64-mingw32'
        env['LIBGCCDLL'] = "libgcc_s_sjlj-1.dll"
    elif ARGUMENTS.get('mingw64', 0):
        env['HOST']='w64'
        env['HOSTBITS']='64'
        env['ARCH']='x86_64-w64-mingw32'
        env['LIBGCCDLL'] = "libgcc_s_seh-1.dll"

    # For plugins
    env['ARCHDIR']='ming${HOST}'

    env.Command("COPYING.dos",
                "COPYING",
                ["unix2dos < COPYING > COPYING.dos"])
    
    env.Command("InstallGiv${VER}-${HOST}.exe",
                ["src/giv.exe",
                 "giv.nsi",
                 ] + glob.glob("src/plugins/*.dll"),
                ["makensis -DHOSTBITS=${HOSTBITS} -DVER=${VER} -DHOST=${HOST} -DSYSROOT=${SYSROOT} -DLIBGCCDLL=${LIBGCCDLL} -DCOMMITIDSHORT=${COMMITIDSHORT} giv.nsi"])
    env.Append(LINKFLAGS=['-mwindows'])

    env['PACKAGE_DOC_DIR'] = '../doc'
    env['PACKAGE_PLUGIN_DIR'] = '../plugins'
    env.Append(CPPFLAGS= ['-mms-bitfields'])
    env['OBJSUFFIX']=".obj"
    env['PROGSUFFIX'] = ".exe"
    env['SHOBJSUFFIX']=".obj"
    env['SHLIBSUFFIX'] = ".dll"
    env['SHLIBPREFIX'] = ""
    env['DLLWRAP_FLAGS'] = "--mno-cygwin --as=${AS} --export-all --driver-name ${CXX} --dll-tool-name ${DLLTOOL} -s"
    env['ROOT'] = ""
    env['SYSROOT'] = r"\\usr\\${ARCH}\\sys-root"
    env['LOCAL_DIR']='ming${HOST}'
    env['CC']='${ARCH}-gcc'
    env['CXX']='${ARCH}-g++'
    env['AR']='${ARCH}-ar'
    env['RANLIB']='${ARCH}-ranlib'
    env['DLLWRAP'] = "${ARCH}-dllwrap"
    env['DLLTOOL'] = "${ARCH}-dlltool"
    env['WINDRES'] = "${ARCH}-windres"
    env['PKGCONFIG'] = "env PKG_CONFIG_PATH=/usr/${ARCH}/sys-root/mingw/lib/pkgconfig:/usr/local/${LOCAL_DIR}/lib/pkgconfig pkg-config"

else:
    # Posix by default
    env['PKGCONFIG'] = "pkg-config"
    env['LIBPATH'] = []
    # Needed for maemo!
    env['SBOX'] = 'SBOX_UNAME_MACHINE' in os.environ
    if env['SBOX']:
        env['ENV'] = os.environ
    env['PKG_CONFIG_PATH'] = "/usr/lib/pkgconfig"
    env['PREFIX'] = "/usr/local"
    env['PACKAGE_DOC_DIR'] = '${PREFIX}/share/doc/giv'
    env['PACKAGE_PLUGIN_DIR'] = '${PREFIX}/lib/giv-1.0/plugins'
    env['ARCHDIR']='linux'

env['VARIANT'] = variant

# Since we don't run configure when doing scons
env.Command("config.h",
            ["SConstruct",
             "configure.in"],
            create_version)
env.Append(CPPPATH=[],
           # Needed for our internal PCRE
           CPPDEFINES=['PCRE_STATIC'],
           LIBPATH=["#/src/agg",
                    "#/src/plis",
                    "#/src/gtkimageviewer",
                    "#/src/glib-jsonrpc",
                    "#/src/glib-jsonrpc/json-glib",
                    ],
           RPATH=["agg/src"],
           LIBS=[#'gtkimageviewer_local',
                 #'agg',
                 #'glib-jsonrpc_local',
                 #'json-glib_local'
               ]
           )

env.ParseConfig("${PKGCONFIG} --cflags --libs gtk+-2.0 glib-2.0 gio-2.0 gmodule-2.0 gthread-2.0")

env.SConscript(['src/SConscript',
                'doc/SConscript',
                ],
               exports='env')

env.Alias("install",
          [env.Install('/usr/local/bin',
                       '#/src/giv'),
           env.Install('/usr/local/lib',
                       [
#                           '#/src/libgiv-widget.so',
                           '#/src/libgiv-image.so',
                           ]
                       ),
           env.Install('${PACKAGE_PLUGIN_DIR}',
                       glob.glob('src/plugins/*.so')),
           env.Install('/usr/local/share/doc/giv',
                       [g for g in glob.glob('doc/*')
                        if re.search('\.(png|html|jpg)$',g)]
                       )
           ])


env.Alias("dist",
          env.Command("giv-${VER}.tar.gz",
                      [],
                      create_dist))

