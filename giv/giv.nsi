Name "Giv"
OutFile "InstallGiv-0.3.4.exe"

SetCompress force ; (can be off or force)
CRCCheck on ; (can be off)

LicenseText "Giv is a free program. Here is its license."
LicenseData "COPYING.dos"

InstallDir "$PROGRAMFILES\Giv"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Giv" ""
DirShow show ; (make this hide to not let the user change it)
DirText "Select the directory to install Giv in:"

Section "" ; (default section)
SetOutPath "$INSTDIR"

; List of files to install
File ..\COPYING
File ..\README
File ..\AUTHORS
File giv.exe
File /r nsi\*.*
File /r ..\doc

WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\giv" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\giv" "DisplayName" "giv (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\giv" "UninstallString" '"$INSTDIR\uninst.exe"'
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
SectionEnd ; end of uninstall section

; eof
