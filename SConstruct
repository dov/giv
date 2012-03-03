import re, os, glob

if ARGUMENTS.get('debug', 0):
    cppflags = ['-g', '-Wall','-fPIC']
    variant = 'Debug'
else:
    cppflags = ['-O2','-fPIC']
    variant = 'Release'

env = Environment(LIBPATH=[],
                  CPPFLAGS = cppflags)

env['SBOX'] = False

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
    out.close()

def create_dist(env, target, source):
    # Skip if this is not a git repo
    if os.path.exists(".git"):
        vdir = "giv-%s"%env['VER']
        os.mkdir(vdir, 0755)
        os.system("tar -cf - `git ls-files` | (cd %s; tar -xf -); "%vdir)
        os.system("tar -zcf %s.tar.gz %s"%(vdir,vdir))
        os.system("rm -rf %s"%vdir)

# All purpose template filling routine
def template_fill(env, target, source):
    out = open(str(target[0]), "wb")
    inp = open(str(source[0]), "r")

    for line in inp.readlines():
        line = re.sub('@(.*?)@',
                      lambda x: env[x.group(1)],
                      line)
        out.write(line)
        
    out.close()
    inp.close()

if ARGUMENTS.get('mingw', 0):
    env['CC']='i686-w64-mingw32-gcc'
    env['CXX']='i686-w64-mingw32-g++'
    env['AR']='i686-w64-mingw32-ar'
    env['RANLIB']='i686-w64-mingw32-ranlib'
    env['PKGCONFIG'] = "env PKG_CONFIG_PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig:/usr/local/mingw32/lib/pkgconfig pkg-config"
    env['OBJSUFFIX']=".obj"
    env['PROGSUFFIX'] = ".exe"
    env['SHOBJSUFFIX']=".obj"
    env['SHLIBSUFFIX'] = ".dll"
    env['SHLIBPREFIX'] = ""
    env['PREFIX'] = "/usr/i686-w64-mingw32/sys-root"
    env['ROOT'] = ""
    env['DLLWRAP'] = "i686-w64-mingw32-dllwrap"
    env['DLLTOOL'] = "i686-w64-mingw32-dlltool"
    env['DLLWRAP_FLAGS'] = "--mno-cygwin --as=${AS} --export-all --driver-name ${CXX} --dll-tool-name ${DLLTOOL} -s"
    env.Append(CPPFLAGS= ['-mms-bitfields'])

    env.Command("giv.wine.nsi",
                ["giv.wine.nsi.in",
                 "SConstruct",
                 "configure.in"
                ],
                template_fill
                )
    env.Command("COPYING.dos",
                "COPYING",
                ["unix2dos < COPYING > COPYING.dos"])
    
    env.Command("InstallGiv" + env['VER'] + ".exe",
                ["src/giv.exe",
                 "giv.wine.nsi",
                 "src/plugins/tiff.dll",
                 "src/plugins/fits.dll",
                 "src/plugins/dicom.dll",
                 "src/plugins/npy.dll",
                 ],
                ["makensis giv.wine.nsi"])
    env.Append(LINKFLAGS=['-mwindows'])

    # TBD - make this installation dependent
    env['PACKAGE_DOC_DIR'] = '../doc'
    env['PACKAGE_PLUGIN_DIR'] = '../plugins'
    env['arch']='mingw32'
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
    env['arch']='linux'

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
           LIBS=['gtkimageviewer_local',
                 'agg',
                 'glib-jsonrpc_local',
                 'json-glib_local']
           )

env.ParseConfig("${PKGCONFIG} --cflags --libs gtk+-2.0 glib-2.0 gio-2.0 gthread-2.0")

SConscript(['src/SConscript',
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
