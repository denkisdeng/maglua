#!/bin/bash

if [ ! -f makefile.common.config ]; then
	echo "makefile.common.config is missing, please use ./configure to create it."
	exit 1
fi


# toggles for building each part (set to anything but 1 to skip)
BUILD_LUA=1
BUILD_FFTW=1
BUILD_LAPACK=1
BUILD_ZLIB=1
BUILD_MAGLUA=1


PWD=`pwd`
mkdir -p deps/lib/pkgconfig
cd deps
DEPDIR=`pwd`
cd ..

if [[ $BUILD_LUA = 1 ]]; then
echo "#####################################################"
echo "###  Building Lua deps and installing in \$DEPDIR  ##"
echo "#####################################################"
echo ""
if [ ! -f lua-5.1.5.tar.gz ]; then
	wget http://www.lua.org/ftp/lua-5.1.5.tar.gz
fi
rm -rf lua-5.1.5
tar -xzf lua-5.1.5.tar.gz
cd lua-5.1.5/src
make all MYCFLAGS="-fPIC -DLUA_USE_LINUX" MYLIBS="-Wl,-E -ldl -lreadline -lhistory -lncurses"
rm -f lua.o luac.o print.o
g++ -shared -o liblua.so *.o
cd ..
make local
mkdir -p ~/bin
rm -f ~/bin/lua
rm -f ~/bin/luac
cp bin/* ~/bin
cp src/liblua.so $DEPDIR/lib
cp -r include $DEPDIR
cd ..
if [ -f lua-5.1.5/etc/lua.pc ] ; then
	#build lua.pc & symlinks
	#echo "Building Lua pkgconfig file"
	head -n 10 lua-5.1.5/etc/lua.pc >  $DEPDIR/lib/pkgconfig/lua.pc
	echo "prefix= $DEPDIR"          >> $DEPDIR/lib/pkgconfig/lua.pc
	tail -n 20 lua-5.1.5/etc/lua.pc >> $DEPDIR/lib/pkgconfig/lua.pc

	rm -f $DEPDIR/lib/pkgconfig/lua5.1.pc
	ln -s $DEPDIR/lib/pkgconfig/lua.pc $DEPDIR/lib/pkgconfig/lua5.1.pc
fi
fi






if [[ $BUILD_ZLIB = 1 ]]; then
echo "#####################################################"
echo "###################  Building zlib ##################"
echo "#####################################################"
if [ ! -f zlib-1.2.8.tar.gz ]; then
  wget http://zlib.net/zlib-1.2.8.tar.gz
fi
rm -rf zlib-1.2.8
tar -xzf zlib-1.2.8.tar.gz
cd zlib-1.2.8
./configure  --prefix=$DEPDIR
make -j10
make install
cd ..
rm -rf zlib-1.2.8
fi



if [[ $BUILD_FFTW = 1 ]]; then
echo "#####################################################"
echo "###############  Building 64-bit fftw ###############"
echo "#####################################################"
if [ ! -f fftw-3.3.2.tar.gz ]; then
  wget http://www.fftw.org/fftw-3.3.2.tar.gz
fi
rm -rf fftw-3.3.2
tar -xzf fftw-3.3.2.tar.gz
cd fftw-3.3.2
./configure  --prefix=$DEPDIR --with-pic --enable-shared
make -j10
make install
cd ..
rm -rf fftw-3.3.2

echo "#####################################################"
echo "###############  Building 32-bit fftw ###############"
echo "#####################################################"
tar -xzf fftw-3.3.2.tar.gz
cd fftw-3.3.2
./configure  --prefix=$DEPDIR --with-pic --enable-shared --enable-single
make -j10
make install
cd ..
rm -rf fftw-3.3.2
fi



if [[ $BUILD_LAPACK = 1 ]]; then
echo "#####################################################"
echo "###  Building Lapack and installing in \$DEPDIR ##"
echo "#####################################################"
echo ""
if [ ! -f lapack-3.4.2.tgz ]; then
  wget http://www.netlib.org/lapack/lapack-3.4.2.tgz
fi
rm -rf lapack-3.4.2
tar -xzf lapack-3.4.2.tgz
cd lapack-3.4.2
mkdir build
cd build
cmake .. -DLAPACKE=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=$DEPDIR
make -j4
make install
cd ../..
fi



if [[ $BUILD_MAGLUA = 1 ]]; then
echo "#####################################################"
echo "#################  Building MagLua ##################"
echo "#####################################################"

export PATH=$HOME/bin:$PATH
export PKG_CONFIG_PATH=$DEPDIR/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$DEPDIR/lib:$LD_LIBRARY_PATH

make 

# installing wrapper rather than binary into ~/bin.

cat << EOF > maglua_wrapper
#!/bin/bash

if [ -n "\$LD_LIBRARY_PATH" ]
then
   LD_LIBRARY_PATH=$PWD/deps/lib:\$LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH=$PWD/deps/lib
fi
export LD_LIBRARY_PATH

exec $PWD/maglua "\$@"
EOF

rm -f ~/bin/maglua
chmod 755 maglua_wrapper
cp maglua_wrapper ~/bin/maglua

rm -rf libs
mkdir -p libs
cd libs
maglua --setup `pwd`
cd ..

make -C modules/common install
make -C modules/cpu install
make -C modules/extras install

maglua --write_docs maglua.html
echo "#####################################################"
echo "######### Help Documentation Generated   ############"
echo "#####################################################"
echo -e "\nHelp file named \"maglua.html\" in:\n     `pwd`\n"


cat <<EOF

#####################################################
###############   Install  Complete   ###############
#####################################################

This installation of MagLua depends on libraries in non-standard
locations. Previously this meant adding directories to your
LD_LIBRARY_PATH but now there is a wrapper script that will do
this for you whenever you run maglua. A file has been created
in the current directory called maglua_wrapper, it has been 
copied to ~/bin and renamed maglua which, when called, will 
alter the local environemnt and execute the compiled binary in t
his directory.

If you want to work on the source and compile maglua, you can
add the following to your ~/.bashrc and either source it or log 
out and back in:

export PATH=\$HOME/bin:\$PATH
export PKG_CONFIG_PATH=$DEPDIR/lib/pkgconfig:\$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$DEPDIR/lib:\$LD_LIBRARY_PATH

If you are only interested in running MagLua, you are done.
*** There is no need to edit ~/.bashrc ***

EOF
fi


