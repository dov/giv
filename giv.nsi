Name "Giv"
OutFile "InstallGiv-${VER}-${HOST}-${COMMITIDSHORT}.exe"
Icon "giv-logo.ico"
UninstallIcon "giv-logo-install.ico"

SetCompress force ; (can be off or force)
CRCCheck on ; (can be off)

LicenseText "Giv is a free program. Here is its license."
LicenseData "COPYING.dos"

InstallDir "$PROGRAMFILES${HOSTBITS}\Giv"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Giv" ""

DirText "Select the directory to install Giv in:"

; optional section
Section "Start Menu Shortcuts"
 CreateDirectory "$SMPROGRAMS\Giv"
 CreateShortCut "$SMPROGRAMS\Giv\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
 CreateShortCut "$SMPROGRAMS\Giv\Giv.lnk" "$INSTDIR\bin\Giv.exe" "" "$INSTDIR\bin\Giv.exe" 0
SectionEnd

Section "" ; (default section)

; List of files to install
SetOutPath "$INSTDIR"
File COPYING
File README.md
File AUTHORS
File NEWS
File /r doc
File /r examples
File /r python

SetOutPath $INSTDIR\bin
File src\Giv.exe
File src\giv-image.dll
File src\giv-remote-client.exe
File ${SYSROOT}\mingw\bin\${LIBGCCDLL}
File ${SYSROOT}\mingw\bin\libexpat-1.dll
File ${SYSROOT}\mingw\bin\libstdc++-6.dll
File ${SYSROOT}\mingw\bin\iconv.dll
File ${SYSROOT}\mingw\bin\libpcre-1.dll
File ${SYSROOT}\mingw\bin\libintl-8.dll
File ${SYSROOT}\mingw\bin\libffi-6.dll
File ${SYSROOT}\mingw\bin\libgdk-win32*.dll
File ${SYSROOT}\mingw\bin\libgdk_pixbuf*.dll
File ${SYSROOT}\mingw\bin\libgtk-win32*.dll
File ${SYSROOT}\mingw\bin\libgio*.dll
File ${SYSROOT}\mingw\bin\libcairo*.dll
File ${SYSROOT}\mingw\bin\libjasper*.dll
File ${SYSROOT}\mingw\bin\zlib*.dll
File ${SYSROOT}\mingw\bin\libglib*.dll
File ${SYSROOT}\mingw\bin\libatk*.dll
File ${SYSROOT}\mingw\bin\libgobject*.dll
File ${SYSROOT}\mingw\bin\libgmodule*.dll
File ${SYSROOT}\mingw\bin\libgthread*.dll
File ${SYSROOT}\mingw\bin\libpango*.dll
File ${SYSROOT}\mingw\bin\libpng*.dll
File ${SYSROOT}\mingw\bin\libtiff*.dll
File ${SYSROOT}\mingw\bin\libjpeg*.dll
File ${SYSROOT}\mingw\bin\libpixman-1-0.dll
File ${SYSROOT}\mingw\bin\libfontconfig*.dll
File ${SYSROOT}\mingw\bin\libfreetype*.dll
File ${SYSROOT}\mingw\bin\libbz2*.dll
File ${SYSROOT}\mingw\bin\libwinpthread*.dll
#File \usr\local\mingw32\bin\libjson-glib-1.0-0.dll
File ${SYSROOT}\mingw\bin\gdk-pixbuf-query-loaders.exe

SetOutPath $INSTDIR\etc
File /r ${SYSROOT}\mingw\etc
SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0
File /r ${SYSROOT}\mingw\lib\gdk-pixbuf-2.0\2.10.0\loaders
SetOutPath $INSTDIR\lib\gdk-pixbuf-2.0\2.10.0
File ${SYSROOT}\mingw\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache
SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\engines
File ${SYSROOT}\mingw\lib\gtk-2.0\2.10.0\engines\*
SetOutPath $INSTDIR\share\themes 
File /r ${SYSROOT}\mingw\share\themes\*

# Plugins
SetOutPath $INSTDIR\plugins
File src\plugins\*.dll

# Build the gdk-pixbuf.loaders file automatically
#ExpandEnvStrings $0 %COMSPEC%
#nsExec::ExecToStack '"$0" /C ""$INSTDIR\bin\gdk-pixbuf-query-loaders" > "$INSTDIR\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache""'

; Set up association with .giv files
DeleteRegKey HKCR ".giv"
DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.giv"
WriteRegStr HKCR ".giv" "" "GivFile"
DeleteRegKey HKCR "GivFile"
DeleteRegKey HKCR "Giv.Document"
WriteRegStr HKCR "GivFile" "" "Giv File"
WriteRegStr HKCR "GivFile\DefaultIcon" "" "$INSTDIR\bin\Giv.exe,1"
WriteRegStr HKCR "GivFile\shell" "" "open"
WriteRegStr HKCR "GivFile\shell\open\command" "" '$INSTDIR\bin\Giv.exe "%1"'

System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'

; Unistaller section. Should really clean up file associations as well.
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\giv" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\giv" "DisplayName" "giv (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\giv" "UninstallString" '"$INSTDIR\bin\uninst.exe"'

; write out uninstaller
WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd ; end of default section

; begin uninstall settings/section
UninstallText "This will uninstall giv from your system"

Section Uninstall
; add delete commands to delete whatever files/registry keys/etc you installed here.
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\giv"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\giv"
RMDir /r "$INSTDIR"
DeleteRegKey HKCR ".giv"
DeleteRegKey HKCR "GivFile"
DeleteRegKey HKCR "Applications\giv.exe"
SectionEnd ; end of uninstall section

; eof
