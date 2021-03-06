/******************************************************************************
* Copyright (C) 2008-2011 Jason Mercer.  All rights reserved.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#ifdef _MPI
 #include <mpi.h>
#endif
#include "libMagLua.h"
#include "shutdown.h"

int main(int argc, char** argv)
{
	int force_quiet = 0;
	int sub_process = 0;
#ifdef _MPI
	MPI_Init(&argc, &argv);
#endif
	
#ifdef _MPI
	int rank = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	if(rank != 0)
		force_quiet = 1; //only rank 0 will chatter
#endif

	lua_State *L = lua_open();
	int ret = libMagLuaArgs(argc, argv, L, sub_process, force_quiet);

        // not using the convenience macro in shutdown.h because we
        // want to hit MPI_Finalize if things go bad
	if(luaL_dostringn(L, __shutdown(), __shutdown_chunkname()))
	{
	    fprintf(stderr, "%s\n", lua_tostring(L, -1));
            ret = ret | 1;
	}

	lua_close(L);
	
#ifdef _MPI
        // if a process threw an error, shut things down with an abort
        // so other processes don't wait around for a dead peer
        if(ret)
            MPI_Abort(MPI_COMM_WORLD, ret);
        else
            MPI_Finalize();
#endif

	return ret;
}
