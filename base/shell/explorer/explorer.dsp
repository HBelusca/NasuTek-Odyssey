# Microsoft Developer Studio Project File - Name="explorer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=explorer - Win32 WineDll
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "explorer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "explorer.mak" CFG="explorer - Win32 WineDll"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "explorer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 Debug Release" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 Unicode Release" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 Unicode Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 WineRelease" (based on "Win32 (x86) Console Application")
!MESSAGE "explorer - Win32 WineDll" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "explorer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /D "NDEBUG" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 shell32.lib comctl32.lib gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /machine:I386 /libpath:"Release" /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /FR /Yu"precomp.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib shell32.lib comctl32.lib gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"Debug" /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 Debug Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DRelease"
# PROP BASE Intermediate_Dir "DRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "DRelease"
# PROP Intermediate_Dir "DRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_ROS_" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /FR /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib comctl32.lib shell32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 shell32.lib comctl32.lib gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "URelease"
# PROP BASE Intermediate_Dir "URelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "URelease"
# PROP Intermediate_Dir "URelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "UNICODE" /D "_ROS_" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "UNICODE" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib comctl32.lib shell32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 shell32.lib comctl32.lib gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /machine:I386 /libpath:"Release" /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "UDebug"
# PROP BASE Intermediate_Dir "UDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "UDebug"
# PROP Intermediate_Dir "UDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "UNICODE" /D "_ROS_" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "UNICODE" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /FR /Yu"precomp.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib comctl32.lib shell32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shell32.lib comctl32.lib gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"Debug" /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32"
# PROP BASE Intermediate_Dir "Win32"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32"
# PROP Intermediate_Dir "Win32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_ROS_" /D _WIN32_IE=0x0501 /D _WIN32_WINNT=0x0501 /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "UNICODE" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /FR /Yu"precomp.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib comctl32.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Win32/wexplorer.exe" /pdbtype:sept /delayload:oleaut32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 WineRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "explorer___Win32_WineRelease"
# PROP BASE Intermediate_Dir "explorer___Win32_WineRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WineRelease"
# PROP Intermediate_Dir "WineRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "UNICODE" /D "WIN32" /D "_ROS_" /D _WIN32_IE=0x0501 /D _WIN32_WINNT=0x0501 /FR /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /D "NDEBUG" /D "UNICODE" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /incremental:no /machine:I386 /out:"WineRelease/wexplorer.exe" /pdbtype:sept /delayload:oleaut32.dll /delayload:wsock32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "explorer - Win32 WineDll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "explorer___Win32_WineDll"
# PROP BASE Intermediate_Dir "explorer___Win32_WineDll"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WineDll"
# PROP Intermediate_Dir "WineDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /D "NDEBUG" /D "UNICODE" /D "WIN32" /D "_ROS_" /D _WIN32_IE=0x0501 /D _WIN32_WINNT=0x0501 /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "UNICODE" /D "WIN32" /D _WIN32_IE=0x0600 /D _WIN32_WINNT=0x0501 /Yu"precomp.h" /FD /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib advapi32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 gdi32.lib user32.lib advapi32.lib ole32.lib delayimp.lib /nologo /subsystem:windows /machine:I386 /out:"WineDll/wexplorer.exe" /pdbtype:sept /delayload:oleaut32.dll /delayload:wsock32.dll
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "explorer - Win32 Release"
# Name "explorer - Win32 Debug"
# Name "explorer - Win32 Debug Release"
# Name "explorer - Win32 Unicode Release"
# Name "explorer - Win32 Unicode Debug"
# Name "explorer - Win32"
# Name "explorer - Win32 WineRelease"
# Name "explorer - Win32 WineDll"
# Begin Group "utility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\utility\dragdropimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\dragdropimpl.h
# End Source File
# Begin Source File

SOURCE=.\utility\shellbrowserimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\shellbrowserimpl.h
# End Source File
# Begin Source File

SOURCE=.\utility\shellclasses.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\shellclasses.h
# End Source File
# Begin Source File

SOURCE=.\utility\treedroptarget.h
# End Source File
# Begin Source File

SOURCE=.\utility\utility.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\utility.h
# End Source File
# Begin Source File

SOURCE=.\utility\window.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\window.h
# End Source File
# Begin Source File

SOURCE=.\utility\xmlstorage.cpp
# End Source File
# Begin Source File

SOURCE=.\utility\xmlstorage.h
# End Source File
# Begin Source File

SOURCE=".\utility\xs-native.cpp"
# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter "bmp,ico"
# Begin Source File

SOURCE=.\res\action.ico
# End Source File
# Begin Source File

SOURCE=.\res\administration.ico
# End Source File
# Begin Source File

SOURCE=.\res\appicon.ico
# End Source File
# Begin Source File

SOURCE=.\res\apps.ico
# End Source File
# Begin Source File

SOURCE=.\res\arrow.ico
# End Source File
# Begin Source File

SOURCE=.\res\arrow_dwn.ico
# End Source File
# Begin Source File

SOURCE=.\res\arrow_up.ico
# End Source File
# Begin Source File

SOURCE=.\res\arrowsel.ico
# End Source File
# Begin Source File

SOURCE=.\res\computer.ico
# End Source File
# Begin Source File

SOURCE=.\res\config.ico
# End Source File
# Begin Source File

SOURCE=".\res\control-panel.ico"
# End Source File
# Begin Source File

SOURCE=".\res\desktop-settings.ico"
# End Source File
# Begin Source File

SOURCE=.\res\documents.ico
# End Source File
# Begin Source File

SOURCE=.\res\dot.ico
# End Source File
# Begin Source File

