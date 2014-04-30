cd libffi

bash configure --enable-static CC="../msvcc.sh -m64" LD=link CPP="cl -nologo -EP" CFLAGS=-Ox

rem cygwin creates a weird fake symlink, which msvc cannot read
rm -f x86_64-unknown-cygwin/include/ffitarget.h
cp -f src/x86/ffitarget.h x86_64-unknown-cygwin/include/ffitarget.h

make

cd ..
