
MinakortControlsps.dll: dlldata.obj MinakortControls_p.obj MinakortControls_i.obj
	link /dll /out:MinakortControlsps.dll /def:MinakortControlsps.def /entry:DllMain dlldata.obj MinakortControls_p.obj MinakortControls_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \
.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0500 /DREGISTER_PROXY_DLL \
		$<
# _WIN32_WINNT=0x0500 is for Win2000, change it to 0x0400 for NT4 or Win95 with DCOM

clean:
	@del MinakortControlsps.dll
	@del MinakortControlsps.lib
	@del MinakortControlsps.exp
	@del dlldata.obj
	@del MinakortControls_p.obj
	@del MinakortControls_i.obj
