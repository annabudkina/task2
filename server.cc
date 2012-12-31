/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>
#include <l4/cxx/ipc_stream>
#include "shared.h"
#include <l4/re/dataspace>      // L4Re::Dataspace
#include <l4/re/rm>             // L4::Rm


static L4Re::Util::Registry_server<> server;

class Encryption_server : public L4::Server_object
{
public:
	
  Encryption_server();   
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
    ~Encryption_server(); 
private:
  L4::Cap<L4Re::Dataspace> ds;
  char* addr;
};
Encryption_server::Encryption_server()
{
	ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	if (!ds.is_valid())
    	{
		printf("Could not get capability slot!\n");
    	}
	printf("got dataspace capability slot!\n");
	int err = L4Re::Env::env()->mem_alloc()->alloc(1024, ds, 0);
	if (err < 0)
	{
		printf("mem_alloc->alloc() failed.\n");
		L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
	}
	err = L4Re::Env::env()->rm()->attach(&addr, ds->size(),
                                           L4Re::Rm::Search_addr, ds);
  	if (err < 0)
	{
		printf("Error attaching data space: %s\n", l4sys_errtostr(err));
		L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
	}
}
int Encryption_server::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
	unsigned long size;
	int err;
	L4::Opcode opcode;
	printf(" %s \n",addr);
	l4_msgtag_t t;
	ios >> t;
	if (t.label() != Protocol::Encr)
		return -L4_EBADPROTO;
	ios >> opcode;
	switch (opcode)
	{
	case Opcode::encrypt:	
		for(unsigned i=0;i!=size-1;i++)
		{
			addr[i]=addr[i]+1;
		}
		printf(" %s \n",addr);
		return L4_EOK;
	case Opcode::decrypt:
		for(unsigned i=0;i!=size-1;i++)
		{
			addr[i]=addr[i]-1;
		}
		return L4_EOK;
		printf(" %s \n",addr);
	case Opcode::dataspace:
		ios<<ds;
		return L4_EOK;
	  
    default:
      return -L4_ENOSYS;
    }
}
Encryption_server::~ Encryption_server()
{
	int err = L4Re::Env::env()->rm()->detach(addr, 0);
	if (err)
	    printf("Failed to detach region\n");
	L4Re::Util::cap_alloc.free(ds, L4Re::This_task);
}


int main()
{
	static Encryption_server encr;
	if (!server.registry()->register_obj(&encr, "encr_server").is_valid())
        {
		printf("Could not register my service, is there a 'encr_server' in the caps table?\n");
		return 1;
        }

	server.loop();
  return 0;
}

