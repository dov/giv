import re, os, glob

if ARGUMENTS.get('debug', 0):
    cppflags = ['-g', '-Wall']
    variant = 'Debug'
else:
    cppflags = ['-O2']
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
    env['CC']='i686-pc-mingw32-gcc'
    env['SHCC']='i686-pc-mingw32-gcc'
    env['CXX']='i686-pc-mingw32-g++'
    env['SHCXX']='i686-pc-mingw32-g++'
    env['AR']='i686-pc-mingw32-ar'
    env['RANLIB']='i686-pc-mingw32-ranlib'
    env['ENV']['PKG_CONFIG_PATH'] = "/usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig"
    env['PKGCONFIG'] = "env PKG_CONFIG_PATH=/usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig:/usr/local/mingw32/lib/pkgconfig pkg-config"
    env['OBJSUFFIX']=".obj"
    env['SHLIBSUFFIX']=".dll"
    env['SHLIBPREFIX']=""
#    env['LIBSUFFIX']=".lib"
    env['PROGSUFFIX'] = ".exe"
    env['CROOT'] = "/home/dov/.wine/drive_c/"
    env['PREFIX'] = "/usr/i686-pc-minw32/sys-root"
    env['DLLWRAP'] = "i686-pc-mingw32-dllwrap"
    env['DLLTOOL'] = "i686-pc-mingw32-dlltool"
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
    env.Append(LINKFLAGS=['-mwindows'],
#               CPPPATH=["/usr/local/mingw32/include"],
#               LIBPATH=["/usr/local/mingw32/lib"],
               )
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
            ["SConstruct"],
            create_version)
env.Append(CPPPATH=[],
           # Needed for our internal PCRE
           CPPDEFINES=['PCRE_STATIC'],
           LIBPATH=["#/src/agg",
                    "#/src/plis",
                    "#/src/gtkimageviewer",
                    ],
           RPATH=["agg/src"],
           LIBS=['gtkimageviewer_local', 'agg']
           )

env.ParseConfig("${PKGCONFIG} --cflags --libs gtk+-2.0 glib-2.0")

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