SOURCE=.\res\dot_red.ico
# End Source File
# Begin Source File

SOURCE=.\res\dot_trans.ico
# End Source File
# Begin Source File

SOURCE=.\res\drivebar.bmp
# End Source File
# Begin Source File

SOURCE=".\explorer-bg.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-bn.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-cn-msvc.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-cn.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-cz.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-de.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-en.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-es.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-fr.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-gr.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-hu.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-id.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-it.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-jp.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-ko.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-nl.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-no.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-pl.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-pt.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-ro.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-ru.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-sk.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-sv.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\explorer-uk.rc"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\explorer.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\res\explorer.ico
# End Source File
# Begin Source File

SOURCE=.\explorer.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\explorer_intres.rc
# End Source File
# Begin Source File

SOURCE=.\res\favorites.ico
# End Source File
# Begin Source File

SOURCE=.\res\floating.ico
# End Source File
# Begin Source File

SOURCE=.\res\folder.ico
# End Source File
# Begin Source File

SOURCE=.\res\icoali10.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig6.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig7.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig8.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icoalig9.bmp
# End Source File
# Begin Source File

SOURCE=.\res\images.bmp
# End Source File
# Begin Source File

SOURCE=.\res\info.ico
# End Source File
# Begin Source File

SOURCE=.\res\logoff.ico
# End Source File
# Begin Source File

SOURCE=.\res\logov.bmp
# End Source File
# Begin Source File

SOURCE=.\res\logov16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\logov256.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mdi.bmp
# End Source File
# Begin Source File

SOURCE=.\res\minimize.ico
# End Source File
# Begin Source File

SOURCE=".\res\network-conns.ico"
# End Source File
# Begin Source File

SOURCE=.\res\network.ico
# End Source File
# Begin Source File

SOURCE=.\res\notify_l.ico
# End Source File
# Begin Source File

SOURCE=.\res\notify_r.ico
# End Source File
# Begin Source File

SOURCE=.\res\printer.ico
# End Source File
# Begin Source File

SOURCE=.\res\odyssey.ico
# End Source File
# Begin Source File

SOURCE=".\res\recent-documents.ico"
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\restart.ico
# End Source File
# Begin Source File

SOURCE=".\res\ros-big.ico"
# End Source File
# Begin Source File

SOURCE=.\res\sdi.bmp
# End Source File
# Begin Source File

SOURCE=".\res\search-doc.ico"
# End Source File
# Begin Source File

SOURCE=.\res\search.ico
# End Source File
# Begin Source File

SOURCE=.\res\shutdown.ico
# End Source File
# Begin Source File

SOURCE=.\res\speaker.ico
# End Source File
# Begin Source File

SOURCE=.\res\startmenu.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# End Group
# Begin Group "taskbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\taskbar\desktopbar.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\desktopbar.h
# End Source File
# Begin Source File

SOURCE=.\taskbar\favorites.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\favorites.h
# End Source File
# Begin Source File

SOURCE=.\notifyhook\notifyhook.h
# End Source File
# Begin Source File

SOURCE=.\taskbar\quicklaunch.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\quicklaunch.h
# End Source File
# Begin Source File

SOURCE=.\taskbar\startmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\startmenu.h
# End Source File
# Begin Source File

SOURCE=.\taskbar\taskbar.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\taskbar.h
# End Source File
# Begin Source File

SOURCE=.\taskbar\traynotify.cpp
# End Source File
# Begin Source File

SOURCE=.\taskbar\traynotify.h
# End Source File
# End Group
# Begin Group "desktop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\desktop\desktop.cpp
# End Source File
# Begin Source File

SOURCE=.\desktop\desktop.h
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\doc\changes.txt
# End Source File
# Begin Source File

SOURCE=.\doc\readme.txt
# End Source File
# Begin Source File

SOURCE=.\doc\TODO.txt
# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shell\entries.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\entries.h
# End Source File
# Begin Source File

SOURCE=.\shell\fatfs.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\fatfs.h
# End Source File
# Begin Source File

SOURCE=.\shell\filechild.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\filechild.h
# End Source File
# Begin Source File

SOURCE=.\shell\mainframe.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\mainframe.h
# End Source File
# Begin Source File

SOURCE=.\shell\ntobjfs.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\ntobjfs.h
# End Source File
# Begin Source File

SOURCE=.\shell\pane.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\pane.h
# End Source File
# Begin Source File

SOURCE=.\shell\regfs.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\regfs.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellbrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\shellbrowser.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellfs.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\shellfs.h
# End Source File
# Begin Source File

SOURCE=.\shell\unixfs.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\shell\unixfs.h
# End Source File
# Begin Source File

SOURCE=.\shell\webchild.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\webchild.h
# End Source File
# Begin Source File

SOURCE=.\shell\winfs.cpp
# End Source File
# Begin Source File

SOURCE=.\shell\winfs.h
# End Source File
# End Group
# Begin Group "dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialogs\searchprogram.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\searchprogram.h
# End Source File
# Begin Source File

SOURCE=.\dialogs\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\settings.h
# End Source File
# End Group
# Begin Group "main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\explorer.cpp
# End Source File
# Begin Source File

SOURCE=.\explorer.h
# End Source File
# Begin Source File

SOURCE=.\externals.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=".\i386-stub-win32.c"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\precomp.cpp
# ADD CPP /Yc"precomp.h"
# End Source File
# Begin Source File

SOURCE=.\precomp.h
# End Source File
# End Group
# Begin Group "services"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\services\shellservices.cpp
# End Source File
# Begin Source File

SOURCE=.\services\shellservices.h
# End Source File
# Begin Source File

SOURCE=.\services\startup.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# End Target
# End Project
