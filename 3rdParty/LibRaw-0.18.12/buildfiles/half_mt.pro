include (libraw-common.pro)
win32 {
	SOURCES=../samples/half_mt_win32.c
} else {
	SOURCES=../samples/half_mt.c
}
