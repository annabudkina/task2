/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "shared.h"
#include <l4/re/dataspace>      // L4Re::Dataspace
#include <l4/re/rm>             // L4::Rm
static int crypt(L4::Cap<void> const &server, char*text, unsigned long size,int opcode,char* ret)
   
{
	char* addr;
	int err;
	L4::Ipc::Iostream s(l4_utcb());
	L4::Cap<L4Re::Dataspace> ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	if (!ds.is_valid())
    	{
		printf("Could not get capability slot!\n");
		return 1;
    	}


	s << l4_umword_t(Opcode::dataspace);
	s <<L4::Ipc::Small_buf(ds);
	err = l4_error(s.call(server.cap(), Protocol::Encr));
	if (err)
		return 1; // failure
	printf("got dataspace\n");
	err = L4Re::Env::env()->rm()->attach(&addr, ds->size(),
                                           L4Re::Rm::Search_addr, ds);
  	if (err < 0)
	{
		printf("Error attaching data space: %s\n", l4sys_errtostr(err));
		L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
		return 1;
	}
	memcpy(addr,text,size);
	printf(" %s \n",addr);



	L4::Ipc::Iostream s1(l4_utcb());



	s1<< l4_umword_t(opcode);
	err = l4_error(s1.call(server.cap(), Protocol::Encr));
	if (err)
		return 1; // failure
	ret=(char*)malloc(size);
	memcpy(ret,addr,size);
	err = L4Re::Env::env()->rm()->detach(addr, 0);
	if (err)
	    printf("Failed to detach region\n");
	L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
	return 0;
}

int main()
   {
	char text[]="Hello World";
	unsigned long size = 12;
	char* ret;
	int err;
	L4::Cap<void> server = L4Re::Env::env()->get_cap<void>("encr_server");
	if (!server.is_valid())
	{
		printf("Could not get server capability!\n");
		return 1;
    	}
	if (crypt(server,text,size,Opcode::encrypt,ret))	
	{
      		printf("Error talking to server\n");
      		return 1;
    	}
  	printf("Encrypted text: %s \n", ret);

	
  	if (crypt(server,ret,size,Opcode::decrypt,ret))
  	{
         	printf("Error talking to server\n");
     		return 1;
        }
	printf("Decrypted text: %s \n",  ret);

	
  return 0;

}
