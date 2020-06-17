@echo off 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
mkdir Release
mkdir Release_out
mkdir Includes
cl /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Fd"Release/vc141.pdb" /Zc:inline /fp:precise /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /FC /Fa"Release/" /EHsc /nologo /Fo"Release/" /diagnostics:classic *.cpp /link /OUT:"Release_out/program.exe" /MANIFEST /LTCG:incremental /NXCOMPAT /PDB:"Release_out/program.pdb" /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG:FULL /MACHINE:X86 /OPT:REF /SAFESEH /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'" /ManifestFile:"Release/program.exe.intermediate.manifest" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1

